/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    ::rtl::OUString msStreamId;
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
    ::rtl::OUString msStreamId;
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
    ::rtl::OUString msStreamId;
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

class OOXMLBreakHandler : public Properties
{
    OOXMLFastContextHandler * mpFastContext;
    sal_Int32 mnType, mnClear;
    Stream & mrStream;
public:
    OOXMLBreakHandler(Stream & rStream, OOXMLFastContextHandler * pContext);
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
    ::rtl::OUString mFieldCode;
    ::rtl::OUString mURL;

public:
    OOXMLHyperlinkHandler(OOXMLFastContextHandler * pContext);
    virtual ~OOXMLHyperlinkHandler();

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);    
};


}}
#endif // INCLUDED_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
