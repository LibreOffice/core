#ifndef _CUI_TBXFORM_HXX
#define _CUI_TBXFORM_HXX

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef _SFXTBXMGR_HXX //autogen
#include <sfx2/tbxmgr.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

//========================================================================

class FmInputRecordNoDialog : public ModalDialog
{
public:
    FixedText       m_aLabel;
    NumericField    m_aRecordNo;
    OKButton        m_aOk;
    CancelButton    m_aCancel;

public:
    FmInputRecordNoDialog(Window * pParent);

    void SetValue(double dNew) { m_aRecordNo.SetValue(dNew); }
    long GetValue() const { return m_aRecordNo.GetValue(); }
};

#endif
