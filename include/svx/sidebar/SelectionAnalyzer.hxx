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
#ifndef INCLUDED_SVX_SIDEBAR_SELECTIONANALYZER_HXX
#define INCLUDED_SVX_SIDEBAR_SELECTIONANALYZER_HXX

#include <vcl/EnumContext.hxx>
#include <svx/svxdllapi.h>

class SdrMarkList;
class SdrObject;
enum class SdrInventor : sal_uInt32;

namespace svx { namespace sidebar {

/** Analyze the current selection of Calc or Draw/Impress documents
    and return the associated sidebar context.

    The decision is based on heuristics.  Do not expect pretty code.
*/
class SVX_DLLPUBLIC SelectionAnalyzer
{
public:
    static vcl::EnumContext::Context GetContextForSelection_SC (
        const SdrMarkList& rMarkList);

    enum class ViewType
    {
        Standard,
        Master,
        Handout,
        Notes,
        MasterNotes,
    };
    static vcl::EnumContext::Context GetContextForSelection_SD (
        const SdrMarkList& rMarkList,
        const ViewType eViewType);

private:
    static vcl::EnumContext::Context GetContextForObjectId_SC (
        const sal_uInt16 nObjectId);
    static vcl::EnumContext::Context GetContextForObjectId_SD (
        const sal_uInt16 nObjectId,
        const ViewType eViewType);
    static SdrInventor GetInventorTypeFromMark (
        const SdrMarkList& rMarkList);
    static sal_uInt16 GetObjectTypeFromMark (
        const SdrMarkList& rMarkList);
    static sal_uInt16 GetObjectTypeFromGroup (
        const SdrObject* pObj);
    static bool IsShapeType (
        const sal_uInt16 nType);
    static bool IsTextObjType (
        const sal_uInt16 nType);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
