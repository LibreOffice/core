/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLDocument.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:48:23 $
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
#ifndef INCLUDED_OOXML_DOCUMENT_HXX
#define INCLUDED_OOXML_DOCUMENT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif // _SAL_TYPES_H_
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEX_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <resourcemodel/WW8ResourceModel.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HOO_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

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

class WRITERFILTER_DLLPUBLIC OOXMLStream
{
public:
    enum StreamType_t { UNKNOWN, DOCUMENT, STYLES, FONTTABLE, NUMBERING, FOOTNOTES, ENDNOTES, COMMENTS, THEME };
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

    /**
       Returns input stream for this stream.
     */
    virtual uno::Reference<io::XInputStream> getInputStream() = 0;

    virtual uno::Reference<io::XInputStream> getDocumentStream() = 0;

    /**
       Returns component context for this stream.
     */
    virtual uno::Reference<uno::XComponentContext> getContext() = 0;

    /**
       Returns target URL from relationships for a given id.

       @param rId           the id to look for

       @return the URL found or an empty string
     */
    virtual ::rtl::OUString getTargetForId(const ::rtl::OUString & rId) = 0;

    virtual uno::Reference<xml::sax::XFastTokenHandler>
    getFastTokenHandler(uno::Reference<uno::XComponentContext> rContext) = 0;

};

class WRITERFILTER_DLLPUBLIC OOXMLDocument : public writerfilter::Reference<Stream>
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

       @param rStream       stream handler to resolve to
       @param rNoteId       id of the footnote to resolve
     */
    virtual void resolveFootnote(Stream & rStream,
                                 const rtl::OUString & rNoteId) = 0;

    /**
       Resolves an endnote to a stream handler.

       @param rStream       stream handler to resolve to
       @param rNoteId       id of the endnote to resolve
     */
    virtual void resolveEndnote(Stream & rStream,
                                const rtl::OUString & rNoteId) = 0;

    /**
       Resolves a comment to a stream handler.

       @param rStream       stream handler to resolve to
       @param rComment      id of the comment to resolve
     */
    virtual void resolveComment(Stream & rStream,
                                const rtl::OUString & rCommentId) = 0;

    /**
       Resolves a picture to a stream handler.

       @param rStream       stream handler to resolve to
       @param rPictureId    id of the picture to resolve
     */
    virtual void resolvePicture(Stream & rStream,
                                const rtl::OUString & rPictureId) = 0;

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
                               const rtl::OUString & rId) = 0;

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
                               const rtl::OUString & rId) = 0;


    /**
       Returns target URL from relationships for a given id.

       @param rId           the id to look for

       @return the URL found or an empty string
     */
    virtual ::rtl::OUString getTargetForId(const ::rtl::OUString & rId) = 0;

    virtual void setModel(uno::Reference<frame::XModel> xModel) = 0;
    virtual uno::Reference<frame::XModel> getModel() = 0;
    virtual void setShapes(uno::Reference<drawing::XShapes> xShapes) = 0;
    virtual uno::Reference<drawing::XShapes> getShapes() = 0;
    virtual uno::Reference<io::XInputStream> getInputStream() = 0;
    virtual uno::Reference<io::XInputStream> getInputStreamForId
    (const ::rtl::OUString & rId) = 0;
    virtual void setXNoteId(const rtl::OUString & rId) = 0;
    virtual const ::rtl::OUString & getXNoteId() const = 0;
};


class WRITERFILTER_DLLPUBLIC OOXMLDocumentFactory
{
public:
    static OOXMLStream::Pointer_t
    createStream(uno::Reference<uno::XComponentContext> rContext,
                 uno::Reference<io::XInputStream> rStream,
                 OOXMLStream::StreamType_t nStreamType = OOXMLStream::DOCUMENT);

    static OOXMLStream::Pointer_t
    createStream(OOXMLStream::Pointer_t pStream,
                 OOXMLStream::StreamType_t nStreamType = OOXMLStream::DOCUMENT);

    static OOXMLStream::Pointer_t
    createStream(OOXMLStream::Pointer_t pStream, const rtl::OUString & rId);

    static OOXMLDocument *
    createDocument(OOXMLStream::Pointer_t pStream);

};

void ooxmlidsToXML(::std::iostream & out);

}}
#endif // INCLUDED_OOXML_DOCUMENT_HXX
