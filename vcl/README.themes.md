# LibreOffice Themes

## How to read this

It is suggested that you have the code open side-by-side and first read some part here and then the code that it talks about.

## VLC Plugins and the UpdateSettings functions

LibreOffice VCL (a bunch of  interfaces and a base implementation) has four main platform  specific implementations (gtk, qt, win,
osx). Each VCL plugin has an `UpdateSettings(AllSettings&  rSettings)` function somewhere. This function reads styling information
like colors from  the platfrom specific widget toolkit and  loads the information into the `StyleSettings`  instance passed in the
argument (indirectly through `AllSettings`).

## The StyleSettings Instance

The `StyleSettings` (SS) class manages the colors. Various parts of the codebase call getters and setters on SS to get the default
color, or to override it.  There exists a static `StyleSettings` instance in the application, and  the instances which are created
here and there are merged with that static SS instance. we  can access the static instance from anywhere in the application by the
following function call.

```cpp
const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
```

## How UserPreferences are Saved (registry)

There exist two kind of files for state/configuration management, .xcu and .xcs files. These are XML files, the .xcs files are XML
schema files which define the "schema" for the configuration like  a colorscheme node will have the following entries colors , and
the colors will have a light and a dark variant... The .xcu files are the configuration data files which define the default values
for the configuration nodes defined in the schema files.

We use the term `registry` to refer to the application's configuration and we save the modifications to the default values (set in
the .xcu files) in a file named `registrymodifications.xcu` which lives in `$XDG_CONFIG_HOME/libreoffice/(somewhere here)`.

## ColorConfig, ColorConfig_Impl, and EditableColorConfig

From the  themes/colors perspective,  think of `ColorConfig_Impl`  as a  code representation  of the colors  in the  registry, and
think  of  `ColorConfig` as  a  *read-only  wrapper over  `ColorConfig_Impl`.  There  exists another  class  in  this bunch  named
`EditableColorConfig`, and as the name suggests it is a read-write wrapper over `ColorConfig_Impl`.

The "Appearance" tab on the "Options" dialog interacts with the registry thanks to an `EditableColorConfig` instance.

## Getting System Colors into the static StyleSettings object

So if you setup some  printfdebugging statements in the `UpdateSettings` functions and in  the `ColorConfig` constructor, you will
find that when the application  starts, first the `UpdateSettings` functions are executed, and  then the first every `ColorConfig`
instance is created.

Also if you  add and set a  non-static flag to the  `StyleSettings` and print it  out from the `UpdateSettings`  functions and the
`ColorConfig` constructor, you  will find that the  flag doesn't make it to  the static instance (accessed  from in `ColorConfig`)
immediately. We use such a flag `mbSystemColorsLoaded` to see if the static `StyleSettings` object has the system colors or not.

## The LibreOfficeTheme registry flag

```xml
<prop oor:name="LibreOfficeTheme" oor:type="xs:short" oor:nillable="false">
  <info>
    <desc>Specifies LibreOfficeTheme state.</desc>
    <label>LibreOffice Theme</label>
    ...
```

To enable or disable theming,  we have a `LibreOfficeTheme` enum in the registry which is  represented by `enum class ThemeState`.
in the code. The  default value is `ENABLED` and the  only way for the user to disable  it is by changing it to  `0` in the expert
configuration.

> It's still a  dispute whether to enable  or disable a theming by  default, so please refer to  the .xcs file and  don't take the
> explaination for implementation.

```cpp
enum class ThemeState
{
    DISABLED = 0,
    ENABLED = 1,
    RESET = 2,
};
```

## High Level Code overview of Themes Implementation

We load the colors from  the widget toolkit into the `StyleSettings` object and set a  flag `mbSystemColorsLoaded` to `true`. Then
in the `ColorConfig` constructor `ColorConfig::SetupTheme()`. We will be back to `SetupTheme` after we understand how theme colors
are stored.

## Talking about Singleton ThemeColors class and the path Colors travel

`themecolors.hxx` defines a singleton class named `ThemeColors`. This class has  two static members. The second one is that of the
class itself, and the first one is a boolean flag which is used to check if theme colors are cached or not.

```cpp
class VCL_DLLPUBLIC ThemeColors
{
    ThemeColors() {}
    static bool m_bIsThemeCached;
    static ThemeColors m_aThemeColors;
    ...
```

All  the   colors  are  essentially  registry   values  grouped  in   colorschemes  and  accessed  using   various  `ColorConfig`s
(ColorConfig_Impl, EditableColorConfig, ColorConfig), we just  talked about it above. So the theme colors  (colors for the UI) are
loaded from the  registry into this singleton `ThemeColors` instance,  and we set the `m_bIsThemeCached` flag  to `true`. Then the
various VCL plugins check  the flag and if the theme colors  are cached, these colors are sent to the  widget toolkit in different
ways depending on the toolkit, like css in case of gtk, QPalette in case of Qt.

Then when the UpdateSettings function  is called again, the colors read from the widget toolkit  are these custom colors. Then the
`StyleSettings` object is loaded with these colors and they make it  to every corner of the application which gets its colors from
`StyleSettings` object.

## Back to ColorConfig::SetupTheme()

So  in `ColorConfig::SetupTheme()`,  we first  check if  `LibreOfficeTheme` enum  is set  to `DISABLED`,  and if  so then  we mark
`ThemeColors` as not cached, so no custom colors are set at the toolkit level and return from the `SetupTheme()` function. Then we
check if `LibreOfficeTheme` is set to `RESET` which happens when  the user presses the `Reset All` button (after which he restarts
the system). If true then we check for `mbSystemColorsLoaded` to see if the default colors from the widget toolkit have made it to
the static StyleSettings instance or not, and if that's true as well, we set `LibreOfficeTheme` enum to `ENABLED`

Then in the  last part of `SetupTheme()`,  which we reach only  if `LibreOfficeTheme` is set  to `ENABLED`, we check  if the theme
colors are cached or not (if the UI colors are loaded from the registry into the static `ThemeColors` instance or not). If cached,
we don't touch those  over and over. If theme colors are  not cached, then we `Load` the `CurrentScheme` which  means that we load
the colors for the current scheme from the registry and store them in `ColorConfig_Impl` instance.

```cpp
    ...
    if (!ThemeColors::IsThemeCached())
    {
        // registry to ColorConfig::m_pImpl
        m_pImpl->Load(GetCurrentSchemeName());
        m_pImpl->CommitCurrentSchemeName();

        // ColorConfig::m_pImpl to static ThemeColors::m_aThemeColors
        LoadThemeColorsFromRegistry();
    }
    ...
```

Then the `LoadThemeColorsFromRegistry` function is called which loads  colors from the registry into the `ThemeColors` instance by
calling `ColorConfig::GetColorValue` for each  entry. In `ColorConfig::GetColorValue` call, if the color value  in the registry is
`COL_AUTO` then we call `ColorConfig::GetDefaultColor` which returns  hardcoded default colors for the document, and StyleSettings
colors for the UI (see `lcl_GetDefaultUIColor`).

If the color value  is not `COL_AUTO`, then the value from  the registry is returned, this way we save  the user's preferences and
get the default colors from hardcoded colors array and StyleSettings.

```cpp
void ColorConfig::LoadThemeColorsFromRegistry()
{
    ThemeColors& rThemeColors = ThemeColors::GetThemeColors();

    rThemeColors.SetWindowColor(GetColorValue(svtools::WINDOWCOLOR).nColor);
    rThemeColors.SetWindowTextColor(GetColorValue(svtools::WINDOWTEXTCOLOR).nColor);
    ...
```

## What happens when "Reset All" is pressed

When the  `Reset All` button  is pressed, all the  registry color values  are set to  `COL_AUTO` and `LibreOfficeTheme` is  set to
`RESET`. Then after restart, the `IsThemeReset` conditional  in `ColorConfig::SetupTheme()` checks if StyleSettings has the system
colors  or not,  and once  it has,  `LibreOfficeTheme`  is set  to `ENABLED`  which  then goes  through the  last conditional  and
`LoadThemeColorsFromRegistry` is called (just explained  above). Since all the registry entries were set  to `COL_AUTO`, we end up
getting default values for all the colors (hardcoded ones for document and StyleSettings colors for UI).

## ColorConfigValue now has nLightColor and nDarkColor entries

```cpp
struct ColorConfigValue
{
    bool        bIsVisible; // validity depends on the element type
    ::Color     nColor; // used as a cache for the current color
    Color       nLightColor;
    Color       nDarkColor;
    ...
```

Each color entry has two color values, one for light and  one for dark. Based on the `ApplicationAppearance`, either light or dark
color value is used. Since the  nColor "variable name" is used in 250+ places in the codebase,  I found it unreasonible to replace
all the 250+ references with a conditional like this.

```cpp
Color nColor;
if (IsDarkMode())
    nColor = aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nDarkColor;
else
    nColor = aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nLightColor;
```

This would have been very inefficient because `IsDarkMode()` is a virtual function (being called 250+ times, maybe every frame??).
So  instaed  of  using a  conditional,  I  use  `nColor`  as  the cache.  When  the  colors  are  loaded from  the  registry  (see
`ColorConfig_Impl::Load`), I cache  the value into `nColor` based  on `ApplicationAppearance` value (whether light  or dark mode).
And since we ask the user to restart the application after changing appearance or color values, caching works without any issues.

## Automatic scheme as the fallback

In case  the scheme  that you  are trying  to load  doesn't exist because  "the extension  was removed?",  or "someone  edited the
registry".. the "Automatic" scheme is used as the fallback.

```cpp
void ColorConfig_Impl::Load(const OUString& rScheme)
    ...
    if (!ThemeColors::IsAutomaticTheme(sScheme))
    {
        uno::Sequence<OUString> aSchemes = GetSchemeNames();
        bool bFound = std::any_of(aSchemes.begin(), aSchemes.end(),
            [&sScheme](const OUString& rSchemeName) { return sScheme == rSchemeName; });

        if (!bFound)
            sScheme = AUTOMATIC_COLOR_SCHEME;
    }
    ...
```
