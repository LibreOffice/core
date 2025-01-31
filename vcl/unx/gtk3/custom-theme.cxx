/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "custom-theme.hxx"
#include <vcl/themecolors.hxx>
#include <gio/gio.h>
#include <unx/gtk/gtkdata.hxx>
#include <gtk/gtk.h>

namespace CustomTheme
{
void LoadColorsFromTheme(StyleSettings& rStyleSet)
{
    const ThemeColors& aThemeColors = ThemeColors::GetThemeColors();

    rStyleSet.SetDialogTextColor(aThemeColors.GetWindowColor());
    rStyleSet.SetDefaultActionButtonTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetActionButtonTextColor(aThemeColors.GetButtonTextColor());
    rStyleSet.SetListBoxWindowTextColor(aThemeColors.GetBaseColor());
    rStyleSet.SetRadioCheckTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetGroupTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetLabelTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetWindowTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetFieldTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetShadowColor(aThemeColors.GetShadeColor());

    rStyleSet.BatchSetBackgrounds(aThemeColors.GetBaseColor());
    rStyleSet.SetDefaultButtonTextColor(aThemeColors.GetButtonTextColor());
    rStyleSet.SetDefaultButtonRolloverTextColor(aThemeColors.GetButtonTextColor());
    rStyleSet.SetDefaultButtonPressedRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetButtonRolloverTextColor(aThemeColors.GetButtonTextColor());
    rStyleSet.SetDefaultActionButtonRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetDefaultActionButtonPressedRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetActionButtonRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetActionButtonPressedRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetFlatButtonTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetFlatButtonPressedRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetFlatButtonRolloverTextColor(aThemeColors.GetFaceColor());
    rStyleSet.SetFieldRolloverTextColor(aThemeColors.GetFaceColor());

    // Some colors like ButtonRolloverTextColor are present in the StyleSettings but
    // not in ThemeColors, hence cannot be customized yet. These will be added to
    // ThemeColors as the themeing system evolves. Leaving as commented for now.

    // rStyleSet.SetButtonRolloverTextColor(Color(0));
    // rStyleSet.SetButtonPressedRolloverTextColor(Color(0));
    // rStyleSet.SetHelpColor(Color(0));
    // rStyleSet.SetHelpTextColor(Color(0));

    rStyleSet.SetAccentColor(aThemeColors.GetAccentColor());
    rStyleSet.SetHighlightColor(aThemeColors.GetAccentColor());
    rStyleSet.SetHighlightTextColor(aThemeColors.GetActiveTextColor());
    rStyleSet.SetListBoxWindowHighlightColor(aThemeColors.GetActiveColor());
    rStyleSet.SetListBoxWindowHighlightTextColor(aThemeColors.GetActiveTextColor());
    rStyleSet.SetActiveColor(aThemeColors.GetActiveColor());
    rStyleSet.SetActiveTextColor(aThemeColors.GetActiveTextColor());

    rStyleSet.SetFieldColor(aThemeColors.GetFieldColor());

    rStyleSet.SetWindowColor(aThemeColors.GetWindowColor());
    rStyleSet.SetListBoxWindowBackgroundColor(aThemeColors.GetWindowColor());

    rStyleSet.SetActiveTabColor(aThemeColors.GetActiveColor());
    rStyleSet.SetInactiveTabColor(aThemeColors.GetInactiveColor());

    rStyleSet.SetMenuColor(aThemeColors.GetMenuColor());
    rStyleSet.SetMenuBarColor(aThemeColors.GetMenuBarColor());
    rStyleSet.SetMenuBarRolloverColor(aThemeColors.GetMenuBarHighlightColor());
    rStyleSet.SetMenuBarTextColor(aThemeColors.GetMenuBarTextColor());
    rStyleSet.SetMenuBarRolloverTextColor(aThemeColors.GetMenuBarHighlightTextColor());
    rStyleSet.SetMenuBarHighlightTextColor(aThemeColors.GetMenuHighlightTextColor());
    rStyleSet.SetMenuTextColor(aThemeColors.GetMenuTextColor());
    rStyleSet.SetMenuHighlightColor(aThemeColors.GetMenuHighlightColor());
    rStyleSet.SetMenuHighlightTextColor(aThemeColors.GetMenuHighlightTextColor());

    rStyleSet.SetLinkColor(aThemeColors.GetActiveColor());

    Color aVisitedLinkColor = aThemeColors.GetActiveColor();
    aVisitedLinkColor.Merge(aThemeColors.GetWindowColor(), 100);
    rStyleSet.SetVisitedLinkColor(aVisitedLinkColor);
    rStyleSet.SetTabTextColor(aThemeColors.GetMenuBarTextColor());
    rStyleSet.SetToolTextColor(aThemeColors.GetWindowTextColor());
    rStyleSet.SetTabRolloverTextColor(aThemeColors.GetMenuBarHighlightTextColor());
}

#if GTK_CHECK_VERSION(4, 0, 0)
void ApplyCustomTheme(GdkDisplay* pGdkDisplay, GtkCssProvider** pCustomThemeProvider)
#else
void ApplyCustomTheme(GdkScreen* pScreen, GtkCssProvider** pCustomThemeProvider)
#endif
{
    if (!ThemeColors::VclPluginCanUseThemeColors())
        return;

    if ((*pCustomThemeProvider) == nullptr)
    {
        (*pCustomThemeProvider) = gtk_css_provider_new();
        OString aStyleString = CreateStyleString();

        css_provider_load_from_data(*pCustomThemeProvider, aStyleString.getStr(),
                                    aStyleString.getLength());

#if GTK_CHECK_VERSION(4, 0, 0)
        gtk_style_context_add_provider_for_display(pGdkDisplay,
                                                   GTK_STYLE_PROVIDER(*pCustomThemeProvider),
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#else
        gtk_style_context_add_provider_for_screen(pScreen,
                                                  GTK_STYLE_PROVIDER(*pCustomThemeProvider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#endif
    }
}

OString CreateStyleString()
{
    const ThemeColors& aThemeColors = ThemeColors::GetThemeColors();

    Color aSpinBack = aThemeColors.GetBaseColor();
    aSpinBack.SetAlpha(20);

    Color aPressedColor = ThemeColors::GetThemeColors().GetWindowColor();
    if (aPressedColor.IsDark())
        aPressedColor.IncreaseLuminance(30);
    else
        aPressedColor.DecreaseLuminance(30);

    // clang-format off
    OUString aStr =
        /***************
             * Base States *
             ***************/
        ".background {"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        ".view {"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "textview text {"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        /*********
           * Links *
           *********/
        "*:link,"
        "link {"
        "  color: #" + aThemeColors.GetActiveColor().AsRGBHexString() + ";"
        "}"

        "*:visited {"
        "  color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        /************
           * Toolbars *
           ************/
        "toolbar {"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "}"

        ".toolbar {"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "}"

        /**************
           * Tree Views *
           **************/
        "treeview.view:selected {"
        "  background-color: #" + aThemeColors.GetAccentColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "treeview.view:drop(active) {"
        "  border-style: solid none;"
        "  border-color: alpha(currentColor, 0.08);"
        "}"

        "treeview.view.expander {"
        "  color: #" + aThemeColors.GetFaceColor().AsRGBHexString() + ";"
        "}"

        "treeview.view.expander:hover,"
        "treeview.view.expander:active {"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "treeview.view.expander:disabled {"
        "  color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "}"

        // this part is still left
        "treeview.view.trough {"
        "  background-color: rgba(239, 241, 245, 0.12);"
        "}"

        "treeview.view header button:not(:focus):not(:hover):not(:active) {"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "treeview.view header button,"
        "treeview.view header button:disabled {"
        "  background-color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "}"

        "treeview.view header button:last-child {"
        "  border-right-style: none;"
        "}"

        "treeview.view button.dnd,"
        "treeview.view header.button.dnd {"
        "  background-color: #ff0000;"
        "  color: #89b4fa;"
        "}"

        /*********
           * Menus *
           *********/
        "menubar,"
        ".menubar {"
        "  background-color: #" + aThemeColors.GetMenuBarColor().AsRGBHexString() + ";"
        "}"

        "menubar > menuitem,"
        ".menubar > menuitem {"
        "  color: #" + aThemeColors.GetMenuBarTextColor().AsRGBHexString() + ";"
        "}"

        "menubar > menuitem:hover,"
        ".menubar > menuitem:hover {"
        "  background-color: #" + aThemeColors.GetMenuBarHighlightColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetMenuBarHighlightTextColor().AsRGBHexString() + ";"
        "}"

        "menubar > menuitem:disabled,"
        ".menubar > menuitem:disabled {"
        "  color: #" + aThemeColors.GetInactiveTextColor().AsRGBHexString() + ";"
        "}"

        "menubar > menuitem label:disabled,"
        ".menubar > menuitem label:disabled {"
        "  color: inherit;"
        "}"

        "menu {"
        "  background-color: #" + aThemeColors.GetMenuColor().AsRGBHexString() + ";"
        "  border: 1px solid #" + aThemeColors.GetMenuBorderColor().AsRGBHexString() + ";"
        "}"

        "menu menuitem {"
        "  color: #" + aThemeColors.GetMenuTextColor().AsRGBHexString() + ";"
        "}"

        "menu menuitem:hover {"
        "  background-color: #" + aThemeColors.GetMenuHighlightColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetMenuHighlightTextColor().AsRGBHexString() + ";"
        "}"

        "menu menuitem:active {"
        "  background-color: alpha(currentColor, 0.12);"
        "}"

        "menu menuitem:disabled {"
        "  background-color: #" + aThemeColors.GetMenuColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetDisabledTextColor().AsRGBHexString() + ";"
        "  border-left: 1px solid #" + aThemeColors.GetMenuBorderColor().AsRGBHexString() + ";"
        "  border-right: 1px solid #" + aThemeColors.GetMenuBorderColor().AsRGBHexString() + ";"
        "}"

        "menu menuitem accelerator {"
        "  color: rgba(239, 241, 245, 0.7);" // ???
        "}"

        "menu menuitem:disabled accelerator {"
        "  color: rgba(239, 241, 245, 0.32);" // ???
        "}"

        "menu menuitem label:dir(rtl),"
        "menu menuitem label:dir(ltr) {"
        "  color: inherit;"
        "}"

        /*************
           * Notebooks *
           *************/

        "tabbox > tab:hover,"
        "notebook > header tab:hover {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  border: 1px solid #" + aThemeColors.GetSeparatorColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "tabbox > tab:checked, notebook > header tab:checked {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  border: 1px solid #" + aThemeColors.GetSeparatorColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "notebook {"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "}"

        "notebook > header {"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "  border-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        "notebook > stack:not(:only-child) {"
        "  background-color: transparent;"
        "}"

        /************
           * Scrollbar*
          *************/

        "scrollbar {"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        "scrollbar slider {"
        "  border: 4px solid transparent;"
        "  background-clip: padding-box;"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "}"

        "scrollbar slider:hover {"
        "  background-color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "scrollbar slider:active {"
        "  background-color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "scrollbar slider:disabled {"
        "  background-color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "}"

        "scrollbar.overlay-indicator:not(.dragging):not(.hovering) {"
        "  background-color: transparent;"
        "}"

        /*****************
           * Check n Radio *
           * ***************/

        "check,"
        "radio {"
        "  background-image: none;"
        "  border: 1px solid #" + aThemeColors.GetSeparatorColor().AsRGBHexString() + ";"
        "}"

        "check:disabled,"
        "radio:disabled {"
        "  background-color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "}"

        "check:checked,"
        "check:indeterminate,"
        "radio:checked,"
        "radio:indeterminate {"
        "  color: rgba(17, 17, 27, 0.87);"
        "  background-color: #" + aThemeColors.GetActiveColor().AsRGBHexString() + ";"
        "}"

        "check:checked:hover,"
        "check:indeterminate:hover,"
        "radio:checked:hover,"
        "radio:indeterminate:hover {"
        "  box-shadow: 0 0 0 6px rgba(137, 180, 250, 0.15);"
        "  background-color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "check:checked:active,"
        "check:indeterminate:active,"
        "radio:checked:active,"
        "radio:indeterminate:active {"
        "  box-shadow: 0 0 0 6px rgba(137, 180, 250, 0.2);"
        "  background-color: #" + aThemeColors.GetActiveColor().AsRGBHexString() + ";"
        "}"

        "check:checked:disabled,"
        "check:indeterminate:disabled,"
        "radio:checked:disabled,"
        "radio:indeterminate:disabled {"
        "  color: rgba(17, 17, 27, 0.38);"
        "  background-color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "}"

        /***************
           * Progressbar *
           * *************/

        "progressbar {"
        "  color: rgba(239, 241, 245, 0.7);"
        "  font-size: smaller;"
        "}"

        "progressbar trough {"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        "progressbar progress {"
        "  background-color: #" + aThemeColors.GetActiveColor().AsRGBHexString() + ";"
        "}"

        "frame > border,"
        ".frame {"
        "  border-radius: 0;"
        "  box-shadow: none;"
        "}"

        ".frame.view {"
        "  border-radius: 6px;"
        "}"

        ".frame.flat {"
        "  border-style: none;"
        "}"

        "separator {"
        "  min-width: 1px;"
        "  min-height: 1px;"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        /***********
           * Buttons *
           ***********/

        ".toolbar button:checked,"
        "#buttonbox_frame button:checked,"
        "layouttabbar button:checked,"
        "messagedialog .dialog-action-box button:checked,"
        "messagedialog .dialog-action-box .linked:not(.vertical) > button:checked,"
        "popover.background.menu button:checked,"
        "popover.background button.model:checked,"
        "headerbar button:checked:not(.suggested-action):not(.destructive-action),"
        "toolbar button:checked,"
        "combobox > .linked:not(.vertical) > button:checked:not(:only-child),"
        "button.flat:checked,"
        "button.flat:checked:hover {"
        "  background-color: alpha(#" + aThemeColors.GetButtonColor().AsRGBHexString() + ", 0.1);"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"
        ""
        "button:active {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"

        "button:disabled {"
        "  background-color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetInactiveTextColor().AsRGBHexString() + ";"
        "  border: 1px solid #" + aThemeColors.GetInactiveBorderColor().AsRGBHexString() + ";"
        "}"

        /*****************
           * GtkSpinButton *
           *****************/

        "spinbutton > entry,"
        "spinbutton > entry:focus,"
        "spinbutton > entry:disabled,"
        ".background:not(.csd) spinbutton > entry,"
        ".background:not(.csd) spinbutton > entry:focus,"
        ".background:not(.csd) spinbutton > entry:disabled {"
        "  border: none;"
        "  box-shadow: none;"
        "  background-color: transparent;"
        "}"

        "spinbutton > button {"
        "  border: solid 6px transparent;"
        "}"

        "spinbutton > button:focus:not(:hover):not(:active):not(:disabled) {"
        "  box-shadow: inset 0 0 0 9999px transparent;"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"

        "spinbutton,"
        "entry {"
        "  background-color: #" + aSpinBack.AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"

        "spinbutton:focus,"
        "entry:focus {"
        "  background-color: #" + aSpinBack.AsRGBHexString() + ";"
        "  box-shadow: inset 0 0 0 2px #" + aThemeColors.GetActiveColor().AsRGBHexString() + ";"
        "}"

        "spinbutton:disabled,"
        "entry:disabled {"
        "  box-shadow: inset 0 0 0 2px transparent;"
        "  background-color: #" + aSpinBack.AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetInactiveTextColor().AsRGBHexString() + ";"
        "}"

        /**************
           * ComboBoxes *
           **************/
        "button.combo:only-child {"
        "  background-color: #" + aThemeColors.GetButtonColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"

        "button.combo:only-child:checked {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  box-shadow: 0 0 0 2px #" + aThemeColors.GetButtonColor().AsRGBHexString() + ";"
        "}"

        "button.combo:only-child:disabled {"
        "  box-shadow: 0 0 0 2px transparent;"
        "  background-color: #" + aThemeColors.GetDisabledColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetInactiveTextColor().AsRGBHexString() + ";"
        "}"

        // this one still needs some digging
        "button.small-button, toolbar.small-button button, box.small-button button {"
        "  background-color: #" + aThemeColors.GetMenuBarColor().AsRGBHexString() + ";"
        "}"

        // for the file picker dialog
        "list {"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "treeview.view header button {"
        "  background-color: #" + aThemeColors.GetWindowColor().AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        /************
           * Tooltips *
           ************/
        "tooltip {"
        "  background-color: #" + aThemeColors.GetBaseColor().AsRGBHexString() + ";"
        "}"

        "tooltip * {"
        "  color: #" + aThemeColors.GetWindowTextColor().AsRGBHexString() + ";"
        "}"

        "actionbar > revealer > box .linked > "
        "button:not(.suggested-action):not(.destructive-action), button {"
        "  background-color: #" + aThemeColors.GetButtonColor().AsRGBHexString() + ";"
        "  background-image: radial-gradient(circle, transparent 10%, transparent 0%);"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"


        "actionbar > revealer > box .linked > "
        "button:checked:not(.suggested-action):not(.destructive-action), button:checked {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"

        "tabbox>tab button,"
        "treeview.view header button button.circular,"
        "scrollbar button,"
        "notebook>header>tabs>arrow,"
        "spinbutton>button,"
        "toolbar button,"
        "button.flat {"
        "  background-color: transparent;"
        "}"

        "button.flat:hover {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  border: 1px solid #" + aThemeColors.GetSeparatorColor().AsRGBHexString() + ";"
        "}"

        // this alone won't do
        ".toolbar button:checked,"
        ".toolbar button:hover,"
        "toolbar button:checked,"
        "toolbar button:hover,"
        "button.flat:checked,"
        "button.label-button,"
        "button.flat:checked:hover {"
        "  background-color: #" + aPressedColor.AsRGBHexString() + ";"
        "  text-shadow: none;"
        "}"

        "button.destructive-action:not(:disabled) {"
        "  background-color: #f38ba8;"
        "  color: #" + aThemeColors.GetButtonTextColor().AsRGBHexString() + ";"
        "}"

        ".view:selected {"
        "  background-color: #" + aThemeColors.GetAccentColor().AsRGBHexString() + ";"
        "}";
    // clang-format on

    return OUStringToOString(aStr, RTL_TEXTENCODING_UTF8);
}
}
