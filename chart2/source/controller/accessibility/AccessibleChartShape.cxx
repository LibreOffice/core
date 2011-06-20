/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "AccessibleChartShape.hxx"
#include "ObjectHierarchy.hxx"
#include "ObjectIdentifier.hxx"

#include <toolkit/helper/vclunohelper.hxx>
#include <svx/ShapeTypeHandler.hxx>
#include <svx/AccessibleShape.hxx>
#include <svx/AccessibleShapeInfo.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;


namespace chart
{

AccessibleChartShape::AccessibleChartShape(
        const AccessibleElementInfo& rAccInfo,
        bool bMayHaveChildren, bool bAlwaysTransparent )
    :impl::AccessibleChartShape_Base( rAccInfo, bMayHaveChildren, bAlwaysTransparent )
    ,m_pAccShape( NULL )
{
    if ( rAccInfo.m_aOID.isAdditionalShape() )
    {
        Reference< drawing::XShape > xShape( rAccInfo.m_aOID.getAdditionalShape() );
        Reference< XAccessible > xParent;
        if ( rAccInfo.m_pParent )
        {
            xParent.set( rAccInfo.m_pParent );
        }
        sal_Int32 nIndex = -1;
        if ( rAccInfo.m_spObjectHierarchy )
        {
            nIndex = rAccInfo.m_spObjectHierarchy->getIndexInParent( rAccInfo.m_aOID );
        }
        ::accessibility::AccessibleShapeInfo aShapeInfo( xShape, xParent, nIndex );

        m_aShapeTreeInfo.SetSdrView( rAccInfo.m_pSdrView );
        m_aShapeTreeInfo.SetController( NULL );
        m_aShapeTreeInfo.SetWindow( VCLUnoHelper::GetWindow( rAccInfo.m_xWindow ) );
        m_aShapeTreeInfo.SetViewForwarder( rAccInfo.m_pViewForwarder );

        ::accessibility::ShapeTypeHandler& rShapeHandler = ::accessibility::ShapeTypeHandler::Instance();
        m_pAccShape = rShapeHandler.CreateAccessibleObject( aShapeInfo, m_aShapeTreeInfo );
        if ( m_pAccShape )
        {
            m_pAccShape->acquire();
            m_pAccShape->Init();
        }
    }
}

AccessibleChartShape::~AccessibleChartShape()
{
    OSL_ASSERT( CheckDisposeState( false /* don't throw exceptions */ ) );

    if ( m_pAccShape )
    {
        m_pAccShape->dispose();
        m_pAccShape->release();
    }
}

// ________ XServiceInfo ________
::rtl::OUString AccessibleChartShape::getImplementationName()
    throw (RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AccessibleChartShape" ) );
}

// ________ XAccessibleContext ________
sal_Int32 AccessibleChartShape::getAccessibleChildCount()
    throw (RuntimeException)
{
    sal_Int32 nCount(0);
    if ( m_pAccShape )
    {
        nCount = m_pAccShape->getAccessibleChildCount();
    }
    return nCount;
}

Reference< XAccessible > AccessibleChartShape::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessible > xChild;
    if ( m_pAccShape )
    {
        xChild = m_pAccShape->getAccessibleChild( i );
    }
    return xChild;
}

sal_Int16 AccessibleChartShape::getAccessibleRole()
    throw (RuntimeException)
{
    sal_Int16 nRole(0);
    if ( m_pAccShape )
    {
        nRole = m_pAccShape->getAccessibleRole();
    }
    return nRole;
}

::rtl::OUString AccessibleChartShape::getAccessibleDescription()
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aDescription;
    if ( m_pAccShape )
    {
        aDescription = m_pAccShape->getAccessibleDescription();
    }
    return aDescription;
}

::rtl::OUString AccessibleChartShape::getAccessibleName()
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aName;
    if ( m_pAccShape )
    {
        aName = m_pAccShape->getAccessibleName();
    }
    return aName;
}

// ________ XAccessibleComponent ________
sal_Bool AccessibleChartShape::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    sal_Bool bReturn = sal_False;
    if ( m_pAccShape )
    {
        bReturn = m_pAccShape->containsPoint( aPoint );
    }
    return bReturn;
}

Reference< XAccessible > AccessibleChartShape::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    Reference< XAccessible > xResult;
    if ( m_pAccShape )
    {
        xResult.set( m_pAccShape->getAccessibleAtPoint( aPoint ) );
    }
    return xResult;
}

awt::Rectangle AccessibleChartShape::getBounds()
    throw (uno::RuntimeException)
{
    awt::Rectangle aBounds;
    if ( m_pAccShape )
    {
        aBounds = m_pAccShape->getBounds();
    }
    return aBounds;
}

awt::Point AccessibleChartShape::getLocation()
    throw (uno::RuntimeException)
{
    awt::Point aLocation;
    if ( m_pAccShape )
    {
        aLocation = m_pAccShape->getLocation();
    }
    return aLocation;
}

awt::Point AccessibleChartShape::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    awt::Point aLocation;
    if ( m_pAccShape )
    {
        aLocation = m_pAccShape->getLocationOnScreen();
    }
    return aLocation;
}

awt::Size AccessibleChartShape::getSize()
    throw (uno::RuntimeException)
{
    awt::Size aSize;
    if ( m_pAccShape )
    {
        aSize = m_pAccShape->getSize();
    }
    return aSize;
}

void AccessibleChartShape::grabFocus()
    throw (uno::RuntimeException)
{
    return AccessibleBase::grabFocus();
}

sal_Int32 AccessibleChartShape::getForeground()
    throw (uno::RuntimeException)
{
    sal_Int32 nColor(0);
    if ( m_pAccShape )
    {
        nColor = m_pAccShape->getForeground();
    }
    return nColor;
}

sal_Int32 AccessibleChartShape::getBackground()
    throw (uno::RuntimeException)
{
    sal_Int32 nColor(0);
    if ( m_pAccShape )
    {
        nColor = m_pAccShape->getBackground();
    }
    return nColor;
}

// ________ XAccessibleExtendedComponent ________
Reference< awt::XFont > AccessibleChartShape::getFont()
    throw (uno::RuntimeException)
{
    Reference< awt::XFont > xFont;
    if ( m_pAccShape )
    {
        xFont.set( m_pAccShape->getFont() );
    }
    return xFont;
}

::rtl::OUString AccessibleChartShape::getTitledBorderText()
    throw (uno::RuntimeException)
{
    ::rtl::OUString aText;
    if ( m_pAccShape )
    {
        aText = m_pAccShape->getTitledBorderText();
    }
    return aText;
}

::rtl::OUString AccessibleChartShape::getToolTipText()
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aText;
    if ( m_pAccShape )
    {
        aText = m_pAccShape->getToolTipText();
    }
    return aText;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
