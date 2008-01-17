/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fragmenthandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:44 $
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
#include "oox/core/recordcontext.hxx"
#include "oox/core/relations.hxx"
#include "oox/core/xmlfilterbase.hxx"

namespace oox {
namespace core {

class RecordParser;

// ============================================================================

class FragmentHandler : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastDocumentHandler >, public RecordContext
{
public:
    explicit            FragmentHandler( const XmlFilterRef& rxFilter, const ::rtl::OUString& rFragmentPath );
    explicit            FragmentHandler( const XmlFilterRef& rxFilter, const ::rtl::OUString& rFragmentPath, RelationsRef xRelations );

    /** Returns the filter instance. */
    inline const XmlFilterRef& getFilter() const { return mxFilter; }
    /** Returns the full fragment path containing these relations. */
    inline const Relations& getRelations() const { return *mxRelations; }
    /** Returns the full fragment path containing these relations. */
    inline const ::rtl::OUString& getFragmentPath() const { return maFragmentPath; }

    /** Returns the full fragment path for the passed relative target. */
    ::rtl::OUString     getFragmentPathFromTarget( const ::rtl::OUString& rTarget ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    ::rtl::OUString     getFragmentPathFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the full fragment path for the passed type. */
    ::rtl::OUString     getFragmentPathFromType( const ::rtl::OUString& rType ) const;

    /** Sets the record parser used to import binary streams. */
    void                setRecordParser( RecordParser& rParser );

    /** Returns access to the record parser used to import binary streams.

        Needed for direct manipulation of the parser context stack via
        RecordParser::pushContext() and RecordParser::popContext().

        Must not be called while loading XML fragments.
     */
    RecordParser&       getRecordParser();

    // resolve ambiguity from base classes
    virtual void SAL_CALL acquire() throw() { ::cppu::OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() { ::cppu::OWeakObject::release(); }

    // com.sun.star.xml.sax.XFastDocumentHandler interface --------------------

    virtual void SAL_CALL startDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

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

private:
    XmlFilterRef        mxFilter;
    RelationsRef        mxRelations;
    ::rtl::OUString     maFragmentPath;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > mxLocator;
    RecordParser*       mpParser;           /// Pointer to record parser for binary import.
};

typedef ::rtl::Reference< FragmentHandler > FragmentHandlerRef;

// ============================================================================

} // namespace core
} // namespace oox

#endif // OOX_CORE_FRAGMENTHANDLER_HXX

