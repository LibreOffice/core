#ifndef INCLUDED_VCL_INC_JSDIALOG_JSDIALOG_HXX
#define INCLUDED_VCL_INC_JSDIALOG_JSDIALOG_HXX

#include <vcl/weld.hxx>
#include <comphelper/string.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/builder.hxx>
#include <salvtables.hxx>

class VCL_DLLPUBLIC JSInstanceBuilder : public SalInstanceBuilder
{
public:
    JSInstanceBuilder(weld::Widget* pParent, const OUString& rUIRoot, const OUString& rUIFile);
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString& id,
                                                      bool bTakeOwnership = true) override;
    virtual std::unique_ptr<weld::Label> weld_label(const OString& id,
                                                    bool bTakeOwnership = false) override;
};

class VCL_DLLPUBLIC JSLabel : public SalInstanceLabel
{
    VclPtr<vcl::Window> m_aOwnedToplevel;

public:
    JSLabel(VclPtr<vcl::Window> aOwnedToplevel, FixedText* pLabel, SalInstanceBuilder* pBuilder,
            bool bTakeOwnership);
    virtual void set_label(const OUString& rText) override;
};

#endif