/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <vcl/uitest/factory.hxx>

#include <memory>

namespace comphelper
{
class OAccessible;
}

enum class FieldUnit : sal_uInt16;

namespace weld
{
class Assistant;
class Box;
class Button;
class Calendar;
class CheckButton;
class ComboBox;
class Container;
class Dialog;
class DrawingArea;
class Entry;
class EntryTreeView;
class Expander;
class FormattedSpinButton;
class Frame;
class Grid;
class IconView;
class Image;
class Label;
class LevelBar;
class LinkButton;
class Menu;
class MenuButton;
class MessageDialog;
class MetricSpinButton;
class Notebook;
class Paned;
class Popover;
class ProgressBar;
class RadioButton;
class Scale;
class Scrollbar;
class ScrolledWindow;
class SizeGroup;
class SpinButton;
class Spinner;
class TextView;
class ToggleButton;
class Toolbar;
class TreeView;
class Widget;
class Window;

class VCL_DLLPUBLIC Builder
{
public:
    virtual std::unique_ptr<MessageDialog> weld_message_dialog(const OUString& id) = 0;
    virtual std::unique_ptr<Dialog> weld_dialog(const OUString& id) = 0;
    virtual std::unique_ptr<Assistant> weld_assistant(const OUString& id) = 0;
    virtual std::unique_ptr<Widget> weld_widget(const OUString& id) = 0;
    virtual std::unique_ptr<Container> weld_container(const OUString& id) = 0;
    virtual std::unique_ptr<Box> weld_box(const OUString& id) = 0;
    virtual std::unique_ptr<Grid> weld_grid(const OUString& id) = 0;
    virtual std::unique_ptr<Paned> weld_paned(const OUString& id) = 0;
    virtual std::unique_ptr<Button> weld_button(const OUString& id) = 0;
    virtual std::unique_ptr<MenuButton> weld_menu_button(const OUString& id) = 0;
    virtual std::unique_ptr<Frame> weld_frame(const OUString& id) = 0;
    /* bUserManagedScrolling of true means that the automatic scrolling of the window is disabled
       and the owner must specifically listen to adjustment changes and react appropriately to them.
    */
    virtual std::unique_ptr<ScrolledWindow> weld_scrolled_window(const OUString& id,
                                                                 bool bUserManagedScrolling = false)
        = 0;
    virtual std::unique_ptr<Notebook> weld_notebook(const OUString& id) = 0;
    virtual std::unique_ptr<ToggleButton> weld_toggle_button(const OUString& id) = 0;
    virtual std::unique_ptr<RadioButton> weld_radio_button(const OUString& id) = 0;
    virtual std::unique_ptr<CheckButton> weld_check_button(const OUString& id) = 0;
    virtual std::unique_ptr<LinkButton> weld_link_button(const OUString& id) = 0;
    virtual std::unique_ptr<SpinButton> weld_spin_button(const OUString& id) = 0;
    virtual std::unique_ptr<MetricSpinButton> weld_metric_spin_button(const OUString& id,
                                                                      FieldUnit eUnit);
    virtual std::unique_ptr<FormattedSpinButton> weld_formatted_spin_button(const OUString& id) = 0;
    virtual std::unique_ptr<ComboBox> weld_combo_box(const OUString& id) = 0;
    virtual std::unique_ptr<TreeView> weld_tree_view(const OUString& id) = 0;
    virtual std::unique_ptr<IconView> weld_icon_view(const OUString& id) = 0;
    virtual std::unique_ptr<Label> weld_label(const OUString& id) = 0;
    virtual std::unique_ptr<TextView> weld_text_view(const OUString& id) = 0;
    virtual std::unique_ptr<Expander> weld_expander(const OUString& id) = 0;
    virtual std::unique_ptr<Entry> weld_entry(const OUString& id) = 0;
    virtual std::unique_ptr<Scale> weld_scale(const OUString& id) = 0;
    virtual std::unique_ptr<ProgressBar> weld_progress_bar(const OUString& id) = 0;
    virtual std::unique_ptr<LevelBar> weld_level_bar(const OUString& id) = 0;
    virtual std::unique_ptr<Spinner> weld_spinner(const OUString& id) = 0;
    virtual std::unique_ptr<Image> weld_image(const OUString& id) = 0;
    virtual std::unique_ptr<Calendar> weld_calendar(const OUString& id) = 0;
    virtual std::unique_ptr<DrawingArea>
    weld_drawing_area(const OUString& id,
                      const rtl::Reference<comphelper::OAccessible>& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr, void* pUserData = nullptr)
        = 0;
    virtual std::unique_ptr<EntryTreeView> weld_entry_tree_view(const OUString& containerid,
                                                                const OUString& entryid,
                                                                const OUString& treeviewid)
        = 0;
    virtual std::unique_ptr<Menu> weld_menu(const OUString& id) = 0;
    virtual std::unique_ptr<Popover> weld_popover(const OUString& id) = 0;
    virtual std::unique_ptr<Toolbar> weld_toolbar(const OUString& id) = 0;
    virtual std::unique_ptr<Scrollbar> weld_scrollbar(const OUString& id) = 0;
    virtual std::unique_ptr<SizeGroup> create_size_group() = 0;
    /* return a Dialog suitable to take a screenshot of containing the contents of the .ui file.

       If the toplevel element is a dialog, that will be returned
       If the toplevel is not a dialog, a dialog will be created and the contents of the .ui
       inserted into it
    */
    virtual std::unique_ptr<Window> create_screenshot_window() = 0;
    virtual ~Builder() {}
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
