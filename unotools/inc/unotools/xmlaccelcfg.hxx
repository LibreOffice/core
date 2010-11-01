/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_unotools_XMLACCELCFG_HXX
#define INCLUDED_unotools_XMLACCELCFG_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
