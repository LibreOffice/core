/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <svtools/valueset.hxx>
#include <map>
#include "ShapesUtil.hxx"

using namespace css;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace svx::sidebar {

/** This panel provides buttons for inserting shapes into a document.
*/
class DefaultShapesPanel
    : public PanelLayout, public SvxShapeCommandsMap
{
public:
    DefaultShapesPanel (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    static VclPtr<PanelLayout> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~DefaultShapesPanel() override;
    virtual void dispose() override;

private:
    std::unique_ptr<ValueSet> mxLineArrowSet;
    std::unique_ptr<weld::CustomWeld> mxLineArrowSetWin;
    std::unique_ptr<ValueSet> mxCurveSet;
    std::unique_ptr<weld::CustomWeld> mxCurveSetWin;
    std::unique_ptr<ValueSet> mxConnectorSet;
    std::unique_ptr<weld::CustomWeld> mxConnectorSetWin;
    std::unique_ptr<ValueSet> mxBasicShapeSet;
    std::unique_ptr<weld::CustomWeld> mxBasicShapeSetWin;
    std::unique_ptr<ValueSet> mxSymbolShapeSet;
    std::unique_ptr<weld::CustomWeld> mxSymbolShapeSetWin;
    std::unique_ptr<ValueSet> mxBlockArrowSet;
    std::unique_ptr<weld::CustomWeld> mxBlockArrowSetWin;
    std::unique_ptr<ValueSet> mxFlowchartSet;
    std::unique_ptr<weld::CustomWeld> mxFlowchartSetWin;
    std::unique_ptr<ValueSet> mxCalloutSet;
    std::unique_ptr<weld::CustomWeld> mxCalloutSetWin;
    std::unique_ptr<ValueSet> mxStarSet;
    std::unique_ptr<weld::CustomWeld> mxStarSetWin;
    std::unique_ptr<ValueSet> mx3DObjectSet;
    std::unique_ptr<weld::CustomWeld> mx3DObjectSetWin;

    Reference< XFrame >       mxFrame;
    std::map<ValueSet*, std::map<sal_uInt16, OUString>> mpShapesSetMap;

    void populateShapes();
    void Initialize();
    DECL_LINK( ShapeSelectHdl, ValueSet*, void );
};

} // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
