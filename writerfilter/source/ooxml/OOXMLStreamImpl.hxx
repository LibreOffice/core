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

    rtl::OUString msId;
    rtl::OUString msPath;
    rtl::OUString msTarget;

    bool lcl_getTarget(uno::Reference<embed::XRelationshipAccess>
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
     uno::Reference<io::XInputStream> xStorageStream,
     StreamType_t nType);
    OOXMLStreamImpl(OOXMLStreamImpl & rStream, const rtl::OUString & rId);

    virtual ~OOXMLStreamImpl();

    virtual uno::Reference<xml::sax::XParser> getParser();
    virtual uno::Reference<xml::sax::XFastParser> getFastParser();
    virtual uno::Reference<io::XInputStream> getDocumentStream();
    virtual uno::Reference<io::XInputStream> getStorageStream();
    virtual uno::Reference<uno::XComponentContext> getContext();
    virtual ::rtl::OUString getTargetForId(const ::rtl::OUString & rId);
    virtual const ::rtl::OUString & getTarget() const;

    virtual uno::Reference<xml::sax::XFastTokenHandler>
    getFastTokenHandler(uno::Reference<uno::XComponentContext> rContext);

    void setInputStream(uno::Reference<io::XInputStream> rxInputStream);
};
}}
#endif // INCLUDED_OOXML_STREAM_IMPL_HXX
