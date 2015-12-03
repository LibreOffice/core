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
#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLDOCUMENTIMPL_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLDOCUMENTIMPL_HXX

#include <ooxml/OOXMLDocument.hxx>

#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>

#include "OOXMLPropertySet.hxx"

#include <vector>

namespace writerfilter {
namespace ooxml
{

class OOXMLDocumentImpl : public OOXMLDocument
{
    OOXMLStream::Pointer_t mpStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;
    sal_Int32 mnXNoteId;
    Id mXNoteType;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::drawing::XDrawPage> mxDrawPage;
    css::uno::Reference<css::xml::dom::XDocument> mxGlossaryDocDom;
    css::uno::Sequence < css::uno::Sequence< css::uno::Any > > mxGlossaryDomList;
    css::uno::Reference<css::xml::sax::XFastShapeContextHandler> mxShapeContext;
    css::uno::Reference<css::xml::dom::XDocument> mxThemeDom;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxCustomXmlDomList;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxCustomXmlDomPropsList;
    css::uno::Reference<css::xml::dom::XDocument> mxCustomXmlProsDom;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxActiveXDomList;
    css::uno::Sequence<css::uno::Reference<css::io::XInputStream> > mxActiveXBinList;
    css::uno::Reference<css::io::XInputStream> mxActiveXBin;
    css::uno::Reference<css::io::XInputStream> mxEmbeddings;
    css::uno::Sequence < css::beans::PropertyValue > mxEmbeddingsList;
    bool mbIsSubstream;
    bool mbSkipImages;
    /// How many paragraphs equal to 1 percent?
    sal_Int32 mnPercentSize;
    /// Position progress when it was last updated, possibly not after every paragraph in case of large documents.
    sal_Int32 mnProgressLastPos;
    /// Current position progress, updated after every paragraph.
    sal_Int32 mnProgressCurrentPos;
    /// End position, i.e. the estimated number of paragraphs.
    sal_Int32 mnProgressEndPos;

protected:
    void resolveFastSubStream(Stream & rStream,
                                      OOXMLStream::StreamType_t nType);

    static void resolveFastSubStreamWithId(Stream & rStream,
                                           writerfilter::Reference<Stream>::Pointer_t pStream,
                                           sal_uInt32 nId);

    css::uno::Reference<css::xml::dom::XDocument> importSubStream(OOXMLStream::StreamType_t nType);

    void importSubStreamRelations(OOXMLStream::Pointer_t pStream, OOXMLStream::StreamType_t nType);

    writerfilter::Reference<Stream>::Pointer_t
    getSubStream(const OUString & rId);

    writerfilter::Reference<Stream>::Pointer_t
    getXNoteStream(OOXMLStream::StreamType_t nType,
                   const Id & rType,
                   const sal_Int32 nNoteId);

    void setIsSubstream( bool bSubstream ) { mbIsSubstream = bSubstream; };
    void resolveCustomXmlStream(Stream & rStream);
    void resolveActiveXStream(Stream & rStream);
    void resolveGlossaryStream(Stream & rStream);
    void resolveEmbeddingsStream(OOXMLStream::Pointer_t pStream);
public:
    OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream, const css::uno::Reference<css::task::XStatusIndicator>& xStatusIndicator, bool bSkipImages);
    virtual ~OOXMLDocumentImpl();

    virtual void resolve(Stream & rStream) override;

    virtual void resolveFootnote(Stream & rStream,
                                 const Id & rType,
                                 const sal_Int32 nNoteId) override;
    virtual void resolveEndnote(Stream & rStream,
                                const Id & rType,
                                const sal_Int32 nNoteId) override;
    virtual void resolveHeader(Stream & rStream,
                               const sal_Int32 type,
                               const OUString & rId) override;
    virtual void resolveFooter(Stream & rStream,
                               const sal_Int32 type,
                               const OUString & rId) override;

    virtual void resolveComment(Stream & rStream, const sal_Int32 nId) override;

    OOXMLPropertySet * getPicturePropSet(const OUString & rId);
    virtual void resolvePicture(Stream & rStream, const OUString & rId) override;

    virtual OUString getTargetForId(const OUString & rId) override;

    virtual void setModel(css::uno::Reference<css::frame::XModel> xModel) override;
    virtual css::uno::Reference<css::frame::XModel> getModel() override;
    virtual void setDrawPage(css::uno::Reference<css::drawing::XDrawPage> xDrawPage) override;
    virtual css::uno::Reference<css::drawing::XDrawPage> getDrawPage() override;
    virtual css::uno::Reference<css::io::XInputStream> getStorageStream() override;
    virtual css::uno::Reference<css::io::XInputStream> getInputStreamForId(const OUString & rId) override;
    virtual void setXNoteId(const sal_Int32 nId) override;
    virtual sal_Int32 getXNoteId() const override;
    virtual void setXNoteType(const Id & rId) override;
    virtual const OUString & getTarget() const override;
    virtual css::uno::Reference<css::xml::sax::XFastShapeContextHandler> getShapeContext( ) override;
    virtual void setShapeContext( css::uno::Reference<css::xml::sax::XFastShapeContextHandler> xContext ) override;
    virtual css::uno::Reference<css::xml::dom::XDocument> getThemeDom() override;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getCustomXmlDomList() override;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getCustomXmlDomPropsList() override;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getActiveXDomList() override;
    virtual css::uno::Sequence<css::uno::Reference<css::io::XInputStream> > getActiveXBinList() override;
    virtual css::uno::Reference<css::xml::dom::XDocument> getGlossaryDocDom() override;
    virtual css::uno::Sequence<css::uno::Sequence< css::uno::Any> >  getGlossaryDomList() override;
    virtual css::uno::Sequence<css::beans::PropertyValue >  getEmbeddingsList() override;

    void incrementProgress();
    bool IsSkipImages() { return mbSkipImages; };
};
}}
#endif // OOXML_DOCUMENT_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
