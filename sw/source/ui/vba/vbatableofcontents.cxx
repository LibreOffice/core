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

::sal_Int32 SAL_CALL SwVbaTableOfContents::getLowerHeadingLevel() throw (uno::RuntimeException, std::exception)
{
    sal_Int16 nLevel = 0;
    mxTocProps->getPropertyValue("Level") >>= nLevel;
    return nLevel;
}

void SAL_CALL SwVbaTableOfContents::setLowerHeadingLevel( ::sal_Int32 _lowerheadinglevel ) throw (uno::RuntimeException, std::exception)
{
    mxTocProps->setPropertyValue("Level", uno::makeAny( sal_Int8( _lowerheadinglevel ) ) );
}

::sal_Int32 SAL_CALL SwVbaTableOfContents::getTabLeader() throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
    return word::WdTabLeader::wdTabLeaderDots;
}

void SAL_CALL SwVbaTableOfContents::setTabLeader( ::sal_Int32 /*_tableader*/ ) throw (uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaTableOfContents::getUseFields() throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool bUseFields = sal_False;
    mxTocProps->getPropertyValue("CreateFromMarks") >>= bUseFields;
    return bUseFields;
}

void SAL_CALL SwVbaTableOfContents::setUseFields( sal_Bool _useFields ) throw (css::uno::RuntimeException, std::exception)
{
    mxTocProps->setPropertyValue("CreateFromMarks", uno::makeAny( _useFields ) );
}

sal_Bool SAL_CALL SwVbaTableOfContents::getUseOutlineLevels() throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool bUseOutlineLevels = sal_False;
    mxTocProps->getPropertyValue("CreateFromOutline") >>= bUseOutlineLevels;
    return bUseOutlineLevels;
}

void SAL_CALL SwVbaTableOfContents::setUseOutlineLevels( sal_Bool _useOutlineLevels ) throw (css::uno::RuntimeException, std::exception)
{
    mxTocProps->setPropertyValue("CreateFromOutline", uno::makeAny( _useOutlineLevels ) );
}

void SAL_CALL SwVbaTableOfContents::Delete(  ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< text::XTextContent > xTextContent( mxDocumentIndex, uno::UNO_QUERY_THROW );
    mxTextDocument->getText()->removeTextContent( xTextContent );
}

void SAL_CALL SwVbaTableOfContents::Update(  ) throw (uno::RuntimeException, std::exception)
{
    mxDocumentIndex->update();
}

OUString
SwVbaTableOfContents::getServiceImplName()
{
    return OUString("SwVbaTableOfContents");
}

uno::Sequence< OUString >
SwVbaTableOfContents::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.TableOfContents";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
