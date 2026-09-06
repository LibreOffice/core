/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWSECURITYLABELTARGET_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWSECURITYLABELTARGET_HXX

#include <svx/seclabel/SecurityLabelTarget.hxx>

class SwWrtShell;

/// Writer implementation of the security-label placement target: header/footer
/// marking via the page style, cover/end-page markings in the body, portion
/// marking at the view cursor, and a watermark; plus the online banner push.
class SwSecurityLabelTarget final : public svx::seclabel::SecurityLabelTarget
{
    SwWrtShell& m_rSh;

public:
    explicit SwSecurityLabelTarget(SwWrtShell& rSh)
        : m_rSh(rSh)
    {
    }

    css::uno::Reference<css::frame::XModel> getModel() const override;
    void applyMarking(const svx::seclabel::LabelPlacement& rPlacement) override;
    void clearMarkings() override;
    void notify(const svx::seclabel::LabelChange& rChange) override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWSECURITYLABELTARGET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
