/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTCOLORWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTCOLORWRAPPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <svx/Palette.hxx>

namespace com::sun::star::frame { class XModel; }

class SvxColorToolBoxControl;
class SvxLineStyleToolBoxControl;

namespace chart::sidebar {

class ChartColorWrapper
{
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

class ChartLineStyleWrapper
{
public:
    ChartLineStyleWrapper(css::uno::Reference<css::frame::XModel> const & xModel,
            SvxLineStyleToolBoxControl* pControl);

    bool operator()(const OUString& rCommand, const css::uno::Any& rValue);

    void updateModel(const css::uno::Reference<css::frame::XModel>& xModel);

    void updateData();

private:

    css::uno::Reference<css::frame::XModel> mxModel;

    SvxLineStyleToolBoxControl* mpControl;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
