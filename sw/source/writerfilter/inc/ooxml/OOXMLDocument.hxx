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

#include <sal/types.h>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <dmapper/resourcemodel.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/graphic/XGraphicMapper.hpp>
#include <oox/shape/ShapeContextHandler.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/ThemeFilterBase.hxx>
#include <vector>
#include <stack>
#include <set>



class SwFmDrawPage;
namespace writerfilter::ooxml { class OOXMLPropertySet; }

/**
   @file OOXMLDocument.hxx

   <h1>Import of OOXML WordprocessingML Documents</h1>

   The following picture shows the classes involved in importing OOXML
   WordprocessingML documents.

   @image html ooxmlimportchain.png

   The DOCX consists of parts. Each part is an XML document.  The
   OOXMLDocument opens the DOCX and creates a SAX parser for the part
   containing the main document content. The OOXMLDocument creates a
   SAX handler, too. This handler is set as the handler for the events
   created by the parser. Finally the OOXMLDocument initiates the
   parsing process.

   The SAX handler hosts a stack of contexts. Each context is an
   instance of a class derived from OOXMLContext. There is a context
   class for each <define> in the model.xml.

   For a detailed information about how the contexts are handled see
   the documentation for OOXMLContext.

   The contexts know how to convert an element in OOXML to the
   intermediate format that the domain mapper understands. They
   enumerate the according entity in OOXML by sending the according
   events to the domain mapper.

   The domain mapper knows how to convert the intermediate format to
   API calls. It takes the events sent by the contexts and uses the
   core API to insert the according elements to the core.
 */

namespace writerfilter::ooxml
{

class OOXMLStream : public virtual SvRefBase
{
public:
    enum class StreamType_t { UNKNOWN, DOCUMENT, STYLES, WEBSETTINGS, FONTTABLE, NUMBERING,
        FOOTNOTES, ENDNOTES, COMMENTS, COMMENTS_EXTENDED, THEME, CUSTOMXML, CUSTOMXMLPROPS, GLOSSARY, CHARTS, EMBEDDINGS, SETTINGS, VBAPROJECT, FOOTER, HEADER, VBADATA };
    typedef tools::SvRef<OOXMLStream> Pointer_t;

    /**
       Returns fast parser for this stream.
     */
    virtual css::uno::Reference<css::xml::sax::XFastParser> getFastParser() = 0;

    virtual css::uno::Reference<css::io::XInputStream> getDocumentStream() = 0;

    /**
       Returns component context for this stream.
     */
    virtual css::uno::Reference<css::uno::XComponentContext> getContext() = 0;

    /**
       Returns target URL from relationships for a given id.

       @param rId           the id to look for

       @return the URL found or an empty string
     */
    virtual OUString getTargetForId(const OUString & rId) = 0;

    virtual const OUString & getTarget() const = 0;

    virtual css::uno::Reference<css::xml::sax::XFastTokenHandler>
    getFastTokenHandler() = 0;

};

class OOXMLDocument final : public writerfilter::Reference<Stream>
{
    OOXMLStream::Pointer_t mpStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;
    writerfilter::Reference<Stream>::Pointer_t mpXFootnoteStream;
    writerfilter::Reference<Stream>::Pointer_t mpXEndnoteStream;
    sal_Int32 mnXNoteId;

    css::uno::Reference<css::frame::XModel> mxModel;
    rtl::Reference<SwFmDrawPage> mxDrawPage;
    css::uno::Reference<css::xml::dom::XDocument> mxGlossaryDocDom;
    css::uno::Sequence < css::uno::Sequence< css::beans::NamedValue > > mxGlossaryDomList;
    /// Stack of shape contexts, 1 element for VML, 1 element / nesting level for drawingML.
    std::stack< rtl::Reference<oox::shape::ShapeContextHandler> > maShapeContexts;
    css::uno::Reference<css::xml::dom::XDocument> mxThemeDom;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxCustomXmlDomList;
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > mxCustomXmlDomPropsList;
    css::uno::Reference<css::xml::dom::XDocument> mxCustomXmlProsDom;
    css::uno::Reference<css::io::XInputStream> mxEmbeddings;
    css::uno::Sequence < css::beans::PropertyValue > mxEmbeddingsList;
    std::vector<css::beans::PropertyValue> m_aEmbeddings;
    std::set<OUString> maSeenStreams;
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
    // For a document there is a single theme in document.xml.rels
    // and the same is used by header and footer as well.
    oox::drawingml::ThemePtr mpTheme;
    rtl::Reference<oox::shape::ShapeFilterBase> mxShapeFilterBase;
    rtl::Reference<oox::drawingml::ThemeFilterBase> mxThemeFilterBase;

    bool mbCommentsExtendedResolved = false;

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
    void resolveCommentsExtendedStream(Stream & rStream);

public:
    OOXMLDocument(OOXMLStream::Pointer_t pStream,
            css::uno::Reference<css::task::XStatusIndicator> xStatusIndicator,
            bool bSkipImages,
            const css::uno::Sequence<css::beans::PropertyValue>& rDescriptor,
            const rtl::Reference<oox::shape::ShapeFilterBase>& rxShapeFilterBase);
    virtual ~OOXMLDocument() override;

    /**
       Resolves this document to a stream handler.

       @param rStream     stream handler to resolve this document to
     */
    virtual void resolve(Stream & rStream) override;

    /**
       Resolves a footnote to a stream handler.

       A footnote is resolved if either the note type or
       note id matches.

       @param rStream       stream handler to resolve to
       @param rNoteType     type of footnote to resolve
       @param rNoteId       id of the footnote to resolve
     */
    void resolveFootnote(Stream & rStream,
                                 Id aType,
                                 const sal_Int32 nNoteId);
    /**
       Resolves an endnote to a stream handler.

       An endnote is resolved if either the note type or
       note id matches.

       @param rStream       stream handler to resolve to
       @param rNoteType     type of footnote to resolve
       @param rNoteId       id of the endnote to resolve
     */
    void resolveEndnote(Stream & rStream,
                                Id aType,
                                const sal_Int32 nNoteId);

    /**
       Resolves a header to a stream handler.

       @param rStream       stream handler to resolve to
       @param type          type of header to resolve:
                            NS_ooxml::LN_Value_ST_HrdFtr_even     header on even page
                            NS_ooxml::LN_Value_ST_HrdFtr_default  header on right page
                            NS_ooxml::LN_Value_ST_HrdFtr_first    header on first page

       @param rId           id of the header
     */
    void resolveHeader(Stream & rStream,
                               const sal_Int32 type,
                               const OUString & rId);
    void resolveFooter(Stream & rStream,
                               const sal_Int32 type,
                               const OUString & rId);

    /**
       Resolves a comment to a stream handler.

       @param rStream       stream handler to resolve to
       @param rComment      id of the comment to resolve
     */
    void resolveComment(Stream & rStream, const sal_Int32 nId);

    OOXMLPropertySet * getPicturePropSet(const OUString & rId);

    /**
       Resolves a picture to a stream handler.

       @param rStream       stream handler to resolve to
       @param rPictureId    id of the picture to resolve
     */
    void resolvePicture(Stream & rStream, const OUString & rId);

    /**
       Returns target URL from relationships for a given id.

       @param rId           the id to look for

       @return the URL found or an empty string
     */
    OUString getTargetForId(const OUString & rId);

    void setModel(const css::uno::Reference<css::frame::XModel>& xModel);
    const css::uno::Reference<css::frame::XModel>& getModel() const;
    void setDrawPage(const rtl::Reference<SwFmDrawPage>& xDrawPage);
    const rtl::Reference<SwFmDrawPage>& getDrawPage() const;
    css::uno::Reference<css::io::XInputStream> getInputStreamForId(const OUString & rId);
    void setXNoteId(const sal_Int32 nId);
    sal_Int32 getXNoteId() const;
    const OUString & getTarget() const;
    rtl::Reference<oox::shape::ShapeContextHandler> getShapeContext( );
    void setShapeContext(const rtl::Reference<oox::shape::ShapeContextHandler>& xContext);
    const oox::drawingml::ThemePtr & getTheme() const
    {
        return mpTheme;
    }
    /// Push context of drawingML shapes, so nested shapes are handled separately.
    void pushShapeContext();
    /// Pop context of a previously pushed drawingML shape.
    void popShapeContext();
    const css::uno::Reference<css::xml::dom::XDocument>& getThemeDom() const;
    const css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> >& getCustomXmlDomList() const;
    const css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> >& getCustomXmlDomPropsList() const;
    const css::uno::Reference<css::xml::dom::XDocument>& getGlossaryDocDom() const;
    const css::uno::Sequence<css::uno::Sequence< css::beans::NamedValue> >&  getGlossaryDomList() const;
    const css::uno::Sequence<css::beans::PropertyValue >&  getEmbeddingsList() const;

    void incrementProgress();
    bool IsSkipImages() const { return mbSkipImages; };
    OUString const& GetDocumentBaseURL() const { return m_rBaseURL; };
    const css::uno::Sequence<css::beans::PropertyValue>& getMediaDescriptor() const;

    const css::uno::Reference<css::graphic::XGraphicMapper>& getGraphicMapper() const
    {
        return mxGraphicMapper;
    }

    void setTheme(const oox::drawingml::ThemePtr& pTheme) { mpTheme = pTheme; }

    const rtl::Reference<oox::shape::ShapeFilterBase> & getShapeFilterBase();
    const rtl::Reference<oox::drawingml::ThemeFilterBase> & getThemeFilterBase();


    /**
       Pointer to this stream.
    */
    typedef tools::SvRef<OOXMLDocument> Pointer_t;

};


class OOXMLDocumentFactory
{
public:
    static OOXMLStream::Pointer_t
    createStream(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                 const css::uno::Reference<css::io::XInputStream>& rStream,
                 bool bRepairStorage);

    static OOXMLStream::Pointer_t
    createStream(const OOXMLStream::Pointer_t& pStream,
                 OOXMLStream::StreamType_t nStreamType);

    static OOXMLStream::Pointer_t
    createStream(const OOXMLStream::Pointer_t& pStream, const OUString & rId);

    static OOXMLDocument *
    createDocument(const OOXMLStream::Pointer_t& pStream,
                 const css::uno::Reference<css::task::XStatusIndicator>& xStatusIndicator,
                 bool bSkipImage, const css::uno::Sequence<css::beans::PropertyValue>& rDescriptor);

};

std::string fastTokenToId(sal_uInt32 nToken);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
