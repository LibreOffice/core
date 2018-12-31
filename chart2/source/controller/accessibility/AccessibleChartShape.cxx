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

#include "AccessibleChartShape.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleShape.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;

namespace chart
{

AccessibleChartShape::AccessibleChartShape(
        const AccessibleElementInfo& rAccInfo )
    :impl::AccessibleChartShape_Base( rAccInfo, true/*bMayHaveChildren*/, false/*bAlwaysTransparent*/ )
{
    if ( rAccInfo.m_aOID.isAdditionalShape() )
    {
        Reference< drawing::XShape > xShape( rAccInfo.m_aOID.getAdditionalShape() );
        Reference< XAccessible > xParent;
        if ( rAccInfo.m_pParent )
        {
            xParent.set( rAccInfo.m_pParent );
        }
        ::accessibility::AccessibleShapeInfo aShapeInfo( xShape, xParent );

        m_aShapeTreeInfo.SetSdrView( rAccInfo.m_pSdrView );
        m_aShapeTreeInfo.SetController( nullptr );
        m_aShapeTreeInfo.SetWindow( VCLUnoHelper::GetWindow( rAccInfo.m_xWindow ) );
        m_aShapeTreeInfo.SetViewForwarder( rAccInfo.m_pViewForwarder );

        ::accessibility::ShapeTypeHandler& rShapeHandler = ::accessibility::ShapeTypeHandler::Instance();
        m_pAccShape = rShapeHandler.CreateAccessibleObject( aShapeInfo, m_aShapeTreeInfo );
        if ( m_pAccShape.is() )
        {
            m_pAccShape->Init();
        }
    }
}

AccessibleChartShape::~AccessibleChartShape()
{
    OSL_ASSERT( CheckDisposeState( false /* don't throw exceptions */ ) );

    if ( m_pAccShape.is() )
    {
        m_pAccShape->dispose();
    }
}

// ________ XServiceInfo ________
OUString AccessibleChartShape::getImplementationName()
{
    return OUString( "AccessibleChartShape" );
}

// ________ XAccessibleContext ________
sal_Int32 AccessibleChartShape::getAccessibleChildCount()
{
    sal_Int32 nCount(0);
    if ( m_pAccShape.is() )
    {
        nCount = m_pAccShape->getAccessibleChildCount();
    }
    return nCount;
}

Reference< XAccessible > AccessibleChartShape::getAccessibleChild( sal_Int32 i )
{
    Reference< XAccessible > xChild;
    if ( m_pAccShape.is() )
    {
        xChild = m_pAccShape->getAccessibleChild( i );
    }
    return xChild;
}

sal_Int16 AccessibleChartShape::getAccessibleRole()
{
    sal_Int16 nRole(0);
    if ( m_pAccShape.is() )
    {
        nRole = m_pAccShape->getAccessibleRole();
    }
    return nRole;
}

OUString AccessibleChartShape::getAccessibleDescription()
{
    OUString aDescription;
    if ( m_pAccShape.is() )
    {
        aDescription = m_pAccShape->getAccessibleDescription();
    }
    return aDescription;
}

OUString AccessibleChartShape::getAccessibleName()
{
    OUString aName;
    if ( m_pAccShape.is() )
    {
        aName = m_pAccShape->getAccessibleName();
    }
    return aName;
}

// ________ XAccessibleComponent ________
sal_Bool AccessibleChartShape::containsPoint( const awt::Point& aPoint )
{
    bool bReturn = false;
    if ( m_pAccShape.is() )
    {
        bReturn = m_pAccShape->containsPoint( aPoint );
    }
    return bReturn;
}

Reference< XAccessible > AccessibleChartShape::getAccessibleAtPoint( const awt::Point& aPoint )
{
    Reference< XAccessible > xResult;
    if ( m_pAccShape.is() )
    {
        xResult.set( m_pAccShape->getAccessibleAtPoint( aPoint ) );
    }
    return xResult;
}

awt::Rectangle AccessibleChartShape::getBounds()
{
    awt::Rectangle aBounds;
    if ( m_pAccShape.is() )
    {
        aBounds = m_pAccShape->getBounds();
    }
    return aBounds;
}

awt::Point AccessibleChartShape::getLocation()
{
    awt::Point aLocation;
    if ( m_pAccShape.is() )
    {
        aLocation = m_pAccShape->getLocation();
    }
    return aLocation;
}

awt::Point AccessibleChartShape::getLocationOnScreen()
{
    awt::Point aLocation;
    if ( m_pAccShape.is() )
    {
        aLocation = m_pAccShape->getLocationOnScreen();
    }
    return aLocation;
}

awt::Size AccessibleChartShape::getSize()
{
    awt::Size aSize;
    if ( m_pAccShape.is() )
    {
        aSize = m_pAccShape->getSize();
    }
    return aSize;
}

void AccessibleChartShape::grabFocus()
{
    return AccessibleBase::grabFocus();
}

sal_Int32 AccessibleChartShape::getForeground()
{
    sal_Int32 nColor(0);
    if ( m_pAccShape.is() )
    {
        nColor = m_pAccShape->getForeground();
    }
    return nColor;
}

sal_Int32 AccessibleChartShape::getBackground()
{
    sal_Int32 nColor(0);
    if ( m_pAccShape.is() )
    {
        nColor = m_pAccShape->getBackground();
    }
    return nColor;
}

// ________ XAccessibleExtendedComponent ________
Reference< awt::XFont > AccessibleChartShape::getFont()
{
    Reference< awt::XFont > xFont;
    if ( m_pAccShape.is() )
    {
        xFont.set( m_pAccShape->getFont() );
    }
    return xFont;
}

OUString AccessibleChartShape::getTitledBorderText()
{
    OUString aText;
    if ( m_pAccShape.is() )
    {
        aText = m_pAccShape->getTitledBorderText();
    }
    return aText;
}

OUString AccessibleChartShape::getToolTipText()
{
    OUString aText;
    if ( m_pAccShape.is() )
    {
        aText = m_pAccShape->getToolTipText();
    }
    return aText;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
