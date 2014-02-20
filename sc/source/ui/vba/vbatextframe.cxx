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
#include <vbahelper/helperdecl.hxx>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/text/XText.hpp>
#include "vbatextframe.hxx"
#include "vbacharacters.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaTextFrame::ScVbaTextFrame( uno::Sequence< uno::Any> const & args, uno::Reference< uno::XComponentContext> const & xContext ) throw ( lang::IllegalArgumentException ) :  ScVbaTextFrame_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext, getXSomethingFromArgs< drawing::XShape >( args, 1, false ) )
{
}

// Methods
uno::Any SAL_CALL
ScVbaTextFrame::Characters() throw (uno::RuntimeException)
{
    uno::Reference< text::XSimpleText > xSimpleText( m_xShape, uno::UNO_QUERY_THROW );
    ScVbaPalette aPalette( SfxObjectShell::Current() );
    uno::Any aStart( sal_Int32( 1 ) );
    uno::Any aLength(sal_Int32( -1 ) );
    return uno::makeAny( uno::Reference< ov::excel::XCharacters >( new ScVbaCharacters( this, mxContext, aPalette, xSimpleText, aStart, aLength, true ) ) );
}

OUString
ScVbaTextFrame::getServiceImplName()
{
    return OUString("ScVbaTextFrame");
}

uno::Sequence< OUString >
ScVbaTextFrame::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.TextFrame";
    }
    return aServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
ScVbaTextFrame_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new ScVbaTextFrame(arguments, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
