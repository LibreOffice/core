/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_TABPAGE_INET_HYPERLINK_HXX
#define _SVX_TABPAGE_INET_HYPERLINK_HXX

#include <svx/hyperdlg.hxx>
#include "cuihyperdlg.hxx"
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
    ImageButton         maBtBrowse;
    FixedText           maFtLogin;
    Edit                maEdLogin;
    ImageButton         maBtTarget;
    FixedText           maFtPassword;
    Edit                maEdPassword;
    CheckBox            maCbAnonymous;

    String              maStrOldUser;
    String              maStrOldPassword;

    sal_Bool                mbMarkWndOpen;

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
    virtual sal_Bool ShouldOpenMarkWnd () {return ( mbMarkWndOpen && maRbtLinktypInternet.IsChecked() );}
    virtual void SetMarkWndShouldOpen (sal_Bool bOpen) {mbMarkWndOpen=bOpen;}

public:
    SvxHyperlinkInternetTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkInternetTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( String& aStrMark );
    virtual void        SetOnlineMode( sal_Bool bEnable );

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_INET_HYPERLINK_HXX
