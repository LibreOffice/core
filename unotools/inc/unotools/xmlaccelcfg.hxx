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



#ifndef INCLUDED_unotools_XMLACCELCFG_HXX
#define INCLUDED_unotools_XMLACCELCFG_HXX

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#include <cppuhelper/weak.hxx>

#include <unotools/accelcfg.hxx>

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

#endif // INCLUDED_unotools_XMLACCELCFG_HXX
