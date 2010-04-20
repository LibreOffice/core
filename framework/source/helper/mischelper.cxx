/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langselectionstatusbarcontroller.cxx,v $
 * $Revision: 1.6.40.1 $
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
#include "precompiled_framework.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <helper/mischelper.hxx>
#include <services.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace framework
{

uno::Reference< linguistic2::XLanguageGuessing > LanguageGuessingHelper::GetGuesser() const
{
    if (!m_xLanguageGuesser.is())
    {
        try
        {
            m_xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                    m_xServiceManager->createInstance(
                        rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                        uno::UNO_QUERY );
        }
        catch (uno::Exception &r)
        {
            (void) r;
            DBG_ASSERT( 0, "failed to get language guessing component" );
        }    
    }
    return m_xLanguageGuesser;
}

::rtl::OUString RetrieveLabelFromCommand( const ::rtl::OUString& aCmdURL
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xServiceFactory
            ,::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >&        _xUICommandLabels
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame
            ,::rtl::OUString& _rModuleIdentifier
            ,sal_Bool& _rIni
            ,const sal_Char* _pName)
{
    ::rtl::OUString aLabel;

    // Retrieve popup menu labels
    if ( !_xUICommandLabels.is() )
    {
        try
        {
            if ( !_rIni )
            {
                _rIni = sal_True;
                Reference< XModuleManager > xModuleManager( _xServiceFactory->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

                try
                {
                    _rModuleIdentifier = xModuleManager->identify( _xFrame );
                }
                catch( Exception& )
                {
                }
            }

            Reference< XNameAccess > xNameAccess( _xServiceFactory->createInstance( SERVICENAME_UICOMMANDDESCRIPTION ), UNO_QUERY );
            if ( xNameAccess.is() )
            {
                xNameAccess->getByName( _rModuleIdentifier ) >>= _xUICommandLabels;
            }
        }
        catch ( Exception& )
        {
        }
    }

    if ( _xUICommandLabels.is() )
    {
        try
        {
            if ( aCmdURL.getLength() > 0 )
            {
                rtl::OUString aStr;
                Sequence< PropertyValue > aPropSeq;
                if ( _xUICommandLabels->getByName( aCmdURL ) >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name.equalsAscii( _pName/*"Label"*/ ))
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }

    return aLabel;
}
    
} // namespace framework

