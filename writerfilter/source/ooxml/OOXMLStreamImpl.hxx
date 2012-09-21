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
#ifndef INCLUDED_OOXML_STREAM_IMPL_HXX
#define INCLUDED_OOXML_STREAM_IMPL_HXX

#include <ooxml/OOXMLDocument.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/io/XStream.hpp>

namespace writerfilter {
namespace ooxml
{

using namespace com::sun::star;

class OOXMLStreamImpl : public OOXMLStream
{
    void init();

    uno::Reference<uno::XComponentContext> mxContext;
    uno::Reference<io::XInputStream> mxStorageStream;
    uno::Reference<embed::XStorage> mxStorage;
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    uno::Reference<io::XStream> mxDocumentStream;
    uno::Reference<xml::sax::XFastParser> mxFastParser;
    uno::Reference<xml::sax::XFastTokenHandler> mxFastTokenHandler;

    StreamType_t mnStreamType;

    OUString msId;
    OUString msPath;
    OUString msTarget;

    bool lcl_getTarget(uno::Reference<embed::XRelationshipAccess>
                       xRelationshipAccess,
                       StreamType_t nStreamType,
                       const OUString & rId,
                       OUString & rDocumentTarget);
public:
    typedef boost::shared_ptr<OOXMLStreamImpl> Pointer_t;

    OOXMLStreamImpl
    (OOXMLStreamImpl & rStream, StreamType_t nType);
    OOXMLStreamImpl
    (uno::Reference<uno::XComponentContext> xContext,
     uno::Reference<io::XInputStream> xStorageStream,
     StreamType_t nType, bool bRepairStorage);
    OOXMLStreamImpl(OOXMLStreamImpl & rStream, const OUString & rId);

    virtual ~OOXMLStreamImpl();

    virtual uno::Reference<xml::sax::XParser> getParser();
    virtual uno::Reference<xml::sax::XFastParser> getFastParser();
    virtual uno::Reference<io::XInputStream> getDocumentStream();
    virtual uno::Reference<io::XInputStream> getStorageStream();
    virtual uno::Reference<uno::XComponentContext> getContext();
    virtual OUString getTargetForId(const OUString & rId);
    virtual const OUString & getTarget() const;

    virtual uno::Reference<xml::sax::XFastTokenHandler>
    getFastTokenHandler(uno::Reference<uno::XComponentContext> rContext);

    void setInputStream(uno::Reference<io::XInputStream> rxInputStream);
};
}}
#endif // INCLUDED_OOXML_STREAM_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
