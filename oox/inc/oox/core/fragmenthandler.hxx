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

#ifndef OOX_CORE_FRAGMENTHANDLER_HXX
#define OOX_CORE_FRAGMENTHANDLER_HXX

#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <cppuhelper/implbase1.hxx>
#include "oox/core/contexthandler.hxx"
#include "oox/core/relations.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }

namespace oox {
namespace core {

// ============================================================================

/** Base data of a fragment.

    This data is stored in a separate struct to make it accessible in every
    child context handler of the fragment.
 */
struct FragmentBaseData
{
    XmlFilterBase&      mrFilter;
    const ::rtl::OUString maFragmentPath;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >
                        mxLocator;
    RelationsRef        mxRelations;

    explicit            FragmentBaseData(
                            XmlFilterBase& rFilter,
                            const ::rtl::OUString& rFragmentPath,
                            RelationsRef xRelations );
};

// ============================================================================

/** Describes record identifiers used to create contexts in a binary stream.

    If a record is used to start a new context, usually the record identifier
    increased by 1 is used to mark the end of this context, e.g. the Excel
    record SHEETDATA == 0x0091 starts the <sheetData> context, and the record
    SHEETDATA_END == 0x0092 ends this context. But some records are used to
    start a new context, though there is no identifier to end this context,
    e.g. the ROW or EXTROW records. These record identifiers can be marked by
    setting the mnEndRecId member of this struct to -1.
 */
struct RecordInfo
{
    sal_Int32           mnStartRecId;       /// Record identifier for context start.
    sal_Int32           mnEndRecId;         /// Record identifier for context end, -1 = no record.
};

// ============================================================================

typedef ::cppu::ImplInheritanceHelper1< ContextHandler, ::com::sun::star::xml::sax::XFastDocumentHandler > FragmentHandlerImplBase;

class FragmentHandler : public FragmentHandlerImplBase
{
public:
    explicit            FragmentHandler( XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath );
    virtual             ~FragmentHandler();

    /** Returns the com.sun.star.xml.sax.XFastContextHandler interface of this context. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler >
                        getFastContextHandler() { return static_cast< ContextHandler* >( this ); }

    // com.sun.star.xml.sax.XFastDocumentHandler interface --------------------

    virtual void SAL_CALL startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& rxLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    // XML stream handling ----------------------------------------------------

    /** Opens the fragment stream referred by the own fragment path. Derived
        classes may provide specilized stream implementations. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        openFragmentStream() const;

    // binary records ---------------------------------------------------------

    virtual const RecordInfo* getRecordInfos() const;

protected:
    explicit            FragmentHandler( XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, RelationsRef xRelations );
};

typedef ::rtl::Reference< FragmentHandler > FragmentHandlerRef;

// ============================================================================

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
