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
    const css::uno::Reference< frame::XModel >& rModel, const OUString& rBookmarkName ) :
    SwVbaBookmark_BASE( rParent, rContext ), mxModel( rModel ), maBookmarkName( rBookmarkName ), mbValid( true )
{
    uno::Reference< text::XBookmarksSupplier > xBookmarksSupplier( mxModel, uno::UNO_QUERY_THROW );
    mxBookmark.set( xBookmarksSupplier->getBookmarks()->getByName( maBookmarkName ), uno::UNO_QUERY_THROW );
}

SwVbaBookmark::~SwVbaBookmark()
{
}

void SwVbaBookmark::checkVality()
{
    if( !mbValid )
        throw uno::RuntimeException("The bookmark is not valid" );
}

void SAL_CALL SwVbaBookmark::Delete()
{
    checkVality();
    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    xTextDocument->getText()->removeTextContent( mxBookmark );
    mbValid = false;
}

void SAL_CALL SwVbaBookmark::Select()
{
    checkVality();
    uno::Reference< view::XSelectionSupplier > xSelectSupp( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
    xSelectSupp->select( uno::makeAny( mxBookmark ) );
}

OUString SAL_CALL SwVbaBookmark::getName()
{
    return maBookmarkName;
}

void SAL_CALL SwVbaBookmark::setName( const OUString& _name )
{
    uno::Reference< container::XNamed > xNamed( mxBookmark, uno::UNO_QUERY_THROW );
    xNamed->setName( _name );
}

uno::Any SAL_CALL SwVbaBookmark::Range()
{
    uno::Reference< text::XTextContent > xTextContent( mxBookmark, uno::UNO_SET_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTextRange( xTextContent->getAnchor(), uno::UNO_SET_THROW );
    return uno::makeAny( uno::Reference< word::XRange>(  new SwVbaRange( this, mxContext, xTextDocument, xTextRange->getStart(), xTextRange->getEnd(), xTextRange->getText() ) ) );
}

OUString
SwVbaBookmark::getServiceImplName()
{
    return "SwVbaBookmark";
}

uno::Sequence< OUString >
SwVbaBookmark::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.Bookmark"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
