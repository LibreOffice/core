/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hlinettp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:14:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_TABPAGE_INET_HYPERLINK_HXX
#define _SVX_TABPAGE_INET_HYPERLINK_HXX

#ifndef _SVX_TAB_HYPERLINK_HXX
#include <hyperdlg.hxx>
#endif

#ifndef _CUI_TAB_HYPERLINK_HXX
#include "cuihyperdlg.hxx"
#endif
#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Internet
|*
\************************************************************************/

class SvxHyperlinkInternetTp : public SvxHyperlinkTabPageBase
{
private:
    FixedLine           maGrpLinkTyp;
    RadioButton         maRbtLinktypInternet;
    RadioButton         maRbtLinktypFTP;
    RadioButton         maRbtLinktypTelnet;
    FixedText           maFtTarget;
    SvxHyperURLBox      maCbbTarget;
    FixedText           maFtLogin;
    Edit                maEdLogin;
    FixedText           maFtPassword;
    Edit                maEdPassword;
    CheckBox            maCbAnonymous;
    ImageButton         maBtBrowse;
    ImageButton         maBtTarget;

    String              maStrOldUser;
    String              maStrOldPassword;

    BOOL                mbMarkWndOpen;

    String              maStrStdDocURL;

    DECL_LINK (Click_SmartProtocol_Impl  , void * );        // Radiobutton clicked: Type Internet, FTP or Telnet
    DECL_LINK (ClickAnonymousHdl_Impl    , void * );        // Checkbox : Anonymer Benutzer
    DECL_LINK (ClickBrowseHdl_Impl       , void * );        // Button : Browse
    DECL_LINK (ClickTargetHdl_Impl       , void * );        // Button : Ziel
    DECL_LINK (ModifiedLoginHdl_Impl     , void * );        // Contens of editfield "Login" modified
    DECL_LINK (LostFocusTargetHdl_Impl   , void * );        // Combobox "Target" lost its focus
    DECL_LINK (ModifiedTargetHdl_Impl    , void * );        // Contens of editfield "Target" modified

    DECL_LINK (TimeoutHdl_Impl           , Timer * );       // Handler for timer -timeout


    void    SetScheme( const String& aScheme );
    void    RemoveImproperProtocol(const String& aProperScheme);
    String  GetSchemeFromButtons() const;
    INetProtocol GetSmartProtocolFromButtons() const;

    String CreateAbsoluteURL() const;

    void   setAnonymousFTPUser();
    void   setFTPUser(const String& rUser, const String& rPassword);
    void   RefreshMarkWindow();

protected:
    virtual void FillDlgFields     ( String& aStrURL );
    virtual void GetCurentItemData ( String& aStrURL, String& aStrName,
                                     String& aStrIntName, String& aStrFrame,
                                     SvxLinkInsertMode& eMode );
    virtual BOOL ShouldOpenMarkWnd () {return ( mbMarkWndOpen && maRbtLinktypInternet.IsChecked() );}
    virtual void SetMarkWndShouldOpen (BOOL bOpen) {mbMarkWndOpen=bOpen;}

public:
    SvxHyperlinkInternetTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkInternetTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( String& aStrMark );
    virtual void        SetOnlineMode( BOOL bEnable );

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_INET_HYPERLINK_HXX
