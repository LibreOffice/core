m4_dnl -*- Mode: HTML -*-x
m4_changequote([,])m4_dnl
<!doctype html>
<html>
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>tabs</title>
    <script>
    (function() {
        var params = new URLSearchParams(window.location.search);
        if (params.get('darkTheme') === 'true') {
            document.documentElement.setAttribute('data-theme', 'dark');
        }
    })();
    </script>
    <link rel="stylesheet" href="color-palette.css" />
    <link rel="stylesheet" href="color-palette-dark.css" />
    <!-- Shared design tokens (--cool-font etc.); before qtapp-tabstrip.css so the strip's rules win. -->
    <link rel="stylesheet" href="cool.css" />
    <link rel="stylesheet" href="qtapp-tabstrip.css" />
    <script type="text/javascript" src="qrc:///qtwebchannel/qwebchannel.js"></script>
    <script src="qtapp-tabstrip.js" defer></script>
  </head>
  <body>
    <div id="strip" role="tablist"></div>
  </body>
</html>
