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
#include "vbaheaderfooter.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <ooo/vba/word/WdHeaderFooterIndex.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include "vbarange.hxx"
#include <vbahelper/vbashapes.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaHeaderFooter::SwVbaHeaderFooter( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& rProps, bool isHeader, sal_Int32 index ) throw ( uno::RuntimeException ) : SwVbaHeaderFooter_BASE( rParent, rContext ), mxModel( xModel ), mxPageStyleProps( rProps ), mbHeader( isHeader ), mnIndex( index )
{
}

sal_Bool SAL_CALL SwVbaHeaderFooter::getIsHeader() throw (uno::RuntimeException, std::exception)
{
    return mbHeader;
}

sal_Bool SAL_CALL SwVbaHeaderFooter::getLinkToPrevious() throw (uno::RuntimeException, std::exception)
{
    // seems always false
    return sal_False;
}

void SAL_CALL SwVbaHeaderFooter::setLinkToPrevious( sal_Bool /*_linktoprevious*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

uno::Reference< word::XRange > SAL_CALL SwVbaHeaderFooter::getRange() throw (uno::RuntimeException, std::exception)
{
    OUString sPropsNameText;
    if( mbHeader )
    {
        sPropsNameText = "HeaderText";
    }
    else
    {
        sPropsNameText = "FooterText";
    }
    if( mnIndex == word::WdHeaderFooterIndex::wdHeaderFooterEvenPages )
    {
        sPropsNameText = sPropsNameText.concat( "Left" );
    }

    uno::Reference< text::XText > xText( mxPageStyleProps->getPropertyValue( sPropsNameText ), uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xDocument( mxModel, uno::UNO_QUERY_THROW );
    return uno::Reference< word::XRange >( new SwVbaRange( this, mxContext, xDocument, xText->getStart(), xText->getEnd(), xText ) );
}

uno::Any SAL_CALL
SwVbaHeaderFooter::Shapes( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    // #FIXME: only get the shapes in the current header/footer
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( mxModel, uno::UNO_QUERY_THROW );
    //uno::Reference< drawing::XShapes > xShapes( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xCol( new ScVbaShapes( this, mxContext, xIndexAccess, mxModel ) );
    if ( index.hasValue() )
        return xCol->Item( index, uno::Any() );
    return uno::makeAny( xCol );
}

OUString
SwVbaHeaderFooter::getServiceImplName()
{
    return OUString("SwVbaHeaderFooter");
}

uno::Sequence< OUString >
SwVbaHeaderFooter::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Pane";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
