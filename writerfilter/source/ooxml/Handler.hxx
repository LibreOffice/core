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

#include <dmapper/resourcemodel.hxx>
#include "OOXMLFastContextHandler.hxx"

namespace writerfilter::ooxml
{
class OOXMLFootnoteHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;

public:
    explicit OOXMLFootnoteHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLFootnoteHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLEndnoteHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;

public:
    explicit OOXMLEndnoteHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLEndnoteHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLFooterHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;
    OUString msStreamId;
    sal_Int32 mnType;

public:
    explicit OOXMLFooterHandler(OOXMLFastContextHandler* pContext);
    void finalize();
    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLHeaderHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;
    OUString msStreamId;
    sal_Int32 mnType;

public:
    explicit OOXMLHeaderHandler(OOXMLFastContextHandler* pContext);
    void finalize();
    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLCommentHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;

public:
    explicit OOXMLCommentHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLCommentHandler() override;
    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLOLEHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;

public:
    explicit OOXMLOLEHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLOLEHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLEmbeddedFontHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;

public:
    explicit OOXMLEmbeddedFontHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLEmbeddedFontHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLBreakHandler : public Properties
{
    sal_Int32 mnType;
    Stream& mrStream;

public:
    explicit OOXMLBreakHandler(Stream& rStream);
    virtual ~OOXMLBreakHandler() override;
    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLPictureHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;

public:
    explicit OOXMLPictureHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLPictureHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLHyperlinkHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;
    OUString mFieldCode;
    OUString mURL;

public:
    explicit OOXMLHyperlinkHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLHyperlinkHandler() override;
    void writetext();

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

class OOXMLHyperlinkURLHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;
    OUString mURL;

public:
    explicit OOXMLHyperlinkURLHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLHyperlinkURLHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};

/// Looks up the stream name for a '<w:altChunk r:id="..."/>' reference.
class OOXMLAltChunkHandler : public Properties
{
    OOXMLFastContextHandler* mpFastContext;
    OUString m_aStreamName;

public:
    explicit OOXMLAltChunkHandler(OOXMLFastContextHandler* pContext);
    virtual ~OOXMLAltChunkHandler() override;

    virtual void attribute(Id name, Value& val) override;
    virtual void sprm(Sprm& sprm) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
