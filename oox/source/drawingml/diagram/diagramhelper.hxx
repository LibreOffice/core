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

#ifndef INCLUDED_OOX_DRAWINGML_DIAGRAM_DIAGRAMHELPER_HXX
#define INCLUDED_OOX_DRAWINGML_DIAGRAM_DIAGRAMHELPER_HXX

#include <rtl/ustring.hxx>
#include <oox/drawingml/theme.hxx>
#include <svx/svdogrp.hxx>

namespace oox::drawingml {

class Diagram;

// Advanced DiagramHelper
//
// This helper tries to hold all neccessary data to re-layout
// all XShapes/SdrObjects of an already imported Diagram. The
// Diagram holds the SmarArt model data before it gets layouted,
// while Theme holds the oox Fill/Line/Style definitions to
// apply.
// Re-Layouting (re-reating) is rather complex, for detailed
// information see ::reLayout implementation.
// This helper class may/should be extended to:
// - deliver representative data from the Diagram-Model
// - modify it eventually
// - im/export Diagram model to other representations
class AdvancedDiagramHelper final : public DiagramHelper
{
    const std::shared_ptr< Diagram >                mpDiagramPtr;
    const std::shared_ptr<::oox::drawingml::Theme>  mpThemePtr;

public:
    AdvancedDiagramHelper(
        const std::shared_ptr< Diagram >& rDiagramPtr,
        const std::shared_ptr<::oox::drawingml::Theme>& rTheme);
    virtual ~AdvancedDiagramHelper();

    virtual void reLayout();
    void doAnchor(SdrObjGroup& rTarget);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
