/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLDocument.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-29 15:29:43 $
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
#include <doctok/WW8ResourceModel.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HOO_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif


namespace ooxml
{

using namespace doctok;
using namespace com::sun::star;

class WRITERFILTER_DLLPUBLIC OOXMLStream
{
public:
    enum StreamType_t { UNKNOWN, DOCUMENT, STYLES, FONTTABLE, NUMBERING, FOOTNOTES, ENDNOTES, COMMENTS };
    typedef boost::shared_ptr<OOXMLStream> Pointer_t;

    virtual ~OOXMLStream() {}

    virtual uno::Reference<xml::sax::XParser> getParser() = 0;
    virtual uno::Reference<io::XInputStream> getInputStream() = 0;
    virtual uno::Reference<uno::XComponentContext> getContext() = 0;
};

class WRITERFILTER_DLLPUBLIC OOXMLDocument : public doctok::Reference<Stream>
{
public:
    /**
       Pointer to this stream.
    */
    typedef boost::shared_ptr<OOXMLDocument> Pointer_t;

    virtual ~OOXMLDocument() {}

    virtual void resolve(Stream &) = 0;

    virtual string getType() const = 0;

    virtual void resolveFootnote(Stream & rStream,
                                 const rtl::OUString & rNoteId) = 0;
    virtual void resolveEndnote(Stream & rStream,
                                const rtl::OUString & rNoteId) = 0;
    virtual void resolveComment(Stream & rStream,
                                const rtl::OUString & rNoteId) = 0;
    virtual void resolveHeader(Stream & rStream,
                               const sal_Int32 type,
                               const rtl::OUString & rId) = 0;
    virtual void resolveFooter(Stream & rStream,
                               const sal_Int32 type,
                               const rtl::OUString & rId) = 0;
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
}
#endif // INCLUDED_OOXML_DOCUMENT_HXX
