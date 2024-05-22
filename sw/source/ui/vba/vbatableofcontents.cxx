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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/word/WdTabLeader.hpp>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaTableOfContents::SwVbaTableOfContents( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< text::XTextDocument >  xDoc, uno::Reference< text::XDocumentIndex >  xDocumentIndex ) :
    SwVbaTableOfContents_BASE( rParent, rContext ), mxTextDocument(std::move( xDoc )), mxDocumentIndex(std::move( xDocumentIndex ))
{
    mxTocProps.set( mxDocumentIndex, uno::UNO_QUERY_THROW );
}

SwVbaTableOfContents::~SwVbaTableOfContents()
{
}

::sal_Int32 SAL_CALL SwVbaTableOfContents::getLowerHeadingLevel()
{
    sal_Int16 nLevel = 0;
    mxTocProps->getPropertyValue(u"Level"_ustr) >>= nLevel;
    return nLevel;
}

void SAL_CALL SwVbaTableOfContents::setLowerHeadingLevel( ::sal_Int32 _lowerheadinglevel )
{
    mxTocProps->setPropertyValue(u"Level"_ustr, uno::Any( sal_Int8( _lowerheadinglevel ) ) );
}

::sal_Int32 SAL_CALL SwVbaTableOfContents::getTabLeader()
{
    // not support in Writer
    return word::WdTabLeader::wdTabLeaderDots;
}

void SAL_CALL SwVbaTableOfContents::setTabLeader( ::sal_Int32 /*_tableader*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL SwVbaTableOfContents::getUseFields()
{
    bool bUseFields = false;
    mxTocProps->getPropertyValue(u"CreateFromMarks"_ustr) >>= bUseFields;
    return bUseFields;
}

void SAL_CALL SwVbaTableOfContents::setUseFields( sal_Bool _useFields )
{
    mxTocProps->setPropertyValue(u"CreateFromMarks"_ustr, uno::Any( _useFields ) );
}

sal_Bool SAL_CALL SwVbaTableOfContents::getUseOutlineLevels()
{
    bool bUseOutlineLevels = false;
    mxTocProps->getPropertyValue(u"CreateFromOutline"_ustr) >>= bUseOutlineLevels;
    return bUseOutlineLevels;
}

void SAL_CALL SwVbaTableOfContents::setUseOutlineLevels( sal_Bool _useOutlineLevels )
{
    mxTocProps->setPropertyValue(u"CreateFromOutline"_ustr, uno::Any( _useOutlineLevels ) );
}

void SAL_CALL SwVbaTableOfContents::Delete(  )
{
    uno::Reference< text::XTextContent > xTextContent( mxDocumentIndex, uno::UNO_QUERY_THROW );
    mxTextDocument->getText()->removeTextContent( xTextContent );
}

void SAL_CALL SwVbaTableOfContents::Update(  )
{
    mxDocumentIndex->update();
}

OUString
SwVbaTableOfContents::getServiceImplName()
{
    return u"SwVbaTableOfContents"_ustr;
}

uno::Sequence< OUString >
SwVbaTableOfContents::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.TableOfContents"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
