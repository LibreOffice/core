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

#ifndef OOX_CORE_FRAGMENTHANDLER2_HXX
#define OOX_CORE_FRAGMENTHANDLER2_HXX

#include "oox/core/contexthandler2.hxx"
#include "oox/core/fragmenthandler.hxx"

namespace oox {
namespace core {

// ============================================================================

class FragmentHandler2 : public FragmentHandler, public ContextHandler2Helper
{
public:
    explicit            FragmentHandler2(
                            XmlFilterBase& rFilter,
                            const ::rtl::OUString& rFragmentPath,
                            bool bEnableTrimSpace = true );
    virtual             ~FragmentHandler2();

    // resolve ambiguity from base classes
    virtual void SAL_CALL acquire() throw() { FragmentHandler::acquire(); }
    virtual void SAL_CALL release() throw() { FragmentHandler::release(); }

    virtual ContextHandler& queryContextHandler();

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL startFastElement(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL characters( const ::rtl::OUString& rChars )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL endFastElement( sal_Int32 nElement )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException );

    // com.sun.star.xml.sax.XFastDocumentHandler interface --------------------

    virtual void SAL_CALL startDocument()
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL endDocument()
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException );

    // oox.core.ContextHandler interface --------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );
    virtual void        onEndRecord();

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual void        initializeImport();
    virtual void        finalizeImport();
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
