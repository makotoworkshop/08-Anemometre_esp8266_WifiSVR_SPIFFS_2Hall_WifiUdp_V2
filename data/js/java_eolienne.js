      google.charts.load('current', {'packages':['gauge']});
      google.charts.setOnLoadCallback(drawChart);

      function drawChart() {
        // Create and populate the data table.
        var dataGaugeEol = google.visualization.arrayToDataTable([
          ['Label', 'Value'],
          ['Eolienne', 0],
        ]);
        // formattage avec un suffixe et # pour arrondir
        var formatter = new google.visualization.NumberFormat({suffix: ' rpm',pattern:'#'});

        // définition des options documentée ici https://developers.google.com/chart/interactive/docs/gallery/gauge
        var options = {
          width: 800, height: 720,
          greenFrom:100, greenTo:700,   // le vert serait la zone ou l'on produit du jus
          yellowFrom:700, yellowTo:950,
          redFrom: 950, redTo: 1200,
          majorTicks: ['0', 200, 0, 600, 0, 1000, 1200],    // graduations
          minorTicks: 5,    // graduations
          max: 1200, min: 0,
          animation:{
             duration: 100,
            easing: 'linear',
          }
        };

        // init du graphisme
        var GaugeEol = new google.visualization.Gauge(document.getElementById('chart_div_eolienne'));

        // tracé du graphisme
        GaugeEol.draw(dataGaugeEol, options);
        updateGauge();  // exécute la fontion
        setInterval(updateGauge, 100); // appelle les données de la fonction et définit la vitesse de rafraichissement

        function updateGauge() {     // fonction de mise raffraichissement des données réccupérée dans le fichier json https://www.w3schools.com/jquery/ajax_getjson.asp

          $.getJSON('mesures.json', function(data) 
            {
              $("#rpmEolienne").text(data.rpmEolienne); // extraire la valeur du json

                  dataGaugeEol.setValue(0, 1, data.rpmEolienne); // premier nb pour le N° de la gauge si plusieurs, puis 0 pour modifier le champ label (sinon:1 pour le champ Value), puis la valeur extraite du json
                  GaugeEol.draw(dataGaugeEol, options); // tracé du graphisme
                  formatter.format(dataGaugeEol, 1); // applique le formattage à la seconde collonne du tableau (1) qui correspond à la valeur

            });
        }
      }
