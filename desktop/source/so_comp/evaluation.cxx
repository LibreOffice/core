/*************************************************************************
 *
 *  $RCSfile: evaluation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 13:52:48 $
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

#include "evaluation.hxx"

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#include "../app/desktop.hrc"


using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;

static SOEvaluation*    pSOEval=0;
static const char*      pSupportedServices[] =
{
    "com.sun.star.beans.XExactName",
    "com.sun.star.beans.XMaterialHolder",
    "com.sun.star.lang.XComponent",
    "com.sun.star.lang.XServiceInfo",
};

OUString SOEvaluation::GetImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.desktop.Evaluation" ));
}

Sequence< OUString > SOEvaluation::GetSupportedServiceNames()
{
    sal_Int32 nSize = sizeof( pSupportedServices ) / sizeof( const char *);
    Sequence< OUString > aResult( nSize );

    for( sal_Int32 i = 0; i < nSize; i++ )
        aResult[i] = OUString::createFromAscii( pSupportedServices[i] );
    return aResult;
}

Reference< XInterface >  SAL_CALL SOEvaluation_CreateInstance( const Reference< XMultiServiceFactory >& rSMgr )
{
    static osl::Mutex   aMutex;

    if ( pSOEval == 0 )
    {
        osl::MutexGuard guard( aMutex );
        if ( pSOEval == 0 )
            return (XComponent*) ( new SOEvaluation( rSMgr ) );
    }

    return (XComponent*)0;
}

SOEvaluation::SOEvaluation( const Reference< XMultiServiceFactory >& xFactory ) :
    m_aListeners( m_aMutex ),
    m_xServiceManager( xFactory )
{
}

SOEvaluation::~SOEvaluation()
{
}

// XComponent
void SAL_CALL SOEvaluation::dispose() throw ( RuntimeException )
{
    EventObject aObject;
    aObject.Source = (XComponent*)this;
    m_aListeners.disposeAndClear( aObject );
}

void SAL_CALL SOEvaluation::addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException )
{
    m_aListeners.addInterface( aListener );
}

void SAL_CALL SOEvaluation::removeEventListener( const Reference< XEventListener > & aListener ) throw ( RuntimeException )
{
    m_aListeners.removeInterface( aListener );
}

// XExactName
rtl::OUString SAL_CALL SOEvaluation::getExactName( const rtl::OUString& rApproximateName ) throw ( RuntimeException )
{
    // get the tabreg service for an evaluation version
    // without this service office shouldn't run at all
    OUString aTitle = rApproximateName;
    OUString aEval;
    sal_Bool bExpired = sal_True;
    Reference < XMaterialHolder > xHolder( m_xServiceManager->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.tab.tabreg" ) ) ), UNO_QUERY );
    if ( xHolder.is() )
    {
        // get a sequence of strings for the defined locales
        // a registered version doesn't provide data
        bExpired = sal_False;
        Any aData = xHolder->getMaterial();
        Sequence < NamedValue > aSeq;
        if ( aData >>= aSeq )
        {
            // this is an evaluation version, because it provides "material"
            bExpired = sal_True;

            // determine current locale
            ::rtl::OUString aLocale;
            ::rtl::OUString aTmp;
            Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
            aRet >>= aLocale;

            sal_Int32 nCount = aSeq.getLength();
            if ( nCount )
            {
                // first entry is for expiry
                const NamedValue& rValue = aSeq[0];
                if ( rValue.Name.equalsAscii("expired") )
                    rValue.Value >>= bExpired;
            }

            // find string for matching locale
            sal_Int32 n;
            for ( n=0; n<nCount; n++ )
            {
                const NamedValue& rValue = aSeq[n];
                if ( rValue.Name == aLocale )
                {
                    rValue.Value >>= aTmp;
                    aEval = aTmp;
                    break;
                }
            }

            if ( n == nCount )
            {
                // try matching only first part of locale, if tab service provides it
                ::rtl::OUString aShortLocale;
                sal_Int32 nPos = aLocale.indexOf('_');
                if ( nPos > 0 )
                {
                    aShortLocale = aLocale.copy( 0, nPos );
                    for ( n=0; n<nCount; n++ )
                    {
                        const NamedValue& rValue = aSeq[n];
                        if ( rValue.Name == aShortLocale )
                        {
                            rValue.Value >>= aTmp;
                            aEval = aTmp;
                            break;
                        }
                    }
                }
            }

            if ( n == nCount )
            {
                // current locale is unknown for service, use default english
                sal_Int32 nCount = aSeq.getLength();
                for ( n=0; n<nCount; n++ )
                {
                    const NamedValue& rValue = aSeq[n];
                    if ( rValue.Name.equalsAscii("en") )
                    {
                        rValue.Value >>= aTmp;
                        aEval = aTmp;
                        break;
                    }
                }

                if ( n == nCount )
                    // strange version, no english string
                    bExpired = sal_True;
            }
        }
    }

    if ( aEval.getLength() )
    {
        OUStringBuffer aBuf( aTitle.getLength() + aEval.getLength() + 1 );

        aBuf.append( aTitle );
        const sal_Unicode* pStr = aTitle.getStr();
        if ( pStr[ aTitle.getLength()-1 ] != (sal_Unicode)' ' )
            aBuf.appendAscii( " " );
        aBuf.append( aEval );
        aTitle = aBuf.makeStringAndClear();
    }

    if ( bExpired )
    {
        // get destop resource manager
        ResMgr* pResMgr = ResMgr::CreateResMgr(
                OString("dkt")+OString::valueOf((long int)SUPD));
        InfoBox* pBox;
        if(pResMgr != NULL){
            pBox = new InfoBox( NULL, ResId(INFOBOX_EXPIRED, pResMgr));
            String aText(ResId(STR_TITLE_EXPIRED, pResMgr));
            pBox->SetText(aText);
        } else {
            pBox = new InfoBox( NULL, aTitle);
        }

        pBox->Execute();

        delete pBox;
        delete pResMgr;

        throw RuntimeException();
    }

    return aTitle;
}

// XMaterialHolder
Any SAL_CALL SOEvaluation::getMaterial() throw( RuntimeException )
{
    // Time bomb implementation. Return empty Any to do nothing or
    // provide a com::sun::star::util::Date with the time bomb date.
    Any a;

#ifdef TIMEBOMB
    // Code for extracting/providing time bomb date!
    int nDay   = TIMEBOMB % 100;
    int nMonth = ( TIMEBOMB % 10000 ) / 100;
    int nYear  = TIMEBOMB / 10000;
    com::sun::star::util::Date  aDate( nDay, nMonth, nYear );
    a <<= aDate;
#endif
    return a;
}

// XServiceInfo
::rtl::OUString SAL_CALL SOEvaluation::getImplementationName()
throw ( RuntimeException )
{
    return SOEvaluation::GetImplementationName();
}

sal_Bool SAL_CALL SOEvaluation::supportsService( const ::rtl::OUString& rServiceName )
throw ( RuntimeException )
{
    sal_Int32 nSize = sizeof( pSupportedServices ) / sizeof( const char *);

    for( sal_Int32 i = 0; i < nSize; i++ )
        if ( rServiceName.equalsAscii( pSupportedServices[i] ))
            return sal_True;
    return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL SOEvaluation::getSupportedServiceNames()
throw ( RuntimeException )
{
    return SOEvaluation::GetSupportedServiceNames();
}

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(   const   sal_Char**          ppEnvironmentTypeName   ,
                                                                uno_Environment**   ppEnvironment           )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL component_writeInfo(  void*   pServiceManager , void*   pRegistryKey    )
{
    Reference< XMultiServiceFactory >  xMan( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
    Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) ) ;

    // Eigentliche Implementierung und ihre Services registrieren
    ::rtl::OUString aTempStr;

    ::rtl::OUString aImpl( RTL_CONSTASCII_USTRINGPARAM("/") );
    aImpl += SOEvaluation::GetImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    Reference< XRegistryKey > xNewKey = xKey->createKey( aImpl );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.office.Evaluation") );

    return sal_True;
}

void* SAL_CALL component_getFactory(    const   sal_Char*   pImplementationName ,
                                                void*       pServiceManager     ,
                                                void*       pRegistryKey        )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory >   xFactory                                             ;
        Reference< XMultiServiceFactory >    xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if ( SOEvaluation::GetImplementationName().compareToAscii( pImplementationName ) == COMPARE_EQUAL )
        {
            xFactory = Reference< XSingleServiceFactory >( cppu::createSingleFactory( xServiceManager, SOEvaluation::GetImplementationName(),
                                                                SOEvaluation_CreateInstance, SOEvaluation::GetSupportedServiceNames() ) );
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"
