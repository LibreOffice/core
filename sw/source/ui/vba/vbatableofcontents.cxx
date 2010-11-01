/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbatableofcontents.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <ooo/vba/word/WdTabLeader.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaTableOfContents::SwVbaTableOfContents( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextDocument >& xDoc, const uno::Reference< text::XDocumentIndex >& xDocumentIndex ) throw ( uno::RuntimeException ) :
    SwVbaTableOfContents_BASE( rParent, rContext ), mxTextDocument( xDoc ), mxDocumentIndex( xDocumentIndex )
{
    mxTocProps.set( mxDocumentIndex, uno::UNO_QUERY_THROW );
}


SwVbaTableOfContents::~SwVbaTableOfContents()
{
}

::sal_Int32 SAL_CALL SwVbaTableOfContents::getLowerHeadingLevel() throw (uno::RuntimeException)
{
    sal_Int16 nLevel = 0;
    mxTocProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Level") ) ) >>= nLevel;
    return nLevel;
}

void SAL_CALL SwVbaTableOfContents::setLowerHeadingLevel( ::sal_Int32 _lowerheadinglevel ) throw (uno::RuntimeException)
{
    mxTocProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Level") ), uno::makeAny( sal_Int8( _lowerheadinglevel ) ) );
}

::sal_Int32 SAL_CALL SwVbaTableOfContents::getTabLeader() throw (uno::RuntimeException)
{
    // not support in Writer
    return word::WdTabLeader::wdTabLeaderDots;
}

void SAL_CALL SwVbaTableOfContents::setTabLeader( ::sal_Int32 /*_tableader*/ ) throw (uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL SwVbaTableOfContents::getUseFields() throw (css::uno::RuntimeException)
{
    sal_Bool bUseFields = sal_False;
    mxTocProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CreateFromMarks") ) ) >>= bUseFields;
    return bUseFields;
}

void SAL_CALL SwVbaTableOfContents::setUseFields( ::sal_Bool _useFields ) throw (css::uno::RuntimeException)
{
    mxTocProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CreateFromMarks") ), uno::makeAny( _useFields ) );
}

::sal_Bool SAL_CALL SwVbaTableOfContents::getUseOutlineLevels() throw (css::uno::RuntimeException)
{
    sal_Bool bUseOutlineLevels = sal_False;
    mxTocProps->getPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CreateFromOutline") ) ) >>= bUseOutlineLevels;
    return bUseOutlineLevels;
}

void SAL_CALL SwVbaTableOfContents::setUseOutlineLevels( ::sal_Bool _useOutlineLevels ) throw (css::uno::RuntimeException)
{
    mxTocProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CreateFromOutline") ), uno::makeAny( _useOutlineLevels ) );
}

void SAL_CALL SwVbaTableOfContents::Delete(  ) throw (uno::RuntimeException)
{
    uno::Reference< text::XTextContent > xTextContent( mxDocumentIndex, uno::UNO_QUERY_THROW );
    mxTextDocument->getText()->removeTextContent( xTextContent );
}

void SAL_CALL SwVbaTableOfContents::Update(  ) throw (uno::RuntimeException)
{
    mxDocumentIndex->update();
}

rtl::OUString&
SwVbaTableOfContents::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaTableOfContents") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaTableOfContents::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.TableOfContents" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
