/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_SECLABELAPPLY_HXX
#define INCLUDED_SW_INC_SECLABELAPPLY_HXX

#include "StanagLabel.hxx"
#include "swdllapi.h"
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <string_view>

namespace com::sun::star::frame
{
class XModel;
}

namespace sw::seclabel
{
/// Schema URI that marks our STANAG customXml part among the document's customXml
/// items (the itemProps ds:schemaRef references it). Matches the 4778 binding ns,
/// and is how we find the existing label to replace or remove (never by index).
inline constexpr OUString STANAG_BINDING_SCHEMA
    = u"urn:nato:stanag:4778:bindinginformation:1:0"_ustr;

/// Store a STANAG customXml part (the 4778 binding) and its itemProps in the
/// document's InteropGrabBag, so the DOCX export writes them as
/// customXml/item*.xml + customXml/itemProps*.xml on the next save. Any label
/// part the document already carries is replaced (found the way readLabel finds
/// it), so re-applying never leaves a duplicate behind.
SW_DLLPUBLIC void storeLabelPart(const css::uno::Reference<css::frame::XModel>& xModel,
                                 std::u16string_view rBindingXml,
                                 std::u16string_view rItemPropsXml);

/// Resolve a SPIF colour (a W3C colour name or #RRGGBB) to an RGB value; unknown
/// names fall back to black.
SW_DLLPUBLIC sal_Int32 resolveColor(const OUString& rColor);

/// Set the page style's header and footer to the marking text (bold, coloured,
/// centred), replacing any existing content (v1 pageTopBottom behaviour).
SW_DLLPUBLIC void applyMarking(const css::uno::Reference<css::frame::XModel>& xModel,
                               const OUString& rMarking, sal_Int32 nColor,
                               const OUString& rPageStyleName);

/// Read a STANAG label from the document's customXml grab-bag into rLabel (handles
/// a 4778 binding wrapper or a standalone 4774 label). Returns false if none found.
SW_DLLPUBLIC bool readLabel(const css::uno::Reference<css::frame::XModel>& xModel,
                            StanagLabel& rLabel);

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_SECLABELAPPLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
