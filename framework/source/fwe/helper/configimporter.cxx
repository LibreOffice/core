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


#include <framework/configimporter.hxx>
#include <framework/toolboxconfiguration.hxx>
#include <com/sun/star/embed/ElementModes.hpp>

#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace framework
{

sal_Bool UIConfigurationImporterOOo1x::ImportCustomToolbars(
    const uno::Reference< ui::XUIConfigurationManager >& rContainerFactory,
    uno::Sequence< uno::Reference< container::XIndexContainer > >& rSeqContainer,
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Reference< embed::XStorage >& rToolbarStorage )
{
    const char USERDEFTOOLBOX[] = "userdeftoolbox0.xml";

    sal_Bool bResult ( sal_False );
    if ( rToolbarStorage.is() && rContainerFactory.is() )
    {
        try
        {
            for ( sal_uInt16 i = 1; i <= 4; i++ )
            {
                rtl::OUStringBuffer aCustomTbxName( 20 );
                aCustomTbxName.appendAscii( USERDEFTOOLBOX );
                aCustomTbxName[14] = aCustomTbxName[14] + i;

                rtl::OUString aTbxStreamName( aCustomTbxName.makeStringAndClear() );
                uno::Reference< io::XStream > xStream = rToolbarStorage->openStreamElement( aTbxStreamName, embed::ElementModes::READ );
                if ( xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInputStream = xStream->getInputStream();
                    if ( xInputStream.is() )
                    {
                        uno::Reference< container::XIndexContainer > xContainer = rContainerFactory->createSettings();
                        if ( ToolBoxConfiguration::LoadToolBox( rxContext, xInputStream, xContainer ))
                        {
                            sal_uInt32 nIndex = rSeqContainer.getLength();
                            rSeqContainer.realloc( nIndex+1 );
                            rSeqContainer[nIndex] = xContainer;
                            bResult = sal_True;
                        }
                    }
                }
            }
        }
        catch ( const uno::RuntimeException& )
        {
            throw;
        }
        catch ( const uno::Exception& )
        {
        }
    }

    return bResult;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
