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

#include "swdllapi.h"
#include <cpo/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <string_view>

namespace com::sun::star::frame
{
class XModel;
}

// Writer-specific placement of a security-label marking (header/footer, body,
// portion). The app-agnostic label storage + colour resolution live in
// <svx/seclabel/SecLabelStore.hxx>.
namespace sw::seclabel
{
/// Mark the header and footer (bold, coloured, centred) of every in-use page style, plus
/// rPageStyleName. The marking is its own paragraph tagged with the "Security Label"
/// character style, coexisting with the user's own header/footer content (not replacing
/// it); a prior marking is cleared first, so re-labelling never stacks banners.
SW_DLLPUBLIC void applyMarking(const cpo::uno::Reference<css::frame::XModel>& xModel,
                               const OUString& rMarking, sal_Int32 nColor,
                               std::u16string_view rPageStyleName);

/// Place the marking as a cover (bStart) and/or end-page (bEnd) paragraph in the
/// document body, bookmarked so re-applying replaces rather than duplicates. A
/// placement not requested is cleared, so a re-label leaves no stale body marking.
SW_DLLPUBLIC void applyBodyMarkings(const cpo::uno::Reference<css::frame::XModel>& xModel,
                                    const OUString& rMarking, sal_Int32 nColor, bool bStart,
                                    bool bEnd);

/// Remove both body (cover/end-page) markings, if present.
SW_DLLPUBLIC void removeBodyMarkings(const cpo::uno::Reference<css::frame::XModel>& xModel);

/// Prefix the portion (the paragraph holding the view cursor) with the marking in
/// parentheses, formatted with the "Security Label" character style. Idempotent: a
/// portion already carrying this prefix is left unchanged. removeLabel (and a re-label
/// that drops the portion placement) removes it, found by that style.
SW_DLLPUBLIC void applyPortionMarking(const cpo::uno::Reference<css::frame::XModel>& xModel,
                                      std::u16string_view rMarking, sal_Int32 nColor);

/// Clear the label's Writer markings: the body (cover/end-page) markings and the
/// header/footer markings of every page style (identified by the "Security Label"
/// character style, so the user's own header/footer content is left intact). The
/// customXml part is removed separately (svx::seclabel::removeLabelPart); the watermark
/// by the caller. rPageStyleName is unused (all styles are swept).
SW_DLLPUBLIC void removeLabel(const cpo::uno::Reference<css::frame::XModel>& xModel,
                              std::u16string_view rPageStyleName);

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_SECLABELAPPLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
