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
#pragma once

#include <map>

#include <ooxml/OOXMLDocument.hxx>
#include <com/sun/star/embed/XRelationshipAccess.hpp>

extern OUString customTarget;
extern OUString embeddingsTarget;

namespace writerfilter::ooxml
{

class OOXMLStreamImpl : public OOXMLStream
{
    void init();

    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::io::XInputStream> mxStorageStream;
    css::uno::Reference<css::embed::XStorage> mxStorage;
    css::uno::Reference<css::embed::XRelationshipAccess> mxRelationshipAccess;
    css::uno::Reference<css::io::XStream> mxDocumentStream;
    css::uno::Reference<css::xml::sax::XFastParser> mxFastParser;
    css::uno::Reference<css::xml::sax::XFastTokenHandler> mxFastTokenHandler;

    StreamType_t mnStreamType;

    OUString msId;
    OUString msPath;
    OUString msTarget;

    /// Cache holding an Id <-> Target map of external relations.
    std::map<OUString, OUString> maIdCache;

    bool lcl_getTarget(const css::uno::Reference<css::embed::XRelationshipAccess>& xRelationshipAccess,
                       StreamType_t nStreamType,
                       const OUString & rId,
                       OUString & rDocumentTarget);
public:
    typedef tools::SvRef<OOXMLStreamImpl> Pointer_t;

    OOXMLStreamImpl
    (OOXMLStreamImpl const & rStream, StreamType_t nType);
    OOXMLStreamImpl
    (css::uno::Reference<css::uno::XComponentContext> const & xContext,
     css::uno::Reference<css::io::XInputStream> const & xStorageStream,
     StreamType_t nType, bool bRepairStorage);
    OOXMLStreamImpl(OOXMLStreamImpl const & rStream, const OUString & rId);

    virtual ~OOXMLStreamImpl() override;

    virtual css::uno::Reference<css::xml::sax::XFastParser> getFastParser() override;
    virtual css::uno::Reference<css::io::XInputStream> getDocumentStream() override;
    virtual css::uno::Reference<css::uno::XComponentContext> getContext() override;
    virtual OUString getTargetForId(const OUString & rId) override;
    virtual const OUString & getTarget() const override;

    virtual css::uno::Reference<css::xml::sax::XFastTokenHandler> getFastTokenHandler() override;

    // Giving access to mxDocumentStream. It is needed by resolving custom xml to get list of customxml's used in document.
    const css::uno::Reference<css::io::XStream>& accessDocumentStream() { return mxDocumentStream;}
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
