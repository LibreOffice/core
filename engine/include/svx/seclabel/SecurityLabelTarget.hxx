/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SECLABEL_SECURITYLABELTARGET_HXX
#define INCLUDED_SVX_SECLABEL_SECURITYLABELTARGET_HXX

#include <svx/svxdllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star::frame
{
class XModel;
}

namespace svx::seclabel
{
/// The visible marking to place, and which policy-driven placements are active.
/// The header/footer ("page") marking is always applied; the flags gate the rest.
struct LabelPlacement
{
    OUString aMarking; ///< the marking string to render
    sal_Int32 nColor = 0; ///< RGB (from the classification colour)
    bool bCoverStart = false; ///< documentStart: cover-page marking
    bool bCoverEnd = false; ///< documentEnd: end-page marking
    bool bPortion = false; ///< portionMarking: mark the selected portion
    bool bWatermark = false; ///< waterMark
};

/// A security-label lifecycle event, pushed to the browser so every client's banner
/// updates and the host receives a postMessage. The wire form is the enriched
/// `.uno:SecurityLabel` statechanged JSON (broadcast at the wsd layer).
struct LabelChange
{
    /// "applied" (no prior label), "changed" (prior label replaced), or "removed".
    OUString aAction;
    OUString aClassification; ///< the new classification (empty on removed)
    OUString aMarking; ///< the new visual marking (empty on removed)
    OUString aOldClassification; ///< the prior classification (empty on applied)
    OUString aOldMarking; ///< the prior visual marking (empty on applied)
};

/// App-specific placement of a security-label marking. The (app-agnostic) dialog
/// computes the marking + placements from the SPIF policy and drives one of these;
/// each app (Writer/Calc/Impress) renders the marking its own way. Label storage is
/// app-agnostic and handled separately (see <svx/seclabel/SecLabelStore.hxx>).
class SVXCORE_DLLPUBLIC SecurityLabelTarget
{
public:
    virtual ~SecurityLabelTarget();

    /// The document the label applies to (for the app-agnostic storage helpers).
    virtual css::uno::Reference<css::frame::XModel> getModel() const = 0;

    /// Render the marking with the requested placements, replacing any prior one.
    virtual void applyMarking(const LabelPlacement& rPlacement) = 0;

    /// Clear all label markings this target renders (page/body/watermark).
    virtual void clearMarkings() = 0;

    /// Push a label lifecycle event to any live UI: every client's classification
    /// banner (broadcast) and a host postMessage. No-op off-line.
    virtual void notify(const LabelChange& rChange) = 0;
};

} // namespace svx::seclabel

#endif // INCLUDED_SVX_SECLABEL_SECURITYLABELTARGET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
