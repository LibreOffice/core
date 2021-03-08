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


#include <svl/eitem.hxx>
#include <vcl/toolbox.hxx>

#include <svx/tbxctl.hxx>

#include <com/sun/star/frame/XLayoutManager.hpp>

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlDraw, SfxBoolItem);

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;


SvxTbxCtlDraw::SvxTbxCtlDraw( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::CHECKABLE | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

void SAL_CALL SvxTbxCtlDraw::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::ToolboxController::initialize(aArguments);
    /*
     * Toolbar name is defined as "private:resource/toolbar/drawbar" in writer and calc,
     * "private:resource/toolbar/toolbar" in draw and impress. Control is added for this
     * difference.
     */
    if( m_aCommandURL==".uno:TrackChangesBar")
        m_sToolboxName="private:resource/toolbar/changes";
    else if ( m_sModuleName == "com.sun.star.presentation.PresentationDocument" || m_sModuleName == "com.sun.star.drawing.DrawingDocument" )
        m_sToolboxName="private:resource/toolbar/toolbar";
    else
        m_sToolboxName="private:resource/toolbar/drawbar";
}


void SvxTbxCtlDraw::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), ( eState != SfxItemState::DISABLED ) );
    SfxToolBoxControl::StateChanged( nSID, eState, pState );

    Reference< XLayoutManager > xLayoutMgr = getLayoutManager();
    if ( xLayoutMgr.is() )
        GetToolBox().CheckItem(
            GetId(), xLayoutMgr->isElementVisible( m_sToolboxName ) );
}


void SvxTbxCtlDraw::toggleToolbox()
{
    Reference< XLayoutManager > xLayoutMgr = getLayoutManager();
    if ( !xLayoutMgr.is() )
        return;

    bool bCheck = false;
    if ( xLayoutMgr->isElementVisible( m_sToolboxName ) )
    {
        xLayoutMgr->hideElement( m_sToolboxName );
        xLayoutMgr->destroyElement( m_sToolboxName );
    }
    else
    {
        bCheck = true;
        xLayoutMgr->createElement( m_sToolboxName );
        xLayoutMgr->showElement( m_sToolboxName );
    }

    GetToolBox().CheckItem( GetId(), bCheck );
}


void SvxTbxCtlDraw::Select(sal_uInt16 /*nSelectModifier*/)
{
    toggleToolbox();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
