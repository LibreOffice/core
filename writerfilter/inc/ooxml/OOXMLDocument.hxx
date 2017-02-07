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
#ifndef INCLUDED_WRITERFILTER_INC_OOXML_OOXMLDOCUMENT_HXX
#define INCLUDED_WRITERFILTER_INC_OOXML_OOXMLDOCUMENT_HXX

#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <dmapper/resourcemodel.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/XFastShapeContextHandler.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

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

namespace writerfilter {
namespace ooxml
{

class OOXMLStream
{
public:
    enum StreamType_t { UNKNOWN, DOCUMENT, STYLES, WEBSETTINGS, FONTTABLE, NUMBERING,
        FOOTNOTES, ENDNOTES, COMMENTS, THEME, CUSTOMXML, CUSTOMXMLPROPS, ACTIVEX, ACTIVEXBIN, GLOSSARY, CHARTS, EMBEDDINGS, SETTINGS, VBAPROJECT, FOOTER, HEADER };
    typedef std::shared_ptr<OOXMLStream> Pointer_t;

    virtual ~OOXMLStream() {}

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

class OOXMLDocument : public writerfilter::Reference<Stream>
{
public:
    /**
       Pointer to this stream.
    */
    typedef std::shared_ptr<OOXMLDocument> Pointer_t;

    virtual ~OOXMLDocument() {}

    /**
       Resolves this document to a stream handler.

       @param rStream     stream handler to resolve this document to
     */
    virtual void resolve(Stream & rStream) override = 0;

    /**
       Resolves a footnote to a stream handler.

       A footnote is resolved if either the note type or
       note id matches.

       @param rStream       stream handler to resolve to
       @param rNoteType     type of footnote to resolve
       @param rNoteId       id of the footnote to resolve
     */
    virtual void resolveFootnote(Stream & rStream,
                                 Id aNoteType,
                                 const sal_Int32 nNoteId) = 0;
    /**
       Resolves an endnote to a stream handler.

       An endnote is resolved if either the note type or
       note id matches.

       @param rStream       stream handler to resolve to
       @param rNoteType     type of footnote to resolve
       @param rNoteId       id of the endnote to resolve
     */
    virtual void resolveEndnote(Stream & rStream,
                                Id  aNoteType,
                                const sal_Int32 NoteId) = 0;

    /**
       Resolves a comment to a stream handler.

       @param rStream       stream handler to resolve to
       @param rComment      id of the comment to resolve
     */
    virtual void resolveComment(Stream & rStream,
                                const sal_Int32 nCommentId) = 0;

    /**
       Resolves a picture to a stream handler.

       @param rStream       stream handler to resolve to
       @param rPictureId    id of the picture to resolve
     */
    virtual void resolvePicture(Stream & rStream,
                                const OUString & rPictureId) = 0;

    /**
       Resolves a header to a stream handler.

       @param rStream       stream handler to resolve to
       @param type          type of header to resolve:
                            NS_ooxml::LN_Value_ST_HrdFtr_even     header on even page
                            NS_ooxml::LN_Value_ST_HrdFtr_default  header on right page
                            NS_ooxml::LN_Value_ST_HrdFtr_first    header on first page

       @param rId           id of the header
     */
    virtual void resolveHeader(Stream & rStream,
                               const sal_Int32 type,
                               const OUString & rId) = 0;

    /**
       Resolves a footer to a stream handler.

       @param rStream       stream handler to resolve to
       @param type          type of footer to resolve:
                            NS_ooxml::LN_Value_ST_HrdFtr_even     header on even page
                            NS_ooxml::LN_Value_ST_HrdFtr_default  header on right page
                            NS_ooxml::LN_Value_ST_HrdFtr_first    header on first page

       @param rId           id of the header
    */
    virtual void resolveFooter(Stream & rStream,
                               const sal_Int32 type,
                               const OUString & rId) = 0;


    /**
       Returns target URL from relationships for a given id.

       @param rId           the id to look for

       @return the URL found or an empty string
     */
    virtual OUString getTargetForId(const OUString & rId) = 0;

    virtual void setModel(css::uno::Reference<css::frame::XModel> xModel) = 0;
    virtual css::uno::Reference<css::frame::XModel> getModel() = 0;
    virtual void setDrawPage(css::uno::Reference<css::drawing::XDrawPage> xDrawPage) = 0;
    virtual css::uno::Reference<css::drawing::XDrawPage> getDrawPage() = 0;
    virtual css::uno::Reference<css::io::XInputStream> getInputStreamForId(const OUString & rId) = 0;
    virtual void setXNoteId(const sal_Int32 nId) = 0;
    virtual sal_Int32 getXNoteId() const = 0;
    virtual void setXNoteType(Id nId) = 0;
    virtual const OUString & getTarget() const = 0;
    virtual css::uno::Reference<css::xml::sax::XFastShapeContextHandler> getShapeContext( ) = 0;
    virtual void setShapeContext( css::uno::Reference<css::xml::sax::XFastShapeContextHandler> xContext ) = 0;
    virtual css::uno::Reference<css::xml::dom::XDocument> getThemeDom( ) = 0;
    virtual css::uno::Reference<css::xml::dom::XDocument> getGlossaryDocDom( ) = 0;
    virtual css::uno::Sequence<css::uno::Sequence< css::uno::Any> > getGlossaryDomList() = 0;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getCustomXmlDomList( ) = 0;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getCustomXmlDomPropsList( ) = 0;
    virtual css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument> > getActiveXDomList( ) = 0;
    virtual css::uno::Sequence<css::uno::Reference<css::io::XInputStream> > getActiveXBinList() = 0;
    virtual css::uno::Sequence<css::beans::PropertyValue > getEmbeddingsList() = 0;
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

}}
#endif // INCLUDED_WRITERFILTER_INC_OOXML_OOXMLDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
