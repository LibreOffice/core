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

#include <framework/configimporter.hxx>
#include <framework/toolboxconfiguration.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>

#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace framework
{

bool UIConfigurationImporterOOo1x::ImportCustomToolbars(
    const uno::Reference< ui::XUIConfigurationManager2 >& rContainerFactory,
    std::vector< uno::Reference< container::XIndexContainer > >& rSeqContainer,
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Reference< embed::XStorage >& rToolbarStorage )
{
    bool bResult ( false );
    if ( rToolbarStorage.is() && rContainerFactory.is() )
    {
        try
        {
            for ( sal_uInt16 i = 1; i <= 4; i++ )
            {
                OUString aTbxStreamName = "userdeftoolbox" + OUString::number(i) + ".xml";
                uno::Reference< io::XStream > xStream = rToolbarStorage->openStreamElement( aTbxStreamName, embed::ElementModes::READ );
                if ( xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInputStream = xStream->getInputStream();
                    if ( xInputStream.is() )
                    {
                        uno::Reference< container::XIndexContainer > xContainer = rContainerFactory->createSettings();
                        if ( ToolBoxConfiguration::LoadToolBox( rxContext, xInputStream, xContainer ))
                        {
                            rSeqContainer.push_back( xContainer );
                            bResult = true;
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
