#ifndef _CUI_TBXFORM_HXX
#define _CUI_TBXFORM_HXX

#include <sfx2/tbxctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

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

    void SetValue(long dNew) { m_aRecordNo.SetValue(dNew); }
    long GetValue() const { return static_cast<long>(m_aRecordNo.GetValue()); }
};

#endif
