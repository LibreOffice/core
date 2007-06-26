#ifndef _CUI_IMAPWND_HXX
#define _CUI_IMAPWND_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX //autogen
#include <svtools/imapobj.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
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
