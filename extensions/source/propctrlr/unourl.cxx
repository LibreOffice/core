/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unourl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:24:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_UNOURL_HXX
#include "unourl.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;

    //====================================================================
    //= UnoURL
    //====================================================================
    UnoURL::UnoURL( const ::rtl::OUString& _rCompleteURL, const Reference< XMultiServiceFactory >& _rxORB )
    {
        m_aURL.Complete = _rCompleteURL;

        OSL_ENSURE( _rxORB.is(), "UnoURL::UnoURL: invalid ORB!" );
        Reference< XURLTransformer > xTransform;
        try
        {
            if ( _rxORB.is() )
            {
                xTransform = xTransform.query( _rxORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ) ) );
                OSL_ENSURE( xTransform.is(), "UnoURL::UnoURL: could not create an URL transformer!" );
                if ( xTransform.is() )
                    xTransform->parseStrict( m_aURL );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "UnoURL::UnoURL: caught an exception!" );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

