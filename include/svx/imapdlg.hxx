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


#ifndef _IMAPDLG_HXX_
#define _IMAPDLG_HXX_

#include <svtools/inettbc.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>

#include <vcl/combobox.hxx>

#include <vcl/edit.hxx>

#include <vcl/toolbox.hxx>

#include <vcl/status.hxx>
#include "svx/svxdllapi.h"
#include <vector>


#ifndef _GOMISC_HXX
class ImageMap;
#endif


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

    SvxIMapDlgChildWindow( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW_WITHID( SvxIMapDlgChildWindow );

    static void UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                               const TargetList* pTargetList = NULL, void* pEditingObj = NULL );
};


#ifndef _REDUCED_IMAPDLG_HXX_
#define _REDUCED_IMAPDLG_HXX_

class SvxIMapDlg;

class SvxIMapDlgItem : public SfxControllerItem
{
    SvxIMapDlg& rIMap;


protected:

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState );


public:

    SvxIMapDlgItem( sal_uInt16 nId, SvxIMapDlg& rIMapDlg, SfxBindings& rBindings );
};

class IMapOwnData;
class IMapWindow;

class SVX_DLLPUBLIC SvxIMapDlg : public SfxModelessDialog // SfxFloatingWindow
{
    friend class IMapOwnData;
    friend class IMapWindow;
    using Window::Update;

    ToolBox             aTbxIMapDlg1;
    FixedText           aFtURL;
    SvtURLBox           maURLBox;
    FixedText           aFtText;
    Edit                aEdtText;
    FixedText           maFtTarget;
    ComboBox            maCbbTarget;
    StatusBar           aStbStatus;
    ImageList           maImageList;

    Size                aLastSize;
    IMapWindow*         pIMapWnd;
    IMapOwnData*        pOwnData;
    void*               pCheckObj;
    SvxIMapDlgItem      aIMapItem;

    virtual void        Resize();
    virtual sal_Bool        Close();

#ifdef _IMAPDLG_PRIVATE

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
    sal_Bool                DoSave();

#endif


public:

                        SvxIMapDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    Window* pParent, const ResId& rResId );
                        ~SvxIMapDlg();

    void                SetExecState( sal_Bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap() const;

    void                SetTargetList( const TargetList& rTargetList );

    void                Update( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                                const TargetList* pTargetList = NULL, void* pEditingObj = NULL );

    virtual void        KeyInput( const KeyEvent& rKEvt );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void ApplyImageList();
};


/*************************************************************************
|*
|* Defines
|*
\************************************************************************/

#define SVXIMAPDLG() ( (SvxIMapDlg*) ( SfxViewFrame::Current()->GetChildWindow(         \
                        SvxIMapDlgChildWindow::GetChildWindowId() )->   \
                        GetWindow() ) )


#endif // _REDUCED_IMAPDLG_HXX_
#endif // _IMAPDLG_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
