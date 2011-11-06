/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

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
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager,
    const uno::Reference< embed::XStorage >& rToolbarStorage )
{
    const char USERDEFTOOLBOX[] = "userdeftoolbox0.xml";
    uno::Reference< lang::XMultiServiceFactory > rSrvMgr( rServiceManager );

    sal_Bool bResult ( sal_False );
    if ( rToolbarStorage.is() && rContainerFactory.is() )
    {
        try
        {
            for ( sal_uInt16 i = 1; i <= 4; i++ )
            {
                rtl::OUStringBuffer aCustomTbxName( 20 );
                aCustomTbxName.appendAscii( USERDEFTOOLBOX );
                aCustomTbxName.setCharAt( 14, aCustomTbxName.charAt( 14 ) + i );

                rtl::OUString aTbxStreamName( aCustomTbxName.makeStringAndClear() );
                uno::Reference< io::XStream > xStream = rToolbarStorage->openStreamElement( aTbxStreamName, embed::ElementModes::READ );
                if ( xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInputStream = xStream->getInputStream();
                    if ( xInputStream.is() )
                    {
                        uno::Reference< container::XIndexContainer > xContainer = rContainerFactory->createSettings();
                        if ( ToolBoxConfiguration::LoadToolBox( rSrvMgr, xInputStream, xContainer ))
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
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
        }
    }

    return bResult;
}

} // namespace framework
