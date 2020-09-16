/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/XmpMetadata.hxx>
#include <tools/XmlWriter.hxx>

namespace vcl::pdf
{
namespace
{
constexpr const char* constPadding = "                                        "
                                     "                                        "
                                     "                                        "
                                     "                                        "
                                     "                                        "
                                     "\n";
}

XmpMetadata::XmpMetadata()
    : mbWritten(false)
    , mnPDF_A(0)
    , mbPDF_UA(false)
{
}

void XmpMetadata::write()
{
    mpMemoryStream = std::make_unique<SvMemoryStream>(4096 /*Initial*/, 64 /*Resize*/);

    // Header
    mpMemoryStream->WriteOString("<?xpacket begin=\"");
    mpMemoryStream->WriteOString(OUStringToOString(OUString(u'\xFEFF'), RTL_TEXTENCODING_UTF8));
    mpMemoryStream->WriteOString("\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n");

    {
        tools::XmlWriter aXmlWriter(mpMemoryStream.get());
        aXmlWriter.startDocument(2, false);
        aXmlWriter.startElement("x", "xmpmeta", "adobe:ns:meta/");
        aXmlWriter.startElement("rdf", "RDF", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");

        // PDF/A part ( ISO 19005-1:2005 - 6.7.11 )
        if (mnPDF_A > 0)
        {
            OString sPdfVersion = OString::number(mnPDF_A);
            OString sPdfConformance = (mnPDF_A == 1) ? "A" : "B";

            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", OString(""));
            aXmlWriter.attribute("xmlns:pdfaid", OString("http://www.aiim.org/pdfa/ns/id/"));

            aXmlWriter.startElement("pdfaid:part");
            aXmlWriter.content(sPdfVersion);
            aXmlWriter.endElement();

            aXmlWriter.startElement("pdfaid:conformance");
            aXmlWriter.content(sPdfConformance);
            aXmlWriter.endElement();

            aXmlWriter.endElement();
        }

        // Dublin Core properties
        if (!msTitle.isEmpty() || !msAuthor.isEmpty() || !msSubject.isEmpty())
        {
            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", OString(""));
            aXmlWriter.attribute("xmlns:dc", OString("http://purl.org/dc/elements/1.1/"));
            if (!msTitle.isEmpty())
            {
                // this is according to PDF/A-1, technical corrigendum 1 (2007-04-01)
                aXmlWriter.startElement("dc:title");
                aXmlWriter.startElement("rdf:Alt");
                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("xml:lang", OString("x-default"));
                aXmlWriter.content(msTitle);
                aXmlWriter.endElement();
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!msAuthor.isEmpty())
            {
                aXmlWriter.startElement("dc:creator");
                aXmlWriter.startElement("rdf:Seq");
                aXmlWriter.startElement("rdf:li");
                aXmlWriter.content(msAuthor);
                aXmlWriter.endElement();
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!msSubject.isEmpty())
            {
                aXmlWriter.startElement("dc:description");
                aXmlWriter.startElement("rdf:Alt");
                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("xml:lang", OString("x-default"));
                aXmlWriter.content(msSubject);
                aXmlWriter.endElement();
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            aXmlWriter.endElement();
        }

        // PDF/UA
        if (mbPDF_UA)
        {
            OString sPdfUaVersion = OString::number(1);
            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", OString(""));
            aXmlWriter.attribute("xmlns:pdfuaid", OString("http://www.aiim.org/pdfua/ns/id/"));

            aXmlWriter.startElement("pdfuaid:part");
            aXmlWriter.content(sPdfUaVersion);
            aXmlWriter.endElement();

            aXmlWriter.endElement();
        }

        // PDF properties
        if (!msProducer.isEmpty() || !msKeywords.isEmpty())
        {
            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", OString(""));
            aXmlWriter.attribute("xmlns:pdf", OString("http://ns.adobe.com/pdf/1.3/"));
            if (!msProducer.isEmpty())
            {
                aXmlWriter.startElement("pdf:Producer");
                aXmlWriter.content(msProducer);
                aXmlWriter.endElement();
            }
            if (!msKeywords.isEmpty())
            {
                aXmlWriter.startElement("pdf:Keywords");
                aXmlWriter.content(msKeywords);
                aXmlWriter.endElement();
            }
            aXmlWriter.endElement();
        }

        aXmlWriter.startElement("rdf:Description");
        aXmlWriter.attribute("rdf:about", OString(""));
        aXmlWriter.attribute("xmlns:xmp", OString("http://ns.adobe.com/xap/1.0/"));
        if (!m_sCreatorTool.isEmpty())
        {
            aXmlWriter.startElement("xmp:CreatorTool");
            aXmlWriter.content(m_sCreatorTool);
            aXmlWriter.endElement();
        }
        aXmlWriter.startElement("xmp:CreateDate");
        aXmlWriter.content(m_sCreateDate);
        aXmlWriter.endElement();
        aXmlWriter.endElement();

        aXmlWriter.endElement();
        aXmlWriter.endElement();
        aXmlWriter.endDocument();
    }

    // add padding (needed so the metadata can be changed in-place"
    for (sal_Int32 nSpaces = 1; nSpaces <= 21; nSpaces++)
        mpMemoryStream->WriteOString(constPadding);

    mpMemoryStream->WriteOString("<?xpacket end=\"w\"?>\n");
    mbWritten = true;
}

sal_uInt64 XmpMetadata::getSize()
{
    if (!mbWritten)
        write();
    return mpMemoryStream->GetSize();
}

const void* XmpMetadata::getData()
{
    if (!mbWritten)
        write();
    return mpMemoryStream->GetData();
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
