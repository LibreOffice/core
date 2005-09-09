/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlstreamio.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:28:35 $
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

#ifndef _XMLSTREAMIO_XMLSECIMPL_HXX_
#define _XMLSTREAMIO_XMLSECIMPL_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_CRYPTO_XURIBINDING_HPP_
#include <com/sun/star/xml/crypto/XUriBinding.hpp>
#endif

int xmlEnableStreamInputCallbacks( void ) ;
void xmlDisableStreamInputCallbacks( void ) ;

int xmlRegisterStreamInputCallbacks(
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XUriBinding >& aUriBinding
) ;

int xmlUnregisterStreamInputCallbacks( void ) ;

#endif //_XMLSTREAMIO_XMLSECIMPL_HXX_

