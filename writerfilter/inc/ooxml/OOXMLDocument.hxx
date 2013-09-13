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
#ifndef INCLUDED_OOXML_DOCUMENT_HXX
#define INCLUDED_OOXML_DOCUMENT_HXX

#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <resourcemodel/WW8ResourceModel.hxx>
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

using namespace com::sun::star;

class WRITERFILTER_OOXML_DLLPUBLIC OOXMLStream
{
public:
    enum StreamType_t { UNKNOWN, DOCUMENT, STYLES, FONTTABLE, NUMBERING,
        FOOTNOTES, ENDNOTES, COMMENTS, THEME, SETTINGS, VBAPROJECT };
    typedef boost::shared_ptr<OOXMLStream> Pointer_t;

    virtual ~OOXMLStream() {}

    /**
       Returns parser for this stream.
     */
    virtual uno::Reference<xml::sax::XParser> getParser() = 0;

    /**
       Returns fast parser for this stream.
     */
    virtual uno::Reference<xml::sax::XFastParser> getFastParser() = 0;

    virtual uno::Reference<io::XInputStream> getDocumentStream() = 0;

    virtual uno::Reference<io::XInputStream> getStorageStream() = 0;

    /**
       Returns component context for this stream.
     */
    virtual uno::Reference<uno::XComponentContext> getContext() = 0;

    /**
       Returns target URL from relationships for a given id.

       @param rId           the id to look for

       @return the URL found or an empty string
     */
    virtual OUString getTargetForId(const OUString & rId) = 0;

    virtual const OUString & getTarget() const = 0;

    virtual uno::Reference<xml::sax::XFastTokenHandler>
    getFastTokenHandler(uno::Reference<uno::XComponentContext> rContext) = 0;

};

class WRITERFILTER_OOXML_DLLPUBLIC OOXMLDocument : public writerfilter::Reference<Stream>
{
public:
    /**
       Pointer to this stream.
    */
    typedef boost::shared_ptr<OOXMLDocument> Pointer_t;

    virtual ~OOXMLDocument() {}

    /**
       Resolves this document to a stream handler.

       @param rStream     stream handler to resolve this document to
     */
    virtual void resolve(Stream & rStream) = 0;

    /**
       Returns string representation of the type of this reference.

       DEBUGGING PURPOSE ONLY.
     */
    virtual string getType() const = 0;

    /**
       Resolves a footnote to a stream handler.

       A footnote is resolved if either the note type or
       note id matches.

       @param rStream       stream handler to resolve to
       @param rNoteType     type of footnote to resolve
       @param rNoteId       id of the footnote to resolve
     */
    virtual void resolveFootnote(Stream & rStream,
                                 const Id & rNoteType,
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
                                const Id & rNoteType,
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

    virtual void setModel(uno::Reference<frame::XModel> xModel) = 0;
    virtual uno::Reference<frame::XModel> getModel() = 0;
    virtual void setDrawPage(uno::Reference<drawing::XDrawPage> xDrawPage) = 0;
    virtual uno::Reference<drawing::XDrawPage> getDrawPage() = 0;
    virtual uno::Reference<io::XInputStream> getInputStream() = 0;
    virtual uno::Reference<io::XInputStream> getStorageStream() = 0;
    virtual uno::Reference<io::XInputStream> getInputStreamForId
    (const OUString & rId) = 0;
    virtual void setXNoteId(const sal_Int32 nId) = 0;
    virtual sal_Int32 getXNoteId() const = 0;
    virtual void setXNoteType(const Id & nId) = 0;
    virtual const Id & getXNoteType() const = 0;
    virtual const OUString & getTarget() const = 0;
    virtual uno::Reference<xml::sax::XFastShapeContextHandler> getShapeContext( ) = 0;
    virtual void setShapeContext( uno::Reference<xml::sax::XFastShapeContextHandler> xContext ) = 0;
    virtual uno::Reference<xml::dom::XDocument> getThemeDom( ) = 0;
    virtual void setThemeDom( uno::Reference<xml::dom::XDocument> xThemeDom ) = 0;
};


class WRITERFILTER_OOXML_DLLPUBLIC OOXMLDocumentFactory
{
public:
    static OOXMLStream::Pointer_t
    createStream(uno::Reference<uno::XComponentContext> rContext,
                 uno::Reference<io::XInputStream> rStream,
                 bool bRepairStorage,
                 OOXMLStream::StreamType_t nStreamType = OOXMLStream::DOCUMENT);

    static OOXMLStream::Pointer_t
    createStream(OOXMLStream::Pointer_t pStream,
                 OOXMLStream::StreamType_t nStreamType = OOXMLStream::DOCUMENT);

    static OOXMLStream::Pointer_t
    createStream(OOXMLStream::Pointer_t pStream, const OUString & rId);

    static OOXMLDocument *
    createDocument(OOXMLStream::Pointer_t pStream);

};

void ooxmlidsToXML(::std::iostream & out);

}}
#endif // INCLUDED_OOXML_DOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
