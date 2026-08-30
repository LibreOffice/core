/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SECLABEL_SECLABELSTORE_HXX
#define INCLUDED_SVX_SECLABEL_SECLABELSTORE_HXX

#include <svx/svxdllapi.h>
#include <svx/seclabel/StanagLabel.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <string_view>

namespace com::sun::star::frame
{
class XModel;
}

// App-agnostic security-label helpers: the customXml part carrying the STANAG
// label lives in the document's InteropGrabBag and round-trips generically for
// DOCX/XLSX/PPTX; these read/write/remove it and resolve SPIF colours. Marking
// placement (header/footer, body, watermark) is app-specific and lives elsewhere.
namespace svx::seclabel
{
/// Schema URI that marks our STANAG customXml part among the document's customXml
/// items (the itemProps ds:schemaRef references it). Matches the 4778 binding ns,
/// and is how we find the existing label to replace or remove (never by index).
inline constexpr OUString STANAG_BINDING_SCHEMA
    = u"urn:nato:stanag:4778:bindinginformation:1:0"_ustr;

/// Store a STANAG customXml part (the 4778 binding) and its itemProps in the
/// document's InteropGrabBag, so the OOXML export writes them as
/// customXml/item*.xml + customXml/itemProps*.xml on the next save. Any label
/// part the document already carries is replaced (found the way readLabel finds
/// it), so re-applying never leaves a duplicate behind.
SVXCORE_DLLPUBLIC void storeLabelPart(const css::uno::Reference<css::frame::XModel>& xModel,
                                      std::u16string_view rBindingXml,
                                      std::u16string_view rItemPropsXml);

/// Remove the label's customXml part (found the way readLabel finds it) from the
/// document's InteropGrabBag, if present.
SVXCORE_DLLPUBLIC void removeLabelPart(const css::uno::Reference<css::frame::XModel>& xModel);

/// Read a STANAG label from the document's customXml grab-bag into rLabel (handles
/// a 4778 binding wrapper or a standalone 4774 label). Returns false if none found.
SVXCORE_DLLPUBLIC bool readLabel(const css::uno::Reference<css::frame::XModel>& xModel,
                                 StanagLabel& rLabel);

/// The document's visual marking: read its STANAG label and render the ADatP-4774.2
/// marking with the provisioned SPIF policy it was created under (matched by OID),
/// falling back to the label's self-describing summary when that policy is not
/// provisioned this session. Empty when the document carries no label. The single
/// source of the on-load banner marking, so it matches the dialog's live preview.
SVXCORE_DLLPUBLIC OUString readMarking(const css::uno::Reference<css::frame::XModel>& xModel);

/// Whether xModel's document can carry a security label: true only for the OOXML
/// formats (docx/xlsx/pptx and their template/macro variants), since the label rides
/// in the OOXML customXml part. Determined from the loading filter, which is
/// authoritative regardless of the file's extension. Gates the command and its button.
SVXCORE_DLLPUBLIC bool modelSupportsLabel(const css::uno::Reference<css::frame::XModel>& xModel);

/// Resolve a SPIF colour (a W3C colour name or #RRGGBB) to an RGB value; unknown
/// names fall back to black.
SVXCORE_DLLPUBLIC sal_Int32 resolveColor(const OUString& rColor);

} // namespace svx::seclabel

#endif // INCLUDED_SVX_SECLABEL_SECLABELSTORE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
