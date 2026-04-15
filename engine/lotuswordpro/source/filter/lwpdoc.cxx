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
#include <lwpglobalmgr.hxx>
#include "lwpcharacterstyle.hxx"
#include "lwpdoc.hxx"
#include "lwpfootnote.hxx"
#include "lwppagehint.hxx"
#include "lwpdivinfo.hxx"
#include "lwpholder.hxx"
#include "lwpstory.hxx"
#include "lwpsilverbullet.hxx"
#include "lwpsortopt.hxx"
#include "lwplayout.hxx"
#include "lwppagelayout.hxx"
#include "lwpuidoc.hxx"
#include "lwpusrdicts.hxx"
#include "lwpprtinfo.hxx"
#include "lwpverdocument.hxx"
#include <xfilter/xftextstyle.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <o3tl/sorted_vector.hxx>

LwpDocument::LwpDocument(LwpObjectHeader const& objHdr, LwpSvStream* pStrm)
    : LwpDLNFPVList(objHdr, pStrm)
    , m_bGettingFirstDivisionWithContentsThatIsNotOLE(false)
    , m_bGettingPreviousDivisionWithContents(false)
    , m_bGettingGetLastDivisionWithContents(false)
    , m_nFlags(0)
    , m_nPersistentFlags(0)
{
}

LwpDocument::~LwpDocument() {}
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
        LwpSortOption aDocSort(m_pObjStrm.get());
        LwpUIDocument aUIDoc(m_pObjStrm.get());
    }

    m_oLnOpts.emplace(m_pObjStrm.get());

    //Skip LwpUserDictFiles
    {
        LwpUserDictFiles aUsrDicts(m_pObjStrm.get());
    }

    if (!IsChildDoc())
    {
        //Skip LwpPrinterInfo
        LwpPrinterInfo aPrtInfo(m_pObjStrm.get());
    }

    m_xOwnedFoundry.reset(new LwpFoundry(m_pObjStrm.get(), this));

    m_DivOpts.ReadIndexed(m_pObjStrm.get());

    if (!IsChildDoc())
    {
        m_FootnoteOpts.ReadIndexed(m_pObjStrm.get());
        m_DocData.ReadIndexed(m_pObjStrm.get());
    }
    else
    {
        //Skip the docdata used in old version
        LwpObjectID dummyDocData;
        dummyDocData.ReadIndexed(m_pObjStrm.get());
    }
    m_DivInfo.ReadIndexed(m_pObjStrm.get());
    m_Epoch.Read(m_pObjStrm.get());
    m_WYSIWYGPageHints.ReadIndexed(m_pObjStrm.get());
    m_VerDoc.ReadIndexed(m_pObjStrm.get());
    m_STXInfo.ReadIndexed(m_pObjStrm.get());
}

/**
 * @descr   Read plug related data from m_pObjStram
 **/
void LwpDocument::ReadPlug()
{
    m_DocSockID.ReadIndexed(m_pObjStrm.get());
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

    rtl::Reference<LwpObject> pDocSock = GetSocket().obj(VO_DOCSOCK);
    if (pDocSock.is())
    {
        pDocSock->DoParse(pOutputStream);
    }
}

bool LwpDocument::IsSkippedDivision()
{
    OUString sDivName;
    bool ret = false;
    LwpDivInfo* pDiv = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj(VO_DIVISIONINFO).get());
    if (pDiv == nullptr)
        return true;
    sDivName = pDiv->GetDivName();
    if (!sDivName.isEmpty() && !pDiv->IsGotoable()) //including toa,scripts division
        return true;
    //skip endnote division
    OUString strClassName = pDiv->GetClassName();
    if ((strClassName == STR_DivisionEndnote) || (strClassName == STR_DivisionGroupEndnote)
        || (strClassName == STR_DocumentEndnote))
    {
        LwpPageLayout* pPageLayout
            = dynamic_cast<LwpPageLayout*>(pDiv->GetInitialLayoutID().obj(VO_PAGELAYOUT).get());
        if (pPageLayout)
        {
            LwpStory* pStory
                = dynamic_cast<LwpStory*>(pPageLayout->GetContent().obj(VO_STORY).get());
            if (pStory)
            {
                //This judgement maybe have problem. If there is only one para in the story,
                //this endnote division has no other contents except endnote table.
                LwpObjectID& rFirst = pStory->GetFirstPara();
                LwpObjectID& rLast = pStory->GetLastPara();
                if (rFirst == rLast)
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
    if (pDocSock.is())
    {
        pDocSock->DoRegisterStyle();
    }
}
/**
 * @descr  Register all named para styles
 */
void LwpDocument::RegisterTextStyles()
{
    //Register all text styles: para styles, character styles
    LwpDLVListHeadHolder* pTextStyleHolder
        = m_xOwnedFoundry
              ? dynamic_cast<LwpDLVListHeadHolder*>(m_xOwnedFoundry->GetTextStyleHead().obj().get())
              : nullptr;
    if (pTextStyleHolder)
    {
        LwpTextStyle* pTextStyle
            = dynamic_cast<LwpTextStyle*>(pTextStyleHolder->GetHeadID().obj().get());
        while (pTextStyle)
        {
            if (pTextStyle->GetFoundry())
                throw std::runtime_error("loop in register text style");
            pTextStyle->SetFoundry(m_xOwnedFoundry.get());
            pTextStyle->RegisterStyle();
            pTextStyle = dynamic_cast<LwpTextStyle*>(pTextStyle->GetNext().obj().get());
        }
    }
    ChangeStyleName(); //for click here block
}
/**
 * @descr  Register all layouts styles (page master and master page)
 *       All para styles used in master page (header and footer) are registered and then showed
 */
void LwpDocument::RegisterLayoutStyles()
{
    if (m_xOwnedFoundry)
    {
        //Register all layout styles, before register all styles in para
        m_xOwnedFoundry->RegisterAllLayouts();
    }

    //set initial pagelayout in story for parsing pagelayout
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj(VO_DIVISIONINFO).get());
    if (!pDivInfo)
        return;

    LwpPageLayout* pPageLayout
        = dynamic_cast<LwpPageLayout*>(pDivInfo->GetInitialLayoutID().obj(VO_PAGELAYOUT).get());
    if (pPageLayout)
    {
        //In Ole division, the content of pagelayout is VO_OLEOBJECT
        LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent().obj(VO_STORY).get());
        if (pStory)
        {
            //add all the pagelayout in order into the pagelayout list;
            pStory->SortPageLayout();
            pStory->SetCurrentLayout(pPageLayout);
        }
    }
}
/**
 * @descr  Register all styles used in para
 */
void LwpDocument::RegisterStylesInPara()
{
    //Register all automatic styles in para
    rtl::Reference<LwpHeadContent> xContent(
        m_xOwnedFoundry ? dynamic_cast<LwpHeadContent*>(
                              m_xOwnedFoundry->GetContentManager().GetContentList().obj().get())
                        : nullptr);
    if (!xContent.is())
        return;

    rtl::Reference<LwpStory> xStory(
        dynamic_cast<LwpStory*>(xContent->GetChildHead().obj(VO_STORY).get()));
    o3tl::sorted_vector<LwpStory*> aSeen;
    while (xStory.is())
    {
        bool bAlreadySeen = !aSeen.insert(xStory.get()).second;
        if (bAlreadySeen)
            throw std::runtime_error("loop in conversion");
        //Register the child para
        xStory->SetFoundry(m_xOwnedFoundry.get());
        xStory->DoRegisterStyle();
        xStory.set(dynamic_cast<LwpStory*>(xStory->GetNext().obj(VO_STORY).get()));
    }
}
/**
 * @descr  Register all bullet styles used in this division
 */
void LwpDocument::RegisterBulletStyles()
{
    if (!m_xOwnedFoundry)
        return;
    //Register bullet styles
    LwpDLVListHeadHolder* pBulletHead = dynamic_cast<LwpDLVListHeadHolder*>(
        m_xOwnedFoundry->GetBulletManagerID().obj(VO_HEADHOLDER).get());
    if (!pBulletHead)
        return;
    LwpSilverBullet* pBullet = dynamic_cast<LwpSilverBullet*>(pBulletHead->GetHeadID().obj().get());
    o3tl::sorted_vector<LwpSilverBullet*> aSeen;
    while (pBullet)
    {
        bool bAlreadySeen = !aSeen.insert(pBullet).second;
        if (bAlreadySeen)
            throw std::runtime_error("loop in conversion");
        pBullet->SetFoundry(m_xOwnedFoundry.get());
        pBullet->RegisterStyle();
        pBullet = dynamic_cast<LwpSilverBullet*>(pBullet->GetNext().obj().get());
    }
}
/**
 * @descr  Register all styles used in VO_Graphic
 */
void LwpDocument::RegisterGraphicsStyles()
{
    if (!m_xOwnedFoundry)
        return;
    //Register all graphics styles, the first object should register the next;
    rtl::Reference<LwpObject> pGraphic = m_xOwnedFoundry->GetGraphicListHead().obj(VO_GRAPHIC);
    if (!pGraphic.is())
        return;
    pGraphic->SetFoundry(m_xOwnedFoundry.get());
    pGraphic->DoRegisterStyle();
}
/**
 * @descr  Register line number styles
 */
void LwpDocument::RegisterLinenumberStyles()
{
    if (!m_oLnOpts)
        return;
    m_oLnOpts->RegisterStyle();
}

/**
* @descr   Register footnote/endnote configuration for the entire document
*/
void LwpDocument::RegisterFootnoteStyles()
{
    //Register footnote and endnote configuration for the entire document
    if (!m_FootnoteOpts.IsNull())
    {
        LwpFootnoteOptions* pFootnoteOpts
            = dynamic_cast<LwpFootnoteOptions*>(m_FootnoteOpts.obj().get());
        if (pFootnoteOpts)
        {
            pFootnoteOpts->SetMasterPage(u"Endnote"_ustr);
            pFootnoteOpts->RegisterStyle();
        }
    }
    //Register endnote page style for endnote configuration, use the last division that has endnote for the endnote page style
    //This page style must register after its division default styles have registered
    LwpDocument* pEndnoteDiv = GetLastDivisionThatHasEndnote();
    if (this != pEndnoteDiv)
        return;

    LwpDLVListHeadTailHolder* pHeadTail
        = dynamic_cast<LwpDLVListHeadTailHolder*>(GetPageHintsID().obj().get());
    if (!pHeadTail)
        return;

    LwpPageHint* pPageHint = dynamic_cast<LwpPageHint*>(pHeadTail->GetTail().obj().get());
    if (pPageHint && !pPageHint->GetPageLayoutID().IsNull())
    {
        LwpPageLayout* pPageLayout
            = dynamic_cast<LwpPageLayout*>(pPageHint->GetPageLayoutID().obj().get());
        if (pPageLayout)
        {
            pPageLayout->SetFoundry(GetFoundry());
            pPageLayout->RegisterEndnoteStyle();
        }
    }
}

/**
* @descr   Register default para styles
*/
void LwpDocument::RegisterDefaultParaStyles()
{
    if (IsChildDoc())
        return;

    //Get First Division
    //LwpDocument* pFirstDoc = GetFirstDivision();
    LwpDocument* pFirstDoc = GetFirstDivisionWithContentsThatIsNotOLE();
    if (pFirstDoc)
    {
        LwpVerDocument* pVerDoc = dynamic_cast<LwpVerDocument*>(pFirstDoc->GetVerDoc().obj().get());
        if (pVerDoc)
        {
            pVerDoc->RegisterStyle();
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
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj().get());
    if (pDivInfo == nullptr)
        return;

    rtl::Reference<LwpObject> pLayoutObj = pDivInfo->GetInitialLayoutID().obj();
    if (!pLayoutObj.is())
    {
        //master document not supported now.
        return;
    }
    pLayoutObj->SetFoundry(m_xOwnedFoundry.get());
    pLayoutObj->DoParse(pOutputStream);
}

/**
 * @descr    Get the footnoteoptions from the root document
 */
LwpObjectID* LwpDocument::GetValidFootnoteOpts()
{
    LwpDocument* pRoot = GetRootDocument();
    if (pRoot)
    {
        return &pRoot->GetFootnoteOpts();
    }
    return nullptr;
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
    if (pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetPrevious().obj().get());
    }
    return nullptr;
}
/**
 * @descr    Get next division
 */
LwpDocument* LwpDocument::GetNextDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if (pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetNext().obj().get());
    }
    return nullptr;
}
/**
 * @descr    Get parent division
 */
LwpDocument* LwpDocument::GetParentDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if (pDocSock)
    {
        return dynamic_cast<LwpDocument*>(pDocSock->GetParent().obj().get());
    }
    return nullptr;
}
/**
 * @descr    Get previous division in group, copy from lwp source code
 */
LwpDocument* LwpDocument::GetPreviousInGroup()
{
    LwpDocument* pPrev = nullptr;

    for (pPrev = GetPreviousDivision(); pPrev; pPrev = pPrev->GetPreviousDivision())
    {
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pPrev->GetDivInfoID().obj().get());
        if (pDivInfo && pDivInfo->HasContents())
            return pPrev;
    }
    return nullptr;
}
/**
 * @descr       Get previous division in group, copy from lwp source code
 */
LwpDocument* LwpDocument::GetNextInGroup()
{
    LwpDocument* pNext = nullptr;

    for (pNext = GetNextDivision(); pNext; pNext = pNext->GetNextDivision())
    {
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pNext->GetDivInfoID().obj().get());
        if (pDivInfo && pDivInfo->HasContents())
            return pNext;
    }

    return nullptr;
}
/**
 * @descr    Get previous division which has contents, copy from lwp source code
 */
LwpDocument* LwpDocument::GetPreviousDivisionWithContents()
{
    if (m_bGettingPreviousDivisionWithContents)
        throw std::runtime_error("recursion in page divisions");
    m_bGettingPreviousDivisionWithContents = true;
    LwpDocument* pRet = nullptr;

    if (GetPreviousDivision())
        pRet = GetPreviousDivision()->GetLastDivisionWithContents();
    if (!pRet && GetParentDivision())
        pRet = GetParentDivision()->GetPreviousDivisionWithContents();

    m_bGettingPreviousDivisionWithContents = false;
    return pRet;
}

/**
* @descr    Get last division which has contents, copy from lwp source code
*/
LwpDocument* LwpDocument::GetLastDivisionWithContents()
{
    if (m_bGettingGetLastDivisionWithContents)
        throw std::runtime_error("recursion in page divisions");
    m_bGettingGetLastDivisionWithContents = true;
    LwpDocument* pRet = nullptr;

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj().get());
    if (pDivInfo && pDivInfo->HasContents())
        pRet = this;

    if (!pRet)
    {
        LwpDocument* pDivision = GetLastDivision();

        o3tl::sorted_vector<LwpDocument*> aSeen;
        while (pDivision && pDivision != this)
        {
            bool bAlreadySeen = !aSeen.insert(pDivision).second;
            if (bAlreadySeen)
                throw std::runtime_error("loop in conversion");
            LwpDocument* pContentDivision = pDivision->GetLastDivisionWithContents();
            if (pContentDivision)
            {
                pRet = pContentDivision;
                break;
            }
            pDivision = pDivision->GetPreviousDivision();
        }
    }

    m_bGettingGetLastDivisionWithContents = false;
    return pRet;
}
/**
 * @descr    Get last division in group  which has contents, copy from lwp source code
 */
LwpDocument* LwpDocument::GetLastInGroupWithContents()
{
    LwpDocument* pLast = nullptr;
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
    return nullptr;
}
/**
 * @descr    Get last division
 */
LwpDocument* LwpDocument::GetLastDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if (pDocSock)
        return dynamic_cast<LwpDocument*>(pDocSock->GetChildTail().obj().get());
    return nullptr;
}

/**
 * @descr    Get first division
 */
LwpDocument* LwpDocument::GetFirstDivision()
{
    LwpDocSock* pDocSock = dynamic_cast<LwpDocSock*>(GetSocket().obj().get());
    if (pDocSock)
        return dynamic_cast<LwpDocument*>(pDocSock->GetChildHead().obj().get());
    return nullptr;
}

/**
 * @descr    Get root document
 */
LwpDocument* LwpDocument::GetRootDocument()
{
    LwpDocument* pRoot = this;
    o3tl::sorted_vector<LwpDocument*> aSeen;
    while (pRoot)
    {
        bool bAlreadySeen = !aSeen.insert(pRoot).second;
        if (bAlreadySeen)
            throw std::runtime_error("loop in conversion");
        if (!pRoot->IsChildDoc())
            return pRoot;
        pRoot = pRoot->GetParentDivision();
    }
    return nullptr;
}
/**
 * @descr    Get first division with contents that is not ole, copy from lwp-source code
 */
LwpDocument* LwpDocument::ImplGetFirstDivisionWithContentsThatIsNotOLE()
{
    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj().get());
    if (pDivInfo && pDivInfo->HasContents() && !pDivInfo->IsOleDivision())
        return this;

    LwpDocument* pDivision = GetFirstDivision();
    o3tl::sorted_vector<LwpDocument*> aSeen;
    while (pDivision)
    {
        bool bAlreadySeen = !aSeen.insert(pDivision).second;
        if (bAlreadySeen)
            throw std::runtime_error("loop in conversion");
        LwpDocument* pContentDivision = pDivision->GetFirstDivisionWithContentsThatIsNotOLE();
        if (pContentDivision)
            return pContentDivision;
        pDivision = pDivision->GetNextDivision();
    }
    return nullptr;
}
/**
 * @descr    Get last division that has endnote
 */
LwpDocument* LwpDocument::GetLastDivisionThatHasEndnote()
{
    LwpDocument* pRoot = GetRootDocument();
    LwpDocument* pLastDoc = pRoot ? pRoot->GetLastDivisionWithContents() : nullptr;
    o3tl::sorted_vector<LwpDocument*> aSeen;
    while (pLastDoc)
    {
        bool bAlreadySeen = !aSeen.insert(pLastDoc).second;
        if (bAlreadySeen)
            throw std::runtime_error("loop in conversion");
        if (pLastDoc->GetEnSuperTableLayout().is())
            return pLastDoc;
        pLastDoc = pLastDoc->GetPreviousDivisionWithContents();
    }
    return nullptr;
}
/**
 * @descr    Get endnote supertable layout, every division has only one endnote supertable layout.
 */
rtl::Reference<LwpVirtualLayout> LwpDocument::GetEnSuperTableLayout()
{
    LwpHeadLayout* pHeadLayout
        = dynamic_cast<LwpHeadLayout*>(GetFoundry()->GetLayout().obj().get());
    if (pHeadLayout)
    {
        return pHeadLayout->FindEnSuperTableLayout();
    }
    return rtl::Reference<LwpVirtualLayout>();
}

/**
 * @descr    Get the numbers of page  before pEndDivision, copy from lwp source code
 */
bool LwpDocument::GetNumberOfPages(LwpDocument* pEndDivision, sal_uInt16& nCount)
{
    if (this == pEndDivision)
        return true;

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(m_DivInfo.obj().get());
    if (pDivInfo)
    {
        pDivInfo->GetNumberOfPages(nCount);
    }

    LwpDocument* pDivision = GetFirstDivision();
    while (pDivision)
    {
        if (pDivision->GetNumberOfPages(pEndDivision, nCount))
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
    if (pRoot)
        pRoot->GetNumberOfPages(this, nPageNumber);
    return nPageNumber;
}

/**
 * @descr    Parse the frame which anchor is to page before parse other contents,
 *           This method is called when the document is root document.
 */
void LwpDocument::ParseFrameInPage(IXFStream* pOutputStream)
{
    if (IsChildDoc())
        return;

    rtl::Reference<XFContentContainer> xXFContainer(new XFContentContainer);
    XFConvertFrameInPage(xXFContainer.get());
    xXFContainer->ToXml(pOutputStream);
}

/**
 * @descr    Parse the frame which anchor is to page in the entire document
 */
void LwpDocument::XFConvertFrameInPage(XFContentContainer* pCont)
{
    LwpDocument* pDivision = GetFirstDivision();

    LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(GetDivInfoID().obj().get());
    if (pDivInfo)
    {
        LwpPageLayout* pPageLayout
            = dynamic_cast<LwpPageLayout*>(pDivInfo->GetInitialLayoutID().obj().get());
        if (pPageLayout)
        {
            LwpStory* pStory = dynamic_cast<LwpStory*>(pPageLayout->GetContent().obj().get());
            if (pStory)
                pStory->XFConvertFrameInPage(pCont);
        }
    }
    while (pDivision)
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
    XFTextStyle* pStyle = dynamic_cast<XFTextStyle*>(pXFStyleManager->FindStyle(u"ClickHere"));
    if (pStyle)
    {
        pStyle->SetStyleName(u"Placeholder"_ustr);
    }
}
LwpDocSock::LwpDocSock(LwpObjectHeader const& objHdr, LwpSvStream* pStrm)
    : LwpDLNFVList(objHdr, pStrm)
{
}

/**
 * @descr    read VO_DOCSOCK from file
 */
void LwpDocSock::Read()
{
    LwpDLNFVList::Read();
    m_Doc.ReadIndexed(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}
/**
 * @descr    register styles in documents plugged
 */
void LwpDocSock::RegisterStyle()
{
    rtl::Reference<LwpObject> pDoc = GetNext().obj();
    if (pDoc.is())
        pDoc->DoRegisterStyle();

    pDoc = GetChildHead().obj();
    if (pDoc.is())
        pDoc->DoRegisterStyle();
}
/**
 * @descr    parse contents of documents plugged
 */
void LwpDocSock::Parse(IXFStream* pOutputStream)
{
    rtl::Reference<LwpObject> pDoc = GetChildHead().obj();
    if (pDoc.is())
        pDoc->DoParse(pOutputStream);

    pDoc = GetNext().obj();
    if (pDoc.is())
        pDoc->DoParse(pOutputStream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
