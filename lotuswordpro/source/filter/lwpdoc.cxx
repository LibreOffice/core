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
    : LwpDLNFPVList(objHdr, pStrm)
    , m_pOwnedFoundry(NULL)
    , m_nFlags(0)
    , m_nPersistentFlags(0)
    , m_pLnOpts(NULL)
{
}

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

    //Skip the SortOption and UIDocument
    {
        LwpSortOption m_DocSort( m_pObjStrm );
        LwpUIDocument m_UIDoc( m_pObjStrm );
    }

    m_pLnOpts = new LwpLineNumberOptions(m_pObjStrm);

    //Skip LwpUserDictFiles
    {
        LwpUserDictFiles m_UsrDicts( m_pObjStrm );
    }

    if( !IsChildDoc())
    {
        //Skip LwpPrinterInfo
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
        //Skip the docdata used in old version
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
    //check the name and skip script division
    if (!IsSkippedDivision())
    {
        //the frames which anchor are to page must output before other contents
        ParseFrameInPage(pOutputStream);
        ParseDocContent(pOutputStream);
    }

    rtl::Reference<LwpObject> pDocSock = GetSocket().obj( VO_DOCSOCK );
    if(pDocSock.is())
    {
        pDocSock->Parse(pOutputStream);
    }
}

bool LwpDocument::IsSkippedDivision()
{
    OUString sDivName;
    bool ret = false;
    LwpDivInfo* pDiv = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj(VO_DIVISIONINFO).get());
    if (pDiv == NULL)
        return true;
    sDivName = pDiv->GetDivName();
    if (!sDivName.isEmpty() && !pDiv->IsGotoable()) //including toa,scripts division
        return true;
    //skip endnote division
    OUString strClassName = pDiv->GetClassName();
    if ((strClassName == STR_DivisionEndnote)
        || (strClassName == STR_DivisionGroupEndnote)
        || (strClassName == STR_DocumentEndnote))
    {
        LwpPageLayout* pPageLayout = dynamic_cast<LwpPageLayout*>(pDiv->GetInitialLayoutID().obj(VO_PAGELAYOUT).get());
        if(pPageLayout)
        {
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent().obj(VO_STORY).get());
            if(pStory)
            {
                //This judgement maybe have problem. If there is only one para in the story,
                //this endnote division has no other contents except endnote table.
                LwpObjectID& rFirst = pStory->GetFirstPara();
                LwpObjectID& rLast = pStory->GetLastPara();
                if(rFirst == rLast)
                    ret = true;
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

    //Register styles in other document connected with this document: next doc, children doc
    rtl::Reference<LwpObject> pDocSock = GetSocket().obj();
    if(pDocSock.is())
    {
        pDocSock->RegisterStyle();
    }
}
/**
 * @descr  Register all named para styles
 */
void LwpDocument::RegisterTextStyles()
{
    //Register all text styles: para styles, character styles
    LwpDLVListHeadHolder* pParaStyleHolder = dynamic_cast<LwpDLVListHeadHolder*>(m_pFoundry->GetTextStyleHead().obj().get());
    if(pParaStyleHolder)
    {
        LwpTextStyle* pParaStyle = dynamic_cast<LwpTextStyle*> (pParaStyleHolder->GetHeadID().obj().get());
        while(pParaStyle)
        {
            pParaStyle->SetFoundry(m_pFoundry);
            pParaStyle->RegisterStyle();
            pParaStyle = dynamic_cast<LwpParaStyle*>(pParaStyle->GetNext().obj().get());
        }
    }
    ChangeStyleName();//for click here block
}
/**
 * @descr  Register all layouts styles (page master and master page)
 *       All para styles used in master page (header and footer) are registered and then showed
 */
void LwpDocument::RegisterLayoutStyles()
{
    //Register all layout styles, before register all styles in para
    m_pFoundry->RegisterAllLayouts();

    //set initial pagelayout in story for parsing pagelayout
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*> (m_DivInfo.obj( VO_DIVISIONINFO).get());
    if(pDivInfo)
    {
        LwpPageLayout* pPageLayout = dynamic_cast<LwpPageLayout*>(pDivInfo->GetInitialLayoutID().obj(VO_PAGELAYOUT).get());
        if(pPageLayout)
        {
            //In Ole division, the content of pagelayout is VO_OLEOBJECT
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent().obj(VO_STORY).get());
            if(pStory)
            {
                //add all the pagelayout in order into the pagelayout list;
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
    //Register all automatic styles in para
    LwpHeadContent* pContent = dynamic_cast<LwpHeadContent*> (m_pFoundry->GetContentManager().GetContentList().obj().get());
    if(pContent)
    {
        LwpStory* pStory = dynamic_cast<LwpStory*>(pContent->GetChildHead().obj(VO_STORY).get());
        while(pStory)
        {
            //Register the child para
            pStory->SetFoundry(m_pFoundry);
            pStory->RegisterStyle();
            pStory = dynamic_cast<LwpStory*>(pStory->GetNext().obj(VO_STORY).get());
        }
    }
}
/**
 * @descr  Register all bullet styles used in this division
 */
void LwpDocument::RegisterBulletStyles()
{
    //Register bullet styles
    LwpDLVListHeadHolder* mBulletHead = dynamic_cast<LwpDLVListHeadHolder*>
        (m_pFoundry->GetBulletManagerID().obj(VO_HEADHOLDER).get());
    if( mBulletHead )
    {
        LwpSilverBullet* pBullet = dynamic_cast<LwpSilverBullet*>
                            (mBulletHead->GetHeadID().obj().get());
        while(pBullet)
        {
            pBullet->SetFoundry(m_pFoundry);
            pBullet->RegisterStyle();
            pBullet = dynamic_cast<LwpSilverBullet*> (pBullet->GetNext().obj().get());
        }
    }
}
/**
 * @descr  Register all styles used in VO_Graphic
 */
void LwpDocument::RegisterGraphicsStyles()
{
    //Register all graphics styles, the first object should register the next;
    rtl::Reference<LwpObject> pGraphic = m_pFoundry->GetGraphicListHead().obj(VO_GRAPHIC);
    if(pGraphic.is())
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
    //Register footnote and endnote configuration for the entire document
    if(!m_FootnoteOpts.IsNull())
    {
        LwpFootnoteOptions* pFootnoteOpts = dynamic_cast<LwpFootnoteOptions*>(m_FootnoteOpts.obj().get());
        if (pFootnoteOpts)
        {
            pFootnoteOpts->SetMasterPage("Endnote");
            pFootnoteOpts->RegisterStyle();
        }
    }
    //Register endnote page style for endnote configuration, use the last division that has endnote for the endnote page style
    //This page style must register after its division default styles have registered
    LwpDocument* pEndnoteDiv = GetLastDivisionThatHasEndnote();
    if(this == pEndnoteDiv)
    {
        LwpDLVListHeadTailHolder* pHeadTail = dynamic_cast<LwpDLVListHeadTailHolder*>(GetPageHintsID().obj().get());
        if(pHeadTail)
        {
            LwpPageHint* pPageHint = dynamic_cast<LwpPageHint*>(pHeadTail->GetTail().obj().get());
            if(pPageHint && !pPageHint->GetPageLayoutID().IsNull())
            {
                LwpPageLayout* pPageLayout = dynamic_cast<LwpPageLayout*>(pPageHint->GetPageLayoutID().obj().get());
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
        //Get First Division
        //LwpDocument* pFirstDoc = GetFirstDivision();
        LwpDocument* pFirstDoc = GetFirstDivisionWithContentsThatIsNotOLE();
        if(pFirstDoc)
        {
            LwpVerDocument* pVerDoc = dynamic_cast<LwpVerDocument*>(pFirstDoc->GetVerDoc().obj().get());
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
    //Parse content in PageLayout
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*> (m_DivInfo.obj().get());
    if(pDivInfo==NULL) return;

    rtl::Reference<LwpObject> pLayoutObj = pDivInfo->GetInitialLayoutID().obj();
    if(!pLayoutObj.is())
    {
        //master document not supported now.
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
        return &pRoot->GetFootnoteOpts();
    }
    return NULL;
}

/**
 * @descr    Get the endnote type
 */
sal_uInt16 LwpDocument::GetEndnoteType()
{
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj().get());
    if (!pDivInfo)
        return FN_DONTCARE;
    OUString strClassName = pDivInfo->GetClassName();
    if (strClassName == STR_DivisionEndnote)
        return FN_DIVISION_SEPARATE;
    if (strClassName == STR_DivisionGroupEndnote)
        return FN_DIVISIONGROUP_SEPARATE;
    if (strClassName == STR_DocumentEndnote)
        return FN_DOCUMENT_SEPARATE;
    return FN_DONTCARE;
}
/**
 * @descr       Get previous division
 */
LwpDocument* LwpDocument::GetPreviousDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if(pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetPrevious().obj().get());
    }
    return NULL;
}
/**
 * @descr    Get next division
 */
 LwpDocument* LwpDocument::GetNextDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if(pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetNext().obj().get());
    }
    return NULL;
}
/**
 * @descr    Get parent division
 */
 LwpDocument* LwpDocument::GetParentDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if(pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetParent().obj().get());
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
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pPrev->GetDivInfoID().obj().get());
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
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pNext->GetDivInfoID().obj().get());
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
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj().get());
    if(pDivInfo && pDivInfo->HasContents())
    {
        return this;
    }

    LwpDocument* pDivision = GetLastDivision();

    while(pDivision)
    {
        LwpDocument* pContentDivision = pDivision->GetLastDivisionWithContents();
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
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pNext->GetDivInfoID().obj().get());
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
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if(pDocSock)
        return dynamic_cast<LwpDocument*>(pDocSock->GetChildTail().obj().get());
    return NULL;
}

  /**
 * @descr    Get first division
 */
 LwpDocument* LwpDocument::GetFirstDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if(pDocSock)
        return dynamic_cast<LwpDocument*>(pDocSock->GetChildHead().obj().get());
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
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj().get());
    if(pDivInfo && pDivInfo->HasContents()
        && !pDivInfo->IsOleDivision())
        return this;

    LwpDocument* pDivision = GetFirstDivision();

    while (pDivision)
    {
        LwpDocument* pContentDivision = pDivision->GetFirstDivisionWithContentsThatIsNotOLE();
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
    LwpHeadLayout* pHeadLayout = dynamic_cast<LwpHeadLayout*>(GetFoundry()->GetLayout().obj().get());
    if(pHeadLayout)
    {
        return pHeadLayout->FindEnSuperTableLayout();
    }
    return NULL;
}

/**
 * @descr    Get the numbers of page  before pEndDivision, copy from lwp source code
 */
bool LwpDocument::GetNumberOfPages(LwpDocument* pEndDivision, sal_uInt16& nCount)
{
    if(this == pEndDivision)
        return true;

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj().get());
    if(pDivInfo)
    {
        pDivInfo->GetNumberOfPages(nCount);
    }

    LwpDocument* pDivision = GetFirstDivision();
    while(pDivision)
    {
        if(pDivision->GetNumberOfPages(pEndDivision,nCount))
            return true;
        pDivision = pDivision->GetNextDivision();

    }
    return false;
}
/**
 * @descr    Get the numbers of page  before current division
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

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj().get());
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

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*> (GetDivInfoID().obj().get());
    if(pDivInfo)
    {
        LwpPageLayout*  pPageLayout = dynamic_cast<LwpPageLayout*>(pDivInfo->GetInitialLayoutID().obj().get());
        if(pPageLayout)
        {
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent().obj().get());
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
    XFTextStyle* pStyle = dynamic_cast<XFTextStyle*>(pXFStyleManager->FindStyle("ClickHere"));
    if (pStyle)
    {
        pStyle->SetStyleName("Placeholder");
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
    rtl::Reference<LwpObject> pDoc = GetNext().obj();
    if(pDoc.is())
        pDoc->RegisterStyle();

    pDoc = GetChildHead().obj();
    if(pDoc.is())
        pDoc->RegisterStyle();
}
 /**
 * @descr    parse contents of documents plugged
 */
void LwpDocSock::Parse(IXFStream* pOutputStream)
{
    rtl::Reference<LwpObject> pDoc = GetChildHead().obj();
    if(pDoc.is())
        pDoc->Parse(pOutputStream);

    pDoc = GetNext().obj();
    if(pDoc.is())
        pDoc->Parse(pOutputStream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
