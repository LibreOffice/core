/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _CUI_TBXFORM_HXX
#define _CUI_TBXFORM_HXX

#include <sfx2/tbxctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
