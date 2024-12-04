/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtWidgets/QLabel>

class QtInstanceImage : public QtInstanceWidget, public virtual weld::Image
{
    Q_OBJECT

    QLabel* m_pLabel;

public:
    QtInstanceImage(QLabel* pLabel);

    virtual void set_from_icon_name(const OUString& rIconName) override;
    virtual void set_image(VirtualDevice* pDevice) override;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
