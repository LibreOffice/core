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
#include "vbabookmark.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include "vbarange.hxx"
#include "wordvbahelper.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaBookmark::SwVbaBookmark( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    const css::uno::Reference< frame::XModel >& rModel, const rtl::OUString& rName ) throw ( css::uno::RuntimeException ) :
    SwVbaBookmark_BASE( rParent, rContext ), mxModel( rModel ), maName( rName ), mbValid( sal_True )
{
    uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( mxModel, uno::UNO_QUERY_THROW );
    mxBookmark.set( xBookmarksSupplier->getBookmarks()->getByName( maName ), uno::UNO_QUERY_THROW );
}

SwVbaBookmark::~SwVbaBookmark()
{
}

void SwVbaBookmark::checkVality() throw ( uno::RuntimeException )
{
    if( !mbValid )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("The bookmark is not valid" ) ), uno::Reference< uno::XInterface >() );
}

void SAL_CALL SwVbaBookmark::Delete() throw ( uno::RuntimeException )
{
    checkVality();
    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    xTextDocument->getText()->removeTextContent( mxBookmark );
    mbValid = sal_False;
}

void SAL_CALL SwVbaBookmark::Select() throw ( uno::RuntimeException )
{
    checkVality();
    uno::Reference< view::XSelectionSupplier > xSelectSupp( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::makeAny( mxBookmark ) );
}

rtl::OUString SAL_CALL SwVbaBookmark::getName() throw ( uno::RuntimeException )
{
    return maName;
}

void SAL_CALL SwVbaBookmark::setName( const rtl::OUString& _name ) throw ( uno::RuntimeException )
{
    uno::Reference< container::XNamed > xNamed( mxBookmark, uno::UNO_QUERY_THROW );
    xNamed->setName( _name );
}

uno::Any SAL_CALL SwVbaBookmark::Range() throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextContent > xTextContent( mxBookmark, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTextRange( xTextContent->getAnchor(), uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XRange>(  new SwVbaRange( this, mxContext, xTextDocument, xTextRange->getStart(), xTextRange->getEnd(), xTextRange->getText() ) ) );
}

rtl::OUString&
SwVbaBookmark::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaBookmark") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaBookmark::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Bookmark" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
