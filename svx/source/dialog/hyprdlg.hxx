/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_DLG_HYPERDLG_HXX
#define _SVX_DLG_HYPERDLG_HXX

#include <vcl/combobox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svl/srchcfg.hxx>
#include <sfx2/tbxctrl.hxx>

class SvxHyperlinkDlg;
class SfxViewFrame;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class HyperCombo : public ComboBox
{
    SvxHyperlinkDlg *pDlg;
    long            nMaxWidth;
    long            nMinWidth;
    long            nRatio;

    virtual long Notify( NotifyEvent& rNEvt );
    virtual long PreNotify( NotifyEvent& rNEvt );

public:
    HyperCombo( SvxHyperlinkDlg* pDialog, const ResId& rResId );

    inline void SetRatio( long nR ) { nRatio = nR; }
    inline long GetRatio()          { return nRatio; }
    long        CalcResizeWidth( long nW );
    inline long GetResizeWidth()    { return (nMaxWidth - nMinWidth); }
    void        DoResize( long nW );
};

class HyperFixedText : public FixedInfo
{
    SvxHyperlinkDlg *pDlg;

public:
    HyperFixedText( SvxHyperlinkDlg* pDialog, const ResId& rResId );

};

/*--------------------------------------------------------------------
    Beschreibung: Dialog zum EinfÅgen/éndern eines Hyperlink
 --------------------------------------------------------------------*/

class SvxHyperlinkDlg : public ToolBox, public SfxControllerItem
{
    friend class HyperCombo;
    friend class HyperFixedText;
    using ToolBox::StateChanged;

private:
    SfxStatusForwarder  aForwarder;
    SfxStatusForwarder  aHyperlinkDlgForward;
    HyperCombo          aNameCB;
    HyperFixedText      aUrlFT;
    HyperCombo          aUrlCB;

    SvxSearchConfig     aSearchConfig;

    String              sAddress;
    String              sExplorer;
    String              sOldName;
    String              sSearchTitle;
    PopupMenu           aLinkPopup;
    PopupMenu           *pTargetMenu;
    sal_Bool                bNoDoc;
    sal_Bool                bSend;
    sal_Bool                bHasOldName;
    long                nMaxWidth;
    long                nMinWidth;
    long                nMaxHeight;
    sal_Bool                bHtmlMode;

    SfxImageManager*    mpManager;

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

    // DockingWindow
    virtual void Resize();
    virtual void Resizing(Size& rSize);

    // Drag&Drop
    sal_Bool         GetDragData(sal_uInt16 nItem, sal_uIntPtr nDDFormatId, String& rBuffer);

    DECL_LINK(TBClickHdl, ToolBox *);
    DECL_LINK(TBSelectHdl, ToolBox *);
    DECL_LINK(DropdownClick, ToolBox * );
    DECL_LINK(LinkPopupSelectHdl, Menu *);
    DECL_LINK(SearchPopupSelectHdl, Menu *);
    DECL_LINK(ComboSelectHdl, ComboBox *);
    DECL_LINK(ComboModifyHdl, ComboBox *);
    DECL_LINK(BookmarkFoundHdl, String *);

    void    OpenDoc( const String& rURL, SfxViewFrame* pViewFrame );
    void    EnableLink();
    void    SendToApp(sal_uInt16 nType);
    void    AddToHistory(const String& rName, const String& rURL);
    void    TargetMenu(const String& rSelEntry, sal_Bool bExecute);
    String  GetSelTarget();

    void    SetImages();
public:
    SvxHyperlinkDlg(SfxBindings *pBindings, Window* pWindow);
    ~SvxHyperlinkDlg();
};


#endif // _SVX_DLG_HYPERDLG_HXX

