/*************************************************************************
 *
 *  $RCSfile: unosect.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2000-11-08 12:42:49 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
#endif

#include <cmdid.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <so3/linkmgr.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

/******************************************************************
 *
 ******************************************************************/
TYPEINIT1(SwXTextSection, SwClient);
struct SwTextSectionProperties_Impl
{

    String  sCondition;
    String  sLinkFileName;
    String  sSectionFilter;
    String  sSectionRegion;

    SwFmtCol*   pColItem;
    SvxBrushItem* pBrushItem;
    sal_Bool    bDDE;
    sal_Bool    bHidden;
    sal_Bool    bProtect;

    SwTextSectionProperties_Impl() :
        bDDE(0),
        bHidden(0),
        bProtect(0),
        pColItem(0),
        pBrushItem(0){}

    ~SwTextSectionProperties_Impl()
    {
        delete pColItem;
        delete pBrushItem;
    }
};
/* -----------------------------11.07.00 12:10--------------------------------

 ---------------------------------------------------------------------------*/
SwXTextSection* SwXTextSection::GetImplementation(Reference< XInterface> xRef )
{
    uno::Reference<lang::XUnoTunnel> xTunnel( xRef, uno::UNO_QUERY);
    if(xTunnel.is())
        return (SwXTextSection*)xTunnel->getSomething(SwXTextSection::getUnoTunnelId());
    return 0;
}

/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextSection::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextSection::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/*-- 10.12.98 14:47:05---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSection::SwXTextSection(SwSectionFmt* pFmt) :
        SwClient(pFmt),
        aLstnrCntnr( (text::XTextContent*)this),
        aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_SECTION)),
//          _pMap(aSwMapProvider.getPropertyMap(PROPERTY_MAP_SECTION)),
        m_bIsDescriptor(pFmt == 0),
        pProps(pFmt ? 0 : new SwTextSectionProperties_Impl)
{

}
/*-- 10.12.98 14:47:07---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSection::~SwXTextSection()
{
    delete pProps;
}
/*-- 10.12.98 14:47:08---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextSection >  SwXTextSection::getParentSection(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextSection >  aRef;
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
    {
        SwSectionFmt* pParentFmt = pSectFmt->GetParent();
        if(pParentFmt)
        {
            SwXTextSection* pxSect = (SwXTextSection*)SwClientIter(*pParentFmt).
                                                        First(TYPE(SwXTextSection));
            if(pxSect)
                aRef = pxSect;
            else
                aRef = new SwXTextSection(pParentFmt);
        }
    }
    else
        throw uno::RuntimeException();
    return aRef;
}
/*-- 10.12.98 14:47:08---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Reference< text::XTextSection >  > SwXTextSection::getChildSections(void)
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence<uno::Reference< text::XTextSection > > aSeq;
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
    {
        SwSections aChildren;
        pSectFmt->GetChildSections(aChildren, SORTSECT_NOT, sal_False);
        aSeq.realloc(aChildren.Count());
        uno::Reference< text::XTextSection > * pArray = aSeq.getArray();
        for(sal_uInt16 i = 0; i < aChildren.Count(); i++)
        {
            SwSectionFmt* pChild = aChildren.GetObject(i)->GetFmt();
            SwXTextSection* pxSect = (SwXTextSection*)SwClientIter(*pChild).
                                                        First(TYPE(SwXTextSection));
            if(pxSect)
                pArray[i] = pxSect;
            else
                pArray[i] = new SwXTextSection(pChild);
        }
    }
    return aSeq;

}
/* -----------------18.02.99 13:31-------------------
 *
 * --------------------------------------------------*/
void SwXTextSection::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();

    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc)
    {
        SwUnoInternalPaM aPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        UnoActionContext aCont(pDoc);
        pDoc->StartUndo( UNDO_INSSECTION );

        sal_Bool bRet = sal_False;

        SwSection* pRet = 0;
        if(!m_sName.Len())
            m_sName =  C2S("TextSection");
        SwSection aSect(pProps->bDDE ? DDE_LINK_SECTION :
            pProps->sLinkFileName.Len() || pProps->sSectionRegion.Len() ?  FILE_LINK_SECTION :
                                                                CONTENT_SECTION,
                                            pDoc->GetUniqueSectionName(&m_sName));
        aSect.SetCondition(pProps->sCondition);
        String sLinkName(pProps->sLinkFileName);
        sLinkName += cTokenSeperator;
        sLinkName += pProps->sSectionFilter;
        sLinkName += cTokenSeperator;
        sLinkName += pProps->sSectionRegion;
        aSect.SetLinkFileName(sLinkName);

        aSect.SetHidden(pProps->bHidden);
        aSect.SetProtect(pProps->bProtect);
        SfxItemSet aSet(pDoc->GetAttrPool(),
                    RES_COL, RES_COL,
                    RES_BACKGROUND, RES_BACKGROUND,
                    0);
            if(pProps->pBrushItem)
                aSet.Put(*pProps->pBrushItem);
            if(pProps->pColItem)
                aSet.Put(*pProps->pColItem);
        pRet = pDoc->Insert( aPam, aSect, aSet.Count() ? &aSet : 0 );
        pRet->GetFmt()->Add(this);
        // Undo-Klammerung hier beenden
        pDoc->EndUndo( UNDO_INSSECTION );
        DELETEZ(pProps);
        m_bIsDescriptor = sal_False;
    }
    else
        throw lang::IllegalArgumentException();
}
/*-- 10.12.98 14:47:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::attach(const uno::Reference< text::XTextRange > & xTextRange)
                    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
}

/*-- 10.12.98 14:47:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextSection::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
    {
        const SwSection* pSect;
        const SwNodeIndex* pIdx;
        if( 0 != ( pSect = pSectFmt->GetSection() ) &&
            0 != ( pIdx = pSectFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            SwPaM aPaM(*pIdx);
            aPaM.Move( fnMoveForward, fnGoCntnt );
            SwCursor aCrsr(*aPaM.GetPoint());
            aCrsr.SetMark();
            aCrsr.MoveRegion( fnRegionCurr, fnRegionEnd );
            xRet = ::CreateTextRangeFromPosition(pSectFmt->GetDoc(),
                *aCrsr.Start(), aCrsr.End());
        }
    }
    return xRet;
}
/*-- 10.12.98 14:47:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*  pSectFmt = GetFmt();
    if(pSectFmt)
        pSectFmt->GetDoc()->DelSectionFmt( pSectFmt );
    else
        throw uno::RuntimeException();
}
/*-- 10.12.98 14:47:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 10.12.98 14:47:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 10.12.98 14:47:11---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextSection::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = aPropSet.getPropertySetInfo();
    return aRef;
}
/*-- 10.12.98 14:47:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::setPropertyValue(
    const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt || m_bIsDescriptor)
    {
        SwSection   aSection(CONTENT_SECTION, aEmptyStr);
        SwSection* pSect = pFmt ? pFmt->GetSection() : 0;
        if(pFmt)
            aSection = *pSect;
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            SfxItemSet* pNewAttrSet = 0;
            switch(pMap->nWID)
            {
                case WID_SECT_CONDITION:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    if(m_bIsDescriptor)
                        pProps->sCondition = String(uTmp);
                    else
                        aSection.SetCondition(uTmp);
                }
                break;
                case WID_SECT_DDE_TYPE      :
                case WID_SECT_DDE_FILE      :
                case WID_SECT_DDE_ELEMENT   :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    String sTmp(uTmp);
                    if(m_bIsDescriptor)
                    {
                        if(!pProps->bDDE)
                        {
                            pProps->sLinkFileName = cTokenSeperator;
                            pProps->sLinkFileName += cTokenSeperator;
                            pProps->bDDE = sal_True;
                        }
                        pProps->sLinkFileName.SetToken(pMap->nWID - WID_SECT_DDE_TYPE,cTokenSeperator,sTmp);
                    }
                    else
                    {
                        String sLinkFileName(aSection.GetLinkFileName());
                        if(aSection.GetType() != DDE_LINK_SECTION)
                        {
                            sLinkFileName = cTokenSeperator;
                            sLinkFileName += cTokenSeperator;
                            aSection.SetType(DDE_LINK_SECTION);
                        }
                        sLinkFileName.SetToken(pMap->nWID - WID_SECT_DDE_TYPE,cTokenSeperator, sTmp);
                        aSection.SetLinkFileName(sLinkFileName);
                    }
                }
                break;
                case WID_SECT_LINK     :
                {
                    if(aValue.getValueType() == ::getCppuType((const text::SectionFileLink*)0))
                    {
                         text::SectionFileLink* pLink =  (text::SectionFileLink*)   aValue.getValue();
                        if(m_bIsDescriptor)
                        {
                            pProps->bDDE = sal_False;
                            pProps->sLinkFileName = String(pLink->FileURL);
                            pProps->sSectionFilter = String(pLink->FilterName);
                        }
                        else
                        {
                            if(aSection.GetType() != FILE_LINK_SECTION &&
                                pLink->FileURL.len())
                                aSection.SetType(FILE_LINK_SECTION);
                            String sFileName(URIHelper::SmartRelToAbs( pLink->FileURL) );
                            sFileName += cTokenSeperator;
                            sFileName += String(pLink->FilterName);
                            sFileName += cTokenSeperator;
                            sFileName += aSection.GetLinkFileName().GetToken( 2, cTokenSeperator );
                            aSection.SetLinkFileName(sFileName);
                            if(sFileName.Len() < 3)
                                aSection.SetType(CONTENT_SECTION);
                        }
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                break;
                case WID_SECT_REGION :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    String sLink(uTmp);
                    if(m_bIsDescriptor)
                    {
                        pProps->bDDE = sal_False;
                        pProps->sSectionRegion = sLink.GetToken(2, cTokenSeperator);
                    }
                    else
                    {
                        if(aSection.GetType() != FILE_LINK_SECTION &&
                                sLink.Len())
                                aSection.SetType(FILE_LINK_SECTION);
                        String sSectLink(aSection.GetLinkFileName());
                        while( 3 < sSectLink.GetTokenCount( cTokenSeperator ))
                        {
                            sSectLink += cTokenSeperator;
                        }
                        sSectLink.SetToken(2, cTokenSeperator, sLink);
                        aSection.SetLinkFileName(sSectLink);
                    }
                }
                break;
                case WID_SECT_VISIBLE   :
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    if(m_bIsDescriptor)
                        pProps->bHidden = !bVal;
                    else
                        aSection.SetHidden(!bVal);
                }
                break;
                case WID_SECT_PROTECTED:
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    if(m_bIsDescriptor)
                        pProps->bProtect = bVal;
                    else
                        aSection.SetProtect(bVal);
                }
                break;
                default:
                    if(pFmt)
                    {
                        const SfxItemSet& rOldAttrSet = pFmt->GetAttrSet();
                        pNewAttrSet = new SfxItemSet(*rOldAttrSet.GetPool(),
                                                    pMap->nWID, pMap->nWID, 0);
                        pNewAttrSet->Put(rOldAttrSet);
                        aPropSet.setPropertyValue(rPropertyName, aValue, *pNewAttrSet);
                    }
                    else
                    {
                        SfxPoolItem* pPutItem = 0;
                        if(RES_COL == pMap->nWID)
                        {
                            if(!pProps->pColItem)
                                pProps->pColItem = new SwFmtCol;
                                pPutItem = pProps->pColItem;
                        }
                        else //if(RES_BACKGROUND == pMap->nWID)
                        {
                            if(!pProps->pBrushItem)
                                pProps->pBrushItem = new SvxBrushItem;
                            pPutItem = pProps->pBrushItem;
                        }
                        pPutItem->PutValue(aValue, pMap->nMemberId);
                    }

            }
            if(pFmt)
            {
                SwDoc* pDoc = pFmt->GetDoc();
                const SwSectionFmts& rFmts = pDoc->GetSections();
                UnoActionContext aContext(pDoc);
                for( sal_uInt16 i = 0; i < rFmts.Count(); i++ )
                {
                    if(rFmts[i]->GetSection()->GetName() == pSect->GetName())
                    {
                        pDoc->ChgSection( i, aSection, pNewAttrSet, pDoc->IsInReading());
                        break;
                    }
                }
                delete pNewAttrSet;
            }
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();
}
/*-- 10.12.98 14:47:12---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextSection::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt||m_bIsDescriptor)
    {
        SwSection* pSect = pFmt ? pFmt->GetSection() : 0;
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                            aPropSet.getPropertyMap(), rPropertyName);
        if(pMap)
        {
            switch(pMap->nWID)
            {
                case WID_SECT_CONDITION:
                {
                    OUString uTmp(
                        m_bIsDescriptor ? pProps->sCondition : pSect->GetCondition());
                    aRet <<= uTmp;
                }
                break;
                case WID_SECT_DDE_TYPE      :
                case WID_SECT_DDE_FILE      :
                case WID_SECT_DDE_ELEMENT   :
                {
                    String sRet;
                    if(m_bIsDescriptor)
                    {
                        if(pProps->bDDE)
                            sRet = pProps->sLinkFileName;
                    }
                    else if( DDE_LINK_SECTION == pSect->GetType() )
                    {
                        sRet = pSect->GetLinkFileName();
                    }
                    sRet = sRet.GetToken(pMap->nWID - WID_SECT_DDE_TYPE, cTokenSeperator);
                    aRet <<= OUString(sRet);
                }
                break;
                case WID_SECT_LINK     :
                {
                     text::SectionFileLink aLink;
                    if(m_bIsDescriptor)
                    {
                        if(!pProps->bDDE)
                        {
                            aLink.FileURL = pProps->sLinkFileName;
                            aLink.FilterName = pProps->sSectionFilter;
                        }
                    }
                    else if( FILE_LINK_SECTION == pSect->GetType() )
                    {
                        String sRet( pSect->GetLinkFileName() );
                        aLink.FileURL = sRet.GetToken(0, cTokenSeperator );
                        aLink.FilterName = sRet.GetToken(1, cTokenSeperator );
                    }
                    aRet.setValue(&aLink, ::getCppuType((text::SectionFileLink*)0));
                }
                break;
                case WID_SECT_REGION :
                {
                    String sRet;
                    if(m_bIsDescriptor)
                    {
                        sRet = pProps->sSectionRegion;
                    }
                    else if( FILE_LINK_SECTION == pSect->GetType() )
                        sRet = pSect->GetLinkFileName().GetToken(2, cTokenSeperator);
                    aRet <<= OUString(sRet);
                }
                break;
                case WID_SECT_VISIBLE   :
                {
                    sal_Bool bTemp = m_bIsDescriptor ? !pProps->bHidden : !pSect->IsHidden();
                    aRet.setValue( &bTemp, ::getCppuBooleanType());
                }
                break;
                case WID_SECT_PROTECTED:
                {
                    sal_Bool bTemp = m_bIsDescriptor ? pProps->bProtect : pSect->IsProtect();
                    aRet.setValue( &bTemp, ::getCppuBooleanType());
                }
                break;
                case  FN_PARAM_LINK_DISPLAY_NAME:
                {
                    if(pFmt)
                        aRet <<= OUString(pFmt->GetSection()->GetName());
                }
                break;
                default:
                    if(pFmt)
                        aRet = aPropSet.getPropertyValue(rPropertyName, pFmt->GetAttrSet());
                    else
                    {
                        const SfxPoolItem* pQueryItem = 0;
                        if(RES_COL == pMap->nWID)
                        {
                            if(!pProps->pColItem)
                                pProps->pColItem = new SwFmtCol;
                                pQueryItem = pProps->pColItem;
                        }
                        else //if(RES_BACKGROUND == pMap->nWID)
                        {
                            if(!pProps->pBrushItem)
                                pProps->pBrushItem = new SvxBrushItem;
                            pQueryItem = pProps->pBrushItem;
                        }
                        pQueryItem->QueryValue(aRet, pMap->nMemberId);
                    }
            }
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 10.12.98 14:47:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 10.12.98 14:47:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 10.12.98 14:47:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 10.12.98 14:47:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

/*-- 08.11.00 10:47:55---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState SwXTextSection::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aNames(1);
    aNames.getArray()[0] = rPropertyName;
    return getPropertyStates(aNames).getConstArray()[0];
}
/*-- 08.11.00 10:47:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState > SwXTextSection::getPropertyStates(
    const Sequence< OUString >& rPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< PropertyState > aStates(rPropertyNames.getLength());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt||m_bIsDescriptor)
    {
        PropertyState* pStates = aStates.getArray();
        const OUString* pNames = rPropertyNames.getConstArray();
        for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
        {
            pStates[i] = PropertyState_DEFAULT_VALUE;
            const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                aPropSet.getPropertyMap(), pNames[i]);
            if(!pMap)
            {
                UnknownPropertyException aExcept;
                aExcept.Message = pNames[i];
                throw aExcept;
            }
            switch(pMap->nWID)
            {
                case WID_SECT_CONDITION:
                case WID_SECT_DDE_TYPE      :
                case WID_SECT_DDE_FILE      :
                case WID_SECT_DDE_ELEMENT   :
                case WID_SECT_LINK     :
                case WID_SECT_REGION :
                case WID_SECT_VISIBLE   :
                case WID_SECT_PROTECTED:
                case  FN_PARAM_LINK_DISPLAY_NAME:
                    pStates[i] = PropertyState_DIRECT_VALUE;
                break;
                default:
                    if(pFmt)
                        pStates[i] = aPropSet.getPropertyState(pNames[i], pFmt->GetAttrSet());
                    else
                    {
                        const SfxPoolItem* pQueryItem = 0;
                        if(RES_COL == pMap->nWID)
                        {
                            if(!pProps->pColItem)
                                pStates[i] = PropertyState_DEFAULT_VALUE;
                            else
                                pStates[i] = PropertyState_DIRECT_VALUE;
                        }
                        else //if(RES_BACKGROUND == pMap->nWID)
                        {
                            if(!pProps->pBrushItem)
                                pStates[i] = PropertyState_DEFAULT_VALUE;
                            else
                                pStates[i] = PropertyState_DIRECT_VALUE;
                        }
                    }
            }
        }
    }
    else
        throw RuntimeException();
    return aStates;
}
/*-- 08.11.00 10:47:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt||m_bIsDescriptor)
    {
        SwSection   aSection(CONTENT_SECTION, aEmptyStr);
        SwSection* pSect = pFmt ? pFmt->GetSection() : 0;
        if(pFmt)
            aSection = *pSect;
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                aPropSet.getPropertyMap(), rPropertyName);
        if(!pMap)
            throw UnknownPropertyException();
        SfxItemSet* pNewAttrSet = 0;
        switch(pMap->nWID)
        {
            case WID_SECT_CONDITION:
            {
                if(m_bIsDescriptor)
                    pProps->sCondition = aEmptyStr;
                else
                    aSection.SetCondition(aEmptyStr);
            }
            break;
            case WID_SECT_DDE_TYPE      :
            case WID_SECT_DDE_FILE      :
            case WID_SECT_DDE_ELEMENT   :
            case WID_SECT_LINK     :
            case WID_SECT_REGION :
                aSection.SetType(CONTENT_SECTION);
            break;
            case WID_SECT_VISIBLE   :
            {
                if(m_bIsDescriptor)
                    pProps->bHidden = FALSE;
                else
                    aSection.SetHidden(FALSE);
            }
            break;
            case WID_SECT_PROTECTED:
            {
                if(m_bIsDescriptor)
                    pProps->bProtect = FALSE;
                else
                    aSection.SetProtect(FALSE);
            }
            break;
            default:
                if(pFmt)
                {
                    const SfxItemSet& rOldAttrSet = pFmt->GetAttrSet();
                    pNewAttrSet = new SfxItemSet(*rOldAttrSet.GetPool(),
                                                pMap->nWID, pMap->nWID, 0);
                    pNewAttrSet->ClearItem(pMap->nWID);
                }
                else
                {
                    if(RES_COL == pMap->nWID)
                        DELETEZ(pProps->pColItem);
                    else //if(RES_BACKGROUND == pMap->nWID)
                        DELETEZ(pProps->pBrushItem);
                }

        }
        if(pFmt)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SwSectionFmts& rFmts = pDoc->GetSections();
            UnoActionContext aContext(pDoc);
            for( sal_uInt16 i = 0; i < rFmts.Count(); i++ )
            {
                if(rFmts[i]->GetSection()->GetName() == pSect->GetName())
                {
                    pDoc->ChgSection( i, aSection, pNewAttrSet, pDoc->IsInReading());
                    break;
                }
            }
            delete pNewAttrSet;
        }
    }
    else
        throw RuntimeException();
}
/*-- 08.11.00 10:47:56---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXTextSection::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwSectionFmt*   pFmt = GetFmt();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                            aPropSet.getPropertyMap(), rPropertyName);
    switch(pMap->nWID)
    {
        case WID_SECT_CONDITION:
        case WID_SECT_DDE_TYPE      :
        case WID_SECT_DDE_FILE      :
        case WID_SECT_DDE_ELEMENT   :
        case WID_SECT_REGION :
        case FN_PARAM_LINK_DISPLAY_NAME:
            aRet <<= OUString();
        break;
        case WID_SECT_LINK     :
            aRet <<= SectionFileLink();
        break;
        case WID_SECT_VISIBLE   :
        {
            sal_Bool bTemp = TRUE;
            aRet.setValue( &bTemp, ::getCppuBooleanType());
        }
        break;
        case WID_SECT_PROTECTED:
        {
            sal_Bool bTemp = FALSE;
            aRet.setValue( &bTemp, ::getCppuBooleanType());
        }
        break;
        default:
        if(pFmt)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SfxPoolItem& rDefItem =
                pDoc->GetAttrPool().GetDefaultItem(pMap->nWID);
            rDefItem.QueryValue(aRet, pMap->nMemberId);
        }
    }
    return aRet;
}
/*-- 10.12.98 14:47:15---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextSection::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sRet;
    const SwSectionFmt* pFmt = GetFmt();
    if(pFmt)
        sRet = pFmt->GetSection()->GetName();
    else if(m_bIsDescriptor)
        sRet = m_sName;
    else
        throw uno::RuntimeException();
    return sRet;
}
/*-- 10.12.98 14:47:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::setName(const OUString& rName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwSectionFmt*   pFmt = GetFmt();
    if(pFmt)
    {
        SwSection   aSection(CONTENT_SECTION, aEmptyStr);
        SwSection* pSect = pFmt->GetSection();
        aSection = *pSect;
        String sNewName(rName);
        aSection.SetName(sNewName);

        const SwSectionFmts& rFmts = pFmt->GetDoc()->GetSections();
        sal_uInt16 nApplyPos = USHRT_MAX;
        for( sal_uInt16 i = 0; i < rFmts.Count(); i++ )
        {
            if(rFmts[i]->GetSection() == pSect)
                nApplyPos = i;
            else if(sNewName == rFmts[i]->GetSection()->GetName())
                throw uno::RuntimeException();
        }
        if(nApplyPos != USHRT_MAX)
        {
            UnoActionContext aContext(pFmt->GetDoc());
            pFmt->GetDoc()->ChgSection( nApplyPos, aSection);
        }
    }
    else if(m_bIsDescriptor)
        m_sName = String(rName);
    else
        throw uno::RuntimeException();
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
OUString SwXTextSection::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextSection");
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
sal_Bool SwXTextSection::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.TextSection") ||
                !rServiceName.compareToAscii("com.sun.star.document.LinkTarget") ||
                    !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}
/* -----------------02.11.99 11:30-------------------

 --------------------------------------------------*/
uno::Sequence< OUString > SwXTextSection::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArr = aRet.getArray();
    pArr[0] = C2U("com.sun.star.text.TextSection");
    pArr[1] = C2U("com.sun.star.document.LinkTarget");
    pArr[2] = C2U("com.sun.star.text.TextContent");
    return aRet;
}

/*-- 10.12.98 14:42:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSection::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    if(pOld && pOld->Which() == RES_REMOVE_UNO_OBJECT &&
        (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
    else
        ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}

