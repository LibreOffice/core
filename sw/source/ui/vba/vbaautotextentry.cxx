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
#include "vbaautotextentry.hxx"
#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <tools/diagnose_ex.h>
#include "wordvbahelper.hxx"
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAutoTextEntry::SwVbaAutoTextEntry( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XAutoTextEntry >& xEntry ) :
    SwVbaAutoTextEntry_BASE( rParent, rContext ), mxEntry( xEntry )
{
}

SwVbaAutoTextEntry::~SwVbaAutoTextEntry()
{
}

uno::Reference< word::XRange > SAL_CALL SwVbaAutoTextEntry::Insert( const uno::Reference< word::XRange >& _where, const uno::Any& _richtext )
{
    SwVbaRange* pWhere = dynamic_cast<SwVbaRange*>( _where.get() );
    if( pWhere )
    {
        uno::Reference< text::XTextRange > xTextRange = pWhere->getXTextRange();
        xTextRange->setString( "x" ); // set marker
        uno::Reference< text::XTextRange > xEndMarker = xTextRange->getEnd();
        xEndMarker->setString( "x" ); // set marker
        uno::Reference< text::XText > xText = pWhere->getXText();
        mxEntry->applyTo( xEndMarker->getStart() );
        uno::Reference< text::XTextCursor > xTC = xText->createTextCursorByRange( xTextRange->getStart() );
        xTC->goRight( 1, true );
        xTC->setString( "" ); // remove marker
        // remove the blank paragraph if it is a rich text
        bool bRich = false;
        _richtext >>= bRich;
        if( bRich )
        {
            // check if it is a blank paragraph
            uno::Reference< text::XParagraphCursor > xParaCursor( xTC, uno::UNO_QUERY_THROW );
            if( xParaCursor->isStartOfParagraph() && xParaCursor->isEndOfParagraph() )
            {
                //remove the blank paragraph
                uno::Reference< frame::XModel > xModel( getCurrentWordDoc( mxContext ), uno::UNO_SET_THROW );
                uno::Reference< text::XTextViewCursor > xTVCursor = word::getXTextViewCursor( xModel );
                uno::Reference< text::XTextRange > xCurrentRange( xTC->getEnd(), uno::UNO_SET_THROW );
                xTVCursor->gotoRange( xCurrentRange, false );
                OUString url = ".uno:Delete";
                dispatchRequests( xModel,url );
                xTVCursor->gotoRange( xEndMarker->getEnd(), false );
            }
        }
        xEndMarker->setString( "" ); // remove marker
        xTC = xText->createTextCursorByRange( xEndMarker->getEnd() );
        pWhere->setXTextCursor( xTC );
    }
    return uno::Reference< word::XRange >( pWhere );
}

OUString
SwVbaAutoTextEntry::getServiceImplName()
{
    return "SwVbaAutoTextEntry";
}

uno::Sequence< OUString >
SwVbaAutoTextEntry::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.word.AutoTextEntry"
    };
    return aServiceNames;
}

SwVbaAutoTextEntries::SwVbaAutoTextEntries( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess ) : SwVbaAutoTextEntries_BASE( xParent, xContext, xIndexAccess )
{
}

// XEnumerationAccess
uno::Type
SwVbaAutoTextEntries::getElementType()
{
    return cppu::UnoType<word::XAutoTextEntry>::get();
}
uno::Reference< container::XEnumeration >
SwVbaAutoTextEntries::createEnumeration()
{
    throw uno::RuntimeException("Not implemented" );
}

uno::Any
SwVbaAutoTextEntries::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< text::XAutoTextEntry > xEntry( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XAutoTextEntry >( new SwVbaAutoTextEntry( this, mxContext, xEntry ) ) );
}

OUString
SwVbaAutoTextEntries::getServiceImplName()
{
    return "SwVbaAutoTextEntries";
}

css::uno::Sequence<OUString>
SwVbaAutoTextEntries::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        "ooo.vba.word.AutoTextEntries"
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
