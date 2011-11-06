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

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        endRecord( sal_Int32 nRecId );

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
    virtual void        onEndElement();

    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
    virtual void        onStartRecord( SequenceInputStream& rStrm );
    virtual void        onEndRecord();

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual void        initializeImport();
    virtual void        finalizeImport();
};

// ============================================================================

} // namespace core
} // namespace oox

#endif
