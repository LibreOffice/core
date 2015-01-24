/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_IMAPDLG_HXX
#define INCLUDED_SVX_IMAPDLG_HXX

#include <svtools/inettbc.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>
#include <svx/svxdllapi.h>
#include <vector>

class ImageMap;

/*************************************************************************
|*
|* Derivation from SfxChildWindow as "container" for float
|*
\************************************************************************/

class Graphic;
typedef ::std::vector< OUString > TargetList;

class SVX_DLLPUBLIC SvxIMapDlgChildWindow : public SfxChildWindow
{
 public:

    SvxIMapDlgChildWindow( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW_WITHID( SvxIMapDlgChildWindow );

    static void UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                               const TargetList* pTargetList = NULL, void* pEditingObj = NULL );
};


class SvxIMapDlg;

class SvxIMapDlgItem : public SfxControllerItem
{
    SvxIMapDlg& rIMap;


protected:

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState ) SAL_OVERRIDE;


public:

    SvxIMapDlgItem( sal_uInt16 nId, SvxIMapDlg& rIMapDlg, SfxBindings& rBindings );
};

class IMapOwnData;
class IMapWindow;

class SVX_DLLPUBLIC SvxIMapDlg : public SfxModelessDialog // SfxFloatingWindow
{
    friend class IMapOwnData;
    friend class IMapWindow;

    ToolBox             *m_pTbxIMapDlg1;
    FixedText           *m_pFtURL;
    SvtURLBox           *m_pURLBox;
    FixedText           *m_pFtText;
    Edit                *m_pEdtText;
    FixedText           *m_pFtTarget;
    ComboBox            *m_pCbbTarget;
    StatusBar           *m_pStbStatus;

    sal_uInt16          mnApplyId;
    sal_uInt16          mnOpenId;
    sal_uInt16          mnSaveAsId;
    sal_uInt16          mnSelectId;
    sal_uInt16          mnRectId;
    sal_uInt16          mnCircleId;
    sal_uInt16          mnPolyId;
    sal_uInt16          mnFreePolyId;
    sal_uInt16          mnPolyEditId;
    sal_uInt16          mnPolyMoveId;
    sal_uInt16          mnPolyInsertId;
    sal_uInt16          mnPolyDeleteId;
    sal_uInt16          mnUndoId;
    sal_uInt16          mnRedoId;
    sal_uInt16          mnActiveId;
    sal_uInt16          mnMacroId;
    sal_uInt16          mnPropertyId;

    Size                aLastSize;
    IMapWindow*         pIMapWnd;
    IMapOwnData*        pOwnData;
    void*               pCheckObj;
    SvxIMapDlgItem      aIMapItem;

    virtual bool    Close() SAL_OVERRIDE;

    DECL_LINK( TbxClickHdl, ToolBox* );
    DECL_LINK( InfoHdl, IMapWindow* );
    DECL_LINK( MousePosHdl, IMapWindow* );
    DECL_LINK( GraphSizeHdl, IMapWindow* );
    DECL_LINK( URLModifyHdl, void* );
    DECL_LINK( URLLoseFocusHdl, void* );
    DECL_LINK(UpdateHdl, void *);
    DECL_LINK( TbxUpdateHdl, Timer* );
    DECL_LINK( StateHdl, IMapWindow* );
    DECL_LINK( MiscHdl, void* );

    void                DoOpen();
    bool                DoSave();

public:

                        SvxIMapDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    vcl::Window* pParent );
                        virtual ~SvxIMapDlg();

    void                SetExecState( bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap() const;

    void                SetTargetList( const TargetList& rTargetList );

    void                UpdateLink( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                                const TargetList* pTargetList = NULL, void* pEditingObj = NULL );

    virtual void        KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
};

inline SvxIMapDlg* SVXIMAPDLG()
{
    SfxChildWindow* pWnd = NULL;
    if (SfxViewFrame::Current() && SfxViewFrame::Current()->HasChildWindow(SvxIMapDlgChildWindow::GetChildWindowId()))
        pWnd = SfxViewFrame::Current()->GetChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    return pWnd ? static_cast<SvxIMapDlg*>(pWnd->GetWindow()) : NULL;
}

#endif // INCLUDED_SVX_IMAPDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
