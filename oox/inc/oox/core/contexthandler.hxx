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



#ifndef OOX_CORE_CONTEXTHANDLER_HXX
#define OOX_CORE_CONTEXTHANDLER_HXX

#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <boost/shared_ptr.hpp>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include "oox/token/namespaces.hxx"
#include "oox/token/tokens.hxx"

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XLocator; } }
} } }

namespace oox { class SequenceInputStream; }

namespace oox {
namespace core {

class XmlFilterBase;
class FragmentHandler;
struct Relation;
class Relations;

// ============================================================================

class ContextHandler;
typedef ::rtl::Reference< ContextHandler > ContextHandlerRef;

struct FragmentBaseData;
typedef ::boost::shared_ptr< FragmentBaseData > FragmentBaseDataRef;

typedef ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastContextHandler > ContextHandler_BASE;

class ContextHandler : public ContextHandler_BASE
{
public:
    explicit            ContextHandler( ContextHandler& rParent );
    virtual             ~ContextHandler();

    /** Returns the filter instance. */
    XmlFilterBase&      getFilter() const;
    /** Returns the relations of the current fragment. */
    const Relations&    getRelations() const;
    /** Returns the full path of the current fragment. */
    const ::rtl::OUString& getFragmentPath() const;

    /** Returns the full fragment path for the target of the passed relation. */
    ::rtl::OUString     getFragmentPathFromRelation( const Relation& rRelation ) const;
    /** Returns the full fragment path for the passed relation identifier. */
    ::rtl::OUString     getFragmentPathFromRelId( const ::rtl::OUString& rRelId ) const;
    /** Returns the full fragment path for the first relation of the passed type. */
    ::rtl::OUString     getFragmentPathFromFirstType( const ::rtl::OUString& rType ) const;

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

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

protected:
    /** Helper constructor for the FragmentHandler. */
    explicit            ContextHandler( const FragmentBaseDataRef& rxBaseData );

    void                implSetLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& rxLocator );

private:
    ContextHandler&     operator=( const ContextHandler& );

private:
    FragmentBaseDataRef mxBaseData;         /// Base data of the fragment.
};

// ============================================================================

} // namespace core
} // namespace oox

#endif
