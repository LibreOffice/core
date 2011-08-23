/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*****************************************************************************
* Change History
* <<Date>> <<Name of editor>> <<Description>>
*	2005/2		draft code for XML handler of chart xml.
****************************************************************************/

/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/

#ifndef XFCHARTXMLHANDLER_HXX_
#define XFCHARTXMLHANDLER_HXX_

#include <cppuhelper/weak.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <so3/svstor.hxx>
#include "ixfstream.hxx"
#include "ixfattrlist.hxx"
#include <rtl/ustring.hxx>
using namespace rtl;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

/**
* @brief
* Chart xml handler
*/

class XFChartXMLHandler: public XDocumentHandler,public ::cppu::OWeakObject
{
public:
    XFChartXMLHandler(IXFStream* pOutputStream);

    // XInterface
    virtual void SAL_CALL acquire() throw()
    { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw()
    { OWeakObject::release(); }
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL startDocument(  )
        throw (SAXException, RuntimeException) {}

        virtual void SAL_CALL endDocument(  )
            throw (SAXException, RuntimeException) {}

    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const Reference< XAttributeList >& xAttribs )
        throw (SAXException, RuntimeException);

    virtual void SAL_CALL endElement( const ::rtl::OUString& aName )
        throw (SAXException, RuntimeException);

    virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
        throw (SAXException, RuntimeException);

    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
        throw (SAXException, RuntimeException){}

    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
        throw (SAXException, RuntimeException){}

    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator )
        throw (SAXException, RuntimeException){}
protected:
    IXFStream* m_pOutputStream;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
