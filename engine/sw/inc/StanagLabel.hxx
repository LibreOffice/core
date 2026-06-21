/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_STANAGLABEL_HXX
#define INCLUDED_SW_INC_STANAGLABEL_HXX

#include "swdllapi.h"
#include <rtl/ustring.hxx>
#include <string_view>
#include <vector>

class SvStream;

namespace tools
{
class XmlWriter;
}

namespace sw::seclabel
{
/// One category of a STANAG 4774 label (a tag and its selected values).
struct StanagCategory
{
    OUString aTagName; ///< Category/@TagName
    OUString aType; ///< Category/@Type (PERMISSIVE/RESTRICTIVE/INFORMATIVE)
    std::vector<OUString> aValues; ///< GenericValue elements
};

/// A STANAG 4774 confidentiality label, serializable to its XML form.
/// Timestamps are caller-provided (ISO 8601) so serialization stays deterministic.
class SW_DLLPUBLIC StanagLabel
{
public:
    OUString aPolicyName; ///< PolicyIdentifier element text
    OUString aPolicyId; ///< PolicyIdentifier/@URI (e.g. urn:oid:1.2.826.0.1310.1.2.0)
    OUString aClassification; ///< Classification element text
    std::vector<StanagCategory> aCategories;
    OUString aCreationDateTime; ///< CreationDateTime child element (mandatory for validity)
    OUString aReviewDateTime; ///< ReviewDateTime root attribute (required when no SuccessionHandling)

    /// Serialize to the STANAG 4774 OriginatorConfidentialityLabel XML.
    OUString toXml() const;

    /// Serialize as a STANAG 4778 BindingInformation embedding the 4774 label.
    OUString toBindingXml() const;

    /// Parse a 4774 label (optionally wrapped in a 4778 binding) into this
    /// instance. Returns false if no OriginatorConfidentialityLabel is found.
    bool parse(SvStream& rStream);

private:
    /// Write the OriginatorConfidentialityLabel element into rWriter.
    void writeTo(tools::XmlWriter& rWriter) const;
};

/// Build the OOXML customXml itemProps (ds:datastoreItem + ds:schemaRef) for a
/// STANAG part: rItemId is the "{GUID}" item id, rSchemaUri the part's schema namespace.
SW_DLLPUBLIC OUString buildItemProps(std::u16string_view rItemId, std::u16string_view rSchemaUri);

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_STANAGLABEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
