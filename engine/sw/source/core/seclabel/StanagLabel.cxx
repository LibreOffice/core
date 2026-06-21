/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <StanagLabel.hxx>

#include <tools/XmlWalker.hxx>
#include <tools/XmlWriter.hxx>
#include <tools/stream.hxx>

namespace sw::seclabel
{
namespace
{
OUString streamToString(SvMemoryStream& rStream)
{
    return OUString(static_cast<const char*>(rStream.GetData()),
                    static_cast<sal_Int32>(rStream.GetSize()), RTL_TEXTENCODING_UTF8);
}

OUString toOU(std::string_view rStr) { return OStringToOUString(rStr, RTL_TEXTENCODING_UTF8); }

// Position rWalker at the OriginatorConfidentialityLabel element, descending through
// any wrapper (e.g. a 4778 binding). Returns false if it is not present.
bool descendToLabel(tools::XmlWalker& rWalker)
{
    if (rWalker.name() == "OriginatorConfidentialityLabel")
        return true;
    rWalker.children();
    while (rWalker.isValid())
    {
        if (descendToLabel(rWalker))
            return true;
        rWalker.next();
    }
    rWalker.parent();
    return false;
}
}

void StanagLabel::writeTo(tools::XmlWriter& rWriter) const
{
    rWriter.startElement("OriginatorConfidentialityLabel");
    rWriter.attribute("xmlns", u"urn:nato:stanag:4774:confidentialitymetadatalabel:1:0");
    rWriter.attribute("ConfidentialityLabelVersion", u"1");
    if (!aReviewDateTime.isEmpty())
        rWriter.attribute("ReviewDateTime", aReviewDateTime);

    rWriter.startElement("ConfidentialityInformation");

    rWriter.startElement("PolicyIdentifier");
    if (!aPolicyId.isEmpty())
        rWriter.attribute("URI", aPolicyId);
    rWriter.content(aPolicyName);
    rWriter.endElement();

    rWriter.startElement("Classification");
    rWriter.content(aClassification);
    rWriter.endElement();

    for (const auto& rCategory : aCategories)
    {
        rWriter.startElement("Category");
        rWriter.attribute("TagName", rCategory.aTagName);
        rWriter.attribute("Type", rCategory.aType);
        for (const auto& rValue : rCategory.aValues)
        {
            rWriter.startElement("GenericValue");
            rWriter.content(rValue);
            rWriter.endElement();
        }
        rWriter.endElement(); // Category
    }

    rWriter.endElement(); // ConfidentialityInformation

    if (!aCreationDateTime.isEmpty())
    {
        rWriter.startElement("CreationDateTime");
        rWriter.content(aCreationDateTime);
        rWriter.endElement();
    }

    rWriter.endElement(); // OriginatorConfidentialityLabel
}

OUString StanagLabel::toXml() const
{
    SvMemoryStream aStream;
    {
        tools::XmlWriter aWriter(&aStream);
        aWriter.startDocument(2, true);
        writeTo(aWriter);
        aWriter.endDocument();
    }
    return streamToString(aStream);
}

OUString StanagLabel::toBindingXml() const
{
    SvMemoryStream aStream;
    {
        tools::XmlWriter aWriter(&aStream);
        aWriter.startDocument(2, true);

        aWriter.startElement("BindingInformation");
        aWriter.attribute("xmlns", u"urn:nato:stanag:4778:bindinginformation:1:0");
        aWriter.startElement("MetadataBindingContainer");
        aWriter.startElement("MetadataBinding");

        aWriter.startElement("Metadata");
        writeTo(aWriter); // the 4774 label re-declares its own default namespace
        aWriter.endElement(); // Metadata

        aWriter.startElement("DataReference");
        // Empty URI binds to the whole containing document (4778 embedded approach);
        // the exact OOXML target is NISP/ADatP-34 binding-profile territory.
        aWriter.attribute("URI", u"");
        aWriter.endElement(); // DataReference

        aWriter.endElement(); // MetadataBinding
        aWriter.endElement(); // MetadataBindingContainer
        aWriter.endElement(); // BindingInformation
        aWriter.endDocument();
    }
    return streamToString(aStream);
}

OUString buildItemProps(std::u16string_view rItemId, std::u16string_view rSchemaUri)
{
    SvMemoryStream aStream;
    {
        tools::XmlWriter aWriter(&aStream);
        aWriter.startDocument(2, true);

        // ds: prefix on elements and attributes; declare the namespace explicitly.
        aWriter.startElement("ds:datastoreItem");
        aWriter.attribute("xmlns:ds",
                          u"http://schemas.openxmlformats.org/officeDocument/2006/customXml");
        aWriter.attribute("ds:itemID", rItemId);
        aWriter.startElement("ds:schemaRefs");
        aWriter.startElement("ds:schemaRef");
        aWriter.attribute("ds:uri", rSchemaUri);
        aWriter.endElement(); // ds:schemaRef
        aWriter.endElement(); // ds:schemaRefs
        aWriter.endElement(); // ds:datastoreItem
        aWriter.endDocument();
    }
    return streamToString(aStream);
}

bool StanagLabel::parse(SvStream& rStream)
{
    aPolicyName.clear();
    aPolicyId.clear();
    aClassification.clear();
    aCreationDateTime.clear();
    aReviewDateTime.clear();
    aCategories.clear();

    tools::XmlWalker aWalker;
    if (!aWalker.open(&rStream))
        return false;
    if (!descendToLabel(aWalker))
        return false;

    aReviewDateTime = toOU(aWalker.attribute("ReviewDateTime"_ostr));

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "ConfidentialityInformation")
        {
            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "PolicyIdentifier")
                {
                    aPolicyId = toOU(aWalker.attribute("URI"_ostr));
                    aPolicyName = toOU(aWalker.content());
                }
                else if (aWalker.name() == "Classification")
                {
                    aClassification = toOU(aWalker.content());
                }
                else if (aWalker.name() == "Category")
                {
                    StanagCategory aCategory;
                    aCategory.aTagName = toOU(aWalker.attribute("TagName"_ostr));
                    aCategory.aType = toOU(aWalker.attribute("Type"_ostr));
                    aWalker.children();
                    while (aWalker.isValid())
                    {
                        if (aWalker.name() == "GenericValue")
                            aCategory.aValues.push_back(toOU(aWalker.content()));
                        aWalker.next();
                    }
                    aWalker.parent();
                    aCategories.push_back(aCategory);
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        else if (aWalker.name() == "CreationDateTime")
        {
            aCreationDateTime = toOU(aWalker.content());
        }
        aWalker.next();
    }
    aWalker.parent();
    return true;
}

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
