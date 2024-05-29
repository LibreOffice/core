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

#pragma once

#include <vector>
#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>
#include <svx/svdhdl.hxx>

// Forward declarations
class SdrObjGroup;
class SdrHdlList;

namespace svx { namespace diagram {

// Helper class to visualize an imminently recognizable
// additional visualization for DynamicDiagrams that can also
// be used to show/hide the DiagramDialog by the user
// Note: is also used as additional drag/move handle
class SVXCORE_DLLPUBLIC DiagramFrameHdl final : public SdrHdl
{
    // object dimensions
    basegfx::B2DHomMatrix maTransformation;

    // create marker for this kind
    virtual void CreateB2dIAObject() override;

public:
    DiagramFrameHdl(const basegfx::B2DHomMatrix& rTransformation);

    static void clicked(const Point& rPnt);
};

class DiagramDataState;

// Helper class to allow administer advanced Diagram related
// data and functionality
class SVXCORE_DLLPUBLIC IDiagramHelper
{
private:
    // These values define behaviour to where take data from at re-creation time.
    // Different definitions will have different consequences for re-creation
    // of Diagram visualization (if needed/triggered).
    // The style attributes per shape e.g. can be re-stored frm either an
    // existing Theme, or the preserved key/value list of properties per XShape.
    // With the current default settings the re-creation uses the preserved
    // key/value pairs, but re-creation from Theme may also be desirable. It
    // is also good to preserve both data packages at initial import to allow
    // alternatively one of these two methods for re-construction

    // If true, the oox::Theme data from ::DiagramData get/set/ThemeDocument()
    // aka mxThemeDocument - if it exists - will be used to create the style
    // attributes for the to-be-created XShapes (theoretically allows re-creation
    // with other Theme)
    bool mbUseDiagramThemeData; // false

    // If true, the UNO API form of attributes per Point as Key/value list
    // that was secured after initial XShape creation is used to create the
    // style attributes for the to-be-created XShapes
    bool mbUseDiagramModelData; // true

    // If true and mxThemeDocument exists it will be re-imported to
    // a newly created oox::drawingml::Theme object
    bool mbForceThemePtrRecreation; // false

    // if true, content was self-created using addTo/addShape
    // and the layouting stuff
    bool mbSelfCreated;

protected:
    void anchorToSdrObjGroup(SdrObjGroup& rTarget);

public:
    IDiagramHelper(bool bSelfCreated);
    virtual ~IDiagramHelper();

    // re-create XShapes
    virtual void reLayout(SdrObjGroup& rTarget) = 0;

    // get text representation of data tree
    virtual OUString getString() const = 0;

    // get children of provided data node
    // use empty string for top-level nodes
    // returns vector of (id, text)
    virtual std::vector<std::pair<OUString, OUString>>
    getChildren(const OUString& rParentId) const = 0;

    // add/remove new top-level node to data model, returns its id
    virtual OUString addNode(const OUString& rText) = 0;
    virtual bool removeNode(const OUString& rNodeId) = 0;

    // Undo/Redo helpers for extracting/restoring Diagram-defining data
    virtual std::shared_ptr<svx::diagram::DiagramDataState> extractDiagramDataState() const = 0;
    virtual void applyDiagramDataState(const std::shared_ptr<svx::diagram::DiagramDataState>& rState) = 0;

    bool UseDiagramThemeData() const { return mbUseDiagramThemeData; }
    bool UseDiagramModelData() const { return mbUseDiagramModelData; }
    bool ForceThemePtrRecreation() const { return mbForceThemePtrRecreation; };

    // get/set SelfCreated flag
    bool isSelfCreated() const { return mbSelfCreated; }
    void setSelfCreated() { mbSelfCreated = true; }

    static void AddAdditionalVisualization(const SdrObjGroup& rTarget, SdrHdlList& rHdlList);
};

}} // end of namespace
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
