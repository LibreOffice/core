/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlhelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:22:58 $
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

#ifndef _XMLHELPER_HXX
#define _XMLHELPER_HXX

#include <sal/types.h>


namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace container { class XNameContainer; }
    namespace xml { namespace dom { class XDocumentBuilder; } }
} } }


bool isValidQName( const rtl::OUString& sName,
                   const com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>& xNamespaces );

bool isValidPrefixName( const rtl::OUString& sName,
                        const com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>& xNamespaces );

com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocumentBuilder> getDocumentBuilder();


#endif
