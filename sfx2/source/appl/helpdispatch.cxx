/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpdispatch.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:39:04 $
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

#include "helpdispatch.hxx"
#include "sfxuno.hxx"
#include "newhelp.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

// class HelpInterceptor_Impl --------------------------------------------

HelpDispatch_Impl::HelpDispatch_Impl( HelpInterceptor_Impl& _rInterceptor,
                                      const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::frame::XDispatch >& _xDisp ) :

    m_rInterceptor  ( _rInterceptor ),
    m_xRealDispatch ( _xDisp )

{
}

// -----------------------------------------------------------------------

HelpDispatch_Impl::~HelpDispatch_Impl()
{
}

// -----------------------------------------------------------------------
// XDispatch

void SAL_CALL HelpDispatch_Impl::dispatch(

    const URL& aURL, const Sequence< PropertyValue >& aArgs ) throw( RuntimeException )

{
    DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );

    // search for a keyword (dispatch from the basic ide)
    sal_Bool bHasKeyword = sal_False;
    String sKeyword;
    const PropertyValue* pBegin = aArgs.getConstArray();
    const PropertyValue* pEnd   = pBegin + aArgs.getLength();
    for ( ; pBegin != pEnd; ++pBegin )
    {
        if ( 0 == ( *pBegin ).Name.compareToAscii( "HelpKeyword" ) )
        {
            rtl::OUString sHelpKeyword;
            if ( ( ( *pBegin ).Value >>= sHelpKeyword ) && sHelpKeyword.getLength() > 0 )
            {
                sKeyword = String( sHelpKeyword );
                bHasKeyword = ( sKeyword.Len() > 0 );
                break;
            }
        }
    }

    // if a keyword was found, then open it
    SfxHelpWindow_Impl* pHelpWin = m_rInterceptor.GetHelpWindow();
    DBG_ASSERT( pHelpWin, "invalid HelpWindow" );
    if ( bHasKeyword )
    {
        pHelpWin->OpenKeyword( sKeyword );
        return;
    }

    pHelpWin->loadHelpContent(aURL.Complete);
}

// -----------------------------------------------------------------------

void SAL_CALL HelpDispatch_Impl::addStatusListener(

    const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
    DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );
    m_xRealDispatch->addStatusListener( xControl, aURL );
}

// -----------------------------------------------------------------------

void SAL_CALL HelpDispatch_Impl::removeStatusListener(

    const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
    DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );
    m_xRealDispatch->removeStatusListener( xControl, aURL );
}

