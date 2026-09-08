/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/seclabel/SecurityLabelTarget.hxx>

namespace sd
{
class DrawViewShell;
}

/// Impress implementation of the security-label placement target: the marking is written
/// as the visible footer text of every standard slide (colour follows the master style),
/// plus the online banner push. Cover/portion placements and the watermark are Writer-only
/// (Impress has no native watermark).
class SdSecurityLabelTarget final : public svx::seclabel::SecurityLabelTarget
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
