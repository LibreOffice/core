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

/*************************************************************************
|*
|*
|*
\************************************************************************/

class URLDlg : public ModalDialog
{
    FixedLine           aFlURL;
    FixedText           aFtURL1;
    Edit                aEdtURL;
    FixedText           aFtURLDescription;
    Edit                aEdtURLDescription;
    FixedText           aFtTarget;
    ComboBox            aCbbTargets;
    FixedText           aFtName;
    Edit                aEdtName;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;

public:

                        URLDlg( Window* pWindow,
                                const String& rURL, const String& rDescription,
                                const String& rTarget, const String& rName,
                                TargetList& rTargetList );

    String              GetURL() const { return aEdtURL.GetText(); }
    String              GetDescription() const { return aEdtURLDescription.GetText(); }
    String              GetTarget() const { return aCbbTargets.GetText(); }
    String              GetName() const { return aEdtName.GetText(); }
};

#endif
