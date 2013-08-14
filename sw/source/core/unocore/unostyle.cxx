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


#include <svx/svxids.hrc>
#include <hintids.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/smplhint.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/style.hxx>
#include <svl/itemiter.hxx>
#include <svx/pageitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/paperinf.hxx>
#include <pagedesc.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <cmdid.h>
#include <unostyle.hxx>
#include <unosett.hxx>
#include <docsh.hxx>
#include <swstyle.h>
#include <paratr.hxx>
#include <unoprnms.hxx>
#include <shellio.hxx>
#include <docstyle.hxx>
#include <unotextbodyhf.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include "unoevent.hxx"
#include <fmtruby.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <istyleaccess.hxx>
#include <GetMetricVal.hxx>
#include <fmtfsize.hxx>
#include <numrule.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <boost/shared_ptr.hpp>

#include "ccoll.hxx"
#include "unocore.hrc"

#include <set>

#define STYLE_FAMILY_COUNT 5            // we have 5 style families
#define TYPE_BOOL       0
#define TYPE_SIZE       1
#define TYPE_BRUSH      2
#define TYPE_ULSPACE    3
#define TYPE_SHADOW     4
#define TYPE_LRSPACE    5
#define TYPE_BOX        6

const unsigned short aStyleByIndex[] =
{
    SFX_STYLE_FAMILY_CHAR,
    SFX_STYLE_FAMILY_PARA,
    SFX_STYLE_FAMILY_PAGE     ,
    SFX_STYLE_FAMILY_FRAME    ,
    SFX_STYLE_FAMILY_PSEUDO
};

// Already implemented autostyle families: 3
#define AUTOSTYLE_FAMILY_COUNT 3
const IStyleAccess::SwAutoStyleFamily aAutoStyleByIndex[] =
{
    IStyleAccess::AUTO_STYLE_CHAR,
    IStyleAccess::AUTO_STYLE_RUBY,
    IStyleAccess::AUTO_STYLE_PARA
};

using namespace ::com::sun::star;

//convert FN_... to RES_ in header and footer itemset
static sal_uInt16 lcl_ConvertFNToRES(sal_uInt16 nFNId)
{
    sal_uInt16 nRes = USHRT_MAX;
    switch(nFNId)
    {
        case FN_UNO_FOOTER_ON:
        case FN_UNO_HEADER_ON:
        break;
        case FN_UNO_FOOTER_BACKGROUND:
        case FN_UNO_HEADER_BACKGROUND:      nRes = RES_BACKGROUND;
        break;
        case FN_UNO_FOOTER_BOX:
        case FN_UNO_HEADER_BOX:             nRes = RES_BOX;
        break;
        case FN_UNO_FOOTER_LR_SPACE:
        case FN_UNO_HEADER_LR_SPACE:        nRes = RES_LR_SPACE;
        break;
        case FN_UNO_FOOTER_SHADOW:
        case FN_UNO_HEADER_SHADOW:          nRes = RES_SHADOW;
        break;
        case FN_UNO_FOOTER_BODY_DISTANCE:
        case FN_UNO_HEADER_BODY_DISTANCE:   nRes = RES_UL_SPACE;
        break;
        case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
        case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
        break;
        case FN_UNO_FOOTER_SHARE_CONTENT:
        case FN_UNO_HEADER_SHARE_CONTENT:   nRes = SID_ATTR_PAGE_SHARED;
        break;
        case FN_UNO_FIRST_SHARE_CONTENT:   nRes = SID_ATTR_PAGE_SHARED_FIRST;
        break;
        case FN_UNO_FOOTER_HEIGHT:
        case FN_UNO_HEADER_HEIGHT:          nRes = SID_ATTR_PAGE_SIZE;
        break;
        case FN_UNO_FOOTER_EAT_SPACING:
        case FN_UNO_HEADER_EAT_SPACING:   nRes = RES_HEADER_FOOTER_EAT_SPACING;
        break;
    }
    return nRes;

}

static SwGetPoolIdFromName lcl_GetSwEnumFromSfxEnum ( SfxStyleFamily eFamily )
{
    switch ( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
            return nsSwGetPoolIdFromName::GET_POOLID_CHRFMT;
        case SFX_STYLE_FAMILY_PARA:
            return nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL;
        case SFX_STYLE_FAMILY_FRAME:
            return nsSwGetPoolIdFromName::GET_POOLID_FRMFMT;
        case SFX_STYLE_FAMILY_PAGE:
            return nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC;
        case SFX_STYLE_FAMILY_PSEUDO:
            return nsSwGetPoolIdFromName::GET_POOLID_NUMRULE;
        default:
            OSL_ENSURE(sal_False, "someone asking for all styles in unostyle.cxx!" );
            return nsSwGetPoolIdFromName::GET_POOLID_CHRFMT;
    }
}

class SwAutoStylesEnumImpl
{
    std::vector<SfxItemSet_Pointer_t> mAutoStyles;
    std::vector<SfxItemSet_Pointer_t>::iterator aIter;
    SwDoc* pDoc;
    IStyleAccess::SwAutoStyleFamily eFamily;
public:
    SwAutoStylesEnumImpl( SwDoc* pInitDoc, IStyleAccess::SwAutoStyleFamily eFam );
    ::sal_Bool hasMoreElements() { return aIter != mAutoStyles.end(); }
    SfxItemSet_Pointer_t nextElement() { return *(aIter++); }
    IStyleAccess::SwAutoStyleFamily getFamily() const { return eFamily; }
    SwDoc* getDoc() const { return pDoc; }
};

/******************************************************************
 * SwXStyleFamilies
 ******************************************************************/
OUString SwXStyleFamilies::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXStyleFamilies");
}

sal_Bool SwXStyleFamilies::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyleFamilies::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.style.StyleFamilies";
    return aRet;
}

SwXStyleFamilies::SwXStyleFamilies(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()),
    pDocShell(&rDocShell),
    pxCharStyles(0),
    pxParaStyles(0),
    pxFrameStyles(0),
    pxPageStyles(0),
    pxNumberingStyles(0)
{

}

SwXStyleFamilies::~SwXStyleFamilies()
{
    delete pxCharStyles;
    delete pxParaStyles;
    delete pxFrameStyles;
    delete pxPageStyles;
    delete pxNumberingStyles;
}

uno::Any SAL_CALL SwXStyleFamilies::getByName(const OUString& Name)
    throw(
        container::NoSuchElementException,
        lang::WrappedTargetException,
        uno::RuntimeException )
{
    SolarMutexGuard aGuard;
// der Index kommt aus const unsigned short aStyleByIndex[] =
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();
    if(Name.compareToAscii("CharacterStyles") == 0 )
        aRet = getByIndex(0);
    else if(Name.compareToAscii("ParagraphStyles") == 0)
        aRet = getByIndex(1);
    else if(Name.compareToAscii("FrameStyles") == 0 )
        aRet = getByIndex(3);
    else if(Name.compareToAscii("PageStyles") == 0 )
        aRet = getByIndex(2);
    else if(Name.compareToAscii("NumberingStyles") == 0 )
        aRet = getByIndex(4);
    else
        throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXStyleFamilies::getElementNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aNames(STYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = "CharacterStyles";
    pNames[1] = "ParagraphStyles";
    pNames[2] = "FrameStyles";
    pNames[3] = "PageStyles";
    pNames[4] = "NumberingStyles";
    return aNames;
}

sal_Bool SwXStyleFamilies::hasByName(const OUString& Name) throw( uno::RuntimeException )
{
    if( Name.compareToAscii("CharacterStyles") == 0 ||
        Name.compareToAscii("ParagraphStyles") == 0 ||
        Name.compareToAscii("FrameStyles") == 0 ||
        Name.compareToAscii("PageStyles") == 0 ||
        Name.compareToAscii("NumberingStyles") == 0 )
        return sal_True;
    else
        return sal_False;
}

sal_Int32 SwXStyleFamilies::getCount(void) throw( uno::RuntimeException )
{
    return STYLE_FAMILY_COUNT;
}

uno::Any SwXStyleFamilies::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(nIndex < 0 || nIndex >= STYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(IsValid())
    {
        uno::Reference< container::XNameContainer >  aRef;
        sal_uInt16 nType = aStyleByIndex[nIndex];
        switch( nType )
        {
            case SFX_STYLE_FAMILY_CHAR:
            {
                if(!pxCharStyles)
                {
                    ((SwXStyleFamilies*)this)->pxCharStyles = new uno::Reference< container::XNameContainer > ();
                    *pxCharStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxCharStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PARA:
            {
                if(!pxParaStyles)
                {
                    ((SwXStyleFamilies*)this)->pxParaStyles = new uno::Reference< container::XNameContainer > ();
                    *pxParaStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxParaStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PAGE     :
            {
                if(!pxPageStyles)
                {
                    ((SwXStyleFamilies*)this)->pxPageStyles = new uno::Reference< container::XNameContainer > ();
                    *pxPageStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxPageStyles;
            }
            break;
            case SFX_STYLE_FAMILY_FRAME    :
            {
                if(!pxFrameStyles)
                {
                    ((SwXStyleFamilies*)this)->pxFrameStyles = new uno::Reference< container::XNameContainer > ();
                    *pxFrameStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxFrameStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PSEUDO:
            {
                if(!pxNumberingStyles)
                {
                    ((SwXStyleFamilies*)this)->pxNumberingStyles = new uno::Reference< container::XNameContainer > ();
                    *pxNumberingStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxNumberingStyles;
            }
            break;
        }
        aRet.setValue(&aRef, ::getCppuType((const uno::Reference<container::XNameContainer>*)0));
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SwXStyleFamilies::getElementType(void)
    throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<container::XNameContainer>*)0);

}

sal_Bool SwXStyleFamilies::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

void SwXStyleFamilies::loadStylesFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& aOptions)
    throw( io::IOException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool    bLoadStyleText = sal_True;
    sal_Bool    bLoadStylePage = sal_True;
    sal_Bool    bLoadStyleOverwrite = sal_True;
    sal_Bool    bLoadStyleNumbering = sal_True;
    sal_Bool    bLoadStyleFrame = sal_True;
    if(IsValid() && !rURL.isEmpty())
    {
        const uno::Any* pVal;
        int nCount = aOptions.getLength();
        const beans::PropertyValue* pArray = aOptions.getConstArray();
        for(int i = 0; i < nCount; i++)
            if( ( pVal = &pArray[i].Value)->getValueType() ==
                    ::getBooleanCppuType() )
            {
                const OUString sName = pArray[i].Name;
                sal_Bool bVal = *(sal_Bool*)pVal->getValue();
                if( sName.equalsAscii(SW_PROP_NAME_STR(UNO_NAME_OVERWRITE_STYLES)))
                    bLoadStyleOverwrite = bVal;
                else if( sName.equalsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_NUMBERING_STYLES)))
                    bLoadStyleNumbering = bVal;
                else if( sName.equalsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_PAGE_STYLES)))
                    bLoadStylePage = bVal;
                else if( sName.equalsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_FRAME_STYLES)))
                    bLoadStyleFrame = bVal;
                else if( sName.equalsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_TEXT_STYLES)))
                    bLoadStyleText = bVal;
            }

        SwgReaderOption aOpt;
        aOpt.SetFrmFmts( bLoadStyleFrame );
        aOpt.SetTxtFmts( bLoadStyleText );
        aOpt.SetPageDescs( bLoadStylePage );
        aOpt.SetNumRules( bLoadStyleNumbering );
        aOpt.SetMerge( !bLoadStyleOverwrite );

        sal_uLong nErr = pDocShell->LoadStylesFromFile( rURL, aOpt, sal_True );
        if( nErr )
            throw io::IOException();
    }
    else
        throw uno::RuntimeException();
}

uno::Sequence< beans::PropertyValue > SwXStyleFamilies::getStyleLoaderOptions(void)
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyValue > aSeq(5);
    beans::PropertyValue* pArray = aSeq.getArray();
    uno::Any aVal;
    sal_Bool bTemp = sal_True;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_TEXT_STYLES)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[1] = beans::PropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_FRAME_STYLES)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[2] = beans::PropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_PAGE_STYLES)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[3] = beans::PropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_NUMBERING_STYLES)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[4] = beans::PropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_OVERWRITE_STYLES)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    return aSeq;
}

/******************************************************************
 * SwXStyleFamily
 ******************************************************************/
OUString SwXStyleFamily::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXStyleFamily");
}

sal_Bool SwXStyleFamily::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyleFamily::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.style.StyleFamily";
    return aRet;
}

SwXStyleFamily::SwXStyleFamily(SwDocShell* pDocSh, sal_uInt16 nFamily) :
        eFamily((SfxStyleFamily)nFamily),
        pBasePool(pDocSh->GetStyleSheetPool()),
        pDocShell(pDocSh)
{
    StartListening(*pBasePool);
}

SwXStyleFamily::~SwXStyleFamily()
{

}

static sal_Int32 lcl_GetCountOrName(const SwDoc &rDoc,
    SfxStyleFamily eFamily, OUString *pString, sal_uInt16 nIndex = USHRT_MAX)
{
    sal_Int32 nCount = 0;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            sal_uInt16 nBaseCount =  RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN  +
                                     RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN;
            nIndex = nIndex - nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetCharFmts()->size();
            for( sal_uInt16 i = 0; i < nArrLen; i++ )
            {
                SwCharFmt* pFmt = (*rDoc.GetCharFmts())[ i ];
                if( pFmt->IsDefault() && pFmt != rDoc.GetDfltCharFmt() )
                    continue;
                if ( IsPoolUserFmt ( pFmt->GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        // the default character format needs to be set to "Default!"
                        if(rDoc.GetDfltCharFmt() == pFmt)
                            SwStyleNameMapper::FillUIName(
                                RES_POOLCOLL_STANDARD, *pString );
                        else
                            *pString = pFmt->GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            sal_uInt16 nBaseCount = RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN +
                                    RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                    RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                    RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                    RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                    RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN;
            nIndex = nIndex - nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetTxtFmtColls()->size();
            for ( sal_uInt16 i = 0; i < nArrLen; i++ )
            {
                SwTxtFmtColl * pColl = (*rDoc.GetTxtFmtColls())[i];
                if ( pColl->IsDefault() )
                    continue;
                if ( IsPoolUserFmt ( pColl->GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = pColl->GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME:
        {
            sal_uInt16 nBaseCount = RES_POOLFRM_END - RES_POOLFRM_BEGIN;
            nIndex = nIndex - nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetFrmFmts()->size();
            for( sal_uInt16 i = 0; i < nArrLen; i++ )
            {
                SwFrmFmt* pFmt = (*rDoc.GetFrmFmts())[ i ];
                if(pFmt->IsDefault() || pFmt->IsAuto())
                    continue;
                if ( IsPoolUserFmt ( pFmt->GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = pFmt->GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE:
        {
            sal_uInt16 nBaseCount = RES_POOLPAGE_END - RES_POOLPAGE_BEGIN;
            nIndex = nIndex - nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nArrLen; ++i)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc(i);

                if ( IsPoolUserFmt ( rDesc.GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = rDesc.GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            sal_uInt16 nBaseCount = RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN;
            nIndex = nIndex - nBaseCount;
            const SwNumRuleTbl& rNumTbl = rDoc.GetNumRuleTbl();
            for(sal_uInt16 i = 0; i < rNumTbl.size(); ++i)
            {
                const SwNumRule& rRule = *rNumTbl[ i ];
                if( rRule.IsAutoRule() )
                    continue;
                if ( IsPoolUserFmt ( rRule.GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = rRule.GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;

        default:
            ;
    }
    return nCount;
}

sal_Int32 SwXStyleFamily::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return lcl_GetCountOrName ( *pDocShell->GetDoc(), eFamily, NULL );
}

uno::Any SwXStyleFamily::getByIndex(sal_Int32 nTempIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if ( nTempIndex >= 0 && nTempIndex < USHRT_MAX )
    {
        sal_uInt16 nIndex = static_cast < sal_uInt16 > ( nTempIndex );
        if(pBasePool)
        {
            OUString sStyleName;
            switch( eFamily )
            {
                case SFX_STYLE_FAMILY_CHAR:
                {
                    if ( nIndex < ( RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLCHR_NORMAL_BEGIN + nIndex), sStyleName );
                    else if ( nIndex < ( RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN  +
                                         RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCHR_HTML_BEGIN
                                                        - RES_POOLCHR_NORMAL_END + RES_POOLCHR_NORMAL_BEGIN
                                                        + nIndex, sStyleName );
                }
                break;
                case SFX_STYLE_FAMILY_PARA:
                {
                    if ( nIndex < ( RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLCOLL_TEXT_BEGIN + nIndex), sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_LISTS_BEGIN
                                                        - RES_POOLCOLL_TEXT_END + RES_POOLCOLL_TEXT_BEGIN
                                                        + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex), sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                         RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLCOLL_REGISTER_BEGIN
                                                         - RES_POOLCOLL_EXTRA_END + RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex), sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                         RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                         RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLCOLL_DOC_BEGIN
                                                         - RES_POOLCOLL_REGISTER_END + RES_POOLCOLL_REGISTER_BEGIN
                                                         - RES_POOLCOLL_EXTRA_END + RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex), sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN +
                                         RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                         RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                         RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLCOLL_HTML_BEGIN
                                                         - RES_POOLCOLL_DOC_END + RES_POOLCOLL_DOC_BEGIN
                                                         - RES_POOLCOLL_REGISTER_END + RES_POOLCOLL_REGISTER_BEGIN
                                                         - RES_POOLCOLL_EXTRA_END + RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex), sStyleName );
                }
                break;
                case SFX_STYLE_FAMILY_FRAME:
                {
                    if ( nIndex < ( RES_POOLFRM_END - RES_POOLFRM_BEGIN ) )
                    {
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLFRM_BEGIN + nIndex), sStyleName );
                    }
                }
                break;
                case SFX_STYLE_FAMILY_PAGE:
                {
                    if ( nIndex < ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN ) )
                    {
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLPAGE_BEGIN + nIndex), sStyleName );
                    }
                }
                break;
                case SFX_STYLE_FAMILY_PSEUDO:
                {
                    if ( nIndex < ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN ) )
                    {
                        SwStyleNameMapper::FillUIName ( static_cast< sal_uInt16 >(RES_POOLNUMRULE_BEGIN + nIndex), sStyleName );
                    }
                }
                break;

                default:
                    ;
            }
            if (sStyleName.isEmpty())
                lcl_GetCountOrName ( *pDocShell->GetDoc(), eFamily, &sStyleName, nIndex );

            if (!sStyleName.isEmpty())
            {
                SfxStyleSheetBase* pBase = pBasePool->Find( sStyleName, eFamily );
                if(pBase)
                {
                    uno::Reference< style::XStyle >  xStyle = _FindStyle(sStyleName);
                    if(!xStyle.is())
                    {
                        xStyle = eFamily == SFX_STYLE_FAMILY_PAGE ?
                            new SwXPageStyle(*pBasePool, pDocShell, eFamily, sStyleName) :
                                eFamily == SFX_STYLE_FAMILY_FRAME ?
                                new SwXFrameStyle(*pBasePool, pDocShell->GetDoc(), pBase->GetName()):
                                    new SwXStyle(*pBasePool, eFamily, pDocShell->GetDoc(), sStyleName);
                    }
                    aRet.setValue(&xStyle, ::getCppuType((uno::Reference<style::XStyle>*)0));
                }
                else
                    throw container::NoSuchElementException();
            }
            else
                throw lang::IndexOutOfBoundsException();
        }
        else
            throw uno::RuntimeException();
    }
    else
        throw lang::IndexOutOfBoundsException();

    return aRet;
}

uno::Any SwXStyleFamily::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), true );
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
        {
            uno::Reference< style::XStyle >  xStyle = _FindStyle(sStyleName);
            if(!xStyle.is())
            {
                xStyle = eFamily == SFX_STYLE_FAMILY_PAGE ?
                    new SwXPageStyle(*pBasePool, pDocShell, eFamily, sStyleName) :
                        eFamily == SFX_STYLE_FAMILY_FRAME ?
                        new SwXFrameStyle(*pBasePool, pDocShell->GetDoc(), pBase->GetName()):
                            new SwXStyle(*pBasePool, eFamily, pDocShell->GetDoc(), sStyleName);
            }
            aRet.setValue(&xStyle, ::getCppuType((uno::Reference<style::XStyle>*)0));
        }
        else
            throw container::NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;

}

uno::Sequence< OUString > SwXStyleFamily::getElementNames(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aRet;
    if(pBasePool)
    {
        SfxStyleSheetIteratorPtr pIterator = pBasePool->CreateIterator(eFamily, SFXSTYLEBIT_ALL);
        sal_uInt16 nCount = pIterator->Count();
        aRet.realloc(nCount);
        OUString* pArray = aRet.getArray();
        OUString aString;
        for(sal_uInt16 i = 0; i < nCount; i++)
        {
            SwStyleNameMapper::FillProgName((*pIterator)[i]->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true );
            pArray[i] = aString;
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

sal_Bool SwXStyleFamily::hasByName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        OUString sStyleName;
        SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), true );
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        bRet = 0 != pBase;
    }
    else
        throw uno::RuntimeException();
    return bRet;

}

uno::Type SwXStyleFamily::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<style::XStyle>*)0);

}

sal_Bool SwXStyleFamily::hasElements(void) throw( uno::RuntimeException )
{
    if(!pBasePool)
        throw uno::RuntimeException();
    return sal_True;
}

void SwXStyleFamily::insertByName(const OUString& rName, const uno::Any& rElement)
        throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        OUString sStyleName;
        SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), true);
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        SfxStyleSheetBase* pUINameBase = pBasePool->Find( sStyleName );
        if(pBase || pUINameBase)
            throw container::ElementExistException();
        else
        {
            if(rElement.getValueType().getTypeClass() ==
                                            uno::TypeClass_INTERFACE)
            {
                uno::Reference< uno::XInterface > * pxRef =
                    (uno::Reference< uno::XInterface > *)rElement.getValue();

                uno::Reference<lang::XUnoTunnel> xStyleTunnel( *pxRef, uno::UNO_QUERY);

                SwXStyle* pNewStyle = 0;
                if(xStyleTunnel.is())
                {
                    pNewStyle = reinterpret_cast< SwXStyle * >(
                            sal::static_int_cast< sal_IntPtr >( xStyleTunnel->getSomething( SwXStyle::getUnoTunnelId()) ));
                }
                if(!pNewStyle || !pNewStyle->IsDescriptor() ||
                    pNewStyle->GetFamily() != eFamily)
                        throw lang::IllegalArgumentException();
                if(pNewStyle)
                {
                    sal_uInt16 nMask = SFXSTYLEBIT_ALL;
                    if(eFamily == SFX_STYLE_FAMILY_PARA && !pNewStyle->IsConditional())
                        nMask &= ~SWSTYLEBIT_CONDCOLL;
#if OSL_DEBUG_LEVEL > 1
                    SfxStyleSheetBase& rNewBase =
#endif
                    pBasePool->Make(sStyleName, eFamily, nMask);
                    pNewStyle->SetDoc(pDocShell->GetDoc(), pBasePool);
                    pNewStyle->SetStyleName(sStyleName);
                    const OUString sParentStyleName(pNewStyle->GetParentStyleName());
                    if (!sParentStyleName.isEmpty())
                    {
                        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
                        SfxStyleSheetBase* pParentBase = pBasePool->Find(sParentStyleName);
                        if(pParentBase && pParentBase->GetFamily() == eFamily &&
                            &pParentBase->GetPool() == pBasePool)
                            pBasePool->SetParent( eFamily, sStyleName, sParentStyleName );

                    }
#if OSL_DEBUG_LEVEL > 1
                    (void)rNewBase;
#endif
                    //so, jetzt sollten noch die Properties des Descriptors angewandt werden
                    pNewStyle->ApplyDescriptorProperties();
                }
                else
                    throw lang::IllegalArgumentException();
            }
            else
                throw lang::IllegalArgumentException();
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXStyleFamily::replaceByName(const OUString& rName, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(rName);
        //Ersetzung geht nur fuer benutzerdefinierte Styles
        if(!pBase)
            throw container::NoSuchElementException();
        if(!pBase->IsUserDefined())
            throw lang::IllegalArgumentException();
        //if theres an object available to this style then it must be invalidated
        uno::Reference< style::XStyle >  xStyle = _FindStyle(pBase->GetName());
        if(xStyle.is())
        {
            uno::Reference<lang::XUnoTunnel> xTunnel( xStyle, uno::UNO_QUERY);
            if(xTunnel.is())
            {
                SwXStyle* pStyle = reinterpret_cast< SwXStyle * >(
                        sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething( SwXStyle::getUnoTunnelId()) ));
                pStyle->Invalidate();
            }
        }

        pBasePool->Remove(pBase);
        insertByName(rName, rElement);
    }
    else
        throw uno::RuntimeException();
}

void SwXStyleFamily::removeByName(const OUString& rName) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        OUString aString;
        SwStyleNameMapper::FillUIName(rName, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true );

        SfxStyleSheetBase* pBase = pBasePool->Find( aString );
        if(pBase)
            pBasePool->Remove(pBase);
        else
            throw container::NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL SwXStyleFamily::getPropertySetInfo(  ) throw (uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL SwXStyleFamily::setPropertyValue( const OUString&, const uno::Any& ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

uno::Any SAL_CALL SwXStyleFamily::getPropertyValue( const OUString& sPropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;

    if ( sPropertyName == "DisplayName" )
    {
        SolarMutexGuard aGuard;
        sal_uInt32 nResId = 0;
        switch ( eFamily )
        {
            case SFX_STYLE_FAMILY_CHAR:
                nResId = STR_STYLE_FAMILY_CHARACTER; break;
            case SFX_STYLE_FAMILY_PARA:
                nResId = STR_STYLE_FAMILY_PARAGRAPH; break;
            case SFX_STYLE_FAMILY_FRAME:
                nResId = STR_STYLE_FAMILY_FRAME; break;
            case SFX_STYLE_FAMILY_PAGE:
                nResId = STR_STYLE_FAMILY_PAGE; break;
            case SFX_STYLE_FAMILY_PSEUDO:
                nResId = STR_STYLE_FAMILY_NUMBERING; break;
            default:
                OSL_FAIL( "SwXStyleFamily::getPropertyValue(): invalid family" );
        }
        if ( nResId > 0 )
        {
            aRet = uno::makeAny( SW_RESSTR( nResId ) );
        }
    }
    else
    {
        throw beans::UnknownPropertyException( OUString("unknown property: ") + sPropertyName, static_cast<OWeakObject *>(this) );
    }

    return aRet;
}

void SAL_CALL SwXStyleFamily::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL SwXStyleFamily::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL SwXStyleFamily::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL SwXStyleFamily::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

void SwXStyleFamily::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint *pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && ( pHint->GetId() & SFX_HINT_DYING ) )
    {
        pBasePool = 0;
        pDocShell = 0;
        EndListening(rBC);
    }
}

SwXStyle*   SwXStyleFamily::_FindStyle(const OUString& rStyleName)const
{
    sal_uInt16  nLCount = pBasePool->GetListenerCount();
    SfxListener* pListener = 0;
    for( sal_uInt16 i = 0; i < nLCount; i++)
    {
        pListener = pBasePool->GetListener( i );
        SwXStyle* pTempStyle = dynamic_cast<SwXStyle*>( pListener );
        if(pTempStyle && pTempStyle->GetFamily() == eFamily && pTempStyle->GetStyleName() == rStyleName)
        {
            return pTempStyle;
        }
    }
    return 0;
}

class SwStyleProperties_Impl
{
    const PropertyEntryVector_t aPropertyEntries;
    uno::Any**                  pAnyArr;
    sal_uInt32                  nArrLen;

public:
    SwStyleProperties_Impl(const SfxItemPropertyMap& rMap);
    ~SwStyleProperties_Impl();

    sal_Bool    SetProperty(const OUString& rName, uno::Any aVal);
    sal_Bool    GetProperty(const OUString& rName, uno::Any*& rpAny);
    sal_Bool    ClearProperty( const OUString& rPropertyName );
    void    ClearAllProperties( );
    void        GetProperty(const OUString &rPropertyName, const uno::Reference < beans::XPropertySet > &rxPropertySet, uno::Any& rAny );

    const PropertyEntryVector_t& GetPropertyVector() const {return aPropertyEntries; }

};

SwStyleProperties_Impl::SwStyleProperties_Impl(const SfxItemPropertyMap& rMap) :
    aPropertyEntries( rMap.getPropertyEntries() ),
    nArrLen(0)
{
    nArrLen = aPropertyEntries.size();

    pAnyArr = new uno::Any* [nArrLen];
    for ( sal_uInt32 i =0 ; i < nArrLen; i++ )
        pAnyArr[i] = 0;
}

SwStyleProperties_Impl::~SwStyleProperties_Impl()
{
    for ( sal_uInt16 i =0 ; i < nArrLen; i++ )
        delete pAnyArr[i];
    delete[] pAnyArr;
}

sal_Bool SwStyleProperties_Impl::SetProperty(const OUString& rName, uno::Any aVal)
{
    sal_uInt16 nPos = 0;
    sal_Bool bRet = sal_False;
    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    while( aIt != aPropertyEntries.end() )
    {
        if(rName == aIt->sName)
        {
            delete pAnyArr[nPos];
            pAnyArr[nPos] = new uno::Any ( aVal );
            bRet = sal_True;
            break;
        }
        ++nPos;
        ++aIt;
    }
    return bRet;
}

sal_Bool SwStyleProperties_Impl::ClearProperty( const OUString& rName )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nPos = 0;
    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    while( aIt != aPropertyEntries.end() )
    {
        if( rName == aIt->sName )
        {
            delete pAnyArr[nPos];
            pAnyArr[ nPos ] = 0;
            bRet = sal_True;
            break;
        }
        ++nPos;
        ++aIt;
    }
    return bRet;
}

void SwStyleProperties_Impl::ClearAllProperties( )
{
    for ( sal_uInt16 i = 0; i < nArrLen; i++ )
    {
        delete pAnyArr[i];
        pAnyArr[ i ] = 0;
    }
}

sal_Bool SwStyleProperties_Impl::GetProperty(const OUString& rName, uno::Any*& rpAny )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nPos = 0;
    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    while( aIt != aPropertyEntries.end() )
    {
        if( rName == aIt->sName )
        {
            rpAny = pAnyArr[nPos];
            bRet = sal_True;
            break;
        }
        ++nPos;
        ++aIt;
    }

    return bRet;
}

void SwStyleProperties_Impl::GetProperty( const OUString &rPropertyName, const uno::Reference < beans::XPropertySet > &rxPropertySet, uno::Any & rAny )
{
    rAny = rxPropertySet->getPropertyValue( rPropertyName );
}

namespace
{
    class theSwXStyleUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXStyleUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXStyle::getUnoTunnelId()
{
    return theSwXStyleUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXStyle::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

TYPEINIT1(SwXStyle, SfxListener);

OUString SwXStyle::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXStyle");
}

sal_Bool SwXStyle::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyle::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    long nCount = 1;
    if(SFX_STYLE_FAMILY_PARA == eFamily)
    {
        nCount = 5;
        if(bIsConditional)
            nCount++;
    }
    else if(SFX_STYLE_FAMILY_CHAR == eFamily)
        nCount = 5;
    else if(SFX_STYLE_FAMILY_PAGE == eFamily)
        nCount = 3;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.style.Style";
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pArray[1] = "com.sun.star.style.CharacterStyle";
            pArray[2] = "com.sun.star.style.CharacterProperties";
            pArray[3] = "com.sun.star.style.CharacterPropertiesAsian";
            pArray[4] = "com.sun.star.style.CharacterPropertiesComplex";
        break;
        case SFX_STYLE_FAMILY_PAGE:
            pArray[1] = "com.sun.star.style.PageStyle";
            pArray[2] = "com.sun.star.style.PageProperties";
        break;
        case SFX_STYLE_FAMILY_PARA:
            pArray[1] = "com.sun.star.style.ParagraphStyle";
            pArray[2] = "com.sun.star.style.ParagraphProperties";
            pArray[3] = "com.sun.star.style.ParagraphPropertiesAsian";
            pArray[4] = "com.sun.star.style.ParagraphPropertiesComplex";
        if(bIsConditional)
            pArray[5] = "com.sun.star.style.ConditionalParagraphStyle";
        break;

        default:
            ;
    }
    return aRet;
}

SwXStyle::SwXStyle( SwDoc *pDoc, SfxStyleFamily eFam, sal_Bool bConditional) :
    m_pDoc( pDoc ),
    pBasePool(0),
    eFamily(eFam),
    bIsDescriptor(sal_True),
    bIsConditional(bConditional)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    // get the property set for the default style data
    // First get the model
    uno::Reference < frame::XModel > xModel = pDoc->GetDocShell()->GetBaseModel();
    // Ask the model for it's family supplier interface
    uno::Reference < style::XStyleFamiliesSupplier > xFamilySupplier ( xModel, uno::UNO_QUERY );
    // Get the style families
    uno::Reference < container::XNameAccess > xFamilies = xFamilySupplier->getStyleFamilies();

    uno::Any aAny;
    sal_uInt16 nMapId = PROPERTY_MAP_NUM_STYLE;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            nMapId = PROPERTY_MAP_CHAR_STYLE;
            aAny = xFamilies->getByName ("CharacterStyles");
            // Get the Frame family (and keep it for later)
            aAny >>= mxStyleFamily;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            nMapId = bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE;
            aAny = xFamilies->getByName ("ParagraphStyles");
            // Get the Frame family (and keep it for later)
            aAny >>= mxStyleFamily;
            aAny = mxStyleFamily->getByName ("Standard");
            aAny >>= mxStyleData;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE:
        {
            nMapId = PROPERTY_MAP_PAGE_STYLE;
            aAny = xFamilies->getByName ("PageStyles");
            // Get the Frame family (and keep it for later)
            aAny >>= mxStyleFamily;
            aAny = mxStyleFamily->getByName ("Standard");
            aAny >>= mxStyleData;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME :
        {
            nMapId = PROPERTY_MAP_FRAME_STYLE;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            nMapId = PROPERTY_MAP_NUM_STYLE;
        }
        break;

        default:
            ;
    }
    pPropImpl = new SwStyleProperties_Impl(aSwMapProvider.GetPropertySet(nMapId)->getPropertyMap());
}

SwXStyle::SwXStyle(SfxStyleSheetBasePool& rPool, SfxStyleFamily eFam,
        SwDoc* pDoc, const OUString& rStyleName) :
    m_pDoc(pDoc),
    m_sStyleName(rStyleName),
    pBasePool(&rPool),
    eFamily(eFam),
    bIsDescriptor(sal_False),
    bIsConditional(sal_False),
    pPropImpl(0)
{
    StartListening(rPool);
    if(eFam == SFX_STYLE_FAMILY_PARA)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );
        if(pBase)
        {
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(m_sStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                bIsConditional = ::IsConditionalByPoolId( nId );
            else
                bIsConditional = RES_CONDTXTFMTCOLL == ((SwDocStyleSheet*)pBase)->GetCollection()->Which();
        }
    }
}

SwXStyle::~SwXStyle()
{
    if(pBasePool)
        EndListening(*pBasePool);
    delete pPropImpl;
}

void SwXStyle::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_pDoc = 0;
        mxStyleData.clear();
        mxStyleFamily.clear();
    }
}

OUString SwXStyle::getName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );
        if(!pBase)
            throw uno::RuntimeException();
        OUString aString;
        SwStyleNameMapper::FillProgName(pBase->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true);
        return aString;
    }
    return m_sStyleName;
}

void SwXStyle::setName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );
        bool bExcept = true;
        if(pBase && pBase->IsUserDefined())
        {
            rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
            bExcept = !xTmp->SetName(rName);
            if(!bExcept)
                m_sStyleName = rName;
        }
        if(bExcept)
            throw uno::RuntimeException();
    }
    else
        m_sStyleName = rName;
}

sal_Bool SwXStyle::isUserDefined(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        //if it is not found it must be non user defined
        if(pBase)
            bRet = pBase->IsUserDefined();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXStyle::isInUse(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_USED);
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        if(pBase)
            bRet = pBase->IsUsed();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

OUString SwXStyle::getParentStyle(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString aString;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL);
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        if(pBase)
            aString = pBase->GetParent();
    }
    else if(bIsDescriptor)
        aString = m_sParentStyleName;
    else
        throw uno::RuntimeException();
    SwStyleNameMapper::FillProgName(aString, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true );
    return aString;
}

void SwXStyle::setParentStyle(const OUString& rParentStyle)
            throw( container::NoSuchElementException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString sParentStyle;
    SwStyleNameMapper::FillUIName(rParentStyle, sParentStyle, lcl_GetSwEnumFromSfxEnum ( eFamily ), true );
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        bool bExcept = false;
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xBase( new SwDocStyleSheet(*(SwDocStyleSheet*)pBase) );
            //make it a 'real' style - necessary for pooled styles
            xBase->GetItemSet();
            if(xBase->GetParent() != sParentStyle)
            {
                bExcept = !xBase->SetParent(sParentStyle);
            }
        }
        else
            bExcept = true;
        if(bExcept)
            throw uno::RuntimeException();
    }
    else if(bIsDescriptor)
    {
        m_sParentStyleName = sParentStyle;
        try
        {
            uno::Any aAny = mxStyleFamily->getByName ( sParentStyle );
            aAny >>= mxStyleData;
        }
        catch ( container::NoSuchElementException& )
        {
        }
        catch ( lang::WrappedTargetException& )
        {
        }
        catch ( uno::RuntimeException& )
        {
        }
    }
    else
        throw uno::RuntimeException();
}

static uno::Reference< beans::XPropertySetInfo > lcl_getPropertySetInfo( SfxStyleFamily eFamily, sal_Bool bIsConditional )
{
    uno::Reference< beans::XPropertySetInfo >  xRet;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            static uno::Reference< beans::XPropertySetInfo >  xCharRef;
            if(!xCharRef.is())
            {
                xCharRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_STYLE)->getPropertySetInfo();
            }
            xRet = xCharRef;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            static uno::Reference< beans::XPropertySetInfo > xCondParaRef;
            static uno::Reference< beans::XPropertySetInfo >  xParaRef;
            if(!xParaRef.is())
            {
                xCondParaRef = aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_CONDITIONAL_PARA_STYLE)->getPropertySetInfo();
                xParaRef = aSwMapProvider.GetPropertySet(
                    PROPERTY_MAP_PARA_STYLE)->getPropertySetInfo();
            }
            xRet = bIsConditional ? xCondParaRef : xParaRef;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE     :
        {
            static uno::Reference< beans::XPropertySetInfo >  xPageRef;
            if(!xPageRef.is())
            {
                xPageRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE)->getPropertySetInfo();
            }
            xRet = xPageRef;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME    :
        {
            static uno::Reference< beans::XPropertySetInfo >  xFrameRef;
            if(!xFrameRef.is())
            {
                xFrameRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_FRAME_STYLE)->getPropertySetInfo();
            }
            xRet = xFrameRef;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            static uno::Reference< beans::XPropertySetInfo >  xNumRef;
            if(!xNumRef.is())
            {
                xNumRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_NUM_STYLE)->getPropertySetInfo();
            }
            xRet = xNumRef;
        }
        break;

        default:
            ;
    }
    return xRet;
}

uno::Reference< beans::XPropertySetInfo >  SwXStyle::getPropertySetInfo(void)
    throw( uno::RuntimeException )
{
    return lcl_getPropertySetInfo( eFamily, bIsConditional );
}

void    SwXStyle::ApplyDescriptorProperties()
{
    bIsDescriptor = sal_False;
    mxStyleData.clear();
    mxStyleFamily.clear();

    const PropertyEntryVector_t& rPropertyVector = pPropImpl->GetPropertyVector();
    PropertyEntryVector_t::const_iterator aIt = rPropertyVector.begin();
    while(aIt != rPropertyVector.end())
    {
        uno::Any* pAny;
        pPropImpl->GetProperty(aIt->sName, pAny);
        if(pAny)
            setPropertyValue(aIt->sName, *pAny);
        ++aIt;
    }
}

struct SwStyleBase_Impl
{
    SwDoc&              rDoc;

    const SwPageDesc*   pOldPageDesc;

    rtl::Reference< SwDocStyleSheet > mxNewBase;
    SfxItemSet*         pItemSet;

    const OUString&     rStyleName;
    sal_uInt16              nPDescPos;

    SwStyleBase_Impl(SwDoc& rSwDoc, const OUString& rName) :
        rDoc(rSwDoc),
        pOldPageDesc(0),
        pItemSet(0),
        rStyleName(rName),
        nPDescPos(0xffff)
        {}

    ~SwStyleBase_Impl(){ delete pItemSet; }

    sal_Bool HasItemSet() {return mxNewBase.is();}
    SfxItemSet& GetItemSet()
        {
            OSL_ENSURE(mxNewBase.is(), "no SwDocStyleSheet available");
            if(!pItemSet)
                pItemSet = new SfxItemSet(mxNewBase->GetItemSet());
            return *pItemSet;
        }

        const SwPageDesc& GetOldPageDesc();
};

const SwPageDesc& SwStyleBase_Impl::GetOldPageDesc()
{
    if(!pOldPageDesc)
    {
        sal_uInt16 i;
        sal_uInt16 nPDescCount = rDoc.GetPageDescCnt();
        for(i = 0; i < nPDescCount; i++)
        {
            const SwPageDesc& rDesc = rDoc.GetPageDesc( i );
            if(rDesc.GetName() == rStyleName)
            {
                pOldPageDesc = & rDesc;
                nPDescPos = i;
                break;
            }
        }
        if(!pOldPageDesc)
        {
            for(i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
            {
                const OUString aFmtName(SW_RES(i));
                if(aFmtName == rStyleName)
                {
                    pOldPageDesc = rDoc.GetPageDescFromPool( static_cast< sal_uInt16 >(RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN) );
                    break;
                }
            }
            for(i = 0; i < nPDescCount + 1; i++)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc( i );
                if(rDesc.GetName() == rStyleName)
                {
                    nPDescPos = i;
                    break;
                }
            }
        }
    }
    return *pOldPageDesc;
}

static void lcl_SetStyleProperty(const SfxItemPropertySimpleEntry& rEntry,
                        const SfxItemPropertySet& rPropSet,
                        const uno::Any& rValue,
                        SwStyleBase_Impl& rBase,
                        SfxStyleSheetBasePool* pBasePool,
                        SwDoc* pDoc,
                        SfxStyleFamily eFamily)
                            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                                lang::WrappedTargetException, uno::RuntimeException)

{
    switch(rEntry.nWID)
    {
        case FN_UNO_HIDDEN:
        {
            sal_Bool bHidden = sal_False;
            if ( rValue >>= bHidden )
            {
                //make it a 'real' style - necessary for pooled styles
                rBase.mxNewBase->GetItemSet();
                rBase.mxNewBase->SetHidden( bHidden );
            }
        }
        break;

        case RES_PAPER_BIN:
        {
            SfxPrinter *pPrinter = pDoc->getPrinter( true );
            OUString sTmp;
            sal_uInt16 nBin = USHRT_MAX;
            if ( !( rValue >>= sTmp ) )
                throw lang::IllegalArgumentException();
            if ( sTmp.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "[From printer settings]" ) ) )
                nBin = USHRT_MAX-1;
            else if ( pPrinter )
            {
                for (sal_uInt16 i=0, nEnd = pPrinter->GetPaperBinCount(); i < nEnd; i++ )
                {
                    if (sTmp == OUString ( pPrinter->GetPaperBinName ( i ) ) )
                    {
                        nBin = i;
                        break;
                    }
                }
            }
            if ( nBin == USHRT_MAX )
                throw lang::IllegalArgumentException();
            else
            {
                SfxItemSet& rStyleSet = rBase.GetItemSet();
                SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
                aSet.SetParent(&rStyleSet);
                rPropSet.setPropertyValue(rEntry, uno::makeAny ( static_cast < sal_Int8 > ( nBin == USHRT_MAX-1 ? -1 : nBin ) ), aSet);
                rStyleSet.Put(aSet);
            }
        }
        break;
        case  FN_UNO_NUM_RULES: //Sonderbehandlung fuer das SvxNumRuleItem:
        {
            if(rValue.getValueType() == ::getCppuType((uno::Reference< container::XIndexReplace>*)0) )
            {
                uno::Reference< container::XIndexReplace > * pxRulesRef =
                        (uno::Reference< container::XIndexReplace > *)rValue.getValue();

                uno::Reference<lang::XUnoTunnel> xNumberTunnel( *pxRulesRef, uno::UNO_QUERY);

                SwXNumberingRules* pSwXRules = 0;
                if(xNumberTunnel.is())
                {
                    pSwXRules = reinterpret_cast< SwXNumberingRules * >(
                            sal::static_int_cast< sal_IntPtr >(xNumberTunnel->getSomething( SwXNumberingRules::getUnoTunnelId()) ));
                }
                if(pSwXRules)
                {
                    const String* pCharStyleNames = pSwXRules->GetNewCharStyleNames();
                    const String* pBulletFontNames = pSwXRules->GetBulletFontNames();

                    SwNumRule aSetRule(*pSwXRules->GetNumRule());
                    const SwCharFmts* pFmts = pDoc->GetCharFmts();
                    sal_uInt16 nChCount = pFmts->size();
                    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                    {

                        const SwNumFmt* pFmt = aSetRule.GetNumFmt( i );
                        if(pFmt)
                        {
                            SwNumFmt aFmt(*pFmt);
                            if(
                                !SwXNumberingRules::isInvalidStyle(pCharStyleNames[i]) &&
                                ((pCharStyleNames[i].Len() && !pFmt->GetCharFmt()) ||
                                (pCharStyleNames[i].Len() &&
                                            pFmt->GetCharFmt()->GetName() != pCharStyleNames[i]) ))
                            {

                                SwCharFmt* pCharFmt = 0;
                                if(pCharStyleNames[i].Len())
                                {
                                    for(sal_uInt16 j = 0; j< nChCount; j++)
                                    {
                                        SwCharFmt* pTmp = (*pFmts)[j];
                                        if(pTmp->GetName() == pCharStyleNames[i])
                                        {
                                            pCharFmt = pTmp;
                                            break;
                                        }
                                    }
                                    if(!pCharFmt)
                                    {

                                        SfxStyleSheetBase* pBase;
                                        pBase = ((SfxStyleSheetBasePool*)pBasePool)->Find(pCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                                        if(!pBase)
                                            pBase = &pBasePool->Make(pCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                                        pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();

                                    }

                                    aFmt.SetCharFmt( pCharFmt );
                                }
                            }
                            //jetzt nochmal fuer Fonts
                            if (!SwXNumberingRules::isInvalidStyle(pBulletFontNames[i]) &&
                                ((pBulletFontNames[i].Len() && !pFmt->GetBulletFont()) ||
                                (pBulletFontNames[i].Len() &&
                                        pFmt->GetBulletFont()->GetName() != pBulletFontNames[i]) ))
                            {
                                const SvxFontListItem* pFontListItem =
                                        (const SvxFontListItem* )pDoc->GetDocShell()
                                                            ->GetItem( SID_ATTR_CHAR_FONTLIST );
                                const FontList*  pList = pFontListItem->GetFontList();
                                FontInfo aInfo = pList->Get(
                                    pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                                Font aFont(aInfo);
                                aFmt.SetBulletFont(&aFont);
                            }
                            aSetRule.Set( i, &aFmt );
                        }
                    }
                    rBase.mxNewBase->SetNumRule(aSetRule);
                }
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case RES_PARATR_OUTLINELEVEL:
        {
            sal_Int16 nLevel = 0;
               rValue >>= nLevel;
            if( 0 <= nLevel && nLevel <= MAXLEVEL)
                rBase.mxNewBase->GetCollection()->SetAttrOutlineLevel( nLevel );
        }
        break;
        case FN_UNO_FOLLOW_STYLE:
        {
            OUString sTmp;
            rValue >>= sTmp;
            OUString aString;
            SwStyleNameMapper::FillUIName(sTmp, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true ) ;
            rBase.mxNewBase->SetFollow( aString );
        }
        break;
        case RES_PAGEDESC :
        if( MID_PAGEDESC_PAGEDESCNAME != rEntry.nMemberId)
            goto put_itemset;
        {
            // Sonderbehandlung RES_PAGEDESC
            if(rValue.getValueType() != ::getCppuType((const OUString*)0))
                throw lang::IllegalArgumentException();
            SfxItemSet& rStyleSet = rBase.GetItemSet();

            SwFmtPageDesc* pNewDesc = 0;
            const SfxPoolItem* pItem;
            if(SFX_ITEM_SET == rStyleSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
            {
                pNewDesc = new SwFmtPageDesc(*((SwFmtPageDesc*)pItem));
            }
            if(!pNewDesc)
                pNewDesc = new SwFmtPageDesc();
            OUString uDescName;
            rValue >>= uDescName;
            OUString sDescName;
            SwStyleNameMapper::FillUIName(uDescName, sDescName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
            if(!pNewDesc->GetPageDesc() || pNewDesc->GetPageDesc()->GetName() != sDescName)
            {
                sal_Bool bPut = sal_False;
                if (!sDescName.isEmpty())
                {
                    SwPageDesc* pPageDesc = ::GetPageDescByName_Impl(*pDoc, sDescName);
                    if(pPageDesc)
                    {
                        pNewDesc->RegisterToPageDesc( *pPageDesc );
                        bPut = sal_True;
                    }
                    else
                    {
                        throw lang::IllegalArgumentException();
                    }
                }
                if(!bPut)
                {
                    rStyleSet.ClearItem(RES_BREAK);
                    rStyleSet.Put(SwFmtPageDesc());
                }
                else
                    rStyleSet.Put(*pNewDesc);
            }
            delete pNewDesc;
        }
        break;
        case FN_UNO_IS_AUTO_UPDATE:
        {
            sal_Bool bAuto = *(sal_Bool*)rValue.getValue();
            if(SFX_STYLE_FAMILY_PARA == eFamily)
                rBase.mxNewBase->GetCollection()->SetAutoUpdateFmt(bAuto);
            else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                rBase.mxNewBase->GetFrmFmt()->SetAutoUpdateFmt(bAuto);
        }
        break;
        case FN_UNO_PARA_STYLE_CONDITIONS:
        {
            uno::Sequence< beans::NamedValue > aSeq;
            if (!(rValue >>= aSeq))
                throw lang::IllegalArgumentException();

            OSL_ENSURE(COND_COMMAND_COUNT == 28,
                    "invalid size of comman count?");
            const beans::NamedValue *pSeq = aSeq.getConstArray();
            sal_Int32 nLen = aSeq.getLength();

            sal_Bool bFailed = sal_False;
            SwCondCollItem aCondItem;
            for(sal_uInt16 i = 0; i < nLen; i++)
            {
                OUString aTmp;
                if ((pSeq[i].Value >>= aTmp))
                {
                    // get UI style name from programmatic style name
                    OUString aStyleName;
                    SwStyleNameMapper::FillUIName(aTmp, aStyleName,
                            lcl_GetSwEnumFromSfxEnum(eFamily), true);

                    //
                    // check for correct context and style name
                    //
                    sal_Int16 nIdx = GetCommandContextIndex( pSeq[i].Name );

                    pBasePool->SetSearchMask( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
                    sal_Bool bStyleFound = sal_False;
                    const SfxStyleSheetBase* pBase = pBasePool->First();
                    while (pBase && !bStyleFound)
                    {
                        if(pBase->GetName() == aStyleName)
                            bStyleFound = sal_True;
                        pBase = pBasePool->Next();
                    }

                    if (nIdx == -1 || !bStyleFound)
                    {
                        bFailed = sal_True;
                        break;
                    }

                    aCondItem.SetStyle(&aStyleName, nIdx);
                }
                else
                    bFailed = sal_True;
            }
            if (bFailed)
                throw lang::IllegalArgumentException();
            rBase.GetItemSet().Put( aCondItem );
        }
        break;
        case FN_UNO_CATEGORY:
        {
            if(!rBase.mxNewBase->IsUserDefined())
                throw lang::IllegalArgumentException();
            short nSet = 0;
            rValue >>= nSet;

            sal_uInt16 nId;
            switch( nSet )
            {
                case style::ParagraphStyleCategory::TEXT:
                    nId = SWSTYLEBIT_TEXT;
                    break;
                case style::ParagraphStyleCategory::CHAPTER:
                    nId = SWSTYLEBIT_CHAPTER;
                    break;
                case style::ParagraphStyleCategory::LIST:
                    nId = SWSTYLEBIT_LIST;
                    break;
                case style::ParagraphStyleCategory::INDEX:
                    nId = SWSTYLEBIT_IDX;
                    break;
                case style::ParagraphStyleCategory::EXTRA:
                    nId = SWSTYLEBIT_EXTRA;
                    break;
                case style::ParagraphStyleCategory::HTML:
                    nId = SWSTYLEBIT_HTML;
                    break;
                default: throw lang::IllegalArgumentException();
            }
            rBase.mxNewBase->SetMask( nId|SFXSTYLEBIT_USERDEF );
        }
        break;
        case SID_SWREGISTER_COLLECTION:
        {
            OUString sName;
            rValue >>= sName;
            SwRegisterItem aReg( !sName.isEmpty() );
            aReg.SetWhich(SID_SWREGISTER_MODE);
            rBase.GetItemSet().Put(aReg);
            OUString aString;
            SwStyleNameMapper::FillUIName(sName, aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true);

            rBase.GetItemSet().Put(SfxStringItem(SID_SWREGISTER_COLLECTION, aString ) );
        }
        break;
        case RES_TXTATR_CJK_RUBY:
            if(MID_RUBY_CHARSTYLE == rEntry.nMemberId )
            {
                OUString sTmp;
                if(rValue >>= sTmp)
                {
                    SfxItemSet& rStyleSet = rBase.GetItemSet();
                    SwFmtRuby* pRuby = 0;
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == rStyleSet.GetItemState( RES_TXTATR_CJK_RUBY, sal_True, &pItem ) )
                        pRuby = new SwFmtRuby(*((SwFmtRuby*)pItem));
                    if(!pRuby)
                        pRuby = new SwFmtRuby(aEmptyStr);
                    OUString sStyle;
                    SwStyleNameMapper::FillUIName(sTmp, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                    pRuby->SetCharFmtName( sTmp );
                    pRuby->SetCharFmtId( 0 );
                    if(!sTmp.isEmpty())
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sTmp, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                        pRuby->SetCharFmtId(nId);
                    }
                    rStyleSet.Put(*pRuby);
                    delete pRuby;
                }
                else
                    throw lang::IllegalArgumentException();
            }
        goto put_itemset;
        case RES_PARATR_DROP:
        {
            if( MID_DROPCAP_CHAR_STYLE_NAME == rEntry.nMemberId)
            {
                if(rValue.getValueType() == ::getCppuType((const OUString*)0))
                {
                    SfxItemSet& rStyleSet = rBase.GetItemSet();

                    SwFmtDrop* pDrop = 0;
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == rStyleSet.GetItemState( RES_PARATR_DROP, sal_True, &pItem ) )
                        pDrop = new SwFmtDrop(*((SwFmtDrop*)pItem));
                    if(!pDrop)
                        pDrop = new SwFmtDrop();
                    OUString uStyle;
                    rValue >>= uStyle;
                    OUString sStyle;
                    SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                    SwDocStyleSheet* pStyle =
                        (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
                    if(pStyle)
                        pDrop->SetCharFmt(pStyle->GetCharFmt());
                    else
                        throw lang::IllegalArgumentException();
                    rStyleSet.Put(*pDrop);
                    delete pDrop;
                }
                else
                    throw lang::IllegalArgumentException();
                break;
            }
        }
        //no break!
        default:
put_itemset:
        {
            SfxItemSet& rStyleSet = rBase.GetItemSet();
            SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
            aSet.SetParent(&rStyleSet);
            rPropSet.setPropertyValue(rEntry, rValue, aSet);
            rStyleSet.Put(aSet);
            // #i70223#
            if ( SFX_STYLE_FAMILY_PARA == eFamily &&
                 rEntry.nWID == RES_PARATR_NUMRULE &&
                 rBase.mxNewBase.is() && rBase.mxNewBase->GetCollection() &&
                 rBase.mxNewBase->GetCollection()->IsAssignedToListLevelOfOutlineStyle() )

            {
                OUString sNewNumberingRuleName;
                rValue >>= sNewNumberingRuleName;
                if ( sNewNumberingRuleName.isEmpty() ||
                     sNewNumberingRuleName != pDoc->GetOutlineNumRule()->GetName() )
                {
                    rBase.mxNewBase->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
                }
            }
        }
    }
}

void SAL_CALL SwXStyle::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if ( !m_pDoc )
        throw uno::RuntimeException();
    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA  : nPropSetId = bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
        case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE   ;break;
        default:
            ;
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();

    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();

    SwStyleBase_Impl aBaseImpl(*m_pDoc, m_sStyleName);
    if(pBasePool)
    {
        sal_uInt16 nSaveMask = pBasePool->GetSearchMask();
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        pBasePool->SetSearchMask(eFamily, nSaveMask );
        OSL_ENSURE(pBase, "where is the style?" );
        if(pBase)
            aBaseImpl.mxNewBase = new SwDocStyleSheet(*(SwDocStyleSheet*)pBase);
        else
            throw uno::RuntimeException();
    }

    for(sal_Int16 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[nProp]);

        if(!pEntry ||
           (!bIsConditional && pNames[nProp].equalsAsciiL(SW_PROP_NAME(UNO_NAME_PARA_STYLE_CONDITIONS))))
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if(aBaseImpl.mxNewBase.is())
        {
            lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl,
                                 pBasePool, m_pDoc, eFamily);
        }
        else if(bIsDescriptor)
        {
            if(!pPropImpl->SetProperty(pNames[nProp], pValues[nProp]))
                throw lang::IllegalArgumentException();
        }
        else
            throw uno::RuntimeException();
    }
    if(aBaseImpl.HasItemSet())
        aBaseImpl.mxNewBase->SetItemSet(aBaseImpl.GetItemSet());
}

void SwXStyle::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

static uno::Any lcl_GetStyleProperty(const SfxItemPropertySimpleEntry& rEntry,
                        const SfxItemPropertySet& rPropSet,
                        SwStyleBase_Impl& rBase,
                        SfxStyleSheetBase* pBase,
                        SfxStyleFamily eFamily,
                        SwDoc *pDoc) throw(uno::RuntimeException)
{
    uno::Any aRet;
    if(FN_UNO_IS_PHYSICAL == rEntry.nWID)
    {
        sal_Bool bPhys = pBase != 0;
        if(pBase)
        {
            bPhys = ((SwDocStyleSheet*)pBase)->IsPhysical();
            // The standard character format is not existing physically
            if( bPhys && SFX_STYLE_FAMILY_CHAR == eFamily &&
                ((SwDocStyleSheet*)pBase)->GetCharFmt() &&
                ((SwDocStyleSheet*)pBase)->GetCharFmt()->IsDefault() )
                bPhys = sal_False;
        }
        aRet.setValue(&bPhys, ::getBooleanCppuType());
    }
    else if (FN_UNO_HIDDEN == rEntry.nWID)
    {
        sal_Bool bHidden = sal_False;
        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xBase( new SwDocStyleSheet(*(SwDocStyleSheet*)pBase) );
            bHidden = xBase->IsHidden();
        }
        aRet.setValue(&bHidden, ::getBooleanCppuType());
    }
    else if(pBase)
    {
        if(!rBase.mxNewBase.is())
            rBase.mxNewBase = new SwDocStyleSheet( *(SwDocStyleSheet*)pBase );
        switch(rEntry.nWID)
        {
            case RES_PAPER_BIN:
            {
                SfxItemSet& rSet = rBase.GetItemSet();
                rPropSet.getPropertyValue(rEntry, rSet, aRet);
                sal_Int8 nBin = 0;
                aRet >>= nBin;
                if ( nBin == -1 )
                    aRet <<= OUString( "[From printer settings]" );
                else
                {
                    SfxPrinter *pPrinter = pDoc->getPrinter( false );
                    OUString sTmp;
                    if (pPrinter )
                        sTmp = pPrinter->GetPaperBinName ( nBin );
                    aRet <<= sTmp;
                }
            }
            break;
            case  FN_UNO_NUM_RULES: //Sonderbehandlung fuer das SvxNumRuleItem:
            {
                const SwNumRule* pRule = rBase.mxNewBase->GetNumRule();
                OSL_ENSURE(pRule, "Wo ist die NumRule?");
                uno::Reference< container::XIndexReplace >  xRules = new SwXNumberingRules(*pRule, pDoc);
                aRet.setValue(&xRules, ::getCppuType((uno::Reference<container::XIndexReplace>*)0));
            }
            break;
            case RES_PARATR_OUTLINELEVEL:
            {
                OSL_ENSURE( SFX_STYLE_FAMILY_PARA == eFamily, "only paras" );
                int nLevel = rBase.mxNewBase->GetCollection()->GetAttrOutlineLevel();
                    aRet <<= static_cast<sal_Int16>( nLevel );
            }
            break;
            case FN_UNO_FOLLOW_STYLE:
            {
                OUString aString;
                SwStyleNameMapper::FillProgName(rBase.mxNewBase->GetFollow(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true);
                aRet <<= aString;
            }
            break;
            case RES_PAGEDESC :
            if( MID_PAGEDESC_PAGEDESCNAME != rEntry.nMemberId)
                goto query_itemset;
            {
                // Sonderbehandlung RES_PAGEDESC
                const SfxPoolItem* pItem;
                if(SFX_ITEM_SET == rBase.GetItemSet().GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
                {
                    const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                    if(pDesc)
                    {
                        OUString aString;
                        SwStyleNameMapper::FillProgName(pDesc->GetName(), aString,  nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
                        aRet <<= aString;
                    }
                }
            }
            break;
            case FN_UNO_IS_AUTO_UPDATE:
            {
                sal_Bool bAuto = sal_False;
                if(SFX_STYLE_FAMILY_PARA == eFamily)
                    bAuto = rBase.mxNewBase->GetCollection()->IsAutoUpdateFmt();
                else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                    bAuto = rBase.mxNewBase->GetFrmFmt()->IsAutoUpdateFmt();
                aRet.setValue(&bAuto, ::getBooleanCppuType());
            }
            break;
            case FN_UNO_DISPLAY_NAME:
            {
                OUString sName(rBase.mxNewBase->GetDisplayName());
                aRet <<= sName;
            }
            break;
            case FN_UNO_PARA_STYLE_CONDITIONS:
            {
                OSL_ENSURE(COND_COMMAND_COUNT == 28,
                        "invalid size of comman count?");
                uno::Sequence< beans::NamedValue > aSeq(COND_COMMAND_COUNT);
                beans::NamedValue *pSeq = aSeq.getArray();

                SwFmt *pFmt = ((SwDocStyleSheet*)pBase)->GetCollection();
                const CommandStruct *pCmds = SwCondCollItem::GetCmds();
                for (sal_uInt16 n = 0;  n < COND_COMMAND_COUNT;  ++n)
                {
                    OUString aStyleName;

                    const SwCollCondition* pCond = 0;
                    if( pFmt && RES_CONDTXTFMTCOLL == pFmt->Which() &&
                        0 != ( pCond = ((SwConditionTxtFmtColl*)pFmt)->
                        HasCondition( SwCollCondition( 0, pCmds[n].nCnd, pCmds[n].nSubCond ) ) )
                        && pCond->GetTxtFmtColl() )
                    {
                        // get programmatic style name from UI style name
                        aStyleName = pCond->GetTxtFmtColl()->GetName();
                        SwStyleNameMapper::FillProgName(aStyleName, aStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), true);
                    }

                    pSeq[n].Name  = GetCommandContextByIndex(n);
                    pSeq[n].Value <<= aStyleName;
                }
                aRet <<= aSeq;
            }
            break;
            case FN_UNO_CATEGORY:
            {
                sal_uInt16 nPoolId = rBase.mxNewBase->GetCollection()->GetPoolFmtId();
                short nRet = -1;
                switch ( COLL_GET_RANGE_BITS & nPoolId )
                {
                    case COLL_TEXT_BITS:
                        nRet = style::ParagraphStyleCategory::TEXT;
                        break;
                    case COLL_DOC_BITS:
                        nRet = style::ParagraphStyleCategory::CHAPTER;
                        break;
                    case COLL_LISTS_BITS:
                        nRet = style::ParagraphStyleCategory::LIST;
                        break;
                    case COLL_REGISTER_BITS:
                        nRet = style::ParagraphStyleCategory::INDEX;
                        break;
                    case COLL_EXTRA_BITS:
                        nRet = style::ParagraphStyleCategory::EXTRA;
                        break;
                    case COLL_HTML_BITS:
                        nRet = style::ParagraphStyleCategory::HTML;
                        break;
                }
                aRet <<= nRet;
            }
            break;
            case SID_SWREGISTER_COLLECTION:
            {
                const SwPageDesc *pPageDesc = rBase.mxNewBase->GetPageDesc();
                const SwTxtFmtColl* pCol = 0;
                OUString aString;
                if( pPageDesc )
                    pCol = pPageDesc->GetRegisterFmtColl();
                if( pCol )
                    SwStyleNameMapper::FillProgName(
                                pCol->GetName(), aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
                aRet <<= aString;
            }
            break;
            default:
query_itemset:
            {
                SfxItemSet& rSet = rBase.GetItemSet();
                rPropSet.getPropertyValue(rEntry, rSet, aRet);
            }
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Sequence< uno::Any > SAL_CALL SwXStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    if ( !m_pDoc )
        throw uno::RuntimeException();
    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA  : nPropSetId = bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
        case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE   ;break;
        default:
            ;
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();

    const OUString* pNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aRet(rPropertyNames.getLength());
    uno::Any* pRet = aRet.getArray();
    SwStyleBase_Impl aBase(*m_pDoc, m_sStyleName);
    SfxStyleSheetBase* pBase = 0;
    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[nProp]);
        if(!pEntry ||
           (!bIsConditional && pNames[nProp].equalsAsciiL(SW_PROP_NAME(UNO_NAME_PARA_STYLE_CONDITIONS))))
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if(pBasePool)
        {
            if(!pBase)
            {
                sal_uInt16 nSaveMask = pBasePool->GetSearchMask();
                pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
                pBase = pBasePool->Find(m_sStyleName);
                pBasePool->SetSearchMask(eFamily, nSaveMask );
            }
            pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, eFamily, GetDoc() );
        }
        else if(bIsDescriptor)
        {
            uno::Any *pAny = 0;
            pPropImpl->GetProperty ( pNames[nProp], pAny );
            if( !pAny )
            {
                bool bExcept = false;
                switch( eFamily )
                {
                    case SFX_STYLE_FAMILY_PSEUDO:
                        bExcept = true;
                    break;
                    case SFX_STYLE_FAMILY_PARA:
                    case SFX_STYLE_FAMILY_PAGE:
                        pPropImpl->GetProperty ( pNames[nProp], mxStyleData, pRet[ nProp ] );
                    break;
                    case SFX_STYLE_FAMILY_CHAR:
                    case SFX_STYLE_FAMILY_FRAME :
                    {
                        if (pEntry->nWID >= POOLATTR_BEGIN && pEntry->nWID < RES_UNKNOWNATR_END )
                        {
                            SwFmt * pFmt;
                            if ( eFamily == SFX_STYLE_FAMILY_CHAR )
                                pFmt = m_pDoc->GetDfltCharFmt();
                            else
                                pFmt = m_pDoc->GetDfltFrmFmt();
                            const SwAttrPool * pPool = pFmt->GetAttrSet().GetPool();
                            const SfxPoolItem & rItem = pPool->GetDefaultItem ( pEntry->nWID );
                            rItem.QueryValue ( pRet[nProp], pEntry->nMemberId );
                        }
                        else
                            bExcept = true;
                    }
                    break;

                    default:
                        ;
                }
                if (bExcept )
                {
                    uno::RuntimeException aExcept;
                    aExcept.Message = OUString( "No default value for: " ) + pNames[nProp];
                    throw aExcept;
                }
            }
            else
                pRet [ nProp ] = *pAny;
        }
        else
            throw uno::RuntimeException();
    }
    return aRet;
}

uno::Sequence< uno::Any > SwXStyle::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}

void SwXStyle::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{
}

void SwXStyle::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{
}

void SwXStyle::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{
}

void SwXStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException,
        lang::IllegalArgumentException,
        lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);
    SetPropertyValues_Impl( aProperties, aValues );
}

uno::Any SwXStyle::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];

}

void SwXStyle::addPropertyChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXStyle::removePropertyChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXStyle::addVetoableChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXStyle::removeVetoableChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXStyle::getPropertyState(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames(1);
    OUString* pNames = aNames.getArray();
    pNames[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXStyle::getPropertyStates(
    const uno::Sequence< OUString >& rPropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily );
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );

        if(pBase)
        {
            const OUString* pNames = rPropertyNames.getConstArray();
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
            sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;
            switch(eFamily)
            {
                case SFX_STYLE_FAMILY_PARA  : nPropSetId = bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
                case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE ;break;
                case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE;   break;
                case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE   ;break;
                default:
                    ;
            }
            const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
            const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();

            SfxItemSet aSet = xStyle->GetItemSet();
            for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
            {
                const OUString sPropName = pNames[i];
                const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
                if(!pEntry)
                    throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
                if( FN_UNO_NUM_RULES ==  pEntry->nWID ||
                    FN_UNO_FOLLOW_STYLE == pEntry->nWID )
                {
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else if(SFX_STYLE_FAMILY_PAGE == eFamily &&
                        (sPropName.startsWith("Header") || sPropName.startsWith("Footer")))
                {
                    sal_uInt16 nResId = lcl_ConvertFNToRES(pEntry->nWID);
                    sal_Bool bFooter = sPropName.startsWith("Footer");
                    const SvxSetItem* pSetItem;
                    if(SFX_ITEM_SET == aSet.GetItemState(
                            bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                            sal_False, (const SfxPoolItem**)&pSetItem))
                    {
                        const SfxItemSet& rSet = pSetItem->GetItemSet();
                        SfxItemState eState = rSet.GetItemState(nResId, sal_False);
                        if(SFX_ITEM_SET == eState)
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                        else
                            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                    else
                        pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                }
                else
                {
                    pStates[i] = pPropSet->getPropertyState(*pEntry, aSet);
                    if( SFX_STYLE_FAMILY_PAGE == eFamily &&
                        SID_ATTR_PAGE_SIZE == pEntry->nWID &&
                        beans::PropertyState_DIRECT_VALUE == pStates[i] )
                    {
                        const SvxSizeItem& rSize =
                            static_cast < const SvxSizeItem& >(
                                    aSet.Get(SID_ATTR_PAGE_SIZE) );
                        sal_uInt8 nMemberId = pEntry->nMemberId & 0x7f;
                        if( ( LONG_MAX == rSize.GetSize().Width() &&
                              (MID_SIZE_WIDTH == nMemberId ||
                               MID_SIZE_SIZE == nMemberId ) ) ||
                            ( LONG_MAX == rSize.GetSize().Height() &&
                              MID_SIZE_HEIGHT == nMemberId ) )
                        {
                            pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                        }
                    }
                }
            }
        }
        else
            throw uno::RuntimeException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXStyle::setPropertyToDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    setPropertiesToDefault ( aSequence );
}

void SAL_CALL SwXStyle::setPropertiesToDefault( const uno::Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFmt *pTargetFmt = 0;

    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "Where is the style?");

        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
            switch(eFamily)
            {
                case SFX_STYLE_FAMILY_CHAR: pTargetFmt = xStyle->GetCharFmt(); break;
                case SFX_STYLE_FAMILY_PARA: pTargetFmt = xStyle->GetCollection(); break;
                case SFX_STYLE_FAMILY_FRAME: pTargetFmt = xStyle->GetFrmFmt(); break;
                case SFX_STYLE_FAMILY_PAGE:
                    {
                        sal_uInt16 nPgDscPos = USHRT_MAX;
                        SwPageDesc *pDesc = m_pDoc->FindPageDescByName( xStyle->GetPageDesc()->GetName(), &nPgDscPos );
                        if( pDesc )
                            pTargetFmt = &pDesc->GetMaster();
                    }
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    break;
                default:
                    ;
            }
        }
    }
    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA  : nPropSetId = bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
        case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE; break;
        case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE; break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE; break;
        default:
            ;
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();

    const OUString* pNames = aPropertyNames.getConstArray();

    if ( pTargetFmt )
    {
        for( sal_Int32 nProp = 0, nEnd = aPropertyNames.getLength(); nProp < nEnd; nProp++ )
        {
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[nProp] );
            if( !pEntry )
                throw beans::UnknownPropertyException ( OUString( "Property is unknown: " ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pEntry->nWID == FN_UNO_FOLLOW_STYLE || pEntry->nWID == FN_UNO_NUM_RULES )
                throw uno::RuntimeException ("Cannot reset: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY )
                throw uno::RuntimeException("setPropertiesToDefault: property is read-only: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            if( pEntry->nWID == RES_PARATR_OUTLINELEVEL )
                static_cast<SwTxtFmtColl*>(pTargetFmt)->DeleteAssignmentToListLevelOfOutlineStyle();
            else
                pTargetFmt->ResetFmtAttr( pEntry->nWID );
        }
    }
    else if ( bIsDescriptor )
    {
        for( sal_Int32 nProp = 0, nEnd = aPropertyNames.getLength(); nProp < nEnd; nProp++ )
            pPropImpl->ClearProperty ( pNames[ nProp ] );
    }
}

void SAL_CALL SwXStyle::setAllPropertiesToDefault(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style, you fiend!?");

        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );

            SwFmt *pTargetFmt = 0;
            sal_uInt16 nPgDscPos = USHRT_MAX;
            switch( eFamily )
            {
            case SFX_STYLE_FAMILY_CHAR :
                pTargetFmt = xStyle->GetCharFmt();
                break;
            case SFX_STYLE_FAMILY_PARA :
                {
                    pTargetFmt = xStyle->GetCollection();
                    if ( xStyle->GetCollection() )
                        xStyle->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
                }
                break;
            case SFX_STYLE_FAMILY_FRAME:
                pTargetFmt = xStyle->GetFrmFmt();
                break;
            case SFX_STYLE_FAMILY_PAGE:
                {
                    SwPageDesc *pDesc = m_pDoc->FindPageDescByName( xStyle->GetPageDesc()->GetName(), &nPgDscPos );
                    if( pDesc )
                    {
                        pTargetFmt = &pDesc->GetMaster();
                        pDesc->SetUseOn ( nsUseOnPage::PD_ALL );
                    }
                }
                break;
            case SFX_STYLE_FAMILY_PSEUDO:
                break;

            default:
                ;
            }
            if( pTargetFmt )
            {
                if( USHRT_MAX != nPgDscPos )
                {
                    SwPageDesc& rPageDesc = m_pDoc->GetPageDesc(nPgDscPos);
                    rPageDesc.ResetAllMasterAttr();

                    SvxLRSpaceItem aLR(RES_LR_SPACE);
                    sal_Int32 nSize = GetMetricVal ( CM_1) * 2;
                    aLR.SetLeft ( nSize );
                    aLR.SetLeft ( nSize );
                    SvxULSpaceItem aUL( RES_UL_SPACE );
                    aUL.SetUpper ( static_cast < sal_uInt16 > ( nSize ) );
                    aUL.SetLower ( static_cast < sal_uInt16 > ( nSize ) );
                    pTargetFmt->SetFmtAttr( aLR );
                    pTargetFmt->SetFmtAttr( aUL );

                    SwPageDesc* pStdPgDsc = m_pDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
                    SwFmtFrmSize aFrmSz( ATT_FIX_SIZE );
                    if( RES_POOLPAGE_STANDARD == rPageDesc.GetPoolFmtId() )
                    {
                        if( m_pDoc->getPrinter( false ) )
                        {
                            const Size aPhysSize( SvxPaperInfo::GetPaperSize(
                                        static_cast<Printer*>( m_pDoc->getPrinter( false ) )) );
                            aFrmSz.SetSize( aPhysSize );
                        }
                        else
                            aFrmSz.SetSize( SvxPaperInfo::GetDefaultPaperSize() );

                    }
                    else
                    {
                        aFrmSz = pStdPgDsc->GetMaster().GetFrmSize();
                    }
                    if( pStdPgDsc->GetLandscape() )
                    {
                        SwTwips nTmp = aFrmSz.GetHeight();
                        aFrmSz.SetHeight( aFrmSz.GetWidth() );
                        aFrmSz.SetWidth( nTmp );
                    }
                    pTargetFmt->SetFmtAttr( aFrmSz );
                }
                else
                    pTargetFmt->ResetAllFmtAttr();

                if( USHRT_MAX != nPgDscPos )
                    m_pDoc->ChgPageDesc( nPgDscPos, m_pDoc->GetPageDesc(nPgDscPos) );
            }

        }
        else
            throw uno::RuntimeException();
    }
    else if ( bIsDescriptor )
        pPropImpl->ClearAllProperties();
    else
        throw uno::RuntimeException();
}

uno::Sequence< uno::Any > SAL_CALL SwXStyle::getPropertyDefaults( const uno::Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = aPropertyNames.getLength();
    uno::Sequence < uno::Any > aRet ( nCount );
    if ( nCount )
    {
        if( pBasePool)
        {
            pBasePool->SetSearchMask(eFamily);
            SfxStyleSheetBase* pBase = pBasePool->Find(m_sStyleName);
            OSL_ENSURE(pBase, "Doesn't seem to be a style!");

            if(pBase)
            {
                rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
                sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;
                switch(eFamily)
                {
                    case SFX_STYLE_FAMILY_PARA  : nPropSetId = bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
                    case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE; break;
                    case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE; break;
                    case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE; break;
                    default:
                        ;
                }
                const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
                const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

                const SfxItemSet &rSet = xStyle->GetItemSet(), *pParentSet = rSet.GetParent();
                const OUString *pNames = aPropertyNames.getConstArray();
                uno::Any *pRet = aRet.getArray();
                for ( sal_Int32 i = 0 ; i < nCount; i++)
                {
                    const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[i] );
                    if ( !pEntry )
                        throw beans::UnknownPropertyException ( OUString( "Unknown property: " ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

                    if (pEntry->nWID >= RES_UNKNOWNATR_END)
                    {
                        // these cannot be in an item set, especially not the
                        // parent set, so the default value is void
                        continue;
                    }
                    if( pParentSet )
                        aSwMapProvider.GetPropertySet(nPropSetId)->getPropertyValue(pNames[i], *pParentSet, pRet[i]);
                    else if( pEntry->nWID != rSet.GetPool()->GetSlotId(pEntry->nWID) )
                    {
                        const SfxPoolItem& rItem = rSet.GetPool()->GetDefaultItem(pEntry->nWID);
                        rItem.QueryValue(pRet[i], pEntry->nMemberId);
                    }
                }
            }
            else
                throw uno::RuntimeException();
        }
        else
            throw uno::RuntimeException();
    }
    return aRet;
}

uno::Any SwXStyle::getPropertyDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

void SwXStyle::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint *pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint )
    {
        if(( pHint->GetId() & SFX_HINT_DYING ) || ( pHint->GetId() & SFX_STYLESHEET_ERASED))
        {
            pBasePool = 0;
            EndListening(rBC);
        }
        else if( pHint->GetId() &(SFX_STYLESHEET_CHANGED|SFX_STYLESHEET_ERASED) )
        {
            ((SfxStyleSheetBasePool&)rBC).SetSearchMask(eFamily);
            SfxStyleSheetBase* pOwnBase = ((SfxStyleSheetBasePool&)rBC).Find(m_sStyleName);
            if(!pOwnBase)
            {
                EndListening(rBC);
                Invalidate();
            }
        }
    }
}

void SwXStyle::Invalidate()
{
    m_sStyleName = OUString();
    pBasePool = 0;
    m_pDoc = 0;
    mxStyleData.clear();
    mxStyleFamily.clear();
}

/******************************************************************
 * SwXPageStyle
 ******************************************************************/
SwXPageStyle::SwXPageStyle(SfxStyleSheetBasePool& rPool,
        SwDocShell* pDocSh, SfxStyleFamily eFam,
        const OUString& rStyleName):
    SwXStyle(rPool, eFam, pDocSh->GetDoc(), rStyleName)
{

}

SwXPageStyle::SwXPageStyle(SwDocShell* pDocSh) :
    SwXStyle(pDocSh->GetDoc(), SFX_STYLE_FAMILY_PAGE)
{
}

SwXPageStyle::~SwXPageStyle()
{

}

static void lcl_putItemToSet(const SvxSetItem* pSetItem, sal_uInt16 nRes, sal_uInt16 nItemType, const uno::Any& rVal, sal_uInt8 nMemberId, SwStyleBase_Impl& rBaseImpl)
{
    SvxSetItem* pNewSetItem = (SvxSetItem*)pSetItem->Clone();
    SfxItemSet& rSetSet = pNewSetItem->GetItemSet();
    const SfxPoolItem* pItem = 0;
    SfxPoolItem* pNewItem = 0;
    rSetSet.GetItemState(nRes, sal_True, &pItem);
    if(!pItem && nRes != rSetSet.GetPool()->GetSlotId(nRes))
        pItem = &rSetSet.GetPool()->GetDefaultItem(nRes);
    if(pItem)
    {
        pNewItem = pItem->Clone();
    }
    else
    {
        switch(nItemType)
        {
            case TYPE_BOOL: pNewItem = new SfxBoolItem(nRes);       break;
            case TYPE_SIZE: pNewItem = new SvxSizeItem(nRes);       break;
            case TYPE_BRUSH: pNewItem = new SvxBrushItem(nRes);     break;
            case TYPE_ULSPACE: pNewItem = new SvxULSpaceItem(nRes); break;
            case TYPE_SHADOW : pNewItem = new SvxShadowItem(nRes);  break;
            case TYPE_LRSPACE: pNewItem = new SvxLRSpaceItem(nRes); break;
            case TYPE_BOX: pNewItem = new SvxBoxItem(nRes);         break;
        }
    }
    pNewItem->PutValue(rVal, nMemberId);
    rSetSet.Put(*pNewItem);
    rBaseImpl.GetItemSet().Put(*pNewSetItem);
    delete pNewItem;
    delete pNewSetItem;
}

void SAL_CALL SwXPageStyle::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if(!GetDoc())
        throw uno::RuntimeException();

    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBaseImpl(*GetDoc(), GetStyleName());
    if(GetBasePool())
    {
        sal_uInt16 nSaveMask = GetBasePool()->GetSearchMask();
        GetBasePool()->SetSearchMask(GetFamily());
        SfxStyleSheetBase* pBase = GetBasePool()->Find(GetStyleName());
        GetBasePool()->SetSearchMask(GetFamily(), nSaveMask );
        OSL_ENSURE(pBase, "where is the style?" );
        if(pBase)
            aBaseImpl.mxNewBase = new SwDocStyleSheet(*(SwDocStyleSheet*)pBase);
        else
            throw uno::RuntimeException();
    }

    for(sal_Int16 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[nProp] );
        if (!pEntry)
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

        if(GetBasePool())
        {
            switch(pEntry->nWID)
            {
                case FN_UNO_HEADER_ON:
                case FN_UNO_HEADER_BACKGROUND:
                case FN_UNO_HEADER_BOX:
                case FN_UNO_HEADER_LR_SPACE:
                case FN_UNO_HEADER_SHADOW:
                case FN_UNO_HEADER_BODY_DISTANCE:
                case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_HEADER_SHARE_CONTENT:
                case FN_UNO_HEADER_HEIGHT:
                case FN_UNO_HEADER_EAT_SPACING:

                case FN_UNO_FIRST_SHARE_CONTENT:

                case FN_UNO_FOOTER_ON:
                case FN_UNO_FOOTER_BACKGROUND:
                case FN_UNO_FOOTER_BOX:
                case FN_UNO_FOOTER_LR_SPACE:
                case FN_UNO_FOOTER_SHADOW:
                case FN_UNO_FOOTER_BODY_DISTANCE:
                case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_FOOTER_SHARE_CONTENT:
                case FN_UNO_FOOTER_HEIGHT:
                case FN_UNO_FOOTER_EAT_SPACING:
                {
                    bool bFooter = false;
                    sal_uInt16 nItemType = TYPE_BOOL;
                    sal_uInt16 nRes = 0;
                    switch(pEntry->nWID)
                    {
                        case FN_UNO_FOOTER_ON:                  bFooter = true;
                        //kein break;
                        case FN_UNO_HEADER_ON:                  nRes = SID_ATTR_PAGE_ON;
                        break;
                        case FN_UNO_FOOTER_BACKGROUND:          bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_BACKGROUND:          nRes = RES_BACKGROUND; nItemType = TYPE_BRUSH;
                        break;
                        case FN_UNO_FOOTER_BOX:                 bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_BOX:                 nRes = RES_BOX; nItemType = TYPE_BOX;
                        break;
                        case FN_UNO_FOOTER_LR_SPACE:            bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_LR_SPACE:            nRes = RES_LR_SPACE;nItemType = TYPE_LRSPACE;
                        break;
                        case FN_UNO_FOOTER_SHADOW:              bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_SHADOW:              nRes = RES_SHADOW;nItemType = TYPE_SHADOW;
                        break;
                        case FN_UNO_FOOTER_BODY_DISTANCE:       bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_BODY_DISTANCE:       nRes = RES_UL_SPACE;nItemType = TYPE_ULSPACE;
                        break;
                        case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE: bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
                        break;
                        case FN_UNO_FOOTER_SHARE_CONTENT:       bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_SHARE_CONTENT:       nRes = SID_ATTR_PAGE_SHARED;
                        break;
                        case FN_UNO_FIRST_SHARE_CONTENT: nRes = SID_ATTR_PAGE_SHARED_FIRST;
                        break;
                        case FN_UNO_FOOTER_HEIGHT:              bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_HEIGHT:              nRes = SID_ATTR_PAGE_SIZE;nItemType = TYPE_SIZE;
                        break;
                        case FN_UNO_FOOTER_EAT_SPACING:     bFooter = true;
                        // kein break;
                        case FN_UNO_HEADER_EAT_SPACING:     nRes = RES_HEADER_FOOTER_EAT_SPACING;nItemType = TYPE_SIZE;
                        break;
                    }
                    const SvxSetItem* pSetItem;
                    if(SFX_ITEM_SET == aBaseImpl.GetItemSet().GetItemState(
                            bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                            sal_False, (const SfxPoolItem**)&pSetItem))
                    {
                        lcl_putItemToSet(pSetItem, nRes, nItemType, pValues[nProp], pEntry->nMemberId, aBaseImpl);

                        if (nRes == SID_ATTR_PAGE_SHARED_FIRST)
                        {
                            // Need to add this to the other as well
                            if (SFX_ITEM_SET == aBaseImpl.GetItemSet().GetItemState(
                                        bFooter ? SID_ATTR_PAGE_HEADERSET : SID_ATTR_PAGE_FOOTERSET,
                                        sal_False, (const SfxPoolItem**)&pSetItem))
                                lcl_putItemToSet(pSetItem, nRes, nItemType, pValues[nProp], pEntry->nMemberId, aBaseImpl);
                        }
                    }
                    else if(SID_ATTR_PAGE_ON == nRes )
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                        if(bVal)
                        {
                            SfxItemSet aTempSet(*aBaseImpl.GetItemSet().GetPool(),
                                RES_BACKGROUND, RES_SHADOW,
                                RES_LR_SPACE, RES_UL_SPACE,
                                nRes, nRes,
                                SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                                SID_ATTR_PAGE_DYNAMIC, SID_ATTR_PAGE_DYNAMIC,
                                SID_ATTR_PAGE_SHARED, SID_ATTR_PAGE_SHARED,
                                SID_ATTR_PAGE_SHARED_FIRST, SID_ATTR_PAGE_SHARED_FIRST,
                                0 );
                            aTempSet.Put(SfxBoolItem(nRes, sal_True));
                            aTempSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(MM50, MM50)));
                            aTempSet.Put(SvxLRSpaceItem(RES_LR_SPACE));
                            aTempSet.Put(SvxULSpaceItem(RES_UL_SPACE));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED, sal_True));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED_FIRST, sal_True));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_DYNAMIC, sal_True));

                            SvxSetItem aNewSetItem( bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                                    aTempSet);
                            aBaseImpl.GetItemSet().Put(aNewSetItem);
                        }
                    }
                }
                break;
                case FN_PARAM_FTN_INFO :
                {
                    const SfxPoolItem& rItem = aBaseImpl.GetItemSet().Get(FN_PARAM_FTN_INFO);
                    SfxPoolItem* pNewFtnItem = rItem.Clone();
                    sal_Bool bPut = pNewFtnItem->PutValue(pValues[nProp], pEntry->nMemberId);
                    aBaseImpl.GetItemSet().Put(*pNewFtnItem);
                    delete pNewFtnItem;
                    if(!bPut)
                        throw lang::IllegalArgumentException();
                }
                break;
                case  FN_UNO_HEADER       :
                case  FN_UNO_HEADER_LEFT  :
                case  FN_UNO_HEADER_RIGHT :
                case  FN_UNO_HEADER_FIRST :
                case  FN_UNO_FOOTER       :
                case  FN_UNO_FOOTER_LEFT  :
                case  FN_UNO_FOOTER_RIGHT :
                case  FN_UNO_FOOTER_FIRST :
                    throw lang::IllegalArgumentException();
                //break;
                default:
                    lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl,
                                        GetBasePool(), GetDoc(), GetFamily());
            }
        }
        else if(IsDescriptor())
        {
            if(!GetPropImpl()->SetProperty(pNames[nProp], pValues[nProp]))
                throw lang::IllegalArgumentException();
        }
        else
            throw uno::RuntimeException();
    }
    if(aBaseImpl.HasItemSet())
    {
        ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
        if (undoGuard.UndoWasEnabled())
        {
            // Fix i64460: as long as Undo of page styles with header/footer causes trouble...
            GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
        }
        aBaseImpl.mxNewBase->SetItemSet(aBaseImpl.GetItemSet());
    }
}

void SwXPageStyle::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

static uno::Reference<text::XText>
lcl_makeHeaderFooter(
    const sal_uInt16 nRes, const bool bHeader, SwFrmFmt const*const pFrmFmt)
{
    if (!pFrmFmt) { return 0; }

    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
    const SfxPoolItem* pItem;
    if (SFX_ITEM_SET == rSet.GetItemState(nRes, sal_True, &pItem))
    {
        SwFrmFmt *const pHeadFootFmt = (bHeader)
            ? static_cast<SwFmtHeader*>(const_cast<SfxPoolItem*>(pItem))->
                    GetHeaderFmt()
            : static_cast<SwFmtFooter*>(const_cast<SfxPoolItem*>(pItem))->
                    GetFooterFmt();
        if (pHeadFootFmt)
        {
            return SwXHeadFootText::CreateXHeadFootText(*pHeadFootFmt, bHeader);
        }
    }
    return 0;
}

uno::Sequence< uno::Any > SAL_CALL SwXPageStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString >& rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    if(!GetDoc())
        throw uno::RuntimeException();

    sal_Int32 nLength = rPropertyNames.getLength();
    const OUString* pNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aRet ( nLength );

    uno::Any* pRet = aRet.getArray();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBase(*GetDoc(), GetStyleName());
    SfxStyleSheetBase* pBase = 0;
    for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[nProp] );
        if (!pEntry)
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

        if(GetBasePool())
        {
            if(!pBase)
            {
                sal_uInt16 nSaveMask = GetBasePool()->GetSearchMask();
                GetBasePool()->SetSearchMask(GetFamily(), SFXSTYLEBIT_ALL );
                pBase = GetBasePool()->Find(GetStyleName());
                GetBasePool()->SetSearchMask(GetFamily(), nSaveMask );
            }
            sal_uInt16 nRes = 0;
            bool bHeader = false;
            bool bLeft = false;
            bool bFirst = false;
            switch(pEntry->nWID)
            {
                case FN_UNO_HEADER_ON:
                case FN_UNO_HEADER_BACKGROUND:
                case FN_UNO_HEADER_BOX:
                case FN_UNO_HEADER_LR_SPACE:
                case FN_UNO_HEADER_SHADOW:
                case FN_UNO_HEADER_BODY_DISTANCE:
                case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_HEADER_SHARE_CONTENT:
                case FN_UNO_HEADER_HEIGHT:
                case FN_UNO_HEADER_EAT_SPACING:

                case FN_UNO_FIRST_SHARE_CONTENT:

                case FN_UNO_FOOTER_ON:
                case FN_UNO_FOOTER_BACKGROUND:
                case FN_UNO_FOOTER_BOX:
                case FN_UNO_FOOTER_LR_SPACE:
                case FN_UNO_FOOTER_SHADOW:
                case FN_UNO_FOOTER_BODY_DISTANCE:
                case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_FOOTER_SHARE_CONTENT:
                case FN_UNO_FOOTER_HEIGHT:
                case FN_UNO_FOOTER_EAT_SPACING:
                {
                    SfxStyleSheetBasePool* pBasePool2 = ((SwXPageStyle*)this)->GetBasePool();
                    pBasePool2->SetSearchMask(GetFamily());
                    SfxStyleSheetBase* pBase2 = pBasePool2->Find(GetStyleName());
                    if(pBase2)
                    {
                        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
                        const SfxItemSet& rSet = xStyle->GetItemSet();
                        bool bFooter = false;
                        switch(pEntry->nWID)
                        {
                            case FN_UNO_FOOTER_ON:
                                bFooter = true;
                            // kein break!
                            case FN_UNO_HEADER_ON:
                            {
                                //falls das SetItem nicht da ist, dann ist der Wert sal_False
                                sal_Bool bRet = sal_False;
                                pRet[nProp].setValue(&bRet, ::getCppuBooleanType());
                                nRes = SID_ATTR_PAGE_ON;
                            }
                            break;
                            case FN_UNO_FOOTER_BACKGROUND:      bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_BACKGROUND:      nRes = RES_BACKGROUND;
                            break;
                            case FN_UNO_FOOTER_BOX:             bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_BOX:             nRes = RES_BOX;
                            break;
                            case FN_UNO_FOOTER_LR_SPACE:        bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_LR_SPACE:        nRes = RES_LR_SPACE;
                            break;
                            case FN_UNO_FOOTER_SHADOW:          bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_SHADOW:          nRes = RES_SHADOW;
                            break;
                            case FN_UNO_FOOTER_BODY_DISTANCE:   bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_BODY_DISTANCE:   nRes = RES_UL_SPACE;
                            break;
                            case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE: bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
                            break;
                            case FN_UNO_FOOTER_SHARE_CONTENT:   bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_SHARE_CONTENT:   nRes = SID_ATTR_PAGE_SHARED;
                            break;
                            case FN_UNO_FIRST_SHARE_CONTENT: nRes = SID_ATTR_PAGE_SHARED_FIRST;
                            break;
                            case FN_UNO_FOOTER_HEIGHT:          bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_HEIGHT:          nRes = SID_ATTR_PAGE_SIZE;
                            break;
                            case FN_UNO_FOOTER_EAT_SPACING: bFooter = true;
                            // kein break;
                            case FN_UNO_HEADER_EAT_SPACING: nRes = RES_HEADER_FOOTER_EAT_SPACING;
                            break;
                        }
                        const SvxSetItem* pSetItem;
                        if(SFX_ITEM_SET == rSet.GetItemState(
                                bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                                sal_False, (const SfxPoolItem**)&pSetItem))
                        {
                            const SfxItemSet& rTmpSet = pSetItem->GetItemSet();
                            const SfxPoolItem* pItem = 0;
                            rTmpSet.GetItemState(nRes, sal_True, &pItem);
                            if(!pItem && nRes != rTmpSet.GetPool()->GetSlotId(nRes))
                                pItem = &rTmpSet.GetPool()->GetDefaultItem(nRes);
                            if(pItem)
                                pItem->QueryValue(pRet[nProp], pEntry->nMemberId);
                        }
                    }
                }
                break;
                case  FN_UNO_HEADER       :
                    goto Header;
                case  FN_UNO_HEADER_LEFT  :
                    bLeft = true; goto Header;
                case  FN_UNO_HEADER_FIRST  :
                    bFirst = true; goto Header;
                case  FN_UNO_HEADER_RIGHT :
                    goto Header;
Header:
                    bHeader = true;
                    nRes = RES_HEADER; goto MakeObject;
                case  FN_UNO_FOOTER       :
                    goto Footer;
                case  FN_UNO_FOOTER_LEFT  :
                    bLeft = true; goto Footer;
                case  FN_UNO_FOOTER_FIRST  :
                    bFirst = true; goto Footer;
                case  FN_UNO_FOOTER_RIGHT :
Footer:
                    nRes = RES_FOOTER;
MakeObject:
                {
                    const SwPageDesc& rDesc = aBase.GetOldPageDesc();
                    const SwFrmFmt* pFrmFmt = 0;
                    bool bShare = (bHeader && rDesc.IsHeaderShared())||
                                    (!bHeader && rDesc.IsFooterShared());
                    bool bShareFirst = rDesc.IsFirstShared();
                    // TextLeft returns the left content if there is one,
                    // Text and TextRight return the master content.
                    // TextRight does the same as Text and is for
                    // comptability only.
                    if( bLeft && !bShare )
                    {
                        pFrmFmt = &rDesc.GetLeft();
                    }
                    else if (bFirst && !bShareFirst)
                    {
                        pFrmFmt = &rDesc.GetFirst();
                    }
                    else
                    {
                        pFrmFmt = &rDesc.GetMaster();
                    }
                    const uno::Reference< text::XText > xRet =
                        lcl_makeHeaderFooter(nRes, bHeader, pFrmFmt);
                    if (xRet.is())
                    {
                        pRet[nProp] <<= xRet;
                    }
                }
                break;
                case FN_PARAM_FTN_INFO :
                {
                    rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *(SwDocStyleSheet*)pBase ) );
                    const SfxItemSet& rSet = xStyle->GetItemSet();
                    const SfxPoolItem& rItem = rSet.Get(FN_PARAM_FTN_INFO);
                    rItem.QueryValue(pRet[nProp], pEntry->nMemberId);
                }
                break;
                default:
                pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, GetFamily(), GetDoc() );
            }
        }
        else if(IsDescriptor())
        {
            uno::Any* pAny = 0;
            GetPropImpl()->GetProperty(pNames[nProp], pAny);
            if ( !pAny )
                GetPropImpl()->GetProperty ( pNames[nProp], mxStyleData, pRet[ nProp ] );
            else
                pRet[nProp] = *pAny;
        }
        else
            throw uno::RuntimeException();
    }
    return aRet;
}

uno::Sequence< uno::Any > SwXPageStyle::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames )
        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}

uno::Any SwXPageStyle::getPropertyValue(const OUString& rPropertyName) throw(
    beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];
}

void SwXPageStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException,
        lang::IllegalArgumentException,
        lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);
    SetPropertyValues_Impl( aProperties, aValues );
}

SwXFrameStyle::SwXFrameStyle ( SwDoc *pDoc )
: SwXStyle ( pDoc, SFX_STYLE_FAMILY_FRAME, sal_False)
{
}

SwXFrameStyle::~SwXFrameStyle()
{
}

uno::Sequence< uno::Type > SwXFrameStyle::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes = SwXStyle::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 1);
    aTypes.getArray()[nLen] = ::getCppuType((uno::Reference<XEventsSupplier>*)0);
    return aTypes;
}

uno::Any SwXFrameStyle::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    uno::Any aRet;
    if(rType == ::getCppuType((uno::Reference<XEventsSupplier>*)0))
        aRet <<= uno::Reference<XEventsSupplier>(this);
    else
        aRet = SwXStyle::queryInterface(rType);
    return aRet;
}

uno::Reference< container::XNameReplace > SwXFrameStyle::getEvents(  ) throw(uno::RuntimeException)
{
    return new SwFrameStyleEventDescriptor( *this );
}

SwXAutoStyles::SwXAutoStyles(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()), pDocShell( &rDocShell )
{
}

SwXAutoStyles::~SwXAutoStyles()
{
}

sal_Int32 SwXAutoStyles::getCount(void) throw( uno::RuntimeException )
{
    return AUTOSTYLE_FAMILY_COUNT;
}

uno::Any SwXAutoStyles::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
                uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(nIndex < 0 || nIndex >= AUTOSTYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(IsValid())
    {
        uno::Reference< style::XAutoStyleFamily >  aRef;
        IStyleAccess::SwAutoStyleFamily nType = aAutoStyleByIndex[nIndex];
        switch( nType )
        {
            case IStyleAccess::AUTO_STYLE_CHAR:
            {
                if(!xAutoCharStyles.is())
                    xAutoCharStyles = new SwXAutoStyleFamily(pDocShell, nType);
                aRef = xAutoCharStyles;
            }
            break;
            case IStyleAccess::AUTO_STYLE_RUBY:
            {
                if(!xAutoRubyStyles.is())
                    xAutoRubyStyles = new SwXAutoStyleFamily(pDocShell, nType );
                aRef = xAutoRubyStyles;
            }
            break;
            case IStyleAccess::AUTO_STYLE_PARA:
            {
                if(!xAutoParaStyles.is())
                    xAutoParaStyles = new SwXAutoStyleFamily(pDocShell, nType );
                aRef = xAutoParaStyles;
            }
            break;

            default:
                ;
        }
        aRet.setValue(&aRef, ::getCppuType((const uno::Reference<style::XAutoStyleFamily>*)0));
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SwXAutoStyles::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((const uno::Reference<style::XAutoStyleFamily>*)0);
}

sal_Bool SwXAutoStyles::hasElements(  ) throw(uno::RuntimeException)
{
    return sal_True;
}

uno::Any SwXAutoStyles::getByName(const OUString& Name)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    if(Name.compareToAscii("CharacterStyles") == 0 )
        aRet = getByIndex(0);
    else if(Name.compareToAscii("RubyStyles") == 0 )
        aRet = getByIndex(1);
    else if(Name.compareToAscii("ParagraphStyles") == 0 )
        aRet = getByIndex(2);
    else
        throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXAutoStyles::getElementNames(void)
            throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aNames(AUTOSTYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = "CharacterStyles";
    pNames[1] = "RubyStyles";
    pNames[2] = "ParagraphStyles";
    return aNames;
}

sal_Bool SwXAutoStyles::hasByName(const OUString& Name)
            throw( uno::RuntimeException )
{
    if( Name.compareToAscii("CharacterStyles") == 0 ||
        Name.compareToAscii("RubyStyles") == 0 ||
        Name.compareToAscii("ParagraphStyles") == 0 )
        return sal_True;
    else
        return sal_False;
}

SwXAutoStyleFamily::SwXAutoStyleFamily(SwDocShell* pDocSh, IStyleAccess::SwAutoStyleFamily nFamily) :
    pDocShell( pDocSh ), eFamily(nFamily)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDocSh->GetDoc()->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStyleFamily::~SwXAutoStyleFamily()
{
}

void SwXAutoStyleFamily::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pDocShell = 0;
}

uno::Reference< style::XAutoStyle > SwXAutoStyleFamily::insertStyle(
    const uno::Sequence< beans::PropertyValue >& Values )
        throw (uno::RuntimeException)
{
    if( !pDocShell )
        throw uno::RuntimeException();
    const sal_uInt16* pRange = 0;
    const SfxItemPropertySet* pPropSet = 0;
    switch( eFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            pRange = aCharAutoFmtSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE);
        }
        break;
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            pRange = 0;//aTxtNodeSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_RUBY_AUTO_STYLE);
        }
        break;
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            pRange = aTxtNodeSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARA_AUTO_STYLE);
        }
        break;

        default:
            ;
    }
    SwAttrSet aSet( pDocShell->GetDoc()->GetAttrPool(), pRange );
    const beans::PropertyValue* pSeq = Values.getConstArray();
    sal_Int32 nLen = Values.getLength();
    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        try
        {
            pPropSet->setPropertyValue( pSeq[i].Name, pSeq[i].Value, aSet );
        }
        catch (beans::UnknownPropertyException &)
        {
            OSL_FAIL( "Unknown property" );
        }
        catch (lang::IllegalArgumentException &)
        {
            OSL_FAIL( "Illegal argument" );
        }
    }

    SfxItemSet_Pointer_t pSet = pDocShell->GetDoc()->GetIStyleAccess().cacheAutomaticStyle( aSet, eFamily );
    uno::Reference<style::XAutoStyle> xRet = new SwXAutoStyle(pDocShell->GetDoc(), pSet, eFamily);
    return xRet;
}

uno::Reference< container::XEnumeration > SwXAutoStyleFamily::createEnumeration(  )
        throw (uno::RuntimeException)
{
    if( !pDocShell )
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >
        (new SwXAutoStylesEnumerator( pDocShell->GetDoc(), eFamily ));
}

uno::Type SwXAutoStyleFamily::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((const uno::Reference<style::XAutoStyle>*)0);
}

sal_Bool SwXAutoStyleFamily::hasElements(  ) throw(uno::RuntimeException)
{
    return sal_False;
}

SwAutoStylesEnumImpl::SwAutoStylesEnumImpl( SwDoc* pInitDoc, IStyleAccess::SwAutoStyleFamily eFam )
: pDoc( pInitDoc ), eFamily( eFam )
{
    // special case for ruby auto styles:
    if ( IStyleAccess::AUTO_STYLE_RUBY == eFam )
    {
        std::set< std::pair< sal_uInt16, sal_uInt16 > > aRubyMap;
        SwAttrPool& rAttrPool = pDoc->GetAttrPool();
        sal_uInt32 nCount = rAttrPool.GetItemCount2( RES_TXTATR_CJK_RUBY );

        for ( sal_uInt32 nI = 0; nI < nCount; ++nI )
        {
            const SwFmtRuby* pItem = static_cast<const SwFmtRuby*>(rAttrPool.GetItem2( RES_TXTATR_CJK_RUBY, nI ));
            if ( pItem && pItem->GetTxtRuby() )
            {
                std::pair< sal_uInt16, sal_uInt16 > aPair( pItem->GetPosition(), pItem->GetAdjustment() );
                if ( aRubyMap.find( aPair ) == aRubyMap.end() )
                {
                    aRubyMap.insert( aPair );
                    SfxItemSet_Pointer_t pItemSet( new SfxItemSet( rAttrPool, RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY ) );
                    pItemSet->Put( *pItem );
                    mAutoStyles.push_back( pItemSet );
                }
            }
        }
    }
    else
    {
        pDoc->GetIStyleAccess().getAllStyles( mAutoStyles, eFamily );
    }

    aIter = mAutoStyles.begin();
}

SwXAutoStylesEnumerator::SwXAutoStylesEnumerator( SwDoc* pDoc, IStyleAccess::SwAutoStyleFamily eFam )
: pImpl( new SwAutoStylesEnumImpl( pDoc, eFam ) )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStylesEnumerator::~SwXAutoStylesEnumerator()
{
    delete pImpl;
}

void SwXAutoStylesEnumerator::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        delete pImpl;
        pImpl = 0;
    }
}

::sal_Bool SwXAutoStylesEnumerator::hasMoreElements(  )
    throw (uno::RuntimeException)
{
    if( !pImpl )
        throw uno::RuntimeException();
    return pImpl->hasMoreElements();
}

uno::Any SwXAutoStylesEnumerator::nextElement(  )
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !pImpl )
        throw uno::RuntimeException();
    uno::Any aRet;
    if( pImpl->hasMoreElements() )
    {
        SfxItemSet_Pointer_t pNextSet = pImpl->nextElement();
        uno::Reference< style::XAutoStyle > xAutoStyle = new SwXAutoStyle(pImpl->getDoc(),
                                                        pNextSet, pImpl->getFamily());
        aRet.setValue(&xAutoStyle, ::getCppuType((uno::Reference<style::XAutoStyle>*)0));
    }
    return aRet;
}

SwXAutoStyle::SwXAutoStyle( SwDoc* pDoc, SfxItemSet_Pointer_t pInitSet, IStyleAccess::SwAutoStyleFamily eFam )
: pSet( pInitSet ), eFamily( eFam )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStyle::~SwXAutoStyle()
{
}

void SwXAutoStyle::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        pSet.reset();
}

uno::Reference< beans::XPropertySetInfo > SwXAutoStyle::getPropertySetInfo(  )
                throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySetInfo >  xRet;
    switch( eFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            static uno::Reference< beans::XPropertySetInfo >  xCharRef;
            if(!xCharRef.is())
            {
                xCharRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE)->getPropertySetInfo();
            }
            xRet = xCharRef;
        }
        break;
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            static uno::Reference< beans::XPropertySetInfo >  xRubyRef;
            if(!xRubyRef.is())
            {
                sal_uInt16 nMapId = PROPERTY_MAP_RUBY_AUTO_STYLE;
                xRubyRef = aSwMapProvider.GetPropertySet(nMapId)->getPropertySetInfo();
            }
            xRet = xRubyRef;
        }
        break;
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            static uno::Reference< beans::XPropertySetInfo >  xParaRef;
            if(!xParaRef.is())
            {
                sal_uInt16 nMapId = PROPERTY_MAP_PARA_AUTO_STYLE;
                xParaRef = aSwMapProvider.GetPropertySet(nMapId)->getPropertySetInfo();
            }
            xRet = xParaRef;
        }
        break;

        default:
            ;
    }

    return xRet;
}

void SwXAutoStyle::setPropertyValue( const OUString& /*rPropertyName*/, const uno::Any& /*rValue*/ )
     throw( beans::UnknownPropertyException,
            beans::PropertyVetoException,
            lang::IllegalArgumentException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
}

uno::Any SwXAutoStyle::getPropertyValue( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];
}

void SwXAutoStyle::addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                              const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
}

void SwXAutoStyle::removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                 const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
}

void SwXAutoStyle::addVetoableChangeListener( const OUString& /*PropertyName*/,
                                              const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
}

void SwXAutoStyle::removeVetoableChangeListener( const OUString& /*PropertyName*/,
                                                 const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException )
{
}

void SwXAutoStyle::setPropertyValues(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Sequence< uno::Any >& /*aValues*/ )
            throw (beans::PropertyVetoException, lang::IllegalArgumentException,
                lang::WrappedTargetException, uno::RuntimeException)
{
}

uno::Sequence< uno::Any > SwXAutoStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( !pSet.get() )
        throw uno::RuntimeException();
    // query_item

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(eFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default:
            ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const OUString* pNames = rPropertyNames.getConstArray();

    sal_Int32 nLen = rPropertyNames.getLength();
    uno::Sequence< uno::Any > aRet( nLen );
    uno::Any* pValues = aRet.getArray();

    SfxItemSet& rSet = *pSet.get();

    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
        else if ( RES_TXTATR_AUTOFMT == pEntry->nWID || RES_AUTO_STYLE == pEntry->nWID )
        {
            pValues[i] <<= StylePool::nameOf( pSet );
        }
        else
            pPropSet->getPropertyValue( *pEntry, rSet, pValues[i] );
    }
    return aRet;
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyValues (
        const uno::Sequence< OUString >& rPropertyNames )
            throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}

void SwXAutoStyle::addPropertiesChangeListener(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException)
{
}

void SwXAutoStyle::removePropertiesChangeListener(
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException)
{
}

void SwXAutoStyle::firePropertiesChangeEvent(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException)
{
}

beans::PropertyState SwXAutoStyle::getPropertyState( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames(1);
    OUString* pNames = aNames.getArray();
    pNames[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

void SwXAutoStyle::setPropertyToDefault( const OUString& /*PropertyName*/ )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException )
{
}

uno::Any SwXAutoStyle::getPropertyDefault( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXAutoStyle::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames )
            throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    if( !pSet.get() )
        throw uno::RuntimeException();
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(eFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default:
            ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SfxItemSet& rSet = *pSet.get();
    for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
        pStates[i] = pPropSet->getPropertyState(*pEntry, rSet );
    }
    return aRet;
}

void SwXAutoStyle::setAllPropertiesToDefault(  )
            throw (uno::RuntimeException)
{
}

void SwXAutoStyle::setPropertiesToDefault(
        const uno::Sequence< OUString >& /*aPropertyNames*/ )
            throw (beans::UnknownPropertyException, uno::RuntimeException)
{
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyDefaults(
        const uno::Sequence< OUString >& /*aPropertyNames*/ )
            throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                    uno::RuntimeException)
{
    uno::Sequence< uno::Any > aRet(0);
    return aRet;
}

uno::Sequence< beans::PropertyValue > SwXAutoStyle::getProperties() throw (uno::RuntimeException)
{
    if( !pSet.get() )
        throw uno::RuntimeException();
    SolarMutexGuard aGuard;
    std::vector< beans::PropertyValue > aPropertyVector;

    sal_Int8 nPropSetId = 0;
    switch(eFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default:
            ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    PropertyEntryVector_t aPropVector = rMap.getPropertyEntries();

    SfxItemSet& rSet = *pSet.get();
    SfxItemIter aIter(rSet);
    const SfxPoolItem* pItem = aIter.FirstItem();

    while ( pItem )
    {
        const sal_uInt16 nWID = pItem->Which();

        // TODO: Optimize - and fix! the old iteration filled each WhichId
        // only once but there are more properties than WhichIds
        PropertyEntryVector_t::const_iterator aIt = aPropVector.begin();
        while( aIt != aPropVector.end() )
        {
            if ( aIt->nWID == nWID )
            {
                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = aIt->sName;
                pItem->QueryValue( aPropertyValue.Value, aIt->nMemberId );
                aPropertyVector.push_back( aPropertyValue );
            }
            ++aIt;
        }
        pItem = aIter.NextItem();
    }

    const sal_Int32 nCount = aPropertyVector.size();
    uno::Sequence< beans::PropertyValue > aRet( nCount );
    beans::PropertyValue* pProps = aRet.getArray();

    for ( int i = 0; i < nCount; ++i, pProps++ )
    {
        *pProps = aPropertyVector[i];
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
