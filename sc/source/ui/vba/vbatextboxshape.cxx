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

#include "vbatextboxshape.hxx"
#include "vbacharacters.hxx"
#include <com/sun/star/text/XSimpleText.hpp>
#include <vector>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaTextBoxShape::ScVbaTextBoxShape( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& xShape, const uno::Reference< drawing::XShapes >& xShapes, const uno::Reference< frame::XModel >& xModel ) : TextBoxShapeImpl_BASE( uno::Reference< XHelperInterface >(), xContext, xShape, xShapes, xModel, ScVbaShape::getType( xShape )  )
{
    m_xTextRange.set( xShape , uno::UNO_QUERY_THROW );
    m_xModel.set( xModel );
}

OUString SAL_CALL
ScVbaTextBoxShape::getText() throw (css::uno::RuntimeException, std::exception)
{
    return m_xTextRange->getString();
}

void SAL_CALL
ScVbaTextBoxShape::setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception)
{
    m_xTextRange->setString( _text );
}

uno::Reference< excel::XCharacters > SAL_CALL
ScVbaTextBoxShape::characters( const uno::Any& Start, const uno::Any& Length ) throw (uno::RuntimeException, std::exception)
{
    ScDocShell* pDocShell = excel::getDocShell( m_xModel );
    ScDocument* pDoc = pDocShell ? &pDocShell->GetDocument() : nullptr;

    if ( !pDoc )
        throw uno::RuntimeException("Failed to access document from shell" );
    uno::Reference< text::XSimpleText > xSimple( m_xTextRange, uno::UNO_QUERY_THROW );

    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
    return  new ScVbaCharacters( this, mxContext, aPalette, xSimple, Start, Length, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
