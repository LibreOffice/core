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

SwVbaAutoTextEntry::SwVbaAutoTextEntry( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XAutoTextEntry >& xEntry ) throw ( uno::RuntimeException ) :
    SwVbaAutoTextEntry_BASE( rParent, rContext ), mxEntry( xEntry )
{
}

SwVbaAutoTextEntry::~SwVbaAutoTextEntry()
{
}

uno::Reference< word::XRange > SAL_CALL SwVbaAutoTextEntry::Insert( const uno::Reference< word::XRange >& _where, const uno::Any& _richtext ) throw ( uno::RuntimeException )
{
    SwVbaRange* pWhere = dynamic_cast<SwVbaRange*>( _where.get() );
    if( pWhere )
    {
        uno::Reference< text::XTextRange > xTextRange = pWhere->getXTextRange();
        xTextRange->setString( rtl::OUString("x") ); // set marker
        uno::Reference< text::XTextRange > xEndMarker = xTextRange->getEnd();
        xEndMarker->setString( rtl::OUString("x") ); // set marker
        uno::Reference< text::XText > xText = pWhere->getXText();
        mxEntry->applyTo( xEndMarker->getStart() );
        uno::Reference< text::XTextCursor > xTC = xText->createTextCursorByRange( xTextRange->getStart() );
        xTC->goRight( 1, sal_True );
        xTC->setString( rtl::OUString("") ); // remove marker
        // remove the blank paragraph if it is a rich text
        sal_Bool bRich = sal_False;
        _richtext >>= bRich;
        if( bRich )
        {
            // check if it is a blank paragraph
            uno::Reference< text::XParagraphCursor > xParaCursor( xTC, uno::UNO_QUERY_THROW );
            if( xParaCursor->isStartOfParagraph() && xParaCursor->isEndOfParagraph() )
            {
                //remove the blank paragraph
                uno::Reference< frame::XModel > xModel( getCurrentWordDoc( mxContext ), uno::UNO_QUERY_THROW );
                uno::Reference< text::XTextViewCursor > xTVCursor = word::getXTextViewCursor( xModel );
                uno::Reference< text::XTextRange > xCurrentRange( xTC->getEnd(), uno::UNO_QUERY_THROW );
                xTVCursor->gotoRange( xCurrentRange, sal_False );
                rtl::OUString url = rtl::OUString( ".uno:Delete");
                dispatchRequests( xModel,url );
                xTVCursor->gotoRange( xEndMarker->getEnd(), sal_False );
            }
        }
        xEndMarker->setString( rtl::OUString("") ); // remove marker
        xTC = xText->createTextCursorByRange( xEndMarker->getEnd() );
        pWhere->setXTextCursor( xTC );
    }
    return uno::Reference< word::XRange >( pWhere );
}

rtl::OUString
SwVbaAutoTextEntry::getServiceImplName()
{
    return rtl::OUString("SwVbaAutoTextEntry");
}

uno::Sequence< rtl::OUString >
SwVbaAutoTextEntry::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.word.AutoTextEntry" );
    }
    return aServiceNames;
}


SwVbaAutoTextEntries::SwVbaAutoTextEntries( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess ) throw (uno::RuntimeException) : SwVbaAutoTextEntries_BASE( xParent, xContext, xIndexAccess ), mxAutoTextEntryAccess( xIndexAccess )
{
}

// XEnumerationAccess
uno::Type
SwVbaAutoTextEntries::getElementType() throw (uno::RuntimeException)
{
    return word::XAutoTextEntry::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaAutoTextEntries::createEnumeration() throw (uno::RuntimeException)
{
    throw uno::RuntimeException( rtl::OUString("Not implemented"), uno::Reference< uno::XInterface >() );
}

uno::Any
SwVbaAutoTextEntries::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< text::XAutoTextEntry > xEntry( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< word::XAutoTextEntry >( new SwVbaAutoTextEntry( this, mxContext, xEntry ) ) );
}

rtl::OUString
SwVbaAutoTextEntries::getServiceImplName()
{
    return rtl::OUString("SwVbaAutoTextEntries");
}

css::uno::Sequence<rtl::OUString>
SwVbaAutoTextEntries::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString("ooo.vba.word.AutoTextEntries");
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
