/*************************************************************************
 *
 *  $RCSfile: unocontrolbase.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTEXTLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#endif

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/property.hxx>

#include <tools/debug.hxx>

//  ----------------------------------------------------
//  class UnoControlBase
//  ----------------------------------------------------

sal_Bool UnoControlBase::ImplHasProperty( const ::rtl::OUString& aPropertyName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();
    return xInfo->hasPropertyByName( aPropertyName );
}

void UnoControlBase::ImplSetPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Bool bUpdateThis )
{
    // Model ggf. schon abgemeldet, aber ein Event schlaegt noch zu...
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !bUpdateThis )
            StartUpdatingModel();
        xPSet->setPropertyValue( aPropertyName, aValue );
        if ( !bUpdateThis )
            EndUpdatingModel();
    }
}

::com::sun::star::uno::Any UnoControlBase::ImplGetPropertyValue( const ::rtl::OUString& aPropertyName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
    return xPSet->getPropertyValue( aPropertyName );
}

sal_Bool UnoControlBase::ImplGetPropertyValue_BOOL( sal_uInt16 nProp )
{
    sal_Bool b = sal_False;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= b;
    }
    return b;
}

sal_Int16 UnoControlBase::ImplGetPropertyValue_INT16( sal_uInt16 nProp )
{
    sal_Int16 n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

sal_uInt16 UnoControlBase::ImplGetPropertyValue_UINT16( sal_uInt16 nProp )
{
    sal_uInt16 n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

sal_Int32 UnoControlBase::ImplGetPropertyValue_INT32( sal_uInt16 nProp )
{
    sal_Int32 n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

sal_uInt32 UnoControlBase::ImplGetPropertyValue_UINT32( sal_uInt16 nProp )
{
    sal_uInt32 n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

double UnoControlBase::ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp )
{
    double n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

::rtl::OUString UnoControlBase::ImplGetPropertyValue_UString( sal_uInt16 nProp )
{
    ::rtl::OUString aStr;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= aStr;
    }
    return aStr;
}

::com::sun::star::awt::Size UnoControlBase::Impl_getMinimumSize()
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getMinimumSize();

        if ( !mxPeer.is() || ( mxPeer != xP ) )
            xP->dispose();
    }
    return aSz;
}

::com::sun::star::awt::Size UnoControlBase::Impl_getPreferredSize()
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getPreferredSize();

        if ( !mxPeer.is() || ( mxPeer != xP ) )
            xP->dispose();
    }
    return aSz;
}

::com::sun::star::awt::Size UnoControlBase::Impl_calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize )
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->calcAdjustedSize( rNewSize );

        if ( !mxPeer.is() || ( mxPeer != xP ) )
            xP->dispose();
    }
    return aSz;
}

::com::sun::star::awt::Size UnoControlBase::Impl_getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getMinimumSize( nCols, nLines );

        if ( !mxPeer.is() || ( mxPeer != xP ) )
            xP->dispose();
    }
    return aSz;
}

void UnoControlBase::Impl_getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            xL->getColumnsAndLines( nCols, nLines );

        if ( !mxPeer.is() || ( mxPeer != xP ) )
            xP->dispose();
    }
}



