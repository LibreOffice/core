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
#ifndef INCLUDED_HANDLER_HXX
#define INCLUDED_HANDLER_HXX

#include <resourcemodel/WW8ResourceModel.hxx>
#include "OOXMLFastContextHandler.hxx"

namespace writerfilter {
namespace ooxml
{
class OOXMLFootnoteHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;

public:
    OOXMLFootnoteHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFootnoteHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLEndnoteHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
public:
    OOXMLEndnoteHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLEndnoteHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLFooterHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
    OUString msStreamId;
    sal_Int32 mnType;
public:
    OOXMLFooterHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLFooterHandler();
    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLHeaderHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
    OUString msStreamId;
    sal_Int32 mnType;
public:
    OOXMLHeaderHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLHeaderHandler();
    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLCommentHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
    OUString msStreamId;
public:
    OOXMLCommentHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLCommentHandler();
    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLOLEHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;

public:
    OOXMLOLEHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLOLEHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLEmbeddedFontHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;

public:
    OOXMLEmbeddedFontHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLEmbeddedFontHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLBreakHandler : public Properties
{
    sal_Int32 mnType, mnClear;
    Stream & mrStream;
public:
    OOXMLBreakHandler(Stream & rStream);
    virtual ~OOXMLBreakHandler();
    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLPictureHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
public:
    OOXMLPictureHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLPictureHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};

class OOXMLHyperlinkHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
    OUString mFieldCode;
    OUString mURL;

public:
    OOXMLHyperlinkHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLHyperlinkHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);
};


}}
#endif // INCLUDED_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
