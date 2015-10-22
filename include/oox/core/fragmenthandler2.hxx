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

#ifndef INCLUDED_OOX_CORE_FRAGMENTHANDLER2_HXX
#define INCLUDED_OOX_CORE_FRAGMENTHANDLER2_HXX

#include <oox/core/contexthandler2.hxx>
#include <oox/core/fragmenthandler.hxx>
#include <vector>
#include <oox/dllapi.h>

namespace oox {
namespace core {



class OOX_DLLPUBLIC FragmentHandler2 : public FragmentHandler, public ContextHandler2Helper
{
protected:
    enum MCE_STATE
    {
        MCE_UNUSED,
        MCE_STARTED,
        MCE_FOUND_CHOICE
    };
    ::std::vector<MCE_STATE>           aMceState;

    bool                prepareMceContext( sal_Int32 nElement, const AttributeList& rAttribs );


public:
    explicit            FragmentHandler2(
                            XmlFilterBase& rFilter,
                            const OUString& rFragmentPath,
                            bool bEnableTrimSpace = true );
    virtual             ~FragmentHandler2();

    // resolve ambiguity from base classes
    virtual void SAL_CALL acquire() throw() override { FragmentHandler::acquire(); }
    virtual void SAL_CALL release() throw() override { FragmentHandler::release(); }

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const css::uno::Reference< css::xml::sax::XFastAttributeList >& rxAttribs )
                            throw(  css::xml::sax::SAXException,
                                    css::uno::RuntimeException, std::exception ) final override;

    virtual void SAL_CALL startFastElement(
                            sal_Int32 nElement,
                            const css::uno::Reference< css::xml::sax::XFastAttributeList >& rxAttribs )
                            throw(  css::xml::sax::SAXException,
                                    css::uno::RuntimeException, std::exception ) final override;

    virtual void SAL_CALL characters( const OUString& rChars )
                            throw(  css::xml::sax::SAXException,
                                    css::uno::RuntimeException, std::exception ) final override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement )
                            throw(  css::xml::sax::SAXException,
                                    css::uno::RuntimeException, std::exception ) final override;

    // com.sun.star.xml.sax.XFastDocumentHandler interface --------------------

    virtual void SAL_CALL startDocument()
                            throw(  css::xml::sax::SAXException,
                                    css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL endDocument()
                            throw(  css::xml::sax::SAXException,
                                    css::uno::RuntimeException, std::exception ) override;

    // oox.core.ContextHandler interface --------------------------------------

    virtual ContextHandlerRef createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        endRecord( sal_Int32 nRecId ) override;

    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;

    virtual ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;
    virtual void        onEndRecord() override;

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual void        initializeImport();
    virtual void        finalizeImport();
};

typedef ::rtl::Reference< FragmentHandler2 > FragmentHandler2Ref;



} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
