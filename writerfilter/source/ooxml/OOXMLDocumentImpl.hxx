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


#ifndef INCLUDED_OOXML_DOCUMENT_IMPL_HXX
#define INCLUDED_OOXML_DOCUMENT_IMPL_HXX

#include <ooxml/OOXMLDocument.hxx>

#ifndef _COM_SUN_STAR_XML_SAX_XFAST_TOKEN_HANDLER_HPP_
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#endif

#include "OOXMLPropertySet.hxx"

namespace writerfilter {
namespace ooxml
{

using namespace ::com::sun::star;

class OOXMLDocumentImpl : public OOXMLDocument
{
    OOXMLStream::Pointer_t mpStream;
    rtl::OUString msXNoteId;
    Id mXNoteType;

    uno::Reference<frame::XModel> mxModel;
    uno::Reference<drawing::XDrawPage> mxDrawPage;

    bool mbIsSubstream;

protected:
    virtual void resolveFastSubStream(Stream & rStream,
                                      OOXMLStream::StreamType_t nType);

    virtual void resolveFastSubStreamWithId(Stream & rStream,
                                      writerfilter::Reference<Stream>::Pointer_t pStream,
                      sal_uInt32 nId);

    writerfilter::Reference<Stream>::Pointer_t
    getSubStream(const rtl::OUString & rId);

    writerfilter::Reference<Stream>::Pointer_t
    getXNoteStream(OOXMLStream::StreamType_t nType,
                   const Id & rType,
                   const rtl::OUString & rNoteId);

    void setIsSubstream( bool bSubstream ) { mbIsSubstream = bSubstream; };

public:
    OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream);
    virtual ~OOXMLDocumentImpl();

    virtual void resolve(Stream & rStream);

    virtual string getType() const;

    virtual void resolveFootnote(Stream & rStream,
                                 const Id & rType,
                                 const rtl::OUString & rNoteId);
    virtual void resolveEndnote(Stream & rStream,
                                const Id & rType,
                                const rtl::OUString & rNoteId);
    virtual void resolveHeader(Stream & rStream,
                               const sal_Int32 type,
                               const rtl::OUString & rId);
    virtual void resolveFooter(Stream & rStream,
                               const sal_Int32 type,
                               const rtl::OUString & rId);

    virtual void resolveComment(Stream & rStream, const rtl::OUString & rId);

    virtual OOXMLPropertySet * getPicturePropSet
    (const ::rtl::OUString & rId);
    virtual void resolvePicture(Stream & rStream, const rtl::OUString & rId);

    virtual ::rtl::OUString getTargetForId(const ::rtl::OUString & rId);

    virtual void setModel(uno::Reference<frame::XModel> xModel);
    virtual uno::Reference<frame::XModel> getModel();
    virtual void setDrawPage(uno::Reference<drawing::XDrawPage> xDrawPage);
    virtual uno::Reference<drawing::XDrawPage> getDrawPage();
    virtual uno::Reference<io::XInputStream> getInputStream();
    virtual uno::Reference<io::XInputStream> getStorageStream();
    virtual uno::Reference<io::XInputStream> getInputStreamForId(const rtl::OUString & rId);
    virtual void setXNoteId(const rtl::OUString & rId);
    virtual const ::rtl::OUString & getXNoteId() const;
    virtual void setXNoteType(const Id & rId);
    virtual const Id & getXNoteType() const;
    virtual const ::rtl::OUString & getTarget() const;
};
}}
#endif // OOXML_DOCUMENT_IMPL_HXX
