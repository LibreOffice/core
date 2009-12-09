/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
