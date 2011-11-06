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


#ifndef _SVX_TABBASE_HYPERLINK_HXX
#define _SVX_TABBASE_HYPERLINK_HXX

#define INET_TELNET_SCHEME      "telnet://"

#include <sfx2/app.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <tools/urlobj.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/inettbc.hxx>
#include <vcl/timer.hxx>

#include <dialmgr.hxx>
#include <sfx2/docfile.hxx>
#include <cuires.hrc>
#include <com/sun/star/frame/XFrame.hpp>
#include "helpid.hrc"
#include <svx/hlnkitem.hxx>

#include "hlmarkwn.hxx"
#include "iconcdlg.hxx"


/*************************************************************************
|*
|* ComboBox-Control, wich is filled with all current framenames
|*
\************************************************************************/

class SvxFramesComboBox : public ComboBox
{
public:
    SvxFramesComboBox (Window* pParent, const ResId& rResId, SfxDispatcher* pDispatch);
    ~SvxFramesComboBox ();
};

/*************************************************************************
|*
|* ComboBox-Control for URL's with History and Autocompletion
|*
\************************************************************************/

class SvxHyperURLBox : public SvtURLBox, public DropTargetHelper
{
private:
    sal_Bool   mbAccessAddress;

//  String GetAllEmailNamesFromDragItem( sal_uInt16 nItem );

protected:

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        Select();
    virtual void        Modify();
    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SvxHyperURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_FILE, sal_Bool bAddresses = sal_False );

};

/*************************************************************************
|*
|* Tabpage : Basisclass
|*
\************************************************************************/

class SvxHyperlinkTabPageBase : public IconChoicePage
{
private:
    FixedLine           *mpGrpMore;
    FixedText           *mpFtFrame;
    SvxFramesComboBox   *mpCbbFrame;
    FixedText           *mpFtForm;
    ListBox             *mpLbForm;
    FixedText           *mpFtIndication;
    Edit                *mpEdIndication;
    FixedText           *mpFtText;
    Edit                *mpEdText;
    ImageButton         *mpBtScript;

    sal_Bool            mbIsCloseDisabled;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        mxDocumentFrame;

protected:
    Window*             mpDialog;

    sal_Bool                mbStdControlsInit;

    String              maStrInitURL;

    Timer               maTimer;

    SvxHlinkDlgMarkWnd* mpMarkWnd;

    void InitStdControls ();
    virtual void FillStandardDlgFields ( SvxHyperlinkItem* pHyperlinkItem );
    virtual void FillDlgFields         ( String& aStrURL ) = 0;
    virtual void GetCurentItemData     ( String& aStrURL, String& aStrName,
                                         String& aStrIntName, String& aStrFrame,
                                         SvxLinkInsertMode& eMode ) = 0;
    virtual String CreateUiNameFromURL( const String& aStrURL );

    void         GetDataFromCommonFields( String& aStrName,
                                          String& aStrIntName, String& aStrFrame,
                                          SvxLinkInsertMode& eMode );

    DECL_LINK (ClickScriptHdl_Impl, void * );       // Button : Script

    String              aEmptyStr;

    sal_Bool            FileExists( const INetURLObject& rURL );
    static String   GetSchemeFromURL( String aStrURL );

    inline void     DisableClose( sal_Bool _bDisable ) { mbIsCloseDisabled = _bDisable; }

public:
    SvxHyperlinkTabPageBase (
        Window *pParent,
        const ResId &rResId,
        const SfxItemSet& rItemSet
    );
    virtual ~SvxHyperlinkTabPageBase ();

    void    SetDocumentFrame(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame )
    {
        mxDocumentFrame = rxDocumentFrame;
    }

    virtual sal_Bool AskApply ();
    virtual void DoApply ();
    virtual void SetOnlineMode( sal_Bool bEnable );
    virtual void SetInitFocus();
    virtual void SetMarkStr ( String& aStrMark );
    virtual void Reset( const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void ActivatePage( const SfxItemSet& rItemSet );
    virtual int  DeactivatePage( SfxItemSet* pSet = 0 );

    sal_Bool IsMarkWndVisible ()      { return ((Window*)mpMarkWnd)->IsVisible(); }
    Size GetSizeExtraWnd ()       { return ( mpMarkWnd->GetSizePixel() ); }
    sal_Bool MoveToExtraWnd ( Point aNewPos, sal_Bool bDisConnectDlg = sal_False );

    virtual void        ActivatePage();
    virtual void        DeactivatePage();
    virtual sal_Bool    QueryClose();

protected:
    virtual sal_Bool ShouldOpenMarkWnd();
    virtual void SetMarkWndShouldOpen(sal_Bool bOpen);

    void ShowMarkWnd ();
    void HideMarkWnd ()           { ( ( Window* ) mpMarkWnd )->Hide(); }
    void InvalidateMarkWnd ()     { ( ( Window* ) mpMarkWnd )->Invalidate(); }

    SfxDispatcher* GetDispatcher() const;

    sal_uInt16             GetMacroEvents();
    SvxMacroTableDtor* GetMacroTable();

    sal_Bool IsHTMLDoc() const;
};

#endif // _SVX_TABBASE_HYPERLINK_HXX

