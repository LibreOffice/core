/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_SOURCE_INC_DEFAULTSHAPESPANEL_HXX
#define INCLUDED_SVX_SOURCE_INC_DEFAULTSHAPESPANEL_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld.hxx>
#include <map>
#include <vector>
#include "ShapesUtil.hxx"

namespace svx::sidebar {

/** This panel provides buttons for inserting shapes into a document.
*/
class DefaultShapesPanel final
    : public PanelLayout
{
public:
    DefaultShapesPanel (
        weld::Widget* pParent,
        css::uno::Reference<css::frame::XFrame> xFrame);
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~DefaultShapesPanel() override;

private:
    std::unique_ptr<weld::IconView> mxLineArrowSet;
    std::unique_ptr<weld::IconView> mxCurveSet;
    std::unique_ptr<weld::IconView> mxConnectorSet;
    std::unique_ptr<weld::IconView> mxBasicShapeSet;
    std::unique_ptr<weld::IconView> mxSymbolShapeSet;
    std::unique_ptr<weld::IconView> mxBlockArrowSet;
    std::unique_ptr<weld::IconView> mxFlowchartSet;
    std::unique_ptr<weld::IconView> mxCalloutSet;
    std::unique_ptr<weld::IconView> mxStarSet;
    std::unique_ptr<weld::IconView> mx3DObjectSet;

    css::uno::Reference< css::frame::XFrame >       mxFrame;
    std::vector<std::pair<weld::IconView*, const std::map<sal_uInt16, OUString>*>> maShapeGalleries;

    void populateShapes();
    void Initialize();
    DECL_LINK( ShapeSelectHdl, weld::IconView&, bool );
};

} // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
