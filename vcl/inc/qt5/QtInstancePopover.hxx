/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceContainer.hxx"

class QtInstancePopover : public QtInstanceContainer, public virtual weld::Popover
{
    Q_OBJECT

public:
    explicit QtInstancePopover(QWidget* pWidget);

    virtual void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                               weld::Placement ePlace = weld::Placement::Under) override;
    virtual void popdown() override;

    virtual void resize_to_request() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
