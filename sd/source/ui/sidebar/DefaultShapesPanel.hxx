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
#ifndef INCLUDED_SD_SOURCE_SIDEBAR_INSERT_DEFAULTSHAPESPANEL_HXX
#define INCLUDED_SD_SOURCE_SIDEBAR_INSERT_DEFAULTSHAPESPANEL_HXX

#include <vcl/ctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/toolbox.hxx>

#include <map>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>
#include <svtools/valueset.hxx>

class ToolBox;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace sd { namespace sidebar {

/** This panel provides buttons for inserting shapes into a document.
*/
class ShapesPanel
    : public PanelLayout
{
public:
    ShapesPanel (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~ShapesPanel();
    virtual void dispose() override;

private:
    VclPtr<ValueSet>          mpLineArrowSet;
    VclPtr<ValueSet>          mpCurveSet;
    VclPtr<ValueSet>          mpConnectorSet;
    VclPtr<ValueSet>          mpBasicShapeSet;
    VclPtr<ValueSet>          mpSymbolShapeSet;
    VclPtr<ValueSet>          mpBlockArrowSet;
    VclPtr<ValueSet>          mpFlowchartSet;
    VclPtr<ValueSet>          mpCalloutSet;
    VclPtr<ValueSet>          mpStarSet;
    VclPtr<ValueSet>          mp3DObjectSet;
    VclPtr<VclScrolledWindow> mpScrollWindow;
    Reference< XFrame >       mxFrame;

    void populateShapes();
    void Initialize();
    DECL_LINK_TYPED( LineSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( CurveSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( ConnectorSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( BasicShapeSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( SymbolSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( BlockArrowSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( FlowchartSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( CalloutSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( StarSelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( T3DObjectSelectHdl, ValueSet*, void );
};


} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */