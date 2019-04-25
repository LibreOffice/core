/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <svx/Palette.hxx>

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }

class SvxColorToolBoxControl;

namespace chart { namespace sidebar {

class ChartColorWrapper
{
private:

public:
    ChartColorWrapper(css::uno::Reference<css::frame::XModel> const & xModel,
            SvxColorToolBoxControl* pControl,
            const OUString& rPropertyName);

    void operator()(const OUString& rCommand, const NamedColor& rColor);

    void updateModel(const css::uno::Reference<css::frame::XModel>& xModel);

    void updateData();

private:

    css::uno::Reference<css::frame::XModel> mxModel;

    SvxColorToolBoxControl* mpControl;

    OUString maPropertyName;
};

} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
