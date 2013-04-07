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

#include <unotools/progresshandlerwrap.hxx>

namespace utl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;

ProgressHandlerWrap::ProgressHandlerWrap( ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > xSI )
: m_xStatusIndicator( xSI )
{
}

sal_Bool getStatusFromAny_Impl( const Any& aAny, OUString& aText, sal_Int32& nNum )
{
    sal_Bool bNumIsSet = sal_False;

    Sequence< Any > aSetList;
    if( ( aAny >>= aSetList ) && aSetList.getLength() )
        for( int ind = 0; ind < aSetList.getLength(); ind++ )
        {
            if( !bNumIsSet && ( aSetList[ind] >>= nNum ) )
                bNumIsSet = sal_True;
            else
                aText.isEmpty() && ( aSetList[ind] >>= aText );
        }

    return bNumIsSet;
}

void SAL_CALL ProgressHandlerWrap::push( const Any& Status )
    throw( RuntimeException )
{
    if( !m_xStatusIndicator.is() )
        return;

    OUString aText;
    sal_Int32 nRange;

    if( getStatusFromAny_Impl( Status, aText, nRange ) )
        m_xStatusIndicator->start( aText, nRange );
}

void SAL_CALL ProgressHandlerWrap::update( const Any& Status )
    throw( RuntimeException )
{
    if( !m_xStatusIndicator.is() )
        return;

    OUString aText;
    sal_Int32 nValue;

    if( getStatusFromAny_Impl( Status, aText, nValue ) )
    {
        if( !aText.isEmpty() ) m_xStatusIndicator->setText( aText );
        m_xStatusIndicator->setValue( nValue );
    }
}

void SAL_CALL ProgressHandlerWrap::pop()
        throw( RuntimeException )
{
    if( m_xStatusIndicator.is() )
        m_xStatusIndicator->end();
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
