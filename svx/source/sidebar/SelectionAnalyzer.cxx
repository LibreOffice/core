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

#include <svx/sidebar/SelectionAnalyzer.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <svx/fontworkbar.hxx>

using vcl::EnumContext;

namespace svx::sidebar
{
EnumContext::Context SelectionAnalyzer::GetContextForSelection_SC(const SdrMarkList& rMarkList)
{
    EnumContext::Context eContext = EnumContext::Context::Unknown;

    switch (rMarkList.GetMarkCount())
    {
        case 0:
            // Empty selection.  Return Context::Unknown to let the caller
            // substitute it with the default context.
            break;

        case 1:
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            auto pTextObj = DynCastSdrTextObj(pObj);
            if (pTextObj && pTextObj->IsInEditMode())
            {
                eContext = EnumContext::Context::DrawText;
            }
            else if (svx::checkForFontWork(pObj))
            {
                eContext = EnumContext::Context::DrawFontwork;
            }
            else
            {
                const SdrInventor nInv = pObj->GetObjInventor();
                const SdrObjKind nObjId = pObj->GetObjIdentifier();
                if (nInv == SdrInventor::Default)
                    eContext = GetContextForObjectId_SC(nObjId);
                else if (nInv == SdrInventor::FmForm)
                    eContext = EnumContext::Context::Form;
            }
            break;
        }

        default:
        {
            // Multi selection.
            switch (GetInventorTypeFromMark(rMarkList))
            {
                case SdrInventor::Default:
                {
                    const SdrObjKind nObjId(GetObjectTypeFromMark(rMarkList));
                    if (nObjId == SdrObjKind::NONE)
                        eContext = EnumContext::Context::MultiObject;
                    else
                        eContext = GetContextForObjectId_SC(nObjId);
                    break;
                }

                case SdrInventor::FmForm:
                    eContext = EnumContext::Context::Form;
                    break;

                case SdrInventor::Unknown:
                    eContext = EnumContext::Context::MultiObject;
                    break;

                default:
                    break;
            }
        }
    }

    return eContext;
}

EnumContext::Context SelectionAnalyzer::GetContextForSelection_SD(const SdrMarkList& rMarkList,
                                                                  const ViewType eViewType)
{
    EnumContext::Context eContext = EnumContext::Context::Unknown;

    // Note that some cases are handled by the caller.  They rely on
    // sd specific data.
    switch (rMarkList.GetMarkCount())
    {
        case 0:
            switch (eViewType)
            {
                case ViewType::Standard:
                    eContext = EnumContext::Context::DrawPage;
                    break;
                case ViewType::Master:
                    eContext = EnumContext::Context::MasterPage;
                    break;
                case ViewType::Handout:
                    eContext = EnumContext::Context::HandoutPage;
                    break;
                case ViewType::Notes:
                    eContext = EnumContext::Context::NotesPage;
                    break;
            }
            break;

        case 1:
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            auto pTextObj = DynCastSdrTextObj(pObj);
            if (pTextObj && pTextObj->IsInEditMode())
            {
                if (pObj->GetObjIdentifier() == SdrObjKind::Table)
                {
                    // Let a table object take precedence over text
                    // edit mode.  The panels for text editing are
                    // present for table context as well, anyway.
                    eContext = EnumContext::Context::Table;
                }
                else
                    eContext = EnumContext::Context::DrawText;
            }
            else if (svx::checkForFontWork(pObj))
            {
                eContext = EnumContext::Context::DrawFontwork;
            }
            else
            {
                const SdrInventor nInv = pObj->GetObjInventor();
                SdrObjKind nObjId = pObj->GetObjIdentifier();
                if (nInv == SdrInventor::Default)
                {
                    if (nObjId == SdrObjKind::Group)
                    {
                        nObjId = GetObjectTypeFromGroup(pObj);
                        if (nObjId == SdrObjKind::NONE)
                            nObjId = SdrObjKind::Group;
                    }
                    eContext = GetContextForObjectId_SD(nObjId, eViewType);
                }
                else if (nInv == SdrInventor::E3d)
                {
                    eContext = EnumContext::Context::ThreeDObject;
                }
                else if (nInv == SdrInventor::FmForm)
                {
                    eContext = EnumContext::Context::Form;
                }
            }
            break;
        }

        default:
        {
            switch (GetInventorTypeFromMark(rMarkList))
            {
                case SdrInventor::Default:
                {
                    const SdrObjKind nObjId = GetObjectTypeFromMark(rMarkList);
                    if (nObjId == SdrObjKind::NONE)
                        eContext = EnumContext::Context::MultiObject;
                    else
                        eContext = GetContextForObjectId_SD(nObjId, eViewType);
                    break;
                }

                case SdrInventor::E3d:
                    eContext = EnumContext::Context::ThreeDObject;
                    break;

                case SdrInventor::FmForm:
                    eContext = EnumContext::Context::Form;
                    break;

                case SdrInventor::Unknown:
                    eContext = EnumContext::Context::MultiObject;
                    break;

                default:
                    break;
            }
            break;
        }
    }

    return eContext;
}

EnumContext::Context SelectionAnalyzer::GetContextForObjectId_SC(const SdrObjKind nObjectId)
{
    switch (nObjectId)
    {
        case SdrObjKind::Caption:
        case SdrObjKind::TitleText:
        case SdrObjKind::OutlineText:
        case SdrObjKind::Text:
        case SdrObjKind::Measure:
        case SdrObjKind::Rectangle:
        case SdrObjKind::CircleOrEllipse:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::PathFill:
        case SdrObjKind::Polygon:
        case SdrObjKind::CircleSection:
        case SdrObjKind::CircleArc:
        case SdrObjKind::CircleCut:
        case SdrObjKind::CustomShape:
        case SdrObjKind::Group:
            return EnumContext::Context::Draw;

        case SdrObjKind::PolyLine:
        case SdrObjKind::PathLine:
        case SdrObjKind::FreehandLine:
        case SdrObjKind::Line:
        case SdrObjKind::Edge:
            return EnumContext::Context::DrawLine;

        case SdrObjKind::Graphic:
            return EnumContext::Context::Graphic;

        case SdrObjKind::OLE2:
            return EnumContext::Context::OLE;

        case SdrObjKind::Media:
            return EnumContext::Context::Media;

        default:
            return EnumContext::Context::Unknown;
    }
}

EnumContext::Context SelectionAnalyzer::GetContextForObjectId_SD(const SdrObjKind nObjectId,
                                                                 const ViewType eViewType)
{
    switch (nObjectId)
    {
        case SdrObjKind::Caption:
        case SdrObjKind::Measure:
        case SdrObjKind::Rectangle:
        case SdrObjKind::CircleOrEllipse:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::PathFill:
        case SdrObjKind::Polygon:
        case SdrObjKind::CircleSection:
        case SdrObjKind::CircleArc:
        case SdrObjKind::CircleCut:
        case SdrObjKind::CustomShape:
        case SdrObjKind::Group:
            return EnumContext::Context::Draw;

        case SdrObjKind::Edge:
        case SdrObjKind::PathLine:
        case SdrObjKind::FreehandLine:
        case SdrObjKind::PolyLine:
        case SdrObjKind::Line:
            return EnumContext::Context::DrawLine;

        case SdrObjKind::TitleText:
        case SdrObjKind::OutlineText:
        case SdrObjKind::Text:
            return EnumContext::Context::TextObject;

        case SdrObjKind::Graphic:
            return EnumContext::Context::Graphic;

        case SdrObjKind::OLE2:
            return EnumContext::Context::OLE;

        case SdrObjKind::Media:
            return EnumContext::Context::Media;

        case SdrObjKind::Table:
            return EnumContext::Context::Table;

        case SdrObjKind::Page:
            switch (eViewType)
            {
                case ViewType::Handout:
                    return EnumContext::Context::HandoutPage;
                case ViewType::Notes:
                    return EnumContext::Context::NotesPage;
                default:
                    return EnumContext::Context::Unknown;
            }

        default:
            return EnumContext::Context::Unknown;
    }
}

SdrInventor SelectionAnalyzer::GetInventorTypeFromMark(const SdrMarkList& rMarkList)
{
    const size_t nMarkCount(rMarkList.GetMarkCount());

    if (nMarkCount < 1)
        return SdrInventor::Unknown;

    SdrMark* pMark = rMarkList.GetMark(0);
    SdrObject* pObj = pMark->GetMarkedSdrObj();
    const SdrInventor nFirstInv = pObj->GetObjInventor();

    for (size_t nIndex = 1; nIndex < nMarkCount; ++nIndex)
    {
        pMark = rMarkList.GetMark(nIndex);
        pObj = pMark->GetMarkedSdrObj();
        const SdrInventor nInv(pObj->GetObjInventor());

        if (nInv != nFirstInv)
            return SdrInventor::Unknown;
    }

    return nFirstInv;
}

SdrObjKind SelectionAnalyzer::GetObjectTypeFromGroup(const SdrObject* pObj)
{
    SdrObjList* pObjList = pObj->GetSubList();
    if (pObjList)
    {
        const size_t nSubObjCount(pObjList->GetObjCount());

        if (nSubObjCount > 0)
        {
            SdrObject* pSubObj = pObjList->GetObj(0);
            SdrObjKind nResultType = pSubObj->GetObjIdentifier();

            if (nResultType == SdrObjKind::Group)
                nResultType = GetObjectTypeFromGroup(pSubObj);

            if (IsShapeType(nResultType))
                nResultType = SdrObjKind::CustomShape;

            if (IsTextObjType(nResultType))
                nResultType = SdrObjKind::Text;

            for (size_t nIndex = 1; nIndex < nSubObjCount; ++nIndex)
            {
                pSubObj = pObjList->GetObj(nIndex);
                SdrObjKind nType(pSubObj->GetObjIdentifier());

                if (nType == SdrObjKind::Group)
                    nType = GetObjectTypeFromGroup(pSubObj);

                if (IsShapeType(nType))
                    nType = SdrObjKind::CustomShape;

                if ((nType == SdrObjKind::CustomShape) && (nResultType == SdrObjKind::Text))
                    nType = SdrObjKind::Text;

                if (IsTextObjType(nType))
                    nType = SdrObjKind::Text;

                if ((nType == SdrObjKind::Text) && (nResultType == SdrObjKind::CustomShape))
                    nResultType = SdrObjKind::Text;

                if (nType != nResultType)
                    return SdrObjKind::NONE;
            }

            return nResultType;
        }
    }

    return SdrObjKind::NONE;
}

SdrObjKind SelectionAnalyzer::GetObjectTypeFromMark(const SdrMarkList& rMarkList)
{
    const size_t nMarkCount(rMarkList.GetMarkCount());

    if (nMarkCount < 1)
        return SdrObjKind::NONE;

    SdrMark* pMark = rMarkList.GetMark(0);
    SdrObject* pObj = pMark->GetMarkedSdrObj();
    SdrObjKind nResultType = pObj->GetObjIdentifier();

    if (nResultType == SdrObjKind::Group)
        nResultType = GetObjectTypeFromGroup(pObj);

    if (IsShapeType(nResultType))
        nResultType = SdrObjKind::CustomShape;

    if (IsTextObjType(nResultType))
        nResultType = SdrObjKind::Text;

    for (size_t nIndex = 1; nIndex < nMarkCount; ++nIndex)
    {
        pMark = rMarkList.GetMark(nIndex);
        pObj = pMark->GetMarkedSdrObj();
        SdrObjKind nType = pObj->GetObjIdentifier();

        if (nType == SdrObjKind::Group)
            nType = GetObjectTypeFromGroup(pObj);

        if (IsShapeType(nType))
            nType = SdrObjKind::CustomShape;

        if ((nType == SdrObjKind::CustomShape) && (nResultType == SdrObjKind::Text))
            nType = SdrObjKind::Text;

        if (IsTextObjType(nType))
            nType = SdrObjKind::Text;

        if ((nType == SdrObjKind::Text) && (nResultType == SdrObjKind::CustomShape))
            nResultType = SdrObjKind::Text;

        if (nType != nResultType)
            return SdrObjKind::NONE;
    }

    return nResultType;
}

bool SelectionAnalyzer::IsShapeType(const SdrObjKind nType)
{
    switch (nType)
    {
        case SdrObjKind::Line:
        case SdrObjKind::CircleArc:
        case SdrObjKind::PolyLine:
        case SdrObjKind::PathLine:
        case SdrObjKind::Rectangle:
        case SdrObjKind::CircleOrEllipse:
        case SdrObjKind::CircleSection:
        case SdrObjKind::CircleCut:
        case SdrObjKind::PathFill:
        case SdrObjKind::CustomShape:
        case SdrObjKind::Caption:
        case SdrObjKind::Measure:
        case SdrObjKind::Edge:
        case SdrObjKind::Polygon:
        case SdrObjKind::FreehandLine:
        case SdrObjKind::FreehandFill:

        // #122145# adding SdrObjKind::OLE2 since these also allow line/fill style and may
        // be multiselected/grouped with normal draw objects, e.g. math OLE objects
        case SdrObjKind::OLE2:
            return true;

        default:
            return false;
    }
}

bool SelectionAnalyzer::IsTextObjType(const SdrObjKind nType)
{
    switch (nType)
    {
        case SdrObjKind::Text:
        case SdrObjKind::TitleText:
        case SdrObjKind::OutlineText:
            return true;

        default:
            return false;
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
