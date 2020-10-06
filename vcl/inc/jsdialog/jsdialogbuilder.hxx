/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <comphelper/string.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/builder.hxx>
#include <salvtables.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/fmtfield.hxx>

class ToolBox;
class ComboBox;
class VclMultiLineEdit;

typedef std::map<OString, weld::Widget*> WidgetMap;

class JSDialogNotifyIdle : public Idle
{
    VclPtr<vcl::Window> m_aWindow;
    std::string m_LastNotificationMessage;
    bool m_bForce;

public:
    JSDialogNotifyIdle(VclPtr<vcl::Window> aWindow);

    void Invoke() override;
    void ForceUpdate();
};

class JSDialogSender
{
    std::unique_ptr<JSDialogNotifyIdle> mpIdleNotify;

public:
    JSDialogSender(VclPtr<vcl::Window> aOwnedToplevel)
        : mpIdleNotify(new JSDialogNotifyIdle(aOwnedToplevel))
    {
    }

    void notifyDialogState(bool bForce = false);
};

class JSInstanceBuilder : public SalInstanceBuilder
{
    sal_uInt64 m_nWindowId;
    /// used in case of tab pages where dialog is not a direct top level
    VclPtr<vcl::Window> m_aParentDialog;
    bool m_bHasTopLevelDialog;
    bool m_bIsNotebookbar;

    friend VCL_DLLPUBLIC bool jsdialog::ExecuteAction(sal_uInt64 nWindowId, const OString& rWidget,
                                                      StringMap& rData);

    static std::map<sal_uInt64, WidgetMap>& GetLOKWeldWidgetsMap();
    static void InsertWindowToMap(sal_uInt64 nWindowId);
    void RememberWidget(const OString& id, weld::Widget* pWidget);
    static weld::Widget* FindWeldWidgetsMap(sal_uInt64 nWindowId, const OString& rWidget);

public:
    JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    /// optional nWindowId is used if getting parent id failed
    JSInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                      const css::uno::Reference<css::frame::XFrame>& rFrame,
                      sal_uInt64 nWindowId = 0);
    virtual ~JSInstanceBuilder() override;
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString& id) override;
    virtual std::unique_ptr<weld::Label> weld_label(const OString& id) override;
    virtual std::unique_ptr<weld::Button> weld_button(const OString& id) override;
    virtual std::unique_ptr<weld::Entry> weld_entry(const OString& id) override;
    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString& id) override;
    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString& id) override;
    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString& id) override;
    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString& id) override;
    virtual std::unique_ptr<weld::DrawingArea>
    weld_drawing_area(const OString& id, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr,
                      void* pUserData = nullptr) override;
    std::unique_ptr<weld::Toolbar> weld_toolbar(const OString& id) override;
    std::unique_ptr<weld::TextView> weld_text_view(const OString& id) override;

    static weld::MessageDialog* CreateMessageDialog(weld::Widget* pParent,
                                                    VclMessageType eMessageType,
                                                    VclButtonsType eButtonType,
                                                    const OUString& rPrimaryMessage);
};

template <class BaseInstanceClass, class VclClass>
class JSWidget : public BaseInstanceClass, public JSDialogSender
{
public:
    JSWidget(VclPtr<vcl::Window> aOwnedToplevel, VclClass* pObject, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership)
        : BaseInstanceClass(pObject, pBuilder, bTakeOwnership)
        , JSDialogSender(aOwnedToplevel)
    {
    }

    virtual void show() override
    {
        BaseInstanceClass::show();
        notifyDialogState();
    }

    virtual void hide() override
    {
        BaseInstanceClass::hide();
        notifyDialogState();
    }

    virtual void set_sensitive(bool sensitive) override
    {
        BaseInstanceClass::set_sensitive(sensitive);
        notifyDialogState();
    }
};

class JSDialog : public JSWidget<SalInstanceDialog, ::Dialog>
{
public:
    JSDialog(VclPtr<vcl::Window> aOwnedToplevel, ::Dialog* pDialog, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override;
    virtual void undo_collapse() override;
};

class JSLabel : public JSWidget<SalInstanceLabel, FixedText>
{
public:
    JSLabel(VclPtr<vcl::Window> aOwnedToplevel, FixedText* pLabel, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_label(const OUString& rText) override;
};

class JSButton : public JSWidget<SalInstanceButton, ::Button>
{
public:
    JSButton(VclPtr<vcl::Window> aOwnedToplevel, ::Button* pButton, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);
};

class JSEntry : public JSWidget<SalInstanceEntry, ::Edit>
{
public:
    JSEntry(VclPtr<vcl::Window> aOwnedToplevel, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
};

class JSListBox : public JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>
{
public:
    JSListBox(VclPtr<vcl::Window> aOwnedToplevel, ::ListBox* pListBox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_active(int pos) override;
};

class JSComboBox : public JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>
{
public:
    JSComboBox(VclPtr<vcl::Window> aOwnedToplevel, ::ComboBox* pComboBox,
               SalInstanceBuilder* pBuilder, bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_entry_text(const OUString& rText) override;
    virtual void set_active(int pos) override;
};

class JSNotebook : public JSWidget<SalInstanceNotebook, ::TabControl>
{
public:
    JSNotebook(VclPtr<vcl::Window> aOwnedToplevel, ::TabControl* pControl,
               SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_current_page(int nPage) override;

    virtual void set_current_page(const OString& rIdent) override;

    virtual void remove_page(const OString& rIdent) override;

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override;
};

class JSSpinButton : public JSWidget<SalInstanceSpinButton, ::FormattedField>
{
public:
    JSSpinButton(VclPtr<vcl::Window> aOwnedToplevel, ::FormattedField* pSpin,
                 SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_value(int value) override;
};

class JSMessageDialog : public SalInstanceMessageDialog, public JSDialogSender
{
public:
    JSMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_primary_text(const OUString& rText) override;

    virtual void set_secondary_text(const OUString& rText) override;
};

class JSCheckButton : public JSWidget<SalInstanceCheckButton, ::CheckBox>
{
public:
    JSCheckButton(VclPtr<vcl::Window> aOwnedToplevel, ::CheckBox* pCheckBox,
                  SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_active(bool active) override;
};

class JSDrawingArea : public SalInstanceDrawingArea, public JSDialogSender
{
public:
    JSDrawingArea(VclPtr<vcl::Window> aOwnedToplevel, VclDrawingArea* pDrawingArea,
                  SalInstanceBuilder* pBuilder, const a11yref& rAlly,
                  FactoryFunction pUITestFactoryFunction, void* pUserData);

    virtual void queue_draw() override;
    virtual void queue_draw_area(int x, int y, int width, int height) override;
};

class JSToolbar : public JSWidget<SalInstanceToolbar, ::ToolBox>
{
public:
    JSToolbar(VclPtr<vcl::Window> aOwnedToplevel, ::ToolBox* pToolbox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);

    virtual void signal_clicked(const OString& rIdent) override;
};

class JSTextView : public JSWidget<SalInstanceTextView, ::VclMultiLineEdit>
{
public:
    JSTextView(VclPtr<vcl::Window> aOwnedToplevel, ::VclMultiLineEdit* pTextView,
               SalInstanceBuilder* pBuilder, bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
