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

    SvxIMapDlgChildWindow( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo const * );

    SFX_DECL_CHILDWINDOW_WITHID( SvxIMapDlgChildWindow );

    static void UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap,
                               const TargetList* pTargetList, void* pEditingObj );
};


class SvxIMapDlg;

class SvxIMapDlgItem : public SfxControllerItem
{
    SvxIMapDlg& rIMap;


protected:

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState ) override;


public:

    SvxIMapDlgItem( SvxIMapDlg& rIMapDlg, SfxBindings& rBindings );
};

class IMapOwnData;
class IMapWindow;
class GraphCtrl;

class SVX_DLLPUBLIC SvxIMapDlg : public SfxModelessDialog // SfxFloatingWindow
{
    friend class IMapOwnData;
    friend class IMapWindow;

    VclPtr<ToolBox>             m_pTbxIMapDlg1;
    VclPtr<FixedText>           m_pFtURL;
    VclPtr<SvtURLBox>           m_pURLBox;
    VclPtr<FixedText>           m_pFtText;
    VclPtr<Edit>                m_pEdtText;
    VclPtr<FixedText>           m_pFtTarget;
    VclPtr<ComboBox>            m_pCbbTarget;
    VclPtr<StatusBar>           m_pStbStatus;

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
    sal_uInt16          mnCloseId;

    Size                aLastSize;
    VclPtr<IMapWindow>         pIMapWnd;
    IMapOwnData*        pOwnData;
    void*               pCheckObj;
    SvxIMapDlgItem      aIMapItem;

    virtual bool    Close() override;

    DECL_LINK( TbxClickHdl, ToolBox*, void );
    DECL_LINK( InfoHdl, IMapWindow&, void );
    DECL_LINK( MousePosHdl, GraphCtrl*, void );
    DECL_LINK( GraphSizeHdl, GraphCtrl*, void );
    DECL_LINK( URLModifyHdl, Edit&, void );
    DECL_LINK( URLModifyComboBoxHdl, ComboBox&, void );
    DECL_LINK( URLLoseFocusHdl, Control&, void );
    DECL_LINK( UpdateHdl, Timer *, void );
    DECL_LINK( StateHdl, GraphCtrl*, void );
    DECL_LINK( MiscHdl, LinkParamNone*, void );

    void                DoOpen();
    bool                DoSave();
    void                SetActiveTool( sal_uInt16 nId );

public:

                        SvxIMapDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    vcl::Window* pParent );
                        virtual ~SvxIMapDlg() override;
    virtual void        dispose() override;

    void                SetExecState( bool bEnable );

    const void*         GetEditingObject() const { return pCheckObj; }

    const ImageMap&     GetImageMap() const;

    void                SetTargetList( const TargetList& rTargetList );

    void                UpdateLink( const Graphic& rGraphic, const ImageMap* pImageMap,
                                const TargetList* pTargetList, void* pEditingObj );
};

SVX_DLLPUBLIC SvxIMapDlg* GetIMapDlg();

#endif // INCLUDED_SVX_IMAPDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
