/*************************************************************************
 *
 *  $RCSfile: xmlaccelcfg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 10:04:38 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INCLUDED_SVTOOLS_XMLACCELCFG_HXX
#define INCLUDED_SVTOOLS_XMLACCELCFG_HXX

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <accelcfg.hxx>

class OReadAccelatorDocumentHandler : public ::com::sun::star::xml::sax::XDocumentHandler,
                                      public ::cppu::OWeakObject
{
    public:
        OReadAccelatorDocumentHandler( SvtAcceleratorItemList& aNewAcceleratorItemList ) :
            m_nElementDepth( 0 ),
            m_bAcceleratorMode( sal_False ),
            m_bItemCloseExpected( sal_False ),
            m_xLocator( 0 ),
            m_aReadAcceleratorList( aNewAcceleratorItemList ) {}
        virtual ~OReadAccelatorDocumentHandler() {}

        // XInterface
        virtual void SAL_CALL acquire() throw()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw()
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException );

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL endDocument(void)
            throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL startElement(
            const ::rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > &xAttribs )
            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL endElement(const ::rtl::OUString& aName) throw
            ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL characters(const ::rtl::OUString& aChars)
            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString& aWhitespaces)
            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setDocumentLocator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > &xLocator)
            throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    private:
        ::rtl::OUString getErrorLineString();

        int                     m_nElementDepth;
        sal_Bool                m_bAcceleratorMode;
        sal_Bool                m_bItemCloseExpected;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >    m_xLocator;
        SvtAcceleratorItemList& m_aReadAcceleratorList;
};


class OWriteAccelatorDocumentHandler
{
    public:
        OWriteAccelatorDocumentHandler(
            const SvtAcceleratorItemList& aWriteAcceleratorList,
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > );
        virtual ~OWriteAccelatorDocumentHandler();

        void WriteAcceleratorDocument() throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    private:
        void WriteAcceleratorItem( const SvtAcceleratorConfigItem& aAcceleratorItem ) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > m_xEmptyList;
        ::rtl::OUString                                         m_aAttributeType;
        const SvtAcceleratorItemList&                           m_aWriteAcceleratorList;
};

#endif // INCLUDED_SVTOOLS_XMLACCELCFG_HXX
