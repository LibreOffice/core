/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fragmenthandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:57 $
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

#ifndef OOX_CORE_FRAGMENTHANDLER_HXX
#define OOX_CORE_FRAGMENTHANDLER_HXX

#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <cppuhelper/implbase1.hxx>
#include "oox/core/contexthandler.hxx"
#include "oox/core/relations.hxx"

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

