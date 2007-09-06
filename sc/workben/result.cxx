/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: result.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 14:22:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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



