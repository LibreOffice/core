/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: domimplementation.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 16:43:18 $
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

#ifndef _DOMIMPLEMENTATION_HXX
#define _DOMIMPLEMENTATION_HXX

#include <map>
#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentType.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    class CDOMImplementation : public cppu::WeakImplHelper1< XDOMImplementation >
    {

    public:
        static CDOMImplementation* aDOMImplementation;
        static CDOMImplementation* get();

        /**
        Creates a DOM Document object of the specified type with its document element.
        */
        virtual Reference <XDocument > SAL_CALL createDocument(const OUString& namespaceURI, const OUString& qualifiedName, const Reference< XDocumentType >& doctype)
            throw (RuntimeException);

        /**
        Creates an empty DocumentType node.
        */
        virtual Reference< XDocumentType > SAL_CALL createDocumentType(const OUString& qualifiedName, const OUString& publicId, const OUString& systemId)
            throw (RuntimeException);

        /**
        Test if the DOM implementation implements a specific feature.
        */
        virtual sal_Bool SAL_CALL hasFeature(const OUString& feature, const OUString& ver)
            throw (RuntimeException);
        };
}
#endif
