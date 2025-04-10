/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/uitest/uiobject.hxx>
#include "wizdlg.hxx"

class RoadmapWizardUIObject final : public WindowUIObject
{
    VclPtr<vcl::RoadmapWizard> mxRoadmapWizard;

public:
    RoadmapWizardUIObject(const VclPtr<vcl::RoadmapWizard>& xRoadmapWizard);
    virtual ~RoadmapWizardUIObject() override;

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction, const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

private:
    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
