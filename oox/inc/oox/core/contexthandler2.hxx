/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contexthandler2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:20 $
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

#ifndef OOX_CORE_CONTEXTHANDLER2_HXX
#define OOX_CORE_CONTEXTHANDLER2_HXX

#include <vector>
#include <boost/shared_ptr.hpp>
#include "tokens.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/contexthandler.hxx"
#include "oox/core/namespaces.hxx"

namespace oox {
namespace core {

// Constants ==================================================================

const sal_Int32 XML_ROOT_CONTEXT            = SAL_MAX_INT32;

// ============================================================================

class ContextHandler2Helper;

/** Wrapper for a context handler object.

    This wrapper is used by the ContextHandler2 class to simplify returning
    either the own context handler instance, a new context handler, or null to
    indicate an unsupported context. The implicit constructors allow to return
    either a C++ bool value (true = own context instance, false = null) or a
    new context handler (any class derived from ContextHandler, can be null
    too) in the implementation of the virtual
    ContextHandler2::onCreateContext() or
    ContextHandler2::onCreateRecordContext() functions. This wrapper takes
    ownership of the passed context handler internally, thus it is possible to
    simply pass a new heap-allocated object.
 */
class ContextWrapper
{
public:
    inline /*implicit*/ ContextWrapper( bool bThis ) : mbThis( bThis ) {}
    inline /*implicit*/ ContextWrapper( ContextHandler* pContext ) : mxContext( pContext ), mbThis( false ) {}

    ContextHandler*     getContextHandler( ContextHandler2Helper& rThis ) const;

private:
    ContextHandlerRef   mxContext;
    bool                mbThis;
};

// ============================================================================

struct ContextInfo;

/** Helper class that provides a context identifier stack.

    Fragment handlers and context handlers derived from this helper class will
    track the identifiers of the current context in a stack. The idea is to use
    the same instance of a fragment handler or context handler to process
    several nested elements in an XML stream. For that, the abstract function
    onCreateContext() has to return 'true' for the passed element.

    Derived classes have to implement the createFastChildContext(),
    startFastElement(), characters(), and endFastElement() functions from the
    com.sun.star.xml.sax.XFastContextHandler interface by simply forwarding
    them to the respective implCreateChildContext(), implStartCurrentContext(),
    implCharacters(), and implEndCurrentContext() functions of this helper. The
    new abstract functions have to be implemented according to the elements to
    be processed.

    Similarly, for binary import, derived classes have to forward the
    createRecordContext(), startRecord(), and endRecord() functions from the
    ContextHandler class to the implCreateRecordContext(), implStartRecord(),
    and implEndRecord() functions of this helper.
 */
class ContextHandler2Helper
{
public:
    explicit            ContextHandler2Helper();
    virtual             ~ContextHandler2Helper();

    virtual void SAL_CALL acquire() throw() = 0;
    virtual void SAL_CALL release() throw() = 0;

    virtual ContextHandler& queryContextHandler() = 0;

    // interface --------------------------------------------------------------

    /** Will be called if a new context can be created for the passed element.

        Usually 'true' should be returned to improve performance by reusing the
        same instance to process several elements. Used by OOXML import only.
     */
    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) = 0;

    /** Will be called if a new context element has been started.

        The current element identifier can be accessed by using
        getCurrentContext() or isCurrentContext(). Used by OOXML import only.
     */
    virtual void        onStartElement( const AttributeList& rAttribs ) = 0;

    /** Will be called if the current context element is about to be left.

        The current element identifier can be accessed by using
        getCurrentContext() or isCurrentContext(). Used by OOXML import only.

        @param rChars  The characters collected in this element.
     */
    virtual void        onEndElement( const ::rtl::OUString& rChars ) = 0;

    /** Will be called if a new context can be created for the passed element.

        Usually 'true' should be returned to improve performance by reusing the
        same instance to process several elements. Used by OOBIN import only.
     */
    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm ) = 0;

    /** Will be called if a new record in a binary stream has been started.

        The current record identifier can be accessed by using
        getCurrentContext() or isCurrentContext(). Used by OOBIN import only.
     */
    virtual void        onStartRecord( RecordInputStream& rStrm ) = 0;

    /** Will be called if the current context record is about to be left.

        The current record identifier can be accessed by using
        getCurrentContext() or isCurrentContext(). Used by OOBIN import only.
     */
    virtual void        onEndRecord() = 0;

    // helpers ----------------------------------------------------------------

    /** Returns the element identifier of the current topmost context. */
    sal_Int32           getCurrentElement() const;

    /** Returns true, if nElement contains the current topmost context. */
    inline bool         isCurrentElement( sal_Int32 nElement ) const
                            { return getCurrentElement() == nElement; }

    /** Returns true, if either nElement1 or nElement2 contain the current topmost context. */
    inline bool         isCurrentElement( sal_Int32 nElement1, sal_Int32 nElement2 ) const
                            { return isCurrentElement( nElement1 ) || isCurrentElement( nElement2 ); }

    /** Returns the element identifier of the specified parent context. */
    sal_Int32           getPreviousElement( sal_Int32 nCountBack = 1 ) const;

    /** Returns the element identifier of the specified parent context. */
    inline sal_Int32    isPreviousElement( sal_Int32 nElement, sal_Int32 nCountBack = 1 ) const
                            { return getPreviousElement( nCountBack ) == nElement; }

    // implementation ---------------------------------------------------------

protected:
    /** Must be called from createFastChildContext() in derived classes. */
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler >
                        implCreateChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs );

    /** Must be called from startFastElement() in derived classes. */
    void                implStartCurrentContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs );

    /** Must be called from characters() in derived classes. */
    void                implCharacters( const ::rtl::OUString& rChars );

    /** Must be called from endFastElement() in derived classes. */
    void                implEndCurrentContext( sal_Int32 nElement );

    /** Must be called from createRecordContext() in derived classes. */
    ContextHandlerRef   implCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

    /** Must be called from startRecord() in derived classes. */
    void                implStartRecord( sal_Int32 nRecId, RecordInputStream& rStrm );

    /** Must be called from endRecord() in derived classes. */
    void                implEndRecord( sal_Int32 nRecId );

private:
    ContextHandler2Helper& operator=( const ContextHandler2Helper& );

    ContextInfo&        pushContextInfo( sal_Int32 nElement );
    void                popContextInfo();
    void                appendCollectedChars();

private:
    typedef ::std::vector< ContextInfo >        ContextStack;
    typedef ::boost::shared_ptr< ContextStack > ContextStackRef;

    ContextStackRef     mxContextStack;     /// Stack of processed contexts.
};

// ============================================================================

class ContextHandler2 : public ContextHandler, public ContextHandler2Helper
{
public:
    explicit            ContextHandler2( ContextHandler2Helper& rParent );
    virtual             ~ContextHandler2();

    // resolve ambiguity from base classes
    virtual void SAL_CALL acquire() throw() { ContextHandler::acquire(); }
    virtual void SAL_CALL release() throw() { ContextHandler::release(); }

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

    // oox.core.ContextHandler interface --------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );
    virtual void        onEndRecord();
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

