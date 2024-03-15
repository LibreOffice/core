/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ChartModel.hxx>

#include <sfx2/namedcolor.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::frame { class XModel; }
namespace chart { class ChartModel; }
class SvxColorToolBoxControl;
class SvxLineStyleToolBoxControl;

namespace chart::sidebar {

class ChartColorWrapper
{
public:
    ChartColorWrapper(rtl::Reference<::chart::ChartModel> xModel,
            SvxColorToolBoxControl* pControl,
            OUString  rPropertyName);

    void operator()(const OUString& rCommand, const NamedColor& rColor);
        // ColorSelectFunction signature

    void updateModel(const rtl::Reference<::chart::ChartModel>& xModel);

    void updateData();

private:

    rtl::Reference<::chart::ChartModel> mxModel;

    SvxColorToolBoxControl* mpControl;

    OUString maPropertyName;
};

class ChartLineStyleWrapper
{
public:
    ChartLineStyleWrapper(rtl::Reference<::chart::ChartModel> xModel,
            SvxLineStyleToolBoxControl* pControl);

    bool operator()(std::u16string_view rCommand, const css::uno::Any& rValue);

    void updateModel(const rtl::Reference<::chart::ChartModel>& xModel);

    void updateData();

private:

    rtl::Reference<::chart::ChartModel> mxModel;

    SvxLineStyleToolBoxControl* mpControl;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
