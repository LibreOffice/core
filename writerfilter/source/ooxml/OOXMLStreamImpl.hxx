/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOXMLStreamImpl.hxx,v $
 * $Revision: 1.7 $
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
    uno::Reference<embed::XStorage> mxStorage;
    uno::Reference<io::XInputStream> mxInputStream;
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    uno::Reference<io::XStream> mxDocumentStream;
    uno::Reference<xml::sax::XFastParser> mxFastParser;
    uno::Reference<xml::sax::XFastTokenHandler> mxFastTokenHandler;

    StreamType_t mnStreamType;

    rtl::OUString msId;
    rtl::OUString msPath;

    bool getTarget(uno::Reference<embed::XRelationshipAccess>
                   xRelationshipAccess,
                   StreamType_t nStreamType,
                   const ::rtl::OUString & rId,
                   ::rtl::OUString & rDocumentTarget);
public:
    typedef boost::shared_ptr<OOXMLStreamImpl> Pointer_t;

    OOXMLStreamImpl
    (OOXMLStreamImpl & rStream, StreamType_t nType);
    OOXMLStreamImpl
    (uno::Reference<uno::XComponentContext> xContext,
     uno::Reference<embed::XStorage> xStorage,
     StreamType_t nType);
    OOXMLStreamImpl(OOXMLStreamImpl & rStream, const rtl::OUString & rId);
    OOXMLStreamImpl
    (uno::Reference<uno::XComponentContext> xContext,
     uno::Reference<embed::XStorage> xStorage,
     const rtl::OUString & rId);

    virtual ~OOXMLStreamImpl();

    virtual uno::Reference<xml::sax::XParser> getParser();
    virtual uno::Reference<xml::sax::XFastParser> getFastParser();
    virtual uno::Reference<io::XInputStream> getDocumentStream();
    virtual uno::Reference<io::XInputStream> getInputStream();
    virtual uno::Reference<uno::XComponentContext> getContext();
    ::rtl::OUString getTargetForId(const ::rtl::OUString & rId);
    virtual uno::Reference<xml::sax::XFastTokenHandler>
    getFastTokenHandler(uno::Reference<uno::XComponentContext> rContext);

    void setInputStream(uno::Reference<io::XInputStream> rxInputStream);
};
}}
#endif // INCLUDED_OOXML_STREAM_IMPL_HXX
