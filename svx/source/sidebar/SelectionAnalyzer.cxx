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

#include "svx/sidebar/SelectionAnalyzer.hxx"
#include "svx/svdmrkv.hxx"
#include "svx/svdobj.hxx"
#include "svx/svdotext.hxx"
#include "svx/svdpage.hxx"
#include "svx/fmglob.hxx"
#include "svx/globl3d.hxx"

using sfx2::sidebar::EnumContext;


namespace svx { namespace sidebar {

EnumContext::Context SelectionAnalyzer::GetContextForSelection_SC (const SdrMarkList& rMarkList)
{
    EnumContext::Context eContext = EnumContext::Context_Unknown;

    switch (rMarkList.GetMarkCount())
    {
        case 0:
            // Empty selection.  Return Context_Unknown to let the caller
            // substitute it with the default context.
            break;

        case 1:
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if ( dynamic_cast<const SdrTextObj*>( pObj) != nullptr && static_cast<SdrTextObj*>(pObj)->IsInEditMode() )
            {
                eContext = EnumContext::Context_DrawText;
            }
            else
            {
                const sal_uInt32 nInv = pObj->GetObjInventor();
                const sal_uInt16 nObjId = pObj->GetObjIdentifier();
                if (nInv == SdrInventor)
                    eContext = GetContextForObjectId_SC(nObjId);
                else if (nInv == FmFormInventor)
                    eContext = EnumContext::Context_Form;
            }
            break;
        }

        default:
        {
            // Multi selection.
            switch (GetInventorTypeFromMark(rMarkList))
            {
                case SdrInventor:
                {
                    const sal_uInt16 nObjId (GetObjectTypeFromMark(rMarkList));
                    if (nObjId == 0)
                        eContext = EnumContext::Context_MultiObject;
                    else
                        eContext = GetContextForObjectId_SC(nObjId);
                    break;
                }

                case FmFormInventor:
                    eContext = EnumContext::Context_Form;
                    break;

                case 0:
                    eContext = EnumContext::Context_MultiObject;
                    break;
            }
        }
    }

    return eContext;
}




EnumContext::Context SelectionAnalyzer::GetContextForSelection_SD (
    const SdrMarkList& rMarkList,
    const ViewType eViewType)
{
    EnumContext::Context eContext = EnumContext::Context_Unknown;

    // Note that some cases are handled by the caller.  They rely on
    // sd specific data.
    switch (rMarkList.GetMarkCount())
    {
        case 0:
            switch(eViewType)
            {
                case VT_Standard:
                    eContext = EnumContext::Context_DrawPage;
                    break;
                case VT_Master:
                    eContext = EnumContext::Context_MasterPage;
                    break;
                case VT_Handout:
                    eContext = EnumContext::Context_HandoutPage;
                    break;
                case VT_Notes:
                    eContext = EnumContext::Context_NotesPage;
                    break;
                case VT_Outline:
                    eContext = EnumContext::Context_OutlineText;
                    break;
            }
            break;

        case 1:
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if (dynamic_cast<const SdrTextObj*>( pObj) != nullptr && static_cast<SdrTextObj*>(pObj)->IsInEditMode())
            {
                if (pObj->GetObjIdentifier() == OBJ_TABLE)
                {
                    // Let a table object take precedence over text
                    // edit mode.  The panels for text editing are
                    // present for table context as well, anyway.
                    eContext = EnumContext::Context_Table;
                }
                else
                    eContext = EnumContext::Context_DrawText;
            }
            else
            {
                const sal_uInt32 nInv = pObj->GetObjInventor();
                sal_uInt16 nObjId = pObj->GetObjIdentifier();
                if (nInv == SdrInventor)
                {
                    if (nObjId == OBJ_GRUP)
                    {
                        nObjId = GetObjectTypeFromGroup(pObj);
                        if (nObjId == 0)
                            nObjId = OBJ_GRUP;
                    }
                    eContext = GetContextForObjectId_SD(nObjId, eViewType);
                }
                else if (nInv == E3dInventor)
                {
                    eContext = EnumContext::Context_3DObject;
                }
                else if (nInv == FmFormInventor)
                {
                    eContext = EnumContext::Context_Form;
                }
            }
            break;
        }

        default:
        {
            switch (GetInventorTypeFromMark(rMarkList))
            {
                case SdrInventor:
                {
                    const sal_uInt16 nObjId = GetObjectTypeFromMark(rMarkList);
                    if (nObjId == 0)
                        eContext = EnumContext::Context_MultiObject;
                    else
                        eContext = GetContextForObjectId_SD(nObjId, eViewType);
                    break;
                }

                case E3dInventor:
                    eContext = EnumContext::Context_3DObject;
                    break;

                case FmFormInventor:
                    eContext = EnumContext::Context_Form;
                    break;

                case 0:
                    eContext = EnumContext::Context_MultiObject;
                    break;
            }
            break;
        }
    }

    return eContext;
}




EnumContext::Context SelectionAnalyzer::GetContextForObjectId_SC (const sal_uInt16 nObjectId)
{
    switch (nObjectId)
    {
        case OBJ_CAPTION:
        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
        case OBJ_TEXT:
        case OBJ_TEXTEXT:
        case OBJ_MEASURE:
        case OBJ_RECT:
        case OBJ_CIRC:
        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
        case OBJ_POLY:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
        case OBJ_CUSTOMSHAPE:
        case OBJ_GRUP:
            return EnumContext::Context_Draw;

        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_FREELINE:
        case OBJ_LINE:
        case OBJ_EDGE:
            return EnumContext::Context_DrawLine;

        case OBJ_GRAF:
            return EnumContext::Context_Graphic;

        case OBJ_OLE2:
            return EnumContext::Context_OLE;

        case OBJ_MEDIA:
            return EnumContext::Context_Media;
            break;

        default:
            return EnumContext::Context_Unknown;
    }
}




EnumContext::Context SelectionAnalyzer::GetContextForObjectId_SD (
    const sal_uInt16 nObjectId,
    const ViewType eViewType)
{
    switch (nObjectId)
    {
        case OBJ_CAPTION:
        case OBJ_MEASURE:
        case OBJ_RECT:
        case OBJ_CIRC:
        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
        case OBJ_POLY:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
        case OBJ_CUSTOMSHAPE:
        case OBJ_GRUP:
            return EnumContext::Context_Draw;

        case OBJ_EDGE:
        case OBJ_PATHLINE:
        case OBJ_FREELINE:
        case OBJ_PLIN:
        case OBJ_LINE:
            return EnumContext::Context_DrawLine;

        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
        case OBJ_TEXT:
        case OBJ_TEXTEXT:
            return EnumContext::Context_TextObject;

        case OBJ_GRAF:
            return EnumContext::Context_Graphic;

        case OBJ_OLE2:
            return EnumContext::Context_OLE;

        case OBJ_MEDIA:
            return EnumContext::Context_Media;

        case OBJ_TABLE:
            return EnumContext::Context_Table;

        case OBJ_PAGE:
            switch (eViewType)
            {
                case VT_Handout:
                    return EnumContext::Context_HandoutPage;
                case VT_Notes:
                    return EnumContext::Context_NotesPage;
                case VT_Outline:
                    return EnumContext::Context_OutlineText;
                default:
                    return EnumContext::Context_Unknown;
            }

        default:
            return EnumContext::Context_Unknown;
    }
}




sal_uInt32 SelectionAnalyzer::GetInventorTypeFromMark (const SdrMarkList& rMarkList)
{
    const size_t nMarkCount (rMarkList.GetMarkCount());

    if (nMarkCount < 1)
        return 0;

    SdrMark* pMark = rMarkList.GetMark(0);
    SdrObject* pObj = pMark->GetMarkedSdrObj();
    const sal_uInt32 nFirstInv = pObj->GetObjInventor();

    for (size_t nIndex=1; nIndex<nMarkCount; ++nIndex)
    {
        pMark = rMarkList.GetMark(nIndex);
        pObj = pMark->GetMarkedSdrObj();
        const sal_uInt32 nInv (pObj->GetObjInventor());

        if (nInv != nFirstInv)
            return 0;
    }

    return nFirstInv;
}




sal_uInt16 SelectionAnalyzer::GetObjectTypeFromGroup (const SdrObject* pObj)
{
    SdrObjList* pObjList = pObj->GetSubList();
    if (pObjList)
    {
        const size_t nSubObjCount (pObjList->GetObjCount());

        if (nSubObjCount>0)
        {
            SdrObject* pSubObj = pObjList->GetObj(0);
            sal_uInt16 nResultType = pSubObj->GetObjIdentifier();

            if (nResultType == OBJ_GRUP)
                nResultType = GetObjectTypeFromGroup(pSubObj);

            if (IsShapeType(nResultType))
                nResultType = OBJ_CUSTOMSHAPE;

            if (IsTextObjType(nResultType))
                nResultType = OBJ_TEXT;

            for (size_t nIndex=1; nIndex<nSubObjCount; ++nIndex)
            {
                pSubObj = pObjList->GetObj(nIndex);
                sal_uInt16 nType (pSubObj->GetObjIdentifier());

                if(nType == OBJ_GRUP)
                    nType = GetObjectTypeFromGroup(pSubObj);

                if (IsShapeType(nType))
                    nType = OBJ_CUSTOMSHAPE;

                if ((nType == OBJ_CUSTOMSHAPE) && (nResultType == OBJ_TEXT))
                    nType = OBJ_TEXT;

                if (IsTextObjType(nType))
                    nType = OBJ_TEXT;

                if ((nType == OBJ_TEXT) && (nResultType == OBJ_CUSTOMSHAPE))
                    nResultType = OBJ_TEXT;

                if (nType != nResultType)
                    return 0;
            }

            return nResultType;
        }
    }

    return 0;
}




sal_uInt16  SelectionAnalyzer::GetObjectTypeFromMark (const SdrMarkList& rMarkList)
{
    const size_t nMarkCount (rMarkList.GetMarkCount());

    if (nMarkCount < 1)
        return 0;

    SdrMark* pMark = rMarkList.GetMark(0);
    SdrObject* pObj = pMark->GetMarkedSdrObj();
    sal_uInt16 nResultType = pObj->GetObjIdentifier();

    if(nResultType == OBJ_GRUP)
        nResultType = GetObjectTypeFromGroup(pObj);

    if (IsShapeType(nResultType))
        nResultType = OBJ_CUSTOMSHAPE;

    if (IsTextObjType(nResultType))
        nResultType = OBJ_TEXT;

    for (size_t nIndex=1; nIndex<nMarkCount; ++nIndex)
    {
        pMark = rMarkList.GetMark(nIndex);
        pObj = pMark->GetMarkedSdrObj();
        sal_uInt16 nType = pObj->GetObjIdentifier();

        if(nType == OBJ_GRUP)
            nType = GetObjectTypeFromGroup(pObj);

        if (IsShapeType(nType))
            nType = OBJ_CUSTOMSHAPE;

        if ((nType == OBJ_CUSTOMSHAPE) && (nResultType == OBJ_TEXT))
            nType = OBJ_TEXT;

        if (IsTextObjType(nType))
            nType = OBJ_TEXT;

        if ((nType == OBJ_TEXT) && (nResultType == OBJ_CUSTOMSHAPE))
            nResultType = OBJ_TEXT;

        if (nType != nResultType)
            return 0;
    }

    return nResultType;
}




bool SelectionAnalyzer::IsShapeType (const sal_uInt16 nType)
{
    switch (nType)
    {
        case OBJ_LINE:
        case OBJ_CARC:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_RECT:
        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CCUT:
        case OBJ_PATHFILL:
        case OBJ_CUSTOMSHAPE:
        case OBJ_CAPTION:
        case OBJ_MEASURE:
        case OBJ_EDGE:
        case OBJ_POLY:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:

        // #122145# adding OBJ_OLE2 since these also allow line/fill style and may
        // be multiselected/grouped with normal draw objects, e.g. math OLE objects
        case OBJ_OLE2:
            return true;

        default:
            return false;
    }
}




bool SelectionAnalyzer::IsTextObjType (const sal_uInt16 nType)
{
    switch(nType)
    {
        case OBJ_TEXT:
        case OBJ_TEXTEXT:
        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
            return true;

        default:
            return false;
    }
}



} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
