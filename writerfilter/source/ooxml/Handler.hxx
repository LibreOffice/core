/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Handler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:02:49 $
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
