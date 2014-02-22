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
 *  License at http:
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
#include "lwpglobalmgr.hxx"
#include "lwpdoc.hxx"
#include "lwpfootnote.hxx"
#include "lwppagehint.hxx"
#include "lwpdivinfo.hxx"
#include "lwpholder.hxx"
#include "lwpparastyle.hxx"
#include "lwpstory.hxx"
#include "lwppara.hxx"
#include "lwpsilverbullet.hxx"
#include "lwplayout.hxx"
#include "lwppagelayout.hxx"
#include "xfilter/xfstylemanager.hxx"
#include <osl/thread.h>

LwpDocument::LwpDocument(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpDLNFPVList(objHdr, pStrm), m_pOwnedFoundry(NULL), m_pLnOpts(NULL)
{}

LwpDocument::~LwpDocument()
{
    if(m_pLnOpts)
    {
        delete m_pLnOpts;
    }
    if(m_pOwnedFoundry)
    {
        delete m_pOwnedFoundry;
    }
}
/**
 * @descr   Read VO_Document from object stream
 **/
void LwpDocument::Read()
{
    LwpDLNFPVList::Read();

    ReadPlug();

    m_nPersistentFlags = m_pObjStrm->QuickReaduInt32();

    
    {
        LwpSortOption m_DocSort( m_pObjStrm );
        LwpUIDocument m_UIDoc( m_pObjStrm );
    }

    m_pLnOpts = new LwpLineNumberOptions(m_pObjStrm);

    
    {
        LwpUserDictFiles m_UsrDicts( m_pObjStrm );
    }

    if( !IsChildDoc())
    {
        
        LwpPrinterInfo m_PrtInfo( m_pObjStrm );
    }

    m_pFoundry = m_pOwnedFoundry = new LwpFoundry(m_pObjStrm, this);

    m_DivOpts.ReadIndexed(m_pObjStrm);

    if(!IsChildDoc())
    {
        m_FootnoteOpts.ReadIndexed(m_pObjStrm);
        m_DocData.ReadIndexed(m_pObjStrm);
    }
    else
    {
        
        LwpObjectID dummyDocData;
        dummyDocData.ReadIndexed(m_pObjStrm);
    }
    m_DivInfo.ReadIndexed(m_pObjStrm);
    m_Epoch.Read(m_pObjStrm);
    m_WYSIWYGPageHints.ReadIndexed(m_pObjStrm);
    m_VerDoc.ReadIndexed(m_pObjStrm);
    m_STXInfo.ReadIndexed(m_pObjStrm);
}

/**
 * @descr   Read plug related data from m_pObjStram
 **/
void LwpDocument::ReadPlug()
{
    m_DocSockID.ReadIndexed(m_pObjStrm);
    m_nFlags = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr   Parse obj to IXFStream
 **/
void LwpDocument::Parse(IXFStream* pOutputStream)
{
    
    if (!IsSkippedDivision())
    {
        
        ParseFrameInPage(pOutputStream);
        ParseDocContent(pOutputStream);
    }

    LwpObject* pDocSock = GetSocket()->obj( VO_DOCSOCK );
    if(pDocSock!=NULL)
    {
        pDocSock->Parse(pOutputStream);
    }
}

sal_Bool LwpDocument::IsSkippedDivision()
{
    OUString sDivName;
    sal_uInt8 ret = sal_False;
    LwpDivInfo* pDiv = dynamic_cast<LwpDivInfo*>(GetDivInfoID()->obj(VO_DIVISIONINFO));
    if (pDiv == NULL)
        return sal_True;
    sDivName = pDiv->GetDivName();
    if (!sDivName.isEmpty() && !pDiv->IsGotoable()) 
        return sal_True;
    
    OUString strClassName = pDiv->GetClassName();
    if (strClassName.equals(A2OUSTR(STR_DivisionEndnote))
        || strClassName.equals(A2OUSTR(STR_DivisionGroupEndnote))
        || strClassName.equals(A2OUSTR(STR_DocumentEndnote)))
    {
        LwpPageLayout* pPageLayout = dynamic_cast<LwpPageLayout*>(pDiv->GetInitialLayoutID()->obj(VO_PAGELAYOUT));
        if(pPageLayout)
        {
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent()->obj(VO_STORY));
            if(pStory)
            {
                
                
                LwpObjectID* pFirst = pStory->GetFirstPara();
                LwpObjectID* pLast = pStory->GetLastPara();
                if(*pFirst == *pLast)
                    ret = sal_True;
            }
        }
    }
    return ret;
}

/**
 * @descr  Register all styles in this division
 */
void LwpDocument::RegisterStyle()
{
    RegisterDefaultParaStyles();
    RegisterGraphicsStyles();
    RegisterBulletStyles();

    RegisterTextStyles();
    RegisterLayoutStyles();
    RegisterStylesInPara();

    RegisterLinenumberStyles();
    RegisterFootnoteStyles();

    
    LwpObject* pDocSock = GetSocket()->obj();
    if(pDocSock!=NULL)
    {
        pDocSock->RegisterStyle();
    }
}
/**
 * @descr  Register all named para styles
 */
void LwpDocument::RegisterTextStyles()
{
    
    LwpDLVListHeadHolder* pParaStyleHolder = dynamic_cast<LwpDLVListHeadHolder*>(m_pFoundry->GetTextStyleHead()->obj());
    if(pParaStyleHolder)
    {
        LwpTextStyle* pParaStyle = dynamic_cast<LwpTextStyle*> (pParaStyleHolder->GetHeadID()->obj());
        while(pParaStyle)
        {
            pParaStyle->SetFoundry(m_pFoundry);
            pParaStyle->RegisterStyle();
            pParaStyle = dynamic_cast<LwpParaStyle*>(pParaStyle->GetNext()->obj());
        }
    }
    ChangeStyleName();
}
/**
 * @descr  Register all layouts styles (page master and master page)
 *       All para styles used in master page (header and footer) are registered and then showed
 */
void LwpDocument::RegisterLayoutStyles()
{
    
    m_pFoundry->RegisterAllLayouts();

    
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*> (m_DivInfo.obj( VO_DIVISIONINFO));
    LwpPageLayout* pPageLayout = NULL;
    if(pDivInfo)
    {
        pPageLayout = dynamic_cast<LwpPageLayout*>(pDivInfo->GetInitialLayoutID()->obj(VO_PAGELAYOUT));
        if(pPageLayout)
        {
            
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent()->obj(VO_STORY));
            if(pStory)
            {
                
                pStory->SortPageLayout();
                pStory->SetCurrentLayout(pPageLayout);
            }
        }
    }
}
/**
 * @descr  Register all styles used in para
 */
void LwpDocument::RegisterStylesInPara()
{
    
    LwpHeadContent* pContent = dynamic_cast<LwpHeadContent*> (m_pFoundry->GetContentManager()->GetContentList()->obj());
    if(pContent)
    {
        LwpStory* pStory = dynamic_cast<LwpStory*>(pContent->GetChildHead()->obj(VO_STORY));
        while(pStory)
        {
            
            pStory->SetFoundry(m_pFoundry);
            pStory->RegisterStyle();
            pStory = dynamic_cast<LwpStory*>(pStory->GetNext()->obj(VO_STORY));
        }
    }
}
/**
 * @descr  Register all bullet styles used in this division
 */
void LwpDocument::RegisterBulletStyles()
{
    
    LwpDLVListHeadHolder* mBulletHead = dynamic_cast<LwpDLVListHeadHolder*>
                (m_pFoundry->GetBulletManagerID()->obj(VO_HEADHOLDER));
    if( mBulletHead )
    {
        LwpSilverBullet* pBullet = dynamic_cast<LwpSilverBullet*>
                            (mBulletHead->GetHeadID()->obj());
        while(pBullet)
        {
            pBullet->SetFoundry(m_pFoundry);
            pBullet->RegisterStyle();
            pBullet = dynamic_cast<LwpSilverBullet*> (pBullet->GetNext()->obj());
        }
    }
}
/**
 * @descr  Register all styles used in VO_Graphic
 */
void LwpDocument::RegisterGraphicsStyles()
{
    
    LwpObject* pGraphic = m_pFoundry->GetGraphicListHead()->obj(VO_GRAPHIC);
    if(pGraphic)
    {
        pGraphic->SetFoundry(m_pFoundry);
        pGraphic->RegisterStyle();
    }
}
/**
 * @descr  Register line number styles
 */
void LwpDocument::RegisterLinenumberStyles()
{
    if (!m_pLnOpts)
        return;
    m_pLnOpts->RegisterStyle();
}

/**
* @descr   Register footnote/endnote configuration for the entire document
*/
void LwpDocument::RegisterFootnoteStyles()
{
    
    if(!m_FootnoteOpts.IsNull())
    {
        LwpFootnoteOptions* pFootnoteOpts = dynamic_cast<LwpFootnoteOptions*>(m_FootnoteOpts.obj());
        if (pFootnoteOpts)
        {
            pFootnoteOpts->SetMasterPage(A2OUSTR("Endnote"));
            pFootnoteOpts->RegisterStyle();
        }
    }
    
    
    LwpDocument* pEndnoteDiv = GetLastDivisionThatHasEndnote();
    if(this == pEndnoteDiv)
    {
        LwpDLVListHeadTailHolder* pHeadTail = dynamic_cast<LwpDLVListHeadTailHolder*>(GetPageHintsID()->obj());
        if(pHeadTail)
        {
            LwpPageHint* pPageHint = dynamic_cast<LwpPageHint*>(pHeadTail->GetTail()->obj());
            if(pPageHint && !pPageHint->GetPageLayoutID()->IsNull())
            {
                LwpPageLayout* pPageLayout = dynamic_cast<LwpPageLayout*>(pPageHint->GetPageLayoutID()->obj());
                if(pPageLayout)
                {
                    pPageLayout->SetFoundry(GetFoundry());
                    pPageLayout->RegisterEndnoteStyle();
                }
            }
        }
    }
}

/**
* @descr   Register default para styles
*/
#include "lwpverdocument.hxx"
void LwpDocument::RegisterDefaultParaStyles()
{
    if(!IsChildDoc())
    {
        
        
        LwpDocument* pFirstDoc = GetFirstDivisionWithContentsThatIsNotOLE();
        if(pFirstDoc)
        {
            LwpVerDocument* pVerDoc = dynamic_cast<LwpVerDocument*>(pFirstDoc->GetVerDoc()->obj());
            if(pVerDoc)
            {
                pVerDoc->RegisterStyle();
            }
        }

    }
}

/**
 * @descr  Parse content in this division to IXFStream
 * LwpDocument->LwpDivInfo->LwpPageLayout.Parse()
 */
void LwpDocument::ParseDocContent(IXFStream* pOutputStream)
{
    
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*> (m_DivInfo.obj());
    if(pDivInfo==NULL) return;

    LwpObject* pLayoutObj = pDivInfo->GetInitialLayoutID()->obj();
    if(pLayoutObj==NULL)
    {
        
        return;
    }
    pLayoutObj->SetFoundry(m_pFoundry);
    pLayoutObj->Parse(pOutputStream);
}

/**
 * @descr    Get the footnoteoptions from the root document
 */
LwpObjectID* LwpDocument::GetValidFootnoteOpts()
{
    LwpDocument* pRoot = GetRootDocument();
    if(pRoot)
    {
        return pRoot->GetFootnoteOpts();
    }
    return NULL;
}

/**
 * @descr    Get the endnote type
 */
sal_uInt16 LwpDocument::GetEndnoteType()
{
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID()->obj());
    if (!pDivInfo)
        return FN_DONTCARE;
    OUString strClassName = pDivInfo->GetClassName();
    if (strClassName.equals(A2OUSTR(STR_DivisionEndnote)))
        return FN_DIVISION_SEPARATE;
    if (strClassName.equals(A2OUSTR(STR_DivisionGroupEndnote)))
        return FN_DIVISIONGROUP_SEPARATE;
    if (strClassName.equals(A2OUSTR(STR_DocumentEndnote)))
        return FN_DOCUMENT_SEPARATE;
    return FN_DONTCARE;
}
/**
 * @descr       Get previous division
 */
LwpDocument* LwpDocument::GetPreviousDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket()->obj());
    if(pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetPrevious()->obj());
    }
    return NULL;
}
/**
 * @descr    Get next division
 */
 LwpDocument* LwpDocument::GetNextDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket()->obj());
    if(pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetNext()->obj());
    }
    return NULL;
}
/**
 * @descr    Get parent division
 */
 LwpDocument* LwpDocument::GetParentDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket()->obj());
    if(pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetParent()->obj());
    }
    return NULL;
}
/**
 * @descr    Get previous division in group, copy from lwp source code
 */
 LwpDocument* LwpDocument::GetPreviousInGroup()
{
    LwpDocument* pPrev = NULL;

    for (pPrev = GetPreviousDivision(); pPrev; pPrev = pPrev->GetPreviousDivision())
    {
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pPrev->GetDivInfoID()->obj());
        if(pDivInfo && pDivInfo->HasContents())
            return pPrev;
    }
    return NULL;
}
/**
 * @descr       Get previous division in group, copy from lwp source code
 */
 LwpDocument* LwpDocument::GetNextInGroup()
{
    LwpDocument* pNext = NULL;

    for (pNext = GetNextDivision(); pNext; pNext = pNext->GetNextDivision())
    {
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pNext->GetDivInfoID()->obj());
        if(pDivInfo && pDivInfo->HasContents())
            return pNext;
    }

    return NULL;
}
/**
 * @descr    Get previous division which has contents, copy from lwp source code
 */
 LwpDocument* LwpDocument::GetPreviousDivisionWithContents()
{
    if(GetPreviousDivision())
    {
        LwpDocument* pDoc = GetPreviousDivision()->GetLastDivisionWithContents();
        if (pDoc)
            return pDoc;
    }
    if(GetParentDivision())
        return GetParentDivision()->GetPreviousDivisionWithContents();
    return NULL;
}
 /**
 * @descr    Get last division which has contents, copy from lwp source code
 */
 LwpDocument* LwpDocument::GetLastDivisionWithContents()
{
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID()->obj());
    if(pDivInfo && pDivInfo->HasContents())
    {
        return this;
    }

    LwpDocument* pDivision = GetLastDivision();
    LwpDocument* pContentDivision = NULL;

    while(pDivision)
    {
        pContentDivision = pDivision->GetLastDivisionWithContents();
        if(pContentDivision)
        {
            return pContentDivision;
        }
        pDivision = pDivision->GetPreviousDivision();
    }

    return NULL;
}
 /**
 * @descr    Get last division in group  which has contents, copy from lwp source code
 */
 LwpDocument* LwpDocument::GetLastInGroupWithContents()
{
    LwpDocument* pLast = NULL;
    LwpDocument* pNext = this;

    while (pNext)
    {
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pNext->GetDivInfoID()->obj());
        if (pDivInfo && pDivInfo->HasContents())
            pLast = pNext;
        pNext = pNext->GetNextInGroup();
    }
    if (pLast)
        return pLast;
    return NULL;
}
  /**
 * @descr    Get last division
 */
 LwpDocument* LwpDocument::GetLastDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket()->obj());
    if(pDocSock)
        return dynamic_cast<LwpDocument*>(pDocSock->GetChildTail()->obj());
    return NULL;
}

  /**
 * @descr    Get first division
 */
 LwpDocument* LwpDocument::GetFirstDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket()->obj());
    if(pDocSock)
        return dynamic_cast<LwpDocument*>(pDocSock->GetChildHead()->obj());
    return NULL;
}

 /**
 * @descr    Get root document
 */
 LwpDocument* LwpDocument::GetRootDocument()
{
    LwpDocument* pRoot = this;
    while(pRoot)
    {
        if(!pRoot->IsChildDoc())
            return pRoot;
        pRoot = pRoot->GetParentDivision();
    }
    return NULL;
}
  /**
 * @descr    Get first division with contents that is not ole, copy from lwp-source code
 */
 LwpDocument* LwpDocument::GetFirstDivisionWithContentsThatIsNotOLE()
{
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID()->obj());
    if(pDivInfo && pDivInfo->HasContents()
        && !pDivInfo->IsOleDivision())
        return this;

    LwpDocument* pDivision = GetFirstDivision();

    LwpDocument*  pContentDivision = NULL;

    while (pDivision)
    {
        pContentDivision = pDivision->GetFirstDivisionWithContentsThatIsNotOLE();
        if(pContentDivision)
            return pContentDivision;
        pDivision = pDivision->GetNextDivision();
    }
    return NULL;
}
 /**
 * @descr    Get last division that has endnote
 */
 LwpDocument* LwpDocument::GetLastDivisionThatHasEndnote()
{
    LwpDocument* pRoot = GetRootDocument();
    LwpDocument *pLastDoc = pRoot->GetLastDivisionWithContents();
    while(pLastDoc)
    {
        if(pLastDoc->GetEnSuperTableLayout())
            return pLastDoc;
        pLastDoc = pLastDoc->GetPreviousDivisionWithContents();
    }
    return NULL;

}
 /**
 * @descr    Get endnote supertable layout, every division has only one endnote supertable layout.
 */
 LwpVirtualLayout* LwpDocument::GetEnSuperTableLayout()
{
    LwpHeadLayout* pHeadLayout = dynamic_cast<LwpHeadLayout*>(GetFoundry()->GetLayout()->obj());
    if(pHeadLayout)
    {
        return pHeadLayout->FindEnSuperTableLayout();
    }
    return NULL;
}

/**
 * @descr    Get the numbers of page  before pEndDivision, copy from lwp source code
 */
 sal_Bool LwpDocument::GetNumberOfPages(LwpDocument* pEndDivision, sal_uInt16& nCount)
{
    if(this == pEndDivision)
        return sal_True;

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj());
    if(pDivInfo)
    {
        pDivInfo->GetNumberOfPages(nCount);
    }

    LwpDocument* pDivision = GetFirstDivision();
    while(pDivision)
    {
        if(pDivision->GetNumberOfPages(pEndDivision,nCount))
            return sal_True;
        pDivision = pDivision->GetNextDivision();

    }
    return sal_False;
}
/**
 * @descr    Get the numbers of page  before curruent division
 */
sal_uInt16 LwpDocument::GetNumberOfPagesBefore()
{
    sal_uInt16 nPageNumber = 0;
    LwpDocument* pRoot = GetRootDocument();
    if(pRoot)
        pRoot->GetNumberOfPages(this,nPageNumber);
    return nPageNumber;
}

 /**
 * @descr    Get Max number of pages
 */
 void  LwpDocument::MaxNumberOfPages(sal_uInt16& nNumPages)
{
    LwpDocument* pDivision = GetFirstDivision();

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj());
    if(pDivInfo)
        nNumPages += pDivInfo->GetMaxNumberOfPages();
    while(pDivision)
    {
        pDivision->MaxNumberOfPages(nNumPages);
        pDivision = pDivision->GetNextDivision();
    }
}
  /**
 * @descr    Parse the frame which anchor is to page before parse other contents,
 *          This method is called when the document is root document.
 */
void LwpDocument::ParseFrameInPage(IXFStream * pOutputStream)
{
    if(IsChildDoc())
        return;

    XFContentContainer* pXFContainer = new XFContentContainer;

    XFConvertFrameInPage(pXFContainer);

    pXFContainer->ToXml(pOutputStream);
    delete pXFContainer;
    pXFContainer = NULL;
}
 /**
 * @descr    Parse the frame which anchor is to page in the entire document
 */
void LwpDocument::XFConvertFrameInPage(XFContentContainer * pCont)
{
    LwpDocument* pDivision = GetFirstDivision();

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*> (GetDivInfoID()->obj());
    if(pDivInfo)
    {
        LwpPageLayout*  pPageLayout = dynamic_cast<LwpPageLayout*>(pDivInfo->GetInitialLayoutID()->obj());
        if(pPageLayout)
        {
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent()->obj());
            if(pStory)
                pStory->XFConvertFrameInPage(pCont);
        }
    }
    while(pDivision)
    {
        pDivision->XFConvertFrameInPage(pCont);
        pDivision = pDivision->GetNextDivision();
    }
}
 /**
 * @descr    change click here to placeholder
 */
void LwpDocument::ChangeStyleName()
{
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFTextStyle* pStyle = dynamic_cast<XFTextStyle*>(pXFStyleManager->FindStyle(A2OUSTR("ClickHere")));
    if (pStyle)
    {
        pStyle->SetStyleName(A2OUSTR("Placeholder"));
    }
}
LwpDocSock::LwpDocSock(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    :LwpDLNFVList(objHdr, pStrm)
{}

 /**
 * @descr    read VO_DOCSOCK from file
 */
void LwpDocSock::Read()
{
    LwpDLNFVList::Read();
    m_Doc.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}
 /**
 * @descr    register styles in documents plugged
 */
void LwpDocSock::RegisterStyle()
{
    LwpObject* pDoc = GetNext()->obj();
    if(pDoc)
        pDoc->RegisterStyle();

    pDoc = GetChildHead()->obj();
    if(pDoc)
        pDoc->RegisterStyle();
}
 /**
 * @descr    parse contents of documents plugged
 */
void LwpDocSock::Parse(IXFStream* pOutputStream)
{
    LwpObject* pDoc = GetChildHead()->obj();
    if(pDoc)
        pDoc->Parse(pOutputStream);

    pDoc = GetNext()->obj();
    if(pDoc)
        pDoc->Parse(pOutputStream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
