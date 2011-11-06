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



#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <tools/debug.hxx>
#include <usr/ustring.hxx>

#include "result.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

SV_IMPL_PTRARR( XResultListenerArr_Impl, XResultListenerPtr );

//SMART_UNO_IMPLEMENTATION( ScAddInResult, UsrObject );

//------------------------------------------------------------------------

ScAddInResult::ScAddInResult(const String& rStr) :
    aArg( rStr ),
    nTickCount( 0 )
{
    aTimer.SetTimeout( 1000 );
    aTimer.SetTimeoutHdl( LINK( this, ScAddInResult, TimeoutHdl ) );
    aTimer.Start();
}

void ScAddInResult::NewValue()
{
    ++nTickCount;

    uno::Any aAny;
    if ( TRUE /* nTickCount % 4 */ )
    {
        String aRet = aArg;
        aRet += nTickCount;
        rtl::OUString aUStr = StringToOUString( aRet, CHARSET_SYSTEM );
        aAny <<= aUStr;
    }
    // else void

//  sheet::ResultEvent aEvent( (UsrObject*)this, aAny );
    sheet::ResultEvent aEvent( (cppu::OWeakObject*)this, aAny );

    for ( USHORT n=0; n<aListeners.Count(); n++ )
        (*aListeners[n])->modified( aEvent );
}

IMPL_LINK_INLINE_START( ScAddInResult, TimeoutHdl, Timer*, pT )
{
    NewValue();
    pT->Start();
    return 0;
}
IMPL_LINK_INLINE_END( ScAddInResult, TimeoutHdl, Timer*, pT )

ScAddInResult::~ScAddInResult()
{
}

// XVolatileResult

void SAL_CALL ScAddInResult::addResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference<sheet::XResultListener> *pObj = new uno::Reference<sheet::XResultListener>( aListener );
    aListeners.Insert( pObj, aListeners.Count() );

    if ( aListeners.Count() == 1 )
    {
        acquire();                      // one Ref for all listeners

        NewValue(); //! Test
    }
}

void SAL_CALL ScAddInResult::removeResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XResultListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    acquire();

    USHORT nCount = aListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<sheet::XResultListener> *pObj = aListeners[n];
        if ( *pObj == aListener )
        {
            aListeners.DeleteAndDestroy( n );

            if ( aListeners.Count() == 0 )
            {
                nTickCount = 0; //! Test

                release();                  // release listener Ref
            }

            break;
        }
    }

    release();
}

//------------------------------------------------------------------------



