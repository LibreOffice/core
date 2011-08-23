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
/**
 * @file
 *  For LWP filter architecture prototype - TOC related object
 */
/*************************************************************************
 * Change History
 April 2005		 	Created
 April 2005			Modified
 April 2005 				Modified - add LwpTocLevelData class
 ************************************************************************/
#include "lwptoc.hxx"
#include "lwpfoundry.hxx"
#include "lwpdoc.hxx"
#include "lwpframelayout.hxx"
#include "lwpglobalmgr.hxx"

#include "xfilter/xffont.hxx"
#include "xfilter/xftextstyle.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfparagraph.hxx"
#include "xfilter/xfparastyle.hxx"
#include "xfilter/xfindex.hxx"
#include	"xfilter/xffloatframe.hxx"
#include "xfilter/xfframestyle.hxx"
#include "xfilter/xfframe.hxx"
#include "xfilter/xftable.hxx"

LwpTocSuperLayout::LwpTocSuperLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpSuperTableLayout(objHdr, pStrm)
{}

LwpTocSuperLayout::~LwpTocSuperLayout()
{}
/**
 * @short   Read TOCSUPERTABLELAYOUT object
 * @return  none
 */
void LwpTocSuperLayout::Read()
{
    LwpSuperTableLayout::Read();
    m_TextMarker.Read(m_pObjStrm);
    m_ParentName.Read(m_pObjStrm);
    m_DivisionName.Read(m_pObjStrm);
    m_SectionName.Read(m_pObjStrm);
    m_nFrom = m_pObjStrm->QuickReaduInt16();

    m_SearchItems.Read(m_pObjStrm);

    sal_uInt16 i;
    sal_uInt16 count = m_pObjStrm->QuickReaduInt16();
    for (i = 0; (i < MAX_LEVELS) && (count > 0); i++, count--)
        m_DestName[i].Read(m_pObjStrm);

    count = m_pObjStrm->QuickReaduInt16();
    for (i = 0; (i < MAX_LEVELS) && (count > 0); i++, count--)
        m_DestPGName[i].Read(m_pObjStrm);

    count = m_pObjStrm->QuickReaduInt16();
    for (i = 0; i < count; i++)
        m_nFlags[i] = m_pObjStrm->QuickReaduInt32();

    m_pObjStrm->SkipExtra();
}
/**
 * @short   Register style of TOC
 * @return  none
 */
void LwpTocSuperLayout::RegisterStyle()
{
    LwpSuperTableLayout::RegisterStyle();

    // Get font info of default text style and set into tab style
    XFParaStyle* pBaseStyle = static_cast<XFParaStyle*>(m_pFoundry->GetStyleManager()->GetStyle(*m_pFoundry->GetDefaultTextStyle()));
    XFTextStyle*pTextStyle = new XFTextStyle;
    pTextStyle->SetFont(pBaseStyle->GetFont()); // who delete this font?????
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_TabStyleName = pXFStyleManager->AddStyle(pTextStyle)->GetStyleName();

}
/**
 * @short   Convert TOC
 * @param  pCont - container
 * @return none
 */
void  LwpTocSuperLayout::XFConvert(XFContentContainer* pCont)
{
    XFIndex* pToc = new XFIndex();

    pToc->SetProtected(sal_False);
    pToc->SetIndexType(enumXFIndexTOC);

    // add TOC template
    for (sal_uInt16 i = 1; i<= MAX_LEVELS; i++)
    {
        LwpTocLevelData * pLevel = GetSearchLevelPtr(i);
        XFIndexTemplate * pTemplate = new XFIndexTemplate();

        if(!pLevel)
        {
            // add an blank template so that SODC won't add default style to this level
            pToc->AddTemplate(Int32ToOUString(i),  String::CreateFromAscii(""), pTemplate);
            continue;
        }

        sal_Bool bInserted = sal_False;
        do
        {
            // One level has 1 template
            if (!bInserted)
            {
                pTemplate->SetLevel(Int32ToOUString(i));
                if(pLevel->GetUseLeadingText())
                {
                    pTemplate->AddEntry(enumXFIndexTemplateChapter, pLevel->GetSearchStyle());
                }
                if(pLevel->GetUseText())
                {
                    pTemplate->AddEntry(enumXFIndexTemplateText, pLevel->GetSearchStyle());
                }
                if(GetUsePageNumber(i))
                {
                    sal_uInt16 nLeaderType = GetSeparatorType(i);
                    if (GetRightAlignPageNumber(i))
                    {
                        char cSep = ' ';
                        switch(nLeaderType)
                        {
                        default: // go through
                        case NONE: // no leaders
                            cSep = ' ';
                            break;
                        case LEADERDOTS:
                            cSep = '.';
                            break;
                        case LEADERDASHES:
                            cSep = '-';
                            break;
                        case LEADERUNDERLINE:
                            cSep = '_';
                            break;
                        }

                        pTemplate->AddTabEntry(enumXFTabRight, 0, cSep, 'd', m_TabStyleName);
                    }
                    else
                    {
                        char sSep[8];
                        switch(nLeaderType)
                        {
                        default: // go through
                        case NONE: // no leaders
                            strcpy(sSep, "  ");
                            break;
                        case SEPARATORCOMMA:
                            strcpy(sSep, ", ");
                            break;
                        case SEPARATORDOTS:
                            strcpy(sSep, "...");
                            break;
                        }
                        pTemplate->AddTextEntry(A2OUSTR(sSep), m_TabStyleName);
                    }
                    //"TOC Page Number Text Style" style always exists in Word Pro file
                    pTemplate->AddEntry(enumXFIndexTemplatePage, A2OUSTR("TOC Page Number Text Style"));
                }

                pToc->AddTemplate(Int16ToOUString(i),  m_pFoundry->FindActuralStyleName(pLevel->GetSearchStyle()), pTemplate);
                bInserted = sal_True;
            }

            // 1 style in WordPro may be mapped to several styles in SODC
            LwpDocument * pDocument = m_pFoundry->GetDocument()->GetRootDocument();
            AddSourceStyle(pToc, pLevel,  pDocument->GetFoundry());

            // one level may have several corresponding Styles
            pLevel = GetNextSearchLevelPtr(i, pLevel);  // find next LwpTocLevelData which is same index
        }while (pLevel != NULL);
    }

    m_pCont = pCont;
    // add TOC content
    LwpSuperTableLayout::XFConvert(pToc);

    // if current TOC is located in a cell, we must add a frame between upper level container and TOC
    if ( !GetContainerLayout()->IsCell() )
    {
        pCont->Add(pToc);
    }
}

/**
 * @short   convert frame which anchor to page
 * @param pCont -
 * @return
 */
void  LwpTocSuperLayout::XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart , sal_Int32 nEnd , sal_Bool bAll)
{
    if(m_pFrame)
    {
        XFFrame* pXFFrame = NULL;
        if(nEnd < nStart)
        {
            pXFFrame = new XFFrame();
        }
        else
        {
            pXFFrame = new XFFloatFrame(nStart, nEnd, bAll);
        }

        m_pFrame->Parse(pXFFrame, static_cast<sal_uInt16>(nStart));

        //parse table, and add table to frame or TOC
        LwpTableLayout * pTableLayout = GetTableLayout();
        if (pTableLayout)
        {
            XFContentContainer * pTableContainer = pXFFrame;
            // if *this is a TOCSuperTableLayout and it's located in a cell
            // add the frame to upper level and add TOCSuperTableLayout into the frame
            if ( GetContainerLayout()->IsCell() )
            {
                pTableContainer = pCont; // TOC contain table directly
                pXFFrame->Add(pCont);
                m_pCont->Add(pXFFrame);
            }
            else
            {
                //add frame to the container
                pCont ->Add(pXFFrame);
            }
            pTableLayout->XFConvert(pTableContainer);
        }
    }

}

/**
 * @short   Add source style into TOC
 * @param  pToc - TOC pointer
 * @param  pLevel - TOC level data
 * @param  pFoundry - foundry pointer
 * @return sal_Bool
 */
void LwpTocSuperLayout::AddSourceStyle(XFIndex* pToc, LwpTocLevelData * pLevel, LwpFoundry * pFoundry)
{
    if (!pLevel)
    {
        return;
    }

    OUString sLwpStyleName = pLevel->GetSearchStyle();

    if (pFoundry)
    {
        LwpDocument * pDoc = pFoundry->GetDocument();
        if (pDoc && pDoc->IsChildDoc())
        {
            OUString sSodcStyleName = pFoundry->FindActuralStyleName(sLwpStyleName);
            pToc->AddTocSource(pLevel->GetLevel(), sSodcStyleName);
        }
        else
        {
            pDoc = pDoc->GetFirstDivision();
            while (pDoc)
            {
                AddSourceStyle(pToc, pLevel, pDoc->GetFoundry() );
                pDoc = pDoc->GetNextDivision();
            }
        }
    }
}

/**
 * @short   Get whether page number is right alignment
 * @param  index - TOC level
 * @return sal_Bool
 */
sal_Bool LwpTocSuperLayout::GetRightAlignPageNumber(sal_uInt16 index)
{
    if (index < MAX_LEVELS)
        return (m_nFlags[index] & TS_RIGHTALIGN) ? sal_True : sal_False;
    return sal_False;
}
/**
 * @short   Get whether page number is used in TOC entries
 * @param  index - TOC level
 * @return sal_Bool
 */
sal_Bool LwpTocSuperLayout::GetUsePageNumber(sal_uInt16 index)
{
    if (index < MAX_LEVELS)
        return (m_nFlags[index] & TS_PAGENUMBER) ? sal_True : sal_False;
    return sal_False;
}
/**
 * @short   Get what is used for separater
 * @param  index - TOC level
 * @return sal_uInt16 - separator type
 */
sal_uInt16 LwpTocSuperLayout::GetSeparatorType(sal_uInt16 index)
{
    sal_uInt16 Flag = (sal_uInt16)m_nFlags[index];

    if (Flag & TS_LEADERDOTS)
        return LEADERDOTS;
    else if (Flag & TS_LEADERDASHES)
        return LEADERDASHES;
    else if (Flag & TS_LEADERUNDERLINE)
        return LEADERUNDERLINE;
    else if (Flag & TS_SEPARATORCOMMA)
        return SEPARATORCOMMA;
    else if (Flag & TS_SEPARATORDOTS)
        return SEPARATORDOTS;
    else
        return NONE;
}

/**
 * @short   Get TOCLEVELDATA obj
 * @param  index - TOC level
 * @return LwpTocLevelData * - pointer to TOCLEVELDATA obj
 */
LwpTocLevelData * LwpTocSuperLayout::GetSearchLevelPtr(sal_uInt16 index)
{
    LwpObjectID * pID = m_SearchItems.GetHead(); // not necessary to check pID NULL or not
    LwpTocLevelData * pObj = static_cast<LwpTocLevelData *>(pID->obj());

    while(pObj)
    {
        if(pObj->GetLevel()== index)
        {
            return pObj;
        }

        pID = pObj->GetNext(); // not necessary to check pID NULL or not
        pObj = static_cast<LwpTocLevelData *>(pID->obj());
    }

    return NULL;
}
/**
 * @short   Get next TOCLEVELDATA obj from current position
 * @param  index - TOC level
 * @param  pCurData - current LwpTocLevelData
 * @return LwpTocLevelData * - pointer to TOCLEVELDATA obj
 */
LwpTocLevelData * LwpTocSuperLayout::GetNextSearchLevelPtr(sal_uInt16 index, LwpTocLevelData * pCurData)
{
    LwpObjectID * pID = pCurData->GetNext();
    LwpTocLevelData * pObj = static_cast<LwpTocLevelData *>(pID->obj());

    while(pObj)
    {
        if(pObj->GetLevel()== index)
        {
            return pObj;
        }

        pID = pObj->GetNext(); // not necessary to check pID NULL or not
        pObj = static_cast<LwpTocLevelData *>(pID->obj());
    }

    return NULL;
}

LwpTocLevelData::LwpTocLevelData(LwpObjectHeader &objHdr, LwpSvStream* pStrm):LwpDLVList(objHdr, pStrm)
{
    m_nFlags = 0;
    m_nLevel = 0;
}
LwpTocLevelData::~LwpTocLevelData()
{
}
/**
 * @short   Register style
 * @param
 * @return
 */
void LwpTocLevelData::RegisterStyle()
{
}
/**
 * @short   Convert
 * @param  pCont - container
 * @return none
 */
void LwpTocLevelData::XFConvert(XFContentContainer* /*pCont*/)
{
}
/**
 * @short   Read TOCLEVELDATA obj
 * @param
 * @return
 */
void LwpTocLevelData::Read()
{
    LwpDLVList::Read();
    m_nFlags = m_pObjStrm->QuickReaduInt16();
    m_nLevel = m_pObjStrm->QuickReaduInt16();
    m_SearchName.Read(m_pObjStrm);

    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
