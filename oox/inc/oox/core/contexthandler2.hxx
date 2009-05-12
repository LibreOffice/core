/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: contexthandler2.hxx,v $
 * $Revision: 1.5 $
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
    explicit            ContextHandler2Helper( bool bEnableTrimSpace );
    explicit            ContextHandler2Helper( const ContextHandler2Helper& rParent );
    virtual             ~ContextHandler2Helper();

    virtual void SAL_CALL acquire() throw() = 0;
    virtual void SAL_CALL release() throw() = 0;

    virtual ContextHandler& queryContextHandler() = 0;

    // interface --------------------------------------------------------------

    /** Will be called if a new context can be created for the passed element.

        Usually 'true' should be returned to improve performance by reusing the
        same instance to process several elements. Used by OOXML import only.
     */
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) = 0;

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
    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm ) = 0;

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

    /** Returns true, if the current element is the root element. */
    bool                isRootElement() const;

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
    size_t              mnRootStackSize;    /// Stack size on construction time.
    bool                mbEnableTrimSpace;  /// True = trim whitespace in characters().
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

    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );
    virtual void        onEndRecord();
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

