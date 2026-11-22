/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sddllapi.h>
#include <svx/seclabel/SecurityLabelTarget.hxx>

class SdDrawDocument;

namespace sd
{
class DrawViewShell;
struct HeaderFooterSettings;
}

namespace sd::seclabel
{
/// Whether rSettings show the security label marking of rDocument as the footer: the footer is
/// visible and its text is the marking stored with the label or the marking derived from its
/// policy.
SD_DLLPUBLIC bool showsMarking(SdDrawDocument& rDocument, const HeaderFooterSettings& rSettings);

/// The footer text of the first slide of rDocument that shows the security label marking, or an
/// empty string when no slide shows it.
SD_DLLPUBLIC OUString findMarkingFooter(SdDrawDocument& rDocument);
}

/// Impress implementation of the security-label placement target: the marking is written
/// as the visible footer text of every standard slide (colour follows the master style),
/// plus the online banner push. Cover/portion placements and the watermark are Writer-only
/// (Impress has no native watermark).
class SD_DLLPUBLIC SdSecurityLabelTarget final : public svx::seclabel::SecurityLabelTarget
{
    sd::DrawViewShell& m_rViewShell;

public:
    explicit SdSecurityLabelTarget(sd::DrawViewShell& rViewShell)
        : m_rViewShell(rViewShell)
    {
    }

    cpo::uno::Reference<css::frame::XModel> getModel() const override;
    void applyMarking(const svx::seclabel::LabelPlacement& rPlacement) override;
    void clearMarkings() override;
    void notify(const svx::seclabel::LabelChange& rChange) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
