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

#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/svxdllapi.h>
#include <memory>
#include <vector>

namespace vcl { class Window; }
namespace weld { class Button; }
namespace weld { class ComboBox; }
namespace weld { class CustomWeld; }
namespace weld { class Entry; }
namespace weld { class Label; }
namespace weld { class Toolbar; }
namespace weld { class Widget; }
namespace weld { class Window; }

class ImageMap;
class SvtURLBox;

/*************************************************************************
|*
|* Derivation from SfxChildWindow as "container" for float
|*
\************************************************************************/

class Graphic;
typedef ::std::vector< OUString > TargetList;

class SVX_DLLPUBLIC SvxIMapDlgChildWindow final : public SfxChildWindow
{
 public:

    SvxIMapDlgChildWindow( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo const * );

    SFX_DECL_CHILDWINDOW_WITHID( SvxIMapDlgChildWindow );

    static void UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap,
                               const TargetList* pTargetList, void* pEditingObj );
};


class SvxIMapDlg;

class SvxIMapDlgItem final : public SfxControllerItem
{
    SvxIMapDlg& rIMap;

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState ) override;


public:

    SvxIMapDlgItem( SvxIMapDlg& rIMapDlg, SfxBindings& rBindings );
};

class IMapOwnData;
class IMapWindow;
class GraphCtrl;

class SVX_DLLPUBLIC SvxIMapDlg : public SfxModelessDialogController
{
    friend class IMapOwnData;
    friend class IMapWindow;

    std::unique_ptr<IMapOwnData> pOwnData;
    void*               pCheckObj;
    SvxIMapDlgItem      aIMapItem;

    std::unique_ptr<IMapWindow> m_xIMapWnd;
    std::unique_ptr<weld::Toolbar> m_xTbxIMapDlg1;
    std::unique_ptr<weld::Label> m_xFtURL;
    std::unique_ptr<SvtURLBox> m_xURLBox;
    std::unique_ptr<weld::Label> m_xFtText;
    std::unique_ptr<weld::Entry> m_xEdtText;
    std::unique_ptr<weld::Label> m_xFtTarget;
    std::unique_ptr<weld::ComboBox> m_xCbbTarget;
    std::unique_ptr<weld::Button> m_xCancelBtn;
    std::unique_ptr<weld::Label> m_xStbStatus1;
    std::unique_ptr<weld::Label> m_xStbStatus2;
    std::unique_ptr<weld::Label> m_xStbStatus3;
    std::unique_ptr<weld::CustomWeld> m_xIMapWndWeld;

    DECL_LINK( TbxClickHdl, const OString&, void );
    DECL_LINK( InfoHdl, IMapWindow&, void );
    DECL_LINK( MousePosHdl, GraphCtrl*, void );
    DECL_LINK( GraphSizeHdl, GraphCtrl*, void );
    DECL_LINK( URLModifyHdl, weld::ComboBox&, void );
    DECL_LINK( EntryModifyHdl, weld::Entry&, void );
    DECL_LINK( URLLoseFocusHdl, weld::Widget&, void );
    DECL_LINK( UpdateHdl, Timer *, void );
    DECL_LINK( StateHdl, GraphCtrl*, void );
    DECL_LINK( CancelHdl, weld::Button&, void );

    void                URLModify();
    void                DoOpen();
    bool                DoSave();
    void                SetActiveTool(std::string_view rId);

public:

    SvxIMapDlg(SfxBindings *pBindings, SfxChildWindow *pCW,
               weld::Window* pParent);
    virtual ~SvxIMapDlg() override;

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
