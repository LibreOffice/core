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

#include <uielement/genericstatusbarcontroller.hxx>
#include <uielement/statusbarmerger.hxx>

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>

#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/XStatusbarItem.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ImageDrawMode.hpp>
#include <com/sun/star/awt/XGraphics2.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

namespace framework
{

GenericStatusbarController::GenericStatusbarController(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& rxFrame,
    const Reference< ui::XStatusbarItem >& rxItem,
    AddonStatusbarItemData *pItemData )
    : svt::StatusbarController( rxContext, rxFrame, OUString(), 0 )
    , m_bEnabled( false )
    , m_bOwnerDraw( false )
    , m_pItemData( pItemData )
{
    m_xStatusbarItem = rxItem;
    if ( m_xStatusbarItem.is() )
    {
        assert(m_aCommandURL.pData);
        m_aCommandURL = m_xStatusbarItem->getCommand();
        m_nID = m_xStatusbarItem->getItemId();
        m_bOwnerDraw = ( m_xStatusbarItem->getStyle() & ui::ItemStyle::OWNER_DRAW ) == ui::ItemStyle::OWNER_DRAW;
        if ( !m_bOwnerDraw && m_pItemData && m_pItemData->aLabel.getLength() )
            m_xStatusbarItem->setText( m_pItemData->aLabel );
    }
}

GenericStatusbarController::~GenericStatusbarController()
{
}

void SAL_CALL GenericStatusbarController::dispose()
{
    svt::StatusbarController::dispose();

    SolarMutexGuard aGuard;
    m_pItemData = nullptr;
    m_xGraphic.clear();
    m_xStatusbarItem.clear();

}

void SAL_CALL GenericStatusbarController::statusChanged(
    const FeatureStateEvent& rEvent)
{
    SolarMutexGuard aGuard;

    if ( m_bDisposed || !m_xStatusbarItem.is() )
        return;

    m_bEnabled = rEvent.IsEnabled;

    OUString aStrValue;
    Reference< graphic::XGraphic > aGraphic;

    if ( rEvent.State >>= aStrValue )
    {
        if ( !m_bOwnerDraw )
            m_xStatusbarItem->setText( aStrValue );
        else
        {
            if ( aStrValue.getLength() )
            {
                m_xStatusbarItem->setQuickHelpText( aStrValue );
            }
        }
    }
    else if ( ( rEvent.State >>= aGraphic ) && m_bOwnerDraw )
    {
        m_xGraphic = aGraphic;
    }

    // when the status is updated, and the controller is responsible for
    // painting the statusbar item content, we must trigger a repaint
    if ( m_bOwnerDraw && m_xStatusbarItem->getVisible() )
    {
        m_xStatusbarItem->repaint();
    }
}

void SAL_CALL GenericStatusbarController::paint(
    const Reference< awt::XGraphics >& xGraphics,
    const awt::Rectangle& rOutputRectangle,
    ::sal_Int32 /*nStyle*/ )
{
    SolarMutexGuard aGuard;

    const Reference< awt::XGraphics2 > xGraphics2(xGraphics, UNO_QUERY);

    if ( !m_xStatusbarItem.is() || !xGraphics2.is() )
        return;

    Reference< beans::XPropertySet > xGraphicProps( m_xGraphic, UNO_QUERY );

    if ( xGraphicProps.is() && m_xGraphic->getType() != graphic::GraphicType::EMPTY )
    {
        awt::Size aGraphicSize;
        xGraphicProps->getPropertyValue( "SizePixel" ) >>= aGraphicSize;
        OSL_ENSURE( aGraphicSize.Height > 0 && aGraphicSize.Width > 0, "Empty status bar graphic!" );

        sal_Int32 nOffset = m_xStatusbarItem->getOffset( );
        awt::Point aPos;
        aPos.X = ( rOutputRectangle.Width + nOffset ) / 2 - aGraphicSize.Width / 2;
        aPos.Y = rOutputRectangle.Height / 2 - aGraphicSize.Height / 2;

        xGraphics2->drawImage( rOutputRectangle.X + aPos.X,
                              rOutputRectangle.Y + aPos.Y,
                              aGraphicSize.Width,
                              aGraphicSize.Height,
                              m_bEnabled ? awt::ImageDrawMode::NONE : awt::ImageDrawMode::DISABLE,
                              m_xGraphic );
    }
    else
    {
        xGraphics2->clear( rOutputRectangle );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
