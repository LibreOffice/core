#ifndef INCLUDED_VCL_INC_JSDIALOG_JSDIALOG_HXX
#define INCLUDED_VCL_INC_JSDIALOG_JSDIALOG_HXX

#include <vcl/weld.hxx>
#include <comphelper/string.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/builder.hxx>
#include <salvtables.hxx>
#include <vcl/combobox.hxx>
#include <vcl/button.hxx>

class JSDialogSender
{
    VclPtr<vcl::Window> m_aOwnedToplevel;

public:
    JSDialogSender(VclPtr<vcl::Window> aOwnedToplevel)
        : m_aOwnedToplevel(aOwnedToplevel)
    {
    }

    void notifyDialogState();
};

class VCL_DLLPUBLIC JSInstanceBuilder : public SalInstanceBuilder
{
public:
    JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString& id,
                                                      bool bTakeOwnership = true) override;
    virtual std::unique_ptr<weld::Label> weld_label(const OString& id,
                                                    bool bTakeOwnership = false) override;
    virtual std::unique_ptr<weld::Button> weld_button(const OString& id,
                                                      bool bTakeOwnership = false) override;
    virtual std::unique_ptr<weld::Entry> weld_entry(const OString& id,
                                                    bool bTakeOwnership = false) override;
    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString& id,
                                                           bool bTakeOwnership = false) override;
    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString& id,
                                                          bool bTakeOwnership = false) override;
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

class VCL_DLLPUBLIC JSLabel : public JSWidget<SalInstanceLabel, FixedText>
{
public:
    JSLabel(VclPtr<vcl::Window> aOwnedToplevel, FixedText* pLabel, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_label(const OUString& rText) override;
};

class VCL_DLLPUBLIC JSButton : public JSWidget<SalInstanceButton, ::Button>
{
public:
    JSButton(VclPtr<vcl::Window> aOwnedToplevel, ::Button* pButton, SalInstanceBuilder* pBuilder,
             bool bTakeOwnership);
};

class VCL_DLLPUBLIC JSEntry : public JSWidget<SalInstanceEntry, ::Edit>
{
public:
    JSEntry(VclPtr<vcl::Window> aOwnedToplevel, ::Edit* pEntry, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_text(const OUString& rText) override;
};

class VCL_DLLPUBLIC JSListBox : public JSWidget<SalInstanceComboBoxWithoutEdit, ::ListBox>
{
public:
    JSListBox(VclPtr<vcl::Window> aOwnedToplevel, ::ListBox* pListBox, SalInstanceBuilder* pBuilder,
              bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
};

class VCL_DLLPUBLIC JSComboBox : public JSWidget<SalInstanceComboBoxWithEdit, ::ComboBox>
{
public:
    JSComboBox(VclPtr<vcl::Window> aOwnedToplevel, ::ComboBox* pComboBox,
               SalInstanceBuilder* pBuilder, bool bTakeOwnership);
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void remove(int pos) override;
    virtual void set_entry_text(const OUString& rText) override;
};

class VCL_DLLPUBLIC JSNotebook : public JSWidget<SalInstanceNotebook, ::TabControl>
{
public:
    JSNotebook(VclPtr<vcl::Window> aOwnedToplevel, ::TabControl* pControl,
               SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_current_page(int nPage) override;

    virtual void set_current_page(const OString& rIdent) override;

    virtual void remove_page(const OString& rIdent) override;

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override;
};

#endif