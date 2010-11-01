/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _CUI_IMAPWND_HXX
#define _CUI_IMAPWND_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/itempool.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/transfer.hxx>
#include <svtools/imap.hxx>
#include <sfx2/frame.hxx>
#include <svtools/svmedit.hxx>


/*************************************************************************
|*
|*
|*
\************************************************************************/

class URLDlg : public ModalDialog
{
    FixedText           maFtURL;
    Edit                maEdtURL;
    FixedText           maFtTarget;
    ComboBox            maCbbTargets;
    FixedText           maFtName;
    Edit                maEdtName;
    FixedText           maFtAlternativeText;
    Edit                maEdtAlternativeText;
    FixedText           maFtDescription;
    MultiLineEdit       maEdtDescription;
    FixedLine           maFlURL;
    HelpButton          maBtnHelp;
    OKButton            maBtnOk;
    CancelButton        maBtnCancel;

public:

                        URLDlg( Window* pWindow,
                                const String& rURL, const String& rAlternativeText, const String& rDescription,
                                const String& rTarget, const String& rName,
                                TargetList& rTargetList );

    String              GetURL() const { return maEdtURL.GetText(); }
    String              GetAltText() const { return maEdtAlternativeText.GetText(); }
    String              GetDesc() const { return maEdtDescription.GetText(); }
    String              GetTarget() const { return maCbbTargets.GetText(); }
    String              GetName() const { return maEdtName.GetText(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
