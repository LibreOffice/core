/*************************************************************************
 *
 *  $RCSfile: msfiltertracer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:03:42 $
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

#ifndef _MS_FILTERTRACER_HXX
#include "msfiltertracer.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/util/logging/LogLevel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHALGORITHMS_HPP_
#include <com/sun/star/util/SearchAlgorithms.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _FILTER_CONFIG_ITEM_HXX_
#include <svtools/FilterConfigItem.hxx>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star;

MSFilterTracer::MSFilterTracer() :
    mpCfgItem( NULL )
{

};

MSFilterTracer::~MSFilterTracer()
{
    delete mpCfgItem;
};

MSFilterTracer& MSFilterTracer::operator=( const MSFilterTracer& rFilterTracer )
{
    delete mpCfgItem, mpCfgItem = new FilterConfigItem( *rFilterTracer.mpCfgItem );
    mbEnabled = rFilterTracer.mbEnabled;
    mxFilterTracer = rFilterTracer.mxFilterTracer;
    mxLogger = rFilterTracer.mxLogger;
    return *this;
}

void MSFilterTracer::StartTracing( const ::rtl::OUString& rConfigPath )
{
    delete mpCfgItem, mpCfgItem = new FilterConfigItem( rConfigPath );

    mbEnabled = mpCfgItem->ReadBool( rtl::OUString::createFromAscii( "Enabled" ), sal_False );
    if ( mbEnabled )
    {
        sal_Int32 nLogLevel = mpCfgItem->ReadInt32( rtl::OUString::createFromAscii( "LogLevel" ), util::logging::LogLevel::ALL );
        ::rtl::OUString aEmptyString;
        ::rtl::OUString aClassFilter( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "ClassFilter" ), aEmptyString ) );
        ::rtl::OUString aMethodFilter( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "MethodFilter" ), aEmptyString ) );
        ::rtl::OUString aMessageFilter( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "MessageFilter" ), aEmptyString ) );
        ::rtl::OUString aURL( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "URL" ), aEmptyString ) );
        util::SearchAlgorithms eSearchAlgorithm = (util::SearchAlgorithms)
            mpCfgItem->ReadInt32( rtl::OUString::createFromAscii( "SearchAlgorithm" ), util::SearchAlgorithms_ABSOLUTE );

        uno::Sequence< uno::Any > aArgument( 1 );
        uno::Sequence< beans::PropertyValue > aPropValues( 5 );
        aPropValues[ 0 ].Name  = ::rtl::OUString::createFromAscii( "LogLevel" );
        aPropValues[ 0 ].Value <<= nLogLevel;
        aPropValues[ 1 ].Name  = ::rtl::OUString::createFromAscii( "ClassFilter" );
        aPropValues[ 1 ].Value <<= aClassFilter;
        aPropValues[ 2 ].Name  = ::rtl::OUString::createFromAscii( "MethodFilter" );
        aPropValues[ 2 ].Value <<= aMethodFilter;
        aPropValues[ 3 ].Name  = ::rtl::OUString::createFromAscii( "MessageFilter" );
        aPropValues[ 3 ].Value <<= aMessageFilter;
        aPropValues[ 4 ].Name  = ::rtl::OUString::createFromAscii( "URL" );
        aPropValues[ 4 ].Value <<= aURL;
        aArgument[ 0 ] <<= aPropValues;

        uno::Reference< lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if ( xMgr.is() )
        {
            mxFilterTracer = xMgr->createInstanceWithArguments( ::rtl::OUString::createFromAscii( "com.sun.star.util.FilterTracer" ), aArgument );
            if ( mxFilterTracer.is() )
            {
                uno::Reference< util::XTextSearch >
                    xTextSearch( mxFilterTracer, uno::UNO_QUERY );
                mxLogger = uno::Reference< util::logging::XLogger >( mxFilterTracer, uno::UNO_QUERY );

                if ( xTextSearch.is() )
                {
                    util::SearchOptions aOptions;
                    aOptions.algorithmType = eSearchAlgorithm;
                    xTextSearch->setOptions( aOptions );
                }
            }
        }
    }
}

void MSFilterTracer::EndTracing()
{
    mbEnabled = sal_False;
    uno::Reference< uno::XInterface > x;
    mxFilterTracer = x;
    delete mpCfgItem, mpCfgItem = NULL;
}

void MSFilterTracer::Trace( const rtl::OUString& rMessage ) const
{
    if ( mbEnabled && mxLogger.is() )
    {
        rtl::OUString aEmpty;
        mxLogger->logp( 0, aEmpty, aEmpty, rMessage );
    }
}

uno::Any MSFilterTracer::GetProperty( const rtl::OUString& rPropName ) const
{
    uno::Any aAny;
    if ( mpCfgItem )
        aAny = mpCfgItem->ReadAny( rPropName, aAny );
    return aAny;
}