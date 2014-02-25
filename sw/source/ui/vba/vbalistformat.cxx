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
#include "vbalistformat.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <ooo/vba/word/WdListApplyTo.hpp>
#include <ooo/vba/word/WdDefaultListBehavior.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "vbalisttemplate.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaListFormat::SwVbaListFormat( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< text::XTextRange >& xTextRange ) throw ( uno::RuntimeException ) : SwVbaListFormat_BASE( rParent, rContext ), mxTextRange( xTextRange )
{
}

SwVbaListFormat::~SwVbaListFormat()
{
}

void SAL_CALL SwVbaListFormat::ApplyListTemplate( const css::uno::Reference< word::XListTemplate >& ListTemplate, const css::uno::Any& ContinuePreviousList, const css::uno::Any& ApplyTo, const css::uno::Any& DefaultListBehavior ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool bContinuePreviousList = sal_True;
    if( ContinuePreviousList.hasValue() )
        ContinuePreviousList >>= bContinuePreviousList;

    // "applyto" must be current selection
    sal_Int32 bApplyTo = word::WdListApplyTo::wdListApplyToSelection;
    if( ApplyTo.hasValue() )
        ApplyTo >>= bApplyTo;
    if( bApplyTo != word::WdListApplyTo::wdListApplyToSelection )
        throw uno::RuntimeException();

    // default behaviour must be wdWord8ListBehavior
    sal_Int32 nDefaultListBehavior = word::WdDefaultListBehavior::wdWord8ListBehavior;
    if( DefaultListBehavior.hasValue() )
        DefaultListBehavior >>= nDefaultListBehavior;
    if( nDefaultListBehavior != word::WdDefaultListBehavior::wdWord8ListBehavior )
        throw uno::RuntimeException();

    SwVbaListTemplate* pListTemplate = dynamic_cast< SwVbaListTemplate* >( ListTemplate.get() );

    uno::Reference< container::XEnumerationAccess > xEnumAccess( mxTextRange, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    sal_Bool isFirstElement = sal_True;
    while( xEnum->hasMoreElements() )
    {
        uno::Reference< beans::XPropertySet > xProps( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        if( isFirstElement )
        {
            sal_Bool isNumberingRestart = !bContinuePreviousList;
            xProps->setPropertyValue("ParaIsNumberingRestart", uno::makeAny( isNumberingRestart ) );
            if( isNumberingRestart )
            {
                sal_Int16 nStartValue = 1;
                xProps->setPropertyValue("NumberingStartValue", uno::makeAny( nStartValue ) );
            }
            isFirstElement = sal_False;
        }
        else
        {
            xProps->setPropertyValue("ParaIsNumberingRestart", uno::makeAny( sal_False ) );
        }
        pListTemplate->applyListTemplate( xProps );
    }
}

void SAL_CALL SwVbaListFormat::ConvertNumbersToText(  ) throw (css::uno::RuntimeException, std::exception)
{
    throw uno::RuntimeException("Not implemented", uno::Reference< uno::XInterface >() );
}

OUString
SwVbaListFormat::getServiceImplName()
{
    return OUString("SwVbaListFormat");
}

uno::Sequence< OUString >
SwVbaListFormat::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.ListFormat";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
