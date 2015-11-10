/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/

#include "lwpfoundry.hxx"
#include "lwpfilehdr.hxx"
#include "lwpdoc.hxx"
#include "lwpmarker.hxx"
#include "lwpholder.hxx"
#include "lwpbulletstylemgr.hxx"
#include "lwpcontent.hxx"
#include "lwpvpointer.hxx"
#include "lwpsection.hxx"
#include "lwpcharacterstyle.hxx"
#include "lwpglobalmgr.hxx"

#include <osl/diagnose.h>


LwpFoundry::LwpFoundry(LwpObjectStream *pStrm, LwpDocument* pDoc)
    : m_pDoc(pDoc)
    , m_bRegisteredAll(false)
    , m_pPieceMgr(nullptr)
    , m_pStyleMgr(nullptr)
{
    Read(pStrm);
    m_pDropcapMgr = new LwpDropcapMgr;
    m_pBulletStyleMgr = new LwpBulletStyleMgr();
    m_pBulletStyleMgr->SetFoundry(this);
}

LwpFoundry::~LwpFoundry()
{
    delete m_pPieceMgr;
    delete m_pStyleMgr;
    delete m_pDropcapMgr;
    delete m_pBulletStyleMgr;
}

void LwpFoundry::Read(LwpObjectStream *pStrm)
{
    if (!m_pDoc->IsChildDoc())
    {
        LwpVersionManager::Read(pStrm);
    }
    m_ObjMgr.Read(pStrm);

    m_MarkerHead.ReadIndexed(pStrm);
    m_FootnoteMgr.ReadIndexed(pStrm);

    m_NumMgr.Read(pStrm);
    m_BulMgr.Read(pStrm);

    m_SectionList.Read(pStrm);
    m_Layout.ReadIndexed(pStrm);

    ReadStyles(pStrm);

    m_BookMarkHead.ReadIndexed(pStrm);
    m_DdeLinkHead.ReadIndexed(pStrm);
    m_DirtBagHead.ReadIndexed(pStrm);
    m_NamedOutlineSeqHead.ReadIndexed(pStrm);

    m_EnumLayoutHead.ReadIndexed(pStrm);
    m_EnumLayoutTail.ReadIndexed(pStrm);
    m_NamedObjects.ReadIndexed(pStrm);

    m_nLastClickHere = pStrm->QuickReaduInt32();
    m_SmartTextMgr.ReadIndexed(pStrm);

    m_ContentMgr.Read(pStrm);
    m_FontMgr.Read(pStrm);

    if (!m_pDoc->IsChildDoc() && LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_pPieceMgr = new LwpPieceManager();

        m_pPieceMgr->Read(pStrm);
    }

    if( LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_DftDropCapStyle.ReadIndexed(pStrm);
    }
    if( LwpFileHeader::m_nFileRevision >= 0x000F)
    {
        m_DftHeaderStyle.ReadIndexed(pStrm);
        m_DftFooterStyle.ReadIndexed(pStrm);
    }
    pStrm->SkipExtra();

    m_pStyleMgr = new LwpStyleManager();
    m_pStyleMgr->SetFoundry(this);
}

void LwpFoundry::ReadStyles(LwpObjectStream *pStrm)
{
    m_TextStyle.ReadIndexed(pStrm);
    m_DefaultTextStyle.ReadIndexed(pStrm);
    m_DefaultClickStyle.ReadIndexed(pStrm);
    m_PageStyle.ReadIndexed(pStrm);
    m_FrameStyle.ReadIndexed(pStrm);

    m_TableStyle.ReadIndexed(pStrm);
    m_CellStyle.ReadIndexed(pStrm);
    m_DftFrameStyle.ReadIndexed(pStrm);
    m_DftPageStyle.ReadIndexed(pStrm);
    m_DftTableStyle.ReadIndexed(pStrm);

    m_DftCellStyle.ReadIndexed(pStrm);
    m_DftColumnStyle.ReadIndexed(pStrm);
    m_DftLeftColumnStyle.ReadIndexed(pStrm);
    m_DftRighColumnStyle.ReadIndexed(pStrm);

}

#include "xfilter/xfstylemanager.hxx"
#include "lwplayout.hxx"

void LwpFoundry::RegisterAllLayouts()
{
    if (m_bRegisteredAll)
    {
        OSL_FAIL("recursive LwpFoundry::RegisterAllLayouts!\n");
        return;
    }

    m_bRegisteredAll = true;

    //Register CellStyle
    rtl::Reference<LwpObject> pStyle = m_CellStyle.obj();
    if( pStyle.is() )
    {
        pStyle->SetFoundry(this);
        pStyle->RegisterStyle();
    }

    //register content page layout list: Layout
    pStyle = m_Layout.obj();
    if( pStyle.is() )
    {
        pStyle->SetFoundry(this);
        pStyle->RegisterStyle();
    }

    //Register page style layout list: PageStyle, such as "Default Page"
    pStyle = m_PageStyle.obj();
    if( pStyle.is() )
    {
        pStyle->SetFoundry(this);
        pStyle->RegisterStyle();
    }

    //Register FrameStyle
    pStyle = m_FrameStyle.obj();
    if( pStyle.is() )
    {
        pStyle->SetFoundry(this);
        pStyle->RegisterStyle();
    }

}

LwpBookMark* LwpFoundry::GetBookMark(LwpObjectID objMarker)
{
    LwpDLVListHeadHolder* pHeadHolder= static_cast
                    <LwpDLVListHeadHolder*>(m_BookMarkHead.obj().get());
    LwpObjectID& rObjID = pHeadHolder->GetHeadID();
    LwpBookMark* pBookMark;
    pBookMark = static_cast<LwpBookMark*>(rObjID.obj().get());

    while (pBookMark)
    {
        if (pBookMark->IsRightMarker(objMarker))
            return pBookMark;
        rObjID = pBookMark->GetNext();
        pBookMark = static_cast<LwpBookMark*>(rObjID.obj().get());
    }
    return nullptr;
}

/**
* @descr:   Get next content
*
*/
LwpContent* LwpFoundry::EnumContents(LwpContent * pContent)
{
    return GetContentManager().EnumContents(pContent);
}

/**
* @descr:   Get next section
*
*/
LwpSection* LwpFoundry::EnumSections(LwpSection * pSection)
{
    return static_cast<LwpSection*>(m_SectionList.Enumerate(pSection));
}

/**
* @descr:   Get default text style id
*
*/
LwpObjectID * LwpFoundry::GetDefaultTextStyle()
{
    LwpVersionedPointer * pPointer = static_cast<LwpVersionedPointer *>(m_DefaultTextStyle.obj().get());
    if (!pPointer)
        return nullptr;

    return &pPointer->GetPointer();
}

/**
* @descr:   Get paragraph style object id according to its style name
*
*/
LwpObjectID * LwpFoundry::FindParaStyleByName(const OUString& name)
{
    //Register all text styles: para styles, character styles
    LwpDLVListHeadHolder* pParaStyleHolder = static_cast<LwpDLVListHeadHolder*>(GetTextStyleHead().obj().get());
    if(pParaStyleHolder)
    {
        LwpTextStyle* pParaStyle = static_cast<LwpTextStyle*> (pParaStyleHolder->GetHeadID().obj().get());
        while(pParaStyle)
        {
            OUString strName = pParaStyle->GetName().str();
            if(strName == name)
                return &pParaStyle->GetObjectID();
            pParaStyle = static_cast<LwpTextStyle*>(pParaStyle->GetNext().obj().get());
        }
    }

    return nullptr;
}

/**
* @descr:   Get style name registered according the original style name
*
*/
OUString LwpFoundry::FindActuralStyleName(const OUString& name)
{
    LwpObjectID* pID = FindParaStyleByName(name);
    if(pID)
    {
        IXFStyle* pStyle = GetStyleManager()->GetStyle(*pID);
        if(pStyle)
        {
            return pStyle->GetStyleName();
        }
    }

    return name;
}

void LwpVersionManager::Read(LwpObjectStream *pStrm)
{
    // TODO: skip the data for prototype
    Skip(pStrm);
}

void LwpVersionManager::Skip(LwpObjectStream *pStrm)
{
    pStrm->QuickReaduInt32();
    sal_uInt16 Count = pStrm->QuickReaduInt16();

    while(Count--)
    {
        sal_uInt32 tag = pStrm->QuickReaduInt32();
        switch(tag)
        {
            case TAG_USER_VERSION:
                // TODO: skip the CUserVersionControl
                pStrm->SeekRel(pStrm->QuickReaduInt16());
                break;

            default:
                pStrm->SeekRel(pStrm->QuickReaduInt16());
                pStrm->SkipExtra();
                break;
        }
    }
    pStrm->SkipExtra();
}

void LwpObjectManager::Read(LwpObjectStream *pStrm)
{

    LwpObjectID dummy;
    dummy.Read(pStrm);
    // TODO: judge if we need to set the cDelta by the dummy id

    m_Division.ReadIndexed(pStrm);
    pStrm->SkipExtra();
}

void LwpNumberManager::Read(LwpObjectStream *pStrm)
{
    m_TableRange.ReadIndexed(pStrm);
    pStrm->SkipExtra();
}

void LwpBulletManager::Read(LwpObjectStream *pStrm)
{
    m_Head.ReadIndexed(pStrm);
    pStrm->SkipExtra();
}

void LwpContentManager::Read(LwpObjectStream *pStrm)
{
    m_ContentList.ReadIndexed(pStrm);

    // TODO:  to judge the file revision

    m_EnumHead.ReadIndexed(pStrm);
    m_EnumTail.ReadIndexed(pStrm);

    m_OleObjCount.ReadIndexed(pStrm);

    if( LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_GrapHead.ReadIndexed(pStrm);
        m_GrapTail.ReadIndexed(pStrm);
        m_OleHead.ReadIndexed(pStrm);
        m_OleTail.ReadIndexed(pStrm);
    }

    pStrm->SkipExtra();
}

/**
* @descr:   Get next content
*
*/
LwpContent* LwpContentManager::EnumContents(LwpContent* pContent)
{
    if(pContent)
        return pContent->GetNextEnumerated();
    LwpVersionedPointer* pPointer = static_cast<LwpVersionedPointer*>(m_EnumHead.obj().get());
    return pPointer ? static_cast<LwpContent*>(pPointer->GetPointer().obj().get()) : nullptr;
}

void LwpPieceManager::Read(LwpObjectStream *pStrm)
{
    m_GeometryPieceList.ReadIndexed(pStrm);
    m_ScalePieceList.ReadIndexed(pStrm);
    m_MarginsPieceList.ReadIndexed(pStrm);
    m_ColumnsPieceList.ReadIndexed(pStrm);
    m_BorderStuffPieceList.ReadIndexed(pStrm);

    m_GutterStuffPieceList.ReadIndexed(pStrm);
    m_BackgroundStuffPieceList.ReadIndexed(pStrm);
    m_JoinStuffPieceList.ReadIndexed(pStrm);
    m_ShadowPieceList.ReadIndexed(pStrm);
    m_NumericsPieceList.ReadIndexed(pStrm);

    m_RelativityPieceList.ReadIndexed(pStrm);
    m_AlignmentPieceList.ReadIndexed(pStrm);
    m_IndentPieceList.ReadIndexed(pStrm);
    m_ParaBorderPieceList.ReadIndexed(pStrm);
    m_SpacingPieceList.ReadIndexed(pStrm);

    m_BreaksPieceList.ReadIndexed(pStrm);
    m_NumberingPieceList.ReadIndexed(pStrm);
    m_TabPieceList.ReadIndexed(pStrm);
    m_CharacterBorderPieceList.ReadIndexed(pStrm);
    m_AmikakePieceList.ReadIndexed(pStrm);

    if(pStrm->CheckExtra())
    {
        m_ParaBackgroundPieceList.ReadIndexed(pStrm);
        m_ExternalBorderStuffPieceList.ReadIndexed(pStrm);
        m_ExternalJoinStuffPieceList.ReadIndexed(pStrm);
        pStrm->SkipExtra();
    }
}

void LwpOrderedObjectManager::Read(LwpObjectStream *pStrm)
{
    m_Head.ReadIndexed(pStrm);
}

/**
* @descr:   Get next orderedobject, copy from lwp source code
*
*/
LwpOrderedObject* LwpOrderedObjectManager::Enumerate(LwpOrderedObject * pLast)
{
    // If Last has a next, return it.
    if(pLast && !pLast->GetNext().IsNull())
        return static_cast<LwpOrderedObject*>(pLast->GetNext().obj().get());

    LwpListList* pList = nullptr;
    if(pLast)
    {
        // We're at the end of Last's list (not Liszt's list).
        // Start with the next active list
        pList = static_cast<LwpListList*>(pLast->GetListList().obj().get());
        pList= GetNextActiveListList(pList);
    }
    else
    {
        // Start with the first active ListList
        pList = GetNextActiveListList(nullptr);
    }

    if(pList)
    {
        return static_cast<LwpOrderedObject*>(pList->GetHead().obj().get());
    }

    return nullptr;
}

/**
* @descr:   Get next listlist object, copy from lwp source code
*
*/
LwpListList* LwpOrderedObjectManager::GetNextActiveListList(LwpListList * pLast)
{
    LwpListList* pList = nullptr;
    if(pLast)
        pList = static_cast<LwpListList*>(pLast->GetNext().obj().get());
    else
    {
        LwpDLVListHeadHolder* pHeadHolder= static_cast<LwpDLVListHeadHolder*>(m_Head.obj().get());
        if(pHeadHolder)
        {
            pList = static_cast<LwpListList*>(pHeadHolder->GetHeadID().obj().get());
        }
    }

    while(pList)
    {
        LwpContent* pContent = static_cast<LwpContent*>(pList->GetObject().obj().get());
        if(pContent && pContent->HasNonEmbeddedLayouts() &&
            !pContent->IsStyleContent())
            return pList;
        pList = static_cast<LwpListList*>(pList->GetNext().obj().get());
    }
    return nullptr;
}

LwpStyleManager::LwpStyleManager()
    : m_pFoundry(nullptr)
{
}

LwpStyleManager::~LwpStyleManager()
{
    m_StyleList.clear();
}

/*
VO_PARASTYLE/VO_CHARACTERSTYLE call this method to add its created style to XFStyleManager
1. Add the style to XFStyleManager, and return the <office:styles> style name
2. Add it to LwpParaStyleMap.
Prerequisite: pStyle has been created and all properties has been set to it.
Return the XFStyle* added by XFStyleManager
*/
IXFStyle* LwpStyleManager::AddStyle(LwpObjectID styleObjID, IXFStyle* pStyle)
{
    assert(pStyle);
    //pStyle may change if same style is found in XFStyleManager
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pStyle = pXFStyleManager->AddStyle(pStyle).m_pStyle;
    m_StyleList.insert(LwpStyleMap::value_type(styleObjID, pStyle));
    return pStyle;
}

/*
Called by VO_PARA  or other objects to get style name based on the Style object ID
1) Get style from LwpParaStyleMap based on the LwpObjectID of VO_PARASTYLE.
Prerequisite: VO_PARASTYLE/VO_CHARACTERSTYLE should call AddStyle first.
Return empty string if no style found.
*/
IXFStyle* LwpStyleManager::GetStyle(const LwpObjectID &styleObjID)
{
    LwpStyleMap::const_iterator it =  m_StyleList.find(styleObjID);
    if (it != m_StyleList.end()) {
        return((*it).second);
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
