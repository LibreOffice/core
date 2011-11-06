/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
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

sal_Bool getStatusFromAny_Impl( const Any& aAny, ::rtl::OUString& aText, sal_Int32& nNum )
{
    sal_Bool bNumIsSet = sal_False;

    Sequence< Any > aSetList;
    if( ( aAny >>= aSetList ) && aSetList.getLength() )
        for( int ind = 0; ind < aSetList.getLength(); ind++ )
        {
            if( !bNumIsSet && ( aSetList[ind] >>= nNum ) )
                bNumIsSet = sal_True;
            else
                !aText.getLength() && ( aSetList[ind] >>= aText );
        }

    return bNumIsSet;
}

void SAL_CALL ProgressHandlerWrap::push( const Any& Status )
    throw( RuntimeException )
{
    if( !m_xStatusIndicator.is() )
        return;

    ::rtl::OUString aText;
    sal_Int32 nRange;

    if( getStatusFromAny_Impl( Status, aText, nRange ) )
        m_xStatusIndicator->start( aText, nRange );
}

void SAL_CALL ProgressHandlerWrap::update( const Any& Status )
    throw( RuntimeException )
{
    if( !m_xStatusIndicator.is() )
        return;

    ::rtl::OUString aText;
    sal_Int32 nValue;

    if( getStatusFromAny_Impl( Status, aText, nValue ) )
    {
        if( aText.getLength() ) m_xStatusIndicator->setText( aText );
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

