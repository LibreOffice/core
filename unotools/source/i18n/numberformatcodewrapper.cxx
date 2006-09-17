/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numberformatcodewrapper.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:25:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/numberformatcodewrapper.hxx>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#define LOCALEDATA_LIBRARYNAME "i18npool"
#define LOCALEDATA_SERVICENAME "com.sun.star.i18n.NumberFormatMapper"

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


NumberFormatCodeWrapper::NumberFormatCodeWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF,
            const lang::Locale& rLocale
            )
        :
        xSMgr( xSF )
{
    setLocale( rLocale );
    if ( xSMgr.is() )
    {
        try
        {
            xNFC = Reference< XNumberFormatCode > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& e )
        {
            (void)e;
            DBG_ERRORFILE( "NumberFormatCodeWrapper ctor: Exception caught!" );
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "NumberFormatCodeWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::comphelper::getComponentInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( LOCALEDATA_LIBRARYNAME ) ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) );
            if ( xI.is() )
            {
                Any x = xI->queryInterface( ::getCppuType((const Reference< XNumberFormatCode >*)0) );
                x >>= xNFC;
            }
        }
        catch ( Exception& e )
        {
            (void)e;
            DBG_ERRORFILE( "getComponentInstance: Exception caught!" );
        }
    }
    DBG_ASSERT( xNFC.is(), "NumberFormatCodeWrapper: no NumberFormatMapper" );
}


NumberFormatCodeWrapper::~NumberFormatCodeWrapper()
{
}


void NumberFormatCodeWrapper::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    aLocale = rLocale;
}


::com::sun::star::i18n::NumberFormatCode
NumberFormatCodeWrapper::getDefault( sal_Int16 formatType, sal_Int16 formatUsage ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getDefault( formatType, formatUsage, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getDefault: Exception caught!" );
    }
    return ::com::sun::star::i18n::NumberFormatCode();
}


::com::sun::star::i18n::NumberFormatCode
NumberFormatCodeWrapper::getFormatCode( sal_Int16 formatIndex ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getFormatCode( formatIndex, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getFormatCode: Exception caught!" );
    }
    return ::com::sun::star::i18n::NumberFormatCode();
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCode( sal_Int16 formatUsage ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getAllFormatCode( formatUsage, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getAllFormatCode: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCodes() const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getAllFormatCodes( aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getAllFormatCodes: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > (0);
}
