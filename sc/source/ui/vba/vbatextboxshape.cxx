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

rtl::OUString SAL_CALL
ScVbaTextBoxShape::getText() throw (css::uno::RuntimeException)
{
    return m_xTextRange->getString();
}

void SAL_CALL
ScVbaTextBoxShape::setText( const rtl::OUString& _text ) throw (css::uno::RuntimeException)
{
    m_xTextRange->setString( _text );
}

uno::Reference< excel::XCharacters > SAL_CALL
ScVbaTextBoxShape::characters( const uno::Any& Start, const uno::Any& Length ) throw (uno::RuntimeException)
{
    ScDocShell* pDocShell = excel::getDocShell( m_xModel );
    ScDocument* pDoc = pDocShell ? pDocShell->GetDocument() : NULL;

    if ( !pDoc )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access document from shell" ) ), uno::Reference< uno::XInterface >() );
    uno::Reference< text::XSimpleText > xSimple( m_xTextRange, uno::UNO_QUERY_THROW );

    ScVbaPalette aPalette( pDoc->GetDocumentShell() );
    return  new ScVbaCharacters( this, mxContext, aPalette, xSimple, Start, Length, sal_True );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
