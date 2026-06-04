m4_dnl -*- Mode: HTML -*-x
m4_changequote([,])m4_dnl
m4_dnl# m4_foreachq(x, `item_1, item_2, ..., item_n', stmt)
m4_dnl# quoted list, alternate improved version
m4_define([m4_foreachq],[m4_ifelse([$2],[],[],[m4_pushdef([$1])_$0([$1],[$3],[],$2)m4_popdef([$1])])])m4_dnl
m4_define([_m4_foreachq],[m4_ifelse([$#],[3],[],[m4_define([$1],[$4])$2[]$0([$1],[$2],m4_shift(m4_shift(m4_shift($@))))])])m4_dnl
m4_define(_YEAR_,m4_esyscmd(date +%Y|tr -d '\n'))m4_dnl
m4_dnl------------------------------------------------------------------------
m4_dnl# Define MOBILEAPP as true if this is either for the iOS/Android app or for the Collabora Office apps
m4_define([MOBILEAPP],[])m4_dnl
m4_ifelse(IOSAPP,[true],[m4_define([MOBILEAPP],[true])])m4_dnl
m4_ifelse(MACOSAPP,[true],[m4_define([MOBILEAPP],[true])])m4_dnl
m4_ifelse(WINDOWSAPP,[true],[m4_define([MOBILEAPP],[true])])m4_dnl
m4_ifelse(ANDROIDAPP,[true],[m4_define([MOBILEAPP],[true])])m4_dnl
m4_ifelse(QTAPP,[true],[m4_define([MOBILEAPP],[true])])m4_dnl
m4_dnl
m4_dnl# FIXME: This is temporary and not what we actually eventually want.
m4_dnl# What we really want is not a separate HTML file (produced with M4 conditionals on the below
m4_dnl# EMSCRIPTENAPP) for a "WASM app". What we want is that the same cool.html page adapts on demand to
m4_dnl# instead run locally using WASM, if the connection to the COOL server breaks. (And then
m4_dnl# re-connects to the COOL server when possible.)
m4_dnl
m4_ifelse(EMSCRIPTENAPP,[true],[m4_define([MOBILEAPP],[true])])m4_dnl
m4_dnl------------------------------------------------------------------------
<!doctype html>
<html data-theme="%UI_THEME%">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    m4_dnl The desktop/mobile apps load this page in a file:// iframe where the
    m4_dnl <link rel="localizations"> loader never worked, so the surrounding
    m4_dnl dialog was localized but its contents were not. Mirror the main app:
    m4_dnl a small window.LANG -> window.LOCALIZATIONS table (Options strings only,
    m4_dnl see util/create-settings-l10n-js.py), consumed by the override in
    m4_dnl admin/main-admin.js. Online keeps the server-served localizations link.
    m4_ifelse(MOBILEAPP,[true],
      [<script>(function () { var l = new URLSearchParams(window.location.search).get('lang'); window.LANG = (l && l !== 'undefined') ? l : 'en-US'; })();</script>
      <script src="l10n-settings.js"></script>],
      [<link rel="localizations" href="%SERVICE_ROOT%/browser/%VERSION%/l10n/localizations.json" type="application/vnd.oftn.l10n+json"/>])
    <title>Collabora Online - Settings</title>
    <link
      rel="StyleSheet"
      href="m4_ifelse(MOBILEAPP, [], [%SERVICE_ROOT%/browser/%VERSION%/])admin/css/admin.css"
      type="text/css"
    />
    <link rel="stylesheet" href="m4_ifelse(MOBILEAPP, [], [%SERVICE_ROOT%/browser/%VERSION%/])color-palette.css" />
    <link rel="stylesheet" href="m4_ifelse(MOBILEAPP, [], [%SERVICE_ROOT%/browser/%VERSION%/])color-palette-dark.css" />
    <link
      rel="stylesheet"
      href="m4_ifelse(MOBILEAPP, [], [%SERVICE_ROOT%/browser/%VERSION%/])admin/css/adminIntegratorSettings.css"
    />
    <link
      id="settings-css"
      rel="stylesheet"
      href="m4_ifelse(MOBILEAPP, [], [%SERVICE_ROOT%/browser/%VERSION%/])settings.css"
    />
    <script
      src="m4_ifelse(MOBILEAPP, [], [%SERVICE_ROOT%/browser/%VERSION%/])admin-bundle.js"
      defer
    ></script>
    <!--%BRANDING_CSS%-->
    <!--%BRANDING_JS%-->
  </head>
  <body>
    <div id="settingIframe">
      <div id="allConfigSection"></div>
      <input
        type="hidden"
        id="initial-variables"
        data-access-token="%ACCESS_TOKEN%"
        data-access-token-ttl="%ACCESS_TOKEN_TTL%"
        data-no-auth-header="%NO_AUTH_HEADER%"
        data-access-header="%ACCESS_HEADER%"
        data-enable-accessibility="%ENABLE_ACCESSIBILITY%"
        data-enable-debug="%ENABLE_DEBUG%"
        data-wopi-setting-base-url="%WOPI_SETTING_BASE_URL%"
        data-iframe-type="%IFRAME_TYPE%"
        data-service-root="%SERVICE_ROOT%"
        data-css-vars="<!--%CSS_VARIABLES%-->"
        data-version-hash="%VERSION%"
        data-lang="%UI_LANG%"
        data-disable-ai-settings="%DISABLE_AI_SETTINGS%"
        data-show-left-nav="%SHOW_LEFT_NAV%"
      />
    </div>
  </body>
</html>
