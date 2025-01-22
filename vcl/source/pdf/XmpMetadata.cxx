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

XmpMetadata::XmpMetadata() = default;

void XmpMetadata::write()
{
    mpMemoryStream = std::make_unique<SvMemoryStream>(4096 /*Initial*/, 64 /*Resize*/);

    // Header
    mpMemoryStream->WriteOString(
        OStringLiteral(u8"<?xpacket begin=\"\uFEFF\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"));

    {
        tools::XmlWriter aXmlWriter(mpMemoryStream.get());
        aXmlWriter.startDocument(2, false);
        aXmlWriter.startElement("x"_ostr, "xmpmeta"_ostr, "adobe:ns:meta/"_ostr);
        aXmlWriter.startElement("rdf"_ostr, "RDF"_ostr,
                                "http://www.w3.org/1999/02/22-rdf-syntax-ns#"_ostr);

        // PDF/A part ( ISO 19005-1:2005 - 6.7.11 )
        if (mnPDF_A > 0)
        {
            OString sPdfVersion = OString::number(mnPDF_A);

            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", ""_ostr);
            aXmlWriter.attribute("xmlns:pdfaid", "http://www.aiim.org/pdfa/ns/id/"_ostr);

            aXmlWriter.startElement("pdfaid:part");
            aXmlWriter.content(sPdfVersion);
            aXmlWriter.endElement();

            if (mnPDF_A == 4)
            {
                aXmlWriter.startElement("pdfaid:rev");
                aXmlWriter.content("2020"_ostr);
                aXmlWriter.endElement();
            }

            if (!msConformance.isEmpty())
            {
                aXmlWriter.startElement("pdfaid:conformance");
                aXmlWriter.content(msConformance);
                aXmlWriter.endElement();
            }
            aXmlWriter.endElement();
        }

        // Dublin Core properties
        if (!msTitle.isEmpty() || !msAuthor.isEmpty() || !msSubject.isEmpty()
            || !maContributor.empty() || !msCoverage.isEmpty() || !msIdentifier.isEmpty()
            || !maPublisher.empty() || !maRelation.empty() || !msRights.isEmpty()
            || !msSource.isEmpty() || !msType.isEmpty())
        {
            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", ""_ostr);
            aXmlWriter.attribute("xmlns:dc", "http://purl.org/dc/elements/1.1/"_ostr);

            aXmlWriter.startElement("dc:format");
            aXmlWriter.content("application/pdf"_ostr);
            aXmlWriter.endElement();

            if (!msTitle.isEmpty())
            {
                // this is according to PDF/A-1, technical corrigendum 1 (2007-04-01)
                aXmlWriter.startElement("dc:title");
                aXmlWriter.startElement("rdf:Alt");
                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("xml:lang", "x-default"_ostr);
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
                aXmlWriter.attribute("xml:lang", "x-default"_ostr);
                aXmlWriter.content(msSubject);
                aXmlWriter.endElement();
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!maContributor.empty())
            {
                aXmlWriter.startElement("dc:contributor");
                aXmlWriter.startElement("rdf:Bag");
                for (const OString& rContributor : maContributor)
                {
                    aXmlWriter.startElement("rdf:li");
                    aXmlWriter.content(rContributor);
                    aXmlWriter.endElement();
                }
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!msCoverage.isEmpty())
            {
                aXmlWriter.startElement("dc:coverage");
                aXmlWriter.content(msCoverage);
                aXmlWriter.endElement();
            }
            if (!msIdentifier.isEmpty())
            {
                aXmlWriter.startElement("dc:identifier");
                aXmlWriter.content(msIdentifier);
                aXmlWriter.endElement();
            }
            if (!maPublisher.empty())
            {
                aXmlWriter.startElement("dc:publisher");
                aXmlWriter.startElement("rdf:Bag");
                for (const OString& rPublisher : maPublisher)
                {
                    aXmlWriter.startElement("rdf:li");
                    aXmlWriter.content(rPublisher);
                    aXmlWriter.endElement();
                }
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!maRelation.empty())
            {
                aXmlWriter.startElement("dc:relation");
                aXmlWriter.startElement("rdf:Bag");
                for (const OString& rRelation : maRelation)
                {
                    aXmlWriter.startElement("rdf:li");
                    aXmlWriter.content(rRelation);
                    aXmlWriter.endElement();
                }
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!msRights.isEmpty())
            {
                aXmlWriter.startElement("dc:rights");
                aXmlWriter.startElement("rdf:Alt");
                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("xml:lang", "x-default"_ostr);
                aXmlWriter.content(msRights);
                aXmlWriter.endElement();
                aXmlWriter.endElement();
                aXmlWriter.endElement();
            }
            if (!msSource.isEmpty())
            {
                aXmlWriter.startElement("dc:source");
                aXmlWriter.content(msSource);
                aXmlWriter.endElement();
            }
            if (!msType.isEmpty())
            {
                aXmlWriter.startElement("dc:type");
                aXmlWriter.content(msType);
                aXmlWriter.endElement();
            }
            aXmlWriter.endElement();
        }

        // PDF/UA
        if (mnPDF_UA > 0)
        {
            if (mnPDF_A != 0)
            { // tdf#157517 PDF/A extension schema is required
                aXmlWriter.startElement("rdf:Description");
                aXmlWriter.attribute("rdf:about", ""_ostr);
                aXmlWriter.attribute("xmlns:pdfaExtension",
                                     "http://www.aiim.org/pdfa/ns/extension/"_ostr);
                aXmlWriter.attribute("xmlns:pdfaSchema",
                                     "http://www.aiim.org/pdfa/ns/schema#"_ostr);
                aXmlWriter.attribute("xmlns:pdfaProperty",
                                     "http://www.aiim.org/pdfa/ns/property#"_ostr);
                aXmlWriter.startElement("pdfaExtension:schemas");
                aXmlWriter.startElement("rdf:Bag");
                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("rdf:parseType", "Resource"_ostr);
                aXmlWriter.startElement("pdfaSchema:namespaceURI");
                aXmlWriter.content("http://www.aiim.org/pdfua/ns/id/"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaSchema:prefix");
                aXmlWriter.content("pdfuaid"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaSchema:schema");
                aXmlWriter.content("PDF/UA identification schema"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaSchema:property");
                aXmlWriter.startElement("rdf:Seq");

                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("rdf:parseType", "Resource"_ostr);
                aXmlWriter.startElement("pdfaProperty:category");
                aXmlWriter.content("internal"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:description");
                aXmlWriter.content("PDF/UA version identifier"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:name");
                aXmlWriter.content("part"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:valueType");
                aXmlWriter.content("Integer"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.endElement(); // rdf:li

                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("rdf:parseType", "Resource"_ostr);
                aXmlWriter.startElement("pdfaProperty:category");
                aXmlWriter.content("internal"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:description");
                aXmlWriter.content("PDF/UA amendment identifier"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:name");
                aXmlWriter.content("amd"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:valueType");
                aXmlWriter.content("Text"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.endElement(); // rdf:li

                aXmlWriter.startElement("rdf:li");
                aXmlWriter.attribute("rdf:parseType", "Resource"_ostr);
                aXmlWriter.startElement("pdfaProperty:category");
                aXmlWriter.content("internal"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:description");
                aXmlWriter.content("PDF/UA corrigenda identifier"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:name");
                aXmlWriter.content("corr"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.startElement("pdfaProperty:valueType");
                aXmlWriter.content("Text"_ostr);
                aXmlWriter.endElement();
                aXmlWriter.endElement(); // rdf:li

                aXmlWriter.endElement(); // rdf:Seq
                aXmlWriter.endElement(); // pdfaSchema:property
                aXmlWriter.endElement(); // rdf:li
                aXmlWriter.endElement(); // rdf:Bag
                aXmlWriter.endElement(); // pdfaExtension:schemas
                aXmlWriter.endElement(); // rdf:Description
            }
            OString sPdfUaVersion = OString::number(mnPDF_UA);
            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", ""_ostr);
            aXmlWriter.attribute("xmlns:pdfuaid", "http://www.aiim.org/pdfua/ns/id/"_ostr);

            aXmlWriter.startElement("pdfuaid:part");
            aXmlWriter.content(sPdfUaVersion);
            aXmlWriter.endElement();

            if (mnPDF_UA == 2)
            {
                aXmlWriter.startElement("pdfuaid:rev");
                aXmlWriter.content("2024"_ostr);
                aXmlWriter.endElement();
            }
            aXmlWriter.endElement();
        }

        // PDF properties
        if (!msProducer.isEmpty() || !msKeywords.isEmpty() || !msPDFVersion.isEmpty())
        {
            aXmlWriter.startElement("rdf:Description");
            aXmlWriter.attribute("rdf:about", ""_ostr);
            aXmlWriter.attribute("xmlns:pdf", "http://ns.adobe.com/pdf/1.3/"_ostr);
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
            if (!msPDFVersion.isEmpty())
            {
                aXmlWriter.startElement("pdf:PDFVersion");
                aXmlWriter.content(msPDFVersion);
                aXmlWriter.endElement();
            }
            aXmlWriter.endElement();
        }

        // XMP Basic schema
        aXmlWriter.startElement("rdf:Description");
        aXmlWriter.attribute("rdf:about", ""_ostr);
        aXmlWriter.attribute("xmlns:xmp", "http://ns.adobe.com/xap/1.0/"_ostr);
        if (!m_sCreatorTool.isEmpty())
        {
            aXmlWriter.startElement("xmp:CreatorTool");
            aXmlWriter.content(m_sCreatorTool);
            aXmlWriter.endElement();
        }
        aXmlWriter.startElement("xmp:CreateDate");
        aXmlWriter.content(m_sCreateDate);
        aXmlWriter.endElement();

        aXmlWriter.startElement("xmp:ModifyDate");
        aXmlWriter.content(m_sCreateDate);
        aXmlWriter.endElement();

        aXmlWriter.startElement("xmp:MetadataDate");
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
