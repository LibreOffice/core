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

#include <customuigenerator.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <unotools/viewoptions.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <vcl/tabctrl.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <unotools/confignode.hxx>
#include <comphelper/types.hxx>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;
using namespace css;

CustomUIGenerator::CustomUIGenerator()
{
    getCustomizedUIItem();

}

static OUString lcl_getAppName( vcl::EnumContext::Application eApp )
{
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            return OUString( "Writer" );
            break;
        case vcl::EnumContext::Application::Calc:
            return OUString( "Calc" );
            break;
        case vcl::EnumContext::Application::Impress:
            return OUString( "Impress" );
            break;
        case vcl::EnumContext::Application::Draw:
            return OUString( "Draw" );
            break;
        case vcl::EnumContext::Application::Formula:
            return OUString( "Formula" );
            break;
        default:
            return OUString();
            break;
    }
}

static OUStringBuffer getApplicationName()
{
    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;
    const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
    eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));

    OUString appName(lcl_getAppName( eApp ));
    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    aPath.append( appName );
    return aPath;
}

void CustomUIGenerator::getCustomizedUIItem()
{
    OUStringBuffer aPath = getApplicationName();
    const utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath.makeStringAndClear(),
                                        false);

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );
    const sal_Int32 nCount( aModeNodeNames.getLength() );

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aModeNode( aModesNode.openNode( aModeNodeNames[nReadIndex] ) );
        const Any aValue = aModeNode.getNodeValue( "UIItemProperties" );
        Sequence<OUString> aValues;
        aValue >>= aValues;
    }
}

void CustomUIGenerator::setCustomizedUIItem( Sequence<OUString> sUIItemProperties , OUString NotebookBarConfig )
{
    OUStringBuffer aPath = getApplicationName();
    const utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath.makeStringAndClear(),
                                        false);
    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode( aModesNode.openNode( NotebookBarConfig ) );

    css::uno::Any aUIItemProperties(makeAny(sUIItemProperties));
    aModeNode.setNodeValue("UIItemProperties", aUIItemProperties);
    aAppNode.commit();
}