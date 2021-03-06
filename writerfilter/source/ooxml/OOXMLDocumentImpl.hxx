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

#include <ooxml/OOXMLDocument.hxx>

#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/graphic/XGraphicMapper.hpp>

#include "OOXMLPropertySet.hxx"

#include <vector>
#include <stack>

namespace writerfilter::ooxml
{

class OOXMLDocumentImpl : public OOXMLDocument
{
    OOXMLStream::Pointer_t mpStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;
    writerfilter::Reference<Stream>::Pointer_t mpXNoteStream;
    sal_Int32 mnXNoteId;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::drawing::XDrawPage> mxDrawPage;
    css::uno::Reference<css::xml::dom::XDocument> mxGlossaryDocDom;
    css::uno::Sequence < css::uno::Sequence< css::uno::Any > > mxGlossaryDomList;
    /// Stack of shape contexts, 1 element for VML, 1 element / nesting level for drawingML.
    std::stack< css::uno::Reference<css::xml::sax::XFastShapeContextHandler> > maShapeContexts;
    css::uno::Reference<css::xml::dom::XDocument> mxThemeDom;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxCustomXmlDomList;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxCustomXmlDomPropsList;
    css::uno::Reference<css::xml::dom::XDocument> mxCustomXmlProsDom;
    css::uno::Reference<css::io::XInputStream> mxEmbeddings;
    css::uno::Sequence < css::beans::PropertyValue > mxEmbeddingsList;
    std::vector<css::beans::PropertyValue> aEmbeddings;
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
    /// DocumentBaseURL
    OUString m_rBaseURL;
    css::uno::Sequence<css::beans::PropertyValue> maMediaDescriptor;
    /// Graphic mapper
    css::uno::Reference<css::graphic::XGraphicMapper> mxGraphicMapper;

private:
    void resolveFastSubStream(Stream & rStream,
                                      OOXMLStream::StreamType_t nType);

    static void resolveFastSubStreamWithId(Stream & rStream,
                                           const writerfilter::Reference<Stream>::Pointer_t& pStream,
                                           sal_uInt32 nId);

    css::uno::Reference<css::xml::dom::XDocument> importSubStream(OOXMLStream::StreamType_t nType);

    void importSubStreamRelations(const OOXMLStream::Pointer_t& pStream, OOXMLStream::StreamType_t nType);

    writerfilter::Reference<Stream>::Pointer_t
    getSubStream(const OUString & rId);

    writerfilter::Reference<Stream>::Pointer_t
    getXNoteStream(OOXMLStream::StreamType_t nType, const sal_Int32 nNoteId);

    void resolveCustomXmlStream(Stream & rStream);
    void resolveGlossaryStream(Stream & rStream);
    void resolveEmbeddingsStream(const OOXMLStream::Pointer_t& pStream);
public:
    OOXMLDocumentImpl(OOXMLStream::Pointer_t const & pStream, const css::uno::Reference<css::task::XStatusIndicator>& xStatusIndicator, bool bSkipImages, const css::uno::Sequence<css::beans::PropertyValue>& rDescriptor);
    virtual ~OOXMLDocumentImpl() override;

    virtual void resolve(Stream & rStream) override;

    virtual void resolveFootnote(Stream & rStream,
                                 Id aType,
                                 const sal_Int32 nNoteId) override;
    virtual void resolveEndnote(Stream & rStream,
                                Id aType,
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
    virtual css::uno::Reference<css::io::XInputStream> getInputStreamForId(const OUString & rId) override;
    virtual void setXNoteId(const sal_Int32 nId) override;
    virtual sal_Int32 getXNoteId() const override;
    virtual const OUString & getTarget() const override;
    virtual css::uno::Reference<css::xml::sax::XFastShapeContextHandler> getShapeContext( ) override;
    virtual void setShapeContext( css::uno::Reference<css::xml::sax::XFastShapeContextHandler> xContext ) override;
    void pushShapeContext() override;
    void popShapeContext() override;
    virtual css::uno::Reference<css::xml::dom::XDocument> getThemeDom() override;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getCustomXmlDomList() override;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getCustomXmlDomPropsList() override;
    virtual css::uno::Reference<css::xml::dom::XDocument> getGlossaryDocDom() override;
    virtual css::uno::Sequence<css::uno::Sequence< css::uno::Any> >  getGlossaryDomList() override;
    virtual css::uno::Sequence<css::beans::PropertyValue >  getEmbeddingsList() override;

    void incrementProgress();
    bool IsSkipImages() const { return mbSkipImages; };
    OUString const& GetDocumentBaseURL() const { return m_rBaseURL; };
    const css::uno::Sequence<css::beans::PropertyValue>& getMediaDescriptor() const;

    const css::uno::Reference<css::graphic::XGraphicMapper>& getGraphicMapper() const
    {
        return mxGraphicMapper;
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
