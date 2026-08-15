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

class ScTabViewShell;

/// Calc implementation of the security-label placement target: the marking is
/// written as the centre header text of the active sheet's page style; plus the
/// online banner push. Cover/portion/watermark placements do not apply to Calc.
class ScSecurityLabelTarget final : public svx::seclabel::SecurityLabelTarget
{
    ScTabViewShell& m_rViewShell;

public:
    explicit ScSecurityLabelTarget(ScTabViewShell& rViewShell)
        : m_rViewShell(rViewShell)
    {
    }

    css::uno::Reference<css::frame::XModel> getModel() const override;
    void applyMarking(const svx::seclabel::LabelPlacement& rPlacement) override;
    void clearMarkings() override;
    void notify(const OUString& rMarking) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
