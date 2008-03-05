/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexthandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:05 $
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

#ifndef OOX_CORE_CONTEXTHANDLER_HXX
#define OOX_CORE_CONTEXTHANDLER_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XLocator; } }
} } }

namespace oox { class RecordInputStream; }

namespace oox {
namespace core {

class XmlFilterBase;
class FragmentHandler;
class Relations;

// ============================================================================

class ContextHandler;
typedef ::rtl::Reference< ContextHandler > ContextHandlerRef;

struct FragmentBaseData;
typedef ::boost::shared_ptr< FragmentBaseData > FragmentBaseDataRef;

typedef ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastContextHandler > ContextHandlerImplBase;

class ContextHandler : public ContextHandlerImplBase
{
public:
    explicit            ContextHandler( ContextHandler& rParent );
    virtual             ~ContextHandler();

    /** Returns the filter instance. */
    XmlFilterBase&      getFilter() const;
    /** Returns the parent context handler of this context handler. */
    ContextHandler*     getParentHandler() const;
    /** Returns the relations of the current fragment. */
    const Relations&    getRelations() const;
    /** Returns the full path of the current fragment. */
    const ::rtl::OUString& getFragmentPath() const;

    /** Returns the full fragment path for the passed relative target. */
    ::rtl::OUString     getFragmentPathFromTarget( const ::rtl::OUString& rTarget ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    ::rtl::OUString     getFragmentPathFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the full fragment path for the passed type. */
    ::rtl::OUString     getFragmentPathFromType( const ::rtl::OUString& rType ) const;

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

    // record context interface -----------------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

protected:
    /** Helper constructor for the FragmentHandler. */
    explicit            ContextHandler( const FragmentBaseDataRef& rxBaseData );

    void                implSetLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& rxLocator );

private:
    ContextHandler&     operator=( const ContextHandler& );

private:
    FragmentBaseDataRef mxBaseData;         /// Base data of the fragment.
    ContextHandler*     mpParentHandler;    /// Pointer to parent context handler.
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

