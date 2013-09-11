/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef OOX_CORE_CONTEXTHANDLER2_HXX
#define OOX_CORE_CONTEXTHANDLER2_HXX

#include <vector>
#include <boost/shared_ptr.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/core/contexthandler.hxx"
#include "oox/dllapi.h"

namespace oox {
namespace core {

// ============================================================================

const sal_Int32 XML_ROOT_CONTEXT    = SAL_MAX_INT32;

// ============================================================================

struct ElementInfo;

/** Helper class that provides a context stack.

    Fragment handlers and context handlers derived from this helper class will
    track the identifiers of the visited elements in a stack. The idea is to
    use the same instance of a fragment handler or context handler to process
    several nested elements in an XML stream. For that, the abstract function
    onCreateContext() has to return 'this' for the passed element.

    Derived classes have to implement the createFastChildContext(),
    startFastElement(), characters(), and endFastElement() functions from the
    com.sun.star.xml.sax.XFastContextHandler interface by simply forwarding
    them to the respective implCreateChildContext(), implStartElement(),
    implCharacters(), and implEndElement() functions of this helper. This is
    implemented already in the classes ContextHandler2 and FragmentHandler2.
    The new abstract functions have to be implemented according to the elements
    to be processed.

    Similarly, for binary import, derived classes have to forward the
    createRecordContext(), startRecord(), and endRecord() functions from the
    ContextHandler class to the implCreateRecordContext(), implStartRecord(),
    and implEndRecord() functions of this helper. Again, this is implemented
    already in the classes ContextHandler2 and FragmentHandler2.
 */
class OOX_DLLPUBLIC ContextHandler2Helper
{
public:
    explicit            ContextHandler2Helper( bool bEnableTrimSpace );
    explicit            ContextHandler2Helper( const ContextHandler2Helper& rParent );
    virtual             ~ContextHandler2Helper();

    // allow instances to be stored in ::rtl::Reference
    virtual void SAL_CALL acquire() throw() = 0;
    virtual void SAL_CALL release() throw() = 0;

    // interface --------------------------------------------------------------

    /** Will be called to create a context handler for the passed element.

        Usually 'this' can be returned to improve performance by reusing the
        same instance to process several elements. Used by OOXML import only.
     */
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) = 0;

    /** Will be called when a new element has been started.

        This function is called at the context handler returned from
        onCreateContext(), or, for root elements of an XML stream, at the
        fragment handler itself.

        The current element identifier can be accessed with getCurrentElement()
        or isCurrentElement(). Used by OOXML import only.
     */
    virtual void        onStartElement( const AttributeList& rAttribs ) = 0;

    /** Will be called before a new child element starts, or if the current
        element is about to be left.

        This helper function collects all text fragments received by the
        characters() function (such as encoded characters which are passed in
        separate calls to the characters() function), and passes the
        concatenated and trimmed string.

        The current element identifier can be accessed with getCurrentElement()
        or isCurrentElement(). Used by OOXML import only.
     */
    virtual void        onCharacters( const OUString& rChars ) = 0;

    /** Will be called when the current element is about to be left.

        The current element identifier can be accessed with getCurrentElement()
        or isCurrentElement(). Used by OOXML import only.
     */
    virtual void        onEndElement() = 0;

    /** Will be called to create a context handler for the passed record.

        Usually 'this' can be returned to improve performance by reusing the
        same instance to process several records. Used by BIFF import only.
     */
    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) = 0;

    /** Will be called when a new record block in a binary stream has been
        started.

        The current record identifier can be accessed with getCurrentElement()
        or isCurrentElement(). Used by BIFF import only.
     */
    virtual void        onStartRecord( SequenceInputStream& rStrm ) = 0;

    /** Will be called when the current record block is about to be left.

        The current record identifier can be accessed with getCurrentElement()
        or isCurrentElement(). Used by BIFF import only.
     */
    virtual void        onEndRecord() = 0;

    // helpers ----------------------------------------------------------------

    /** Returns the identifier of the currently processed element. Ignores MCE elements in stack */
    sal_Int32           getCurrentElement() const;

    /** Returns the identifier of the currently processed element - Including MCE root elements */
    sal_Int32           getCurrentElementWithMce() const;

    /** Returns true, if nElement contains the identifier of the currently
        processed element. */
    bool         isCurrentElement( sal_Int32 nElement ) const
                            { return getCurrentElement() == nElement; }

    /** Returns true, if either nElement1 or nElement2 contain the identifier
        of the currently processed element. */
    bool         isCurrentElement( sal_Int32 nElement1, sal_Int32 nElement2 ) const
                            { return isCurrentElement( nElement1 ) || isCurrentElement( nElement2 ); }

    /** Returns the identifier of the specified parent element. */
    sal_Int32           getParentElement( sal_Int32 nCountBack = 1 ) const;

    /** Returns true, if nElement contains the identifier of the specified
        parent element. */
    sal_Int32    isParentElement( sal_Int32 nElement, sal_Int32 nCountBack = 1 ) const
                            { return getParentElement( nCountBack ) == nElement; }

    /** Returns true, if the element currently processed is the root element of
        the context or fragment handler. */
    bool                isRootElement() const;

    // implementation ---------------------------------------------------------

protected:
    /** Must be called from createFastChildContext() in derived classes. */
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler >
                        implCreateChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs );

    /** Must be called from startFastElement() in derived classes. */
    void                implStartElement(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs );

    /** Must be called from characters() in derived classes. */
    void                implCharacters( const OUString& rChars );

    /** Must be called from endFastElement() in derived classes. */
    void                implEndElement( sal_Int32 nElement );

    /** Must be called from createRecordContext() in derived classes. */
    ContextHandlerRef   implCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

    /** Must be called from startRecord() in derived classes. */
    void                implStartRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );

    /** Must be called from endRecord() in derived classes. */
    void                implEndRecord( sal_Int32 nRecId );

private:
    ContextHandler2Helper& operator=( const ContextHandler2Helper& );

    ElementInfo&        pushElementInfo( sal_Int32 nElement );
    void                popElementInfo();
    void                processCollectedChars();

private:
    typedef ::std::vector< ElementInfo >        ContextStack;
    typedef ::boost::shared_ptr< ContextStack > ContextStackRef;

    ContextStackRef     mxContextStack;     ///< Stack of all processed elements.
    size_t              mnRootStackSize;    ///< Stack size on construction time.
    bool                mbEnableTrimSpace;  ///< True = trim whitespace in characters().
};

// ============================================================================

class OOX_DLLPUBLIC ContextHandler2 : public ContextHandler, public ContextHandler2Helper
{
public:
    explicit            ContextHandler2( ContextHandler2Helper& rParent );
    virtual             ~ContextHandler2();

    // resolve ambiguity from base classes
    virtual void SAL_CALL acquire() throw() { ContextHandler::acquire(); }
    virtual void SAL_CALL release() throw() { ContextHandler::release(); }

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException ) SAL_FINAL;

    virtual void SAL_CALL startFastElement(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException ) SAL_FINAL;

    virtual void SAL_CALL characters( const OUString& rChars )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException ) SAL_FINAL;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement )
                            throw(  ::com::sun::star::xml::sax::SAXException,
                                    ::com::sun::star::uno::RuntimeException ) SAL_FINAL;

    // oox.core.ContextHandler interface --------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onCharacters( const OUString& rChars );
    virtual void        onEndElement();

    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        onStartRecord( SequenceInputStream& rStrm );
    virtual void        onEndRecord();
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
