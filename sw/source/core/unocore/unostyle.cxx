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
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
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
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <istyleaccess.hxx>
#include <GetMetricVal.hxx>
#include <fmtfsize.hxx>
#include <numrule.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

//UUUU
#include <svx/unobrushitemhelper.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/memberids.hrc>
#include <svx/unoshape.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <swunohelper.hxx>
#include <svx/xbtmpit.hxx>

#include "ccoll.hxx"
#include "unocore.hrc"

#include <cassert>
#include <memory>
#include <set>

#define STYLE_FAMILY_COUNT 5            // we have 5 style families

const sal_uInt16 aStyleByIndex[] =
{
    SFX_STYLE_FAMILY_CHAR,
    SFX_STYLE_FAMILY_PARA,
    SFX_STYLE_FAMILY_PAGE,
    SFX_STYLE_FAMILY_FRAME,
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
            OSL_ENSURE(false, "someone asking for all styles in unostyle.cxx!" );
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
    bool hasMoreElements() { return aIter != mAutoStyles.end(); }
    SfxItemSet_Pointer_t nextElement() { return *(aIter++); }
    IStyleAccess::SwAutoStyleFamily getFamily() const { return eFamily; }
    SwDoc* getDoc() const { return pDoc; }
};

OUString SwXStyleFamilies::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXStyleFamilies");
}

sal_Bool SwXStyleFamilies::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyleFamilies::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aRet { "com.sun.star.style.StyleFamilies" };
    return aRet;
}

SwXStyleFamilies::SwXStyleFamilies(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()),
    m_pDocShell(&rDocShell),
    m_xCharStyles(),
    m_xParaStyles(),
    m_xFrameStyles(),
    m_xPageStyles(),
    m_xNumberingStyles()
{

}

SwXStyleFamilies::~SwXStyleFamilies()
{
}

uno::Any SAL_CALL SwXStyleFamilies::getByName(const OUString& Name)
    throw(
        container::NoSuchElementException,
        lang::WrappedTargetException,
        uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
// the index comes from const unsigned short aStyleByIndex[] = ...
    uno::Any aRet;
    if(!IsValid())
        throw uno::RuntimeException();
    if(Name=="CharacterStyles" )
        aRet = getByIndex(0);
    else if(Name=="ParagraphStyles" )
        aRet = getByIndex(1);
    else if(Name=="FrameStyles" )
        aRet = getByIndex(3);
    else if(Name=="PageStyles" )
        aRet = getByIndex(2);
    else if(Name=="NumberingStyles" )
        aRet = getByIndex(4);
    else
        throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXStyleFamilies::getElementNames() throw( uno::RuntimeException, std::exception )
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

sal_Bool SwXStyleFamilies::hasByName(const OUString& Name) throw( uno::RuntimeException, std::exception )
{
    if( Name=="CharacterStyles" ||
        Name=="ParagraphStyles" ||
        Name=="FrameStyles" ||
        Name=="PageStyles" ||
        Name=="NumberingStyles" )
        return sal_True;
    else
        return sal_False;
}

sal_Int32 SwXStyleFamilies::getCount() throw( uno::RuntimeException, std::exception )
{
    return STYLE_FAMILY_COUNT;
}

uno::Any SwXStyleFamilies::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(nIndex < 0 || nIndex >= STYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(IsValid())
    {
        uno::Reference< container::XNameContainer >  aRef;
        const sal_uInt16 nType = aStyleByIndex[nIndex];
        switch( nType )
        {
            case SFX_STYLE_FAMILY_CHAR:
            {
                if(!m_xCharStyles.is())
                {
                    m_xCharStyles = new SwXStyleFamily(m_pDocShell, nType);
                }
                aRef = m_xCharStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PARA:
            {
                if(!m_xParaStyles.is())
                {
                    m_xParaStyles = new SwXStyleFamily(m_pDocShell, nType);
                }
                aRef = m_xParaStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PAGE     :
            {
                if(!m_xPageStyles.is())
                {
                    m_xPageStyles = new SwXStyleFamily(m_pDocShell, nType);
                }
                aRef = m_xPageStyles;
            }
            break;
            case SFX_STYLE_FAMILY_FRAME    :
            {
                if(!m_xFrameStyles.is())
                {
                    m_xFrameStyles = new SwXStyleFamily(m_pDocShell, nType);
                }
                aRef = m_xFrameStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PSEUDO:
            {
                if(!m_xNumberingStyles.is())
                {
                    m_xNumberingStyles = new SwXStyleFamily(m_pDocShell, nType);
                }
                aRef = m_xNumberingStyles;
            }
            break;
        }
        aRet.setValue(&aRef, cppu::UnoType<container::XNameContainer>::get());
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SwXStyleFamilies::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<container::XNameContainer>::get();

}

sal_Bool SwXStyleFamilies::hasElements() throw( uno::RuntimeException, std::exception )
{
    return sal_True;
}

void SwXStyleFamilies::loadStylesFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& aOptions)
    throw (io::IOException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    if(IsValid() && !rURL.isEmpty())
    {
        bool    bLoadStyleText = true;
        bool    bLoadStylePage = true;
        bool    bLoadStyleOverwrite = true;
        bool    bLoadStyleNumbering = true;
        bool    bLoadStyleFrame = true;

        int nCount = aOptions.getLength();
        const beans::PropertyValue* pArray = aOptions.getConstArray();
        for(int i = 0; i < nCount; i++)
        {
            const uno::Any* pVal = &pArray[i].Value;
            if( pVal->getValueType() == cppu::UnoType<bool>::get() )
            {
                const OUString sName = pArray[i].Name;
                bool bVal = *static_cast<sal_Bool const *>(pVal->getValue());
                if( sName == UNO_NAME_OVERWRITE_STYLES )
                    bLoadStyleOverwrite = bVal;
                else if( sName == UNO_NAME_LOAD_NUMBERING_STYLES )
                    bLoadStyleNumbering = bVal;
                else if( sName == UNO_NAME_LOAD_PAGE_STYLES )
                    bLoadStylePage = bVal;
                else if( sName == UNO_NAME_LOAD_FRAME_STYLES )
                    bLoadStyleFrame = bVal;
                else if( sName == UNO_NAME_LOAD_TEXT_STYLES )
                    bLoadStyleText = bVal;
            }
        }

        SwgReaderOption aOpt;
        aOpt.SetFrameFormats( bLoadStyleFrame );
        aOpt.SetTextFormats( bLoadStyleText );
        aOpt.SetPageDescs( bLoadStylePage );
        aOpt.SetNumRules( bLoadStyleNumbering );
        aOpt.SetMerge( !bLoadStyleOverwrite );

        sal_uLong nErr = m_pDocShell->LoadStylesFromFile( rURL, aOpt, true );
        if( nErr )
            throw io::IOException();
    }
    else
        throw uno::RuntimeException();
}

uno::Sequence< beans::PropertyValue > SwXStyleFamilies::getStyleLoaderOptions()
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyValue > aSeq(5);
    beans::PropertyValue* pArray = aSeq.getArray();
    const uno::Any aVal(true);
    pArray[0] = beans::PropertyValue(UNO_NAME_LOAD_TEXT_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[1] = beans::PropertyValue(UNO_NAME_LOAD_FRAME_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[2] = beans::PropertyValue(UNO_NAME_LOAD_PAGE_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[3] = beans::PropertyValue(UNO_NAME_LOAD_NUMBERING_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[4] = beans::PropertyValue(UNO_NAME_OVERWRITE_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    return aSeq;
}

OUString SwXStyleFamily::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXStyleFamily");
}

sal_Bool SwXStyleFamily::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyleFamily::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aRet { "com.sun.star.style.StyleFamily" };
    return aRet;
}

SwXStyleFamily::SwXStyleFamily(SwDocShell* pDocSh, sal_uInt16 nFamily) :
        m_eFamily((SfxStyleFamily)nFamily),
        m_pBasePool(pDocSh->GetStyleSheetPool()),
        m_pDocShell(pDocSh)
{
    StartListening(*m_pBasePool);
}

SwXStyleFamily::~SwXStyleFamily()
{

}

static bool lcl_GetHeaderFooterItem(
        SfxItemSet const& rSet, OUString const& rPropName, bool const bFooter,
        SvxSetItem const*& o_rpItem)
{
    SfxItemState eState = rSet.GetItemState(
        (bFooter) ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
        false, reinterpret_cast<const SfxPoolItem**>(&o_rpItem));
    if (SfxItemState::SET != eState &&
        rPropName == UNO_NAME_FIRST_IS_SHARED)
    {   // fdo#79269 header may not exist, check footer then
        eState = rSet.GetItemState(
            (!bFooter) ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
            false, reinterpret_cast<const SfxPoolItem**>(&o_rpItem));
    }
    return SfxItemState::SET == eState;
}

static sal_Int32 lcl_GetCountOrName(const SwDoc &rDoc,
    SfxStyleFamily eFamily, OUString *pString, sal_Int32 nIndex = SAL_MAX_INT32)
{
    sal_Int32 nCount = 0;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            const sal_Int32 nBaseCount =
                                     RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN  +
                                     RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN;
            nIndex = nIndex - nBaseCount;
            const size_t nArrLen = rDoc.GetCharFormats()->size();
            for( size_t i = 0; i < nArrLen; ++i )
            {
                SwCharFormat* pFormat = (*rDoc.GetCharFormats())[ i ];
                if( pFormat->IsDefault() && pFormat != rDoc.GetDfltCharFormat() )
                    continue;
                if ( IsPoolUserFormat ( pFormat->GetPoolFormatId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        // the default character format needs to be set to "Default!"
                        if(rDoc.GetDfltCharFormat() == pFormat)
                            SwStyleNameMapper::FillUIName(
                                RES_POOLCOLL_STANDARD, *pString );
                        else
                            *pString = pFormat->GetName();
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
            const sal_Int32 nBaseCount =
                                    RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN +
                                    RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                    RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                    RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                    RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                    RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN;
            nIndex = nIndex - nBaseCount;
            const size_t nArrLen = rDoc.GetTextFormatColls()->size();
            for ( size_t i = 0; i < nArrLen; ++i )
            {
                SwTextFormatColl * pColl = (*rDoc.GetTextFormatColls())[i];
                if ( pColl->IsDefault() )
                    continue;
                if ( IsPoolUserFormat ( pColl->GetPoolFormatId() ) )
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
            const sal_Int32 nBaseCount = RES_POOLFRM_END - RES_POOLFRM_BEGIN;
            nIndex = nIndex - nBaseCount;
            const size_t nArrLen = rDoc.GetFrameFormats()->size();
            for( size_t i = 0; i < nArrLen; ++i )
            {
                const SwFrameFormat* pFormat = (*rDoc.GetFrameFormats())[ i ];
                if(pFormat->IsDefault() || pFormat->IsAuto())
                    continue;
                if ( IsPoolUserFormat ( pFormat->GetPoolFormatId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = pFormat->GetName();
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
            const sal_Int32 nBaseCount = RES_POOLPAGE_END - RES_POOLPAGE_BEGIN;
            nIndex = nIndex - nBaseCount;
            const size_t nArrLen = rDoc.GetPageDescCnt();
            for(size_t i = 0; i < nArrLen; ++i)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc(i);

                if ( IsPoolUserFormat ( rDesc.GetPoolFormatId() ) )
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
            const sal_Int32 nBaseCount = RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN;
            nIndex = nIndex - nBaseCount;
            const SwNumRuleTable& rNumTable = rDoc.GetNumRuleTable();
            for(size_t i = 0; i < rNumTable.size(); ++i)
            {
                const SwNumRule& rRule = *rNumTable[ i ];
                if( rRule.IsAutoRule() )
                    continue;
                if ( IsPoolUserFormat ( rRule.GetPoolFormatId() ) )
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

sal_Int32 SwXStyleFamily::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    return lcl_GetCountOrName ( *m_pDocShell->GetDoc(), m_eFamily, nullptr );
}

uno::Any SwXStyleFamily::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if ( nIndex >= 0 )
    {
        if(m_pBasePool)
        {
            OUString sStyleName;
            switch( m_eFamily )
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
                lcl_GetCountOrName ( *m_pDocShell->GetDoc(), m_eFamily, &sStyleName, nIndex );

            if (!sStyleName.isEmpty())
            {
                SfxStyleSheetBase* pBase = m_pBasePool->Find( sStyleName, m_eFamily );
                if(pBase)
                {
                    uno::Reference< style::XStyle >  xStyle = _FindStyle(sStyleName);
                    if(!xStyle.is())
                    {
                        switch(m_eFamily)
                        {
                            case SFX_STYLE_FAMILY_PAGE:
                                xStyle = new SwXPageStyle(*m_pBasePool, m_pDocShell, m_eFamily, sStyleName);
                                break;
                            case SFX_STYLE_FAMILY_FRAME:
                                xStyle = new SwXFrameStyle(*m_pBasePool, m_pDocShell->GetDoc(), pBase->GetName());
                                break;
                            default:
                                xStyle = new SwXStyle(*m_pBasePool, m_eFamily, m_pDocShell->GetDoc(), sStyleName);
                        }
                    }
                    aRet.setValue(&xStyle, cppu::UnoType<style::XStyle>::get());
                }
                else
                    throw uno::RuntimeException();
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
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true );
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName);
        if(pBase)
        {
            uno::Reference< style::XStyle >  xStyle = _FindStyle(sStyleName);
            if(!xStyle.is())
            {
                switch(m_eFamily)
                {
                    case SFX_STYLE_FAMILY_PAGE:
                        xStyle = new SwXPageStyle(*m_pBasePool, m_pDocShell, m_eFamily, sStyleName);
                        break;
                    case SFX_STYLE_FAMILY_FRAME:
                        xStyle = new SwXFrameStyle(*m_pBasePool, m_pDocShell->GetDoc(), pBase->GetName());
                        break;
                    default:
                        xStyle = new SwXStyle(*m_pBasePool, m_eFamily, m_pDocShell->GetDoc(), sStyleName);
                }
            }
            aRet.setValue(&xStyle, cppu::UnoType<style::XStyle>::get());
        }
        else
            throw container::NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
    return aRet;

}

uno::Sequence< OUString > SwXStyleFamily::getElementNames() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    std::vector< OUString > aRet;
    if(m_pBasePool)
    {
        SfxStyleSheetIteratorPtr pIt = m_pBasePool->CreateIterator(m_eFamily, SFXSTYLEBIT_ALL);
        OUString aString;
        for (SfxStyleSheetBase* pStyle = pIt->First(); pStyle; pStyle = pIt->Next())
        {
            SwStyleNameMapper::FillProgName(pStyle->GetName(), aString,
                                            lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true);
            aRet.push_back(aString);
        }
    }
    else
        throw uno::RuntimeException();

    return comphelper::containerToSequence(aRet);
}

sal_Bool SwXStyleFamily::hasByName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pBasePool)
    {
        OUString sStyleName;
        SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true );
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName);
        bRet = nullptr != pBase;
    }
    else
        throw uno::RuntimeException();
    return bRet;

}

uno::Type SwXStyleFamily::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<style::XStyle>::get();

}

sal_Bool SwXStyleFamily::hasElements() throw( uno::RuntimeException, std::exception )
{
    if(!m_pBasePool)
        throw uno::RuntimeException();
    return sal_True;
}

void SwXStyleFamily::insertByName(const OUString& rName, const uno::Any& rElement)
        throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
    {
        OUString sStyleName;
        SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true);
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName);
        SfxStyleSheetBase* pUINameBase = m_pBasePool->Find( sStyleName );
        if(pBase || pUINameBase)
            throw container::ElementExistException();
        else
        {
            if(rElement.getValueType().getTypeClass() ==
                                            uno::TypeClass_INTERFACE)
            {
                uno::Reference< uno::XInterface > const * pxRef =
                    static_cast<uno::Reference< uno::XInterface > const *>(rElement.getValue());

                uno::Reference<lang::XUnoTunnel> xStyleTunnel( *pxRef, uno::UNO_QUERY);

                SwXStyle* pNewStyle = nullptr;
                if(xStyleTunnel.is())
                {
                    pNewStyle = reinterpret_cast< SwXStyle * >(
                            sal::static_int_cast< sal_IntPtr >( xStyleTunnel->getSomething( SwXStyle::getUnoTunnelId()) ));
                }

                if (!pNewStyle || !pNewStyle->IsDescriptor() || pNewStyle->GetFamily() != m_eFamily)
                    throw lang::IllegalArgumentException();

                sal_uInt16 nMask = SFXSTYLEBIT_ALL;
                if(m_eFamily == SFX_STYLE_FAMILY_PARA && !pNewStyle->IsConditional())
                    nMask &= ~SWSTYLEBIT_CONDCOLL;
#if OSL_DEBUG_LEVEL > 1
                SfxStyleSheetBase& rNewBase =
#endif
                m_pBasePool->Make(sStyleName, m_eFamily, nMask);
                pNewStyle->SetDoc(m_pDocShell->GetDoc(), m_pBasePool);
                pNewStyle->SetStyleName(sStyleName);
                const OUString sParentStyleName(pNewStyle->GetParentStyleName());
                if (!sParentStyleName.isEmpty())
                {
                    m_pBasePool->SetSearchMask(m_eFamily);
                    SfxStyleSheetBase* pParentBase = m_pBasePool->Find(sParentStyleName);
                    if(pParentBase && pParentBase->GetFamily() == m_eFamily &&
                        &pParentBase->GetPool() == m_pBasePool)
                        m_pBasePool->SetParent( m_eFamily, sStyleName, sParentStyleName );

                }
#if OSL_DEBUG_LEVEL > 1
                (void)rNewBase;
#endif
                // after all, we still need to apply the properties of the descriptor
                pNewStyle->ApplyDescriptorProperties();
            }
            else
                throw lang::IllegalArgumentException();
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXStyleFamily::replaceByName(const OUString& rName, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(rName);
        // replacements only for userdefined styles
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

        m_pBasePool->Remove(pBase);
        insertByName(rName, rElement);
    }
    else
        throw uno::RuntimeException();
}

void SwXStyleFamily::removeByName(const OUString& rName) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        OUString aString;
        SwStyleNameMapper::FillUIName(rName, aString, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true );

        SfxStyleSheetBase* pBase = m_pBasePool->Find( aString );
        if(pBase)
            m_pBasePool->Remove(pBase);
        else
            throw container::NoSuchElementException();
    }
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL SwXStyleFamily::getPropertySetInfo(  ) throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL SwXStyleFamily::setPropertyValue( const OUString&, const uno::Any& ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

uno::Any SAL_CALL SwXStyleFamily::getPropertyValue( const OUString& sPropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    uno::Any aRet;

    if ( sPropertyName == "DisplayName" )
    {
        SolarMutexGuard aGuard;
        sal_uInt32 nResId = 0;
        switch ( m_eFamily )
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
        throw beans::UnknownPropertyException( "unknown property: " + sPropertyName, static_cast<OWeakObject *>(this) );
    }

    return aRet;
}

void SAL_CALL SwXStyleFamily::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL SwXStyleFamily::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL SwXStyleFamily::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SAL_CALL SwXStyleFamily::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "###unexpected!" );
}

void SwXStyleFamily::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxSimpleHint *pHint = dynamic_cast<const SfxSimpleHint*>( &rHint );
    if( pHint && ( pHint->GetId() & SFX_HINT_DYING ) )
    {
        m_pBasePool = nullptr;
        m_pDocShell = nullptr;
        EndListening(rBC);
    }
}

SwXStyle*   SwXStyleFamily::_FindStyle(const OUString& rStyleName)const
{
    const size_t nLCount = m_pBasePool->GetSizeOfVector();
    for( size_t i = 0; i < nLCount; ++i)
    {
        SfxListener* pListener = m_pBasePool->GetListener( i );
        SwXStyle* pTempStyle = dynamic_cast<SwXStyle*>( pListener );
        if(pTempStyle && pTempStyle->GetFamily() == m_eFamily && pTempStyle->GetStyleName() == rStyleName)
        {
            return pTempStyle;
        }
    }
    return nullptr;
}

class SwStyleProperties_Impl
{
    const PropertyEntryVector_t aPropertyEntries;
    uno::Any**                  pAnyArr;
    sal_uInt32                  nArrLen;

public:
    explicit SwStyleProperties_Impl(const SfxItemPropertyMap& rMap);
    ~SwStyleProperties_Impl();

    bool    SetProperty(const OUString& rName, const uno::Any& rVal);
    bool    GetProperty(const OUString& rName, uno::Any*& rpAny);
    bool    ClearProperty( const OUString& rPropertyName );
    void    ClearAllProperties( );
    static void GetProperty(const OUString &rPropertyName, const uno::Reference < beans::XPropertySet > &rxPropertySet, uno::Any& rAny );

    const PropertyEntryVector_t& GetPropertyVector() const {return aPropertyEntries; }

};

SwStyleProperties_Impl::SwStyleProperties_Impl(const SfxItemPropertyMap& rMap) :
    aPropertyEntries( rMap.getPropertyEntries() ),
    nArrLen(0)
{
    nArrLen = aPropertyEntries.size();

    pAnyArr = new uno::Any* [nArrLen];
    for ( sal_uInt32 i =0 ; i < nArrLen; i++ )
        pAnyArr[i] = nullptr;
}

SwStyleProperties_Impl::~SwStyleProperties_Impl()
{
    for ( sal_uInt32 i =0 ; i < nArrLen; i++ )
        delete pAnyArr[i];
    delete[] pAnyArr;
}

bool SwStyleProperties_Impl::SetProperty(const OUString& rName, const uno::Any& rVal)
{
    sal_uInt32 nPos = 0;
    bool bRet = false;
    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    while( aIt != aPropertyEntries.end() )
    {
        if(rName == aIt->sName)
        {
            delete pAnyArr[nPos];
            pAnyArr[nPos] = new uno::Any(rVal);
            bRet = true;
            break;
        }
        ++nPos;
        ++aIt;
    }
    return bRet;
}

bool SwStyleProperties_Impl::ClearProperty( const OUString& rName )
{
    bool bRet = false;
    sal_uInt32 nPos = 0;
    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    while( aIt != aPropertyEntries.end() )
    {
        if( rName == aIt->sName )
        {
            delete pAnyArr[nPos];
            pAnyArr[ nPos ] = nullptr;
            bRet = true;
            break;
        }
        ++nPos;
        ++aIt;
    }
    return bRet;
}

void SwStyleProperties_Impl::ClearAllProperties( )
{
    for ( sal_uInt32 i = 0; i < nArrLen; i++ )
    {
        delete pAnyArr[i];
        pAnyArr[ i ] = nullptr;
    }
}

bool SwStyleProperties_Impl::GetProperty(const OUString& rName, uno::Any*& rpAny )
{
    bool bRet = false;
    sal_uInt32 nPos = 0;
    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    while( aIt != aPropertyEntries.end() )
    {
        if( rName == aIt->sName )
        {
            rpAny = pAnyArr[nPos];
            bRet = true;
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
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}


OUString SwXStyle::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXStyle");
}

sal_Bool SwXStyle::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyle::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    long nCount = 1;
    if(SFX_STYLE_FAMILY_PARA == m_eFamily)
    {
        nCount = 5;
        if(m_bIsConditional)
            nCount++;
    }
    else if(SFX_STYLE_FAMILY_CHAR == m_eFamily)
        nCount = 5;
    else if(SFX_STYLE_FAMILY_PAGE == m_eFamily)
        nCount = 3;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.style.Style";
    switch(m_eFamily)
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
        if(m_bIsConditional)
            pArray[5] = "com.sun.star.style.ConditionalParagraphStyle";
        break;

        default:
            ;
    }
    return aRet;
}

SwXStyle::SwXStyle( SwDoc *pDoc, SfxStyleFamily eFam, bool bConditional) :
    m_pDoc( pDoc ),
    m_pBasePool(nullptr),
    m_eFamily(eFam),
    m_bIsDescriptor(true),
    m_bIsConditional(bConditional)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    // get the property set for the default style data
    // First get the model
    uno::Reference < frame::XModel > xModel = pDoc->GetDocShell()->GetBaseModel();
    // Ask the model for it's family supplier interface
    uno::Reference < style::XStyleFamiliesSupplier > xFamilySupplier ( xModel, uno::UNO_QUERY );
    // Get the style families
    uno::Reference < container::XNameAccess > xFamilies = xFamilySupplier->getStyleFamilies();

    uno::Any aAny;
    sal_uInt16 nMapId = PROPERTY_MAP_NUM_STYLE;
    switch( m_eFamily )
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
            nMapId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE;
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
    m_pPropertiesImpl = new SwStyleProperties_Impl(aSwMapProvider.GetPropertySet(nMapId)->getPropertyMap());
}

SwXStyle::SwXStyle(SfxStyleSheetBasePool& rPool, SfxStyleFamily eFam,
        SwDoc* pDoc, const OUString& rStyleName) :
    m_pDoc(pDoc),
    m_sStyleName(rStyleName),
    m_pBasePool(&rPool),
    m_eFamily(eFam),
    m_bIsDescriptor(false),
    m_bIsConditional(false),
    m_pPropertiesImpl(nullptr)
{
    StartListening(rPool);
    if(eFam == SFX_STYLE_FAMILY_PARA)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );
        if(pBase)
        {
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(m_sStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                m_bIsConditional = ::IsConditionalByPoolId( nId );
            else
                m_bIsConditional = RES_CONDTXTFMTCOLL == static_cast<SwDocStyleSheet*>(pBase)->GetCollection()->Which();
        }
    }
}

SwXStyle::~SwXStyle()
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
        EndListening(*m_pBasePool);
    delete m_pPropertiesImpl;
    if(GetRegisteredIn())
        GetRegisteredIn()->Remove( this );
}

void SwXStyle::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_pDoc = nullptr;
        mxStyleData.clear();
        mxStyleFamily.clear();
    }
}

OUString SwXStyle::getName() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );
        if(!pBase)
            throw uno::RuntimeException();
        OUString aString;
        SwStyleNameMapper::FillProgName(pBase->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true);
        return aString;
    }
    return m_sStyleName;
}

void SwXStyle::setName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );
        bool bExcept = true;
        if(pBase && pBase->IsUserDefined())
        {
            rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
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

sal_Bool SwXStyle::isUserDefined() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        //if it is not found it must be non user defined
        if(pBase)
            bRet = pBase->IsUserDefined();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXStyle::isInUse() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily, SFXSTYLEBIT_USED);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        if(pBase)
            bRet = pBase->IsUsed();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

OUString SwXStyle::getParentStyle() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString aString;
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        if(pBase)
            aString = pBase->GetParent();
    }
    else if(m_bIsDescriptor)
        aString = m_sParentStyleName;
    else
        throw uno::RuntimeException();
    SwStyleNameMapper::FillProgName(aString, aString, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true );
    return aString;
}

void SwXStyle::setParentStyle(const OUString& rParentStyle)
            throw( container::NoSuchElementException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString sParentStyle;
    SwStyleNameMapper::FillUIName(rParentStyle, sParentStyle, lcl_GetSwEnumFromSfxEnum ( m_eFamily ), true );
    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        bool bExcept = false;
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xBase( new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)) );
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
    else if(m_bIsDescriptor)
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

static uno::Reference< beans::XPropertySetInfo > lcl_getPropertySetInfo( SfxStyleFamily eFamily, bool bIsConditional )
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

uno::Reference< beans::XPropertySetInfo >  SwXStyle::getPropertySetInfo()
    throw( uno::RuntimeException, std::exception )
{
    return lcl_getPropertySetInfo( m_eFamily, m_bIsConditional );
}

void    SwXStyle::ApplyDescriptorProperties()
{
    m_bIsDescriptor = false;
    mxStyleData.clear();
    mxStyleFamily.clear();

    const PropertyEntryVector_t& rPropertyVector = m_pPropertiesImpl->GetPropertyVector();
    PropertyEntryVector_t::const_iterator aIt = rPropertyVector.begin();
    while(aIt != rPropertyVector.end())
    {
        uno::Any* pAny(nullptr);
        m_pPropertiesImpl->GetProperty(aIt->sName, pAny);
        if (pAny)
            setPropertyValue(aIt->sName, *pAny);
        ++aIt;
    }
}

class SwStyleBase_Impl
{
private:
    SwDoc& mrDoc;
    const SwPageDesc* mpOldPageDesc;
    rtl::Reference< SwDocStyleSheet > mxNewBase;
    SfxItemSet* mpItemSet;
    OUString mrStyleName;
    const SwAttrSet*                    mpParentStyle;

public:
    SwStyleBase_Impl(SwDoc& rSwDoc, const OUString& rName, const SwAttrSet* pParentStyle)
        : mrDoc(rSwDoc)
        , mpOldPageDesc(nullptr)
        , mpItemSet(nullptr)
        , mrStyleName(rName)
        , mpParentStyle(pParentStyle)
    {
    }

    ~SwStyleBase_Impl()
    {
        delete mpItemSet;
    }

    rtl::Reference< SwDocStyleSheet >& getNewBase()
    {
        return mxNewBase;
    }

    void setNewBase(SwDocStyleSheet* pNew)
    {
        mxNewBase = pNew;
    }

    bool HasItemSet() const
    {
        return mxNewBase.is();
    }

    SfxItemSet* replaceItemSet(SfxItemSet* pNew)
    {
        SfxItemSet* pRetval = mpItemSet;
        mpItemSet = pNew;
        return pRetval;
    }

    SfxItemSet& GetItemSet()
    {
        OSL_ENSURE(mxNewBase.is(), "no SwDocStyleSheet available");
        if (!mpItemSet)
        {
            mpItemSet = new SfxItemSet(mxNewBase->GetItemSet());

            //UUUU set parent style to have the correct XFillStyle setting as XFILL_NONE
            if(!mpItemSet->GetParent() && mpParentStyle)
            {
                mpItemSet->SetParent(mpParentStyle);
            }
        }
        return *mpItemSet;
    }

    const SwPageDesc* GetOldPageDesc();
};

const SwPageDesc* SwStyleBase_Impl::GetOldPageDesc()
{
    if(!mpOldPageDesc)
    {
        SwPageDesc *pd = mrDoc.FindPageDesc( mrStyleName );
        if( pd )
            mpOldPageDesc = pd;

        if(!mpOldPageDesc)
        {
            for(sal_uInt16 i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
            {
                if(SW_RESSTR(i) == mrStyleName)
                {
                    mpOldPageDesc = mrDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( static_cast< sal_uInt16 >(RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN) );
                    break;
                }
            }
        }
    }
    return mpOldPageDesc;
}

static void lcl_SetStyleProperty(const SfxItemPropertySimpleEntry& rEntry,
                        const SfxItemPropertySet& rPropSet,
                        const uno::Any& rValue,
                        SwStyleBase_Impl& rBase,
                        SfxStyleSheetBasePool* pBasePool,
                        SwDoc* pDoc,
                        SfxStyleFamily eFamily)
                            throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                                  lang::WrappedTargetException, uno::RuntimeException,
                                  std::exception)

{
    //UUUU adapted switch logic to a more readable state; removed goto's and made
    // execution of standard setting of proerty in ItemSet dependent of this variable
    bool bDone(false);

    //UUUU
    const sal_uInt8 nMemberId(rEntry.nMemberId & (~SFX_METRIC_ITEM));
    uno::Any aValue(rValue);

    //UUUU check for needed metric translation
    if(rEntry.nMemberId & SFX_METRIC_ITEM)
    {
        bool bDoIt(true);

        if(XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
        {
            // exception: If these ItemTypes are used, do not convert when these are negative
            // since this means they are intended as percent values
            sal_Int32 nValue = 0;

            if(aValue >>= nValue)
            {
                bDoIt = nValue > 0;
            }
        }

        if(bDoIt && pDoc)
        {
            const SfxItemPool& rPool = pDoc->GetAttrPool();
            const SfxMapUnit eMapUnit(rPool.GetMetric(rEntry.nWID));

            if(eMapUnit != SFX_MAPUNIT_100TH_MM)
            {
                SvxUnoConvertFromMM(eMapUnit, aValue);
            }
        }
    }

    switch(rEntry.nWID)
    {
        case FN_UNO_HIDDEN:
        {
            bool bHidden = false;
            if ( rValue >>= bHidden )
            {
                //make it a 'real' style - necessary for pooled styles
                rBase.getNewBase()->GetItemSet();
                rBase.getNewBase()->SetHidden( bHidden );
            }
        }
        break;

        case FN_UNO_STYLE_INTEROP_GRAB_BAG:
        {
            rBase.getNewBase()->GetItemSet();
            rBase.getNewBase()->SetGrabBagItem(rValue);
        }
        break;

        case XATTR_FILLGRADIENT:
        case XATTR_FILLHATCH:
        case XATTR_FILLBITMAP:
        case XATTR_FILLFLOATTRANSPARENCE:
        // not yet needed; activate when LineStyle support may be added
        // case XATTR_LINESTART:
        // case XATTR_LINEEND:
        // case XATTR_LINEDASH:
        {
            if(MID_NAME == nMemberId)
            {
                //UUUU add set commands for FillName items
                OUString aTempName;
                SfxItemSet& rStyleSet = rBase.GetItemSet();

                if(!(aValue >>= aTempName))
                {
                    throw lang::IllegalArgumentException();
                }

                SvxShape::SetFillAttribute(rEntry.nWID, aTempName, rStyleSet);
                bDone = true;
            }
            else if(MID_GRAFURL == nMemberId)
            {
                if(XATTR_FILLBITMAP == rEntry.nWID)
                {
                    //UUUU Bitmap also has the MID_GRAFURL mode where a Bitmap URL is used
                    const Graphic aNullGraphic;
                    SfxItemSet& rStyleSet = rBase.GetItemSet();
                    XFillBitmapItem aXFillBitmapItem(rStyleSet.GetPool(), aNullGraphic);

                    aXFillBitmapItem.PutValue(aValue, nMemberId);
                    rStyleSet.Put(aXFillBitmapItem);
                    bDone = true;
                }
            }

            break;
        }
        case RES_BACKGROUND:
        {
            //UUUU
            SfxItemSet& rStyleSet = rBase.GetItemSet();
            const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rStyleSet, RES_BACKGROUND, true, pDoc->IsInXMLImport()));
            SvxBrushItem aChangedBrushItem(aOriginalBrushItem);

            aChangedBrushItem.PutValue(aValue, nMemberId);

            if (!(aChangedBrushItem == aOriginalBrushItem) ||
                // 0xff is already the default - but if BackTransparent is set
                // to true, it must be applied in the item set on ODF import
                // to potentially override parent style, which is unknown yet
                (MID_GRAPHIC_TRANSPARENT == nMemberId && aValue.has<bool>() && aValue.get<bool>()))
            {
                setSvxBrushItemAsFillAttributesToTargetSet(aChangedBrushItem, rStyleSet);
            }

            bDone = true;
            break;
        }
        case OWN_ATTR_FILLBMP_MODE:
        {
            //UUUU
            drawing::BitmapMode eMode;

            if(!(aValue >>= eMode))
            {
                sal_Int32 nMode = 0;

                if(!(aValue >>= nMode))
                {
                    throw lang::IllegalArgumentException();
                }

                eMode = (drawing::BitmapMode)nMode;
            }

            SfxItemSet& rStyleSet = rBase.GetItemSet();

            rStyleSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
            rStyleSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));

            bDone = true;
            break;
        }
        case RES_PAPER_BIN:
        {
            SfxPrinter *pPrinter = pDoc->getIDocumentDeviceAccess().getPrinter( true );
            OUString sTmp;
            sal_uInt16 nBin = USHRT_MAX;
            if ( !( aValue >>= sTmp ) )
                throw lang::IllegalArgumentException();
            if ( sTmp == "[From printer settings]" )
                nBin = USHRT_MAX-1;
            else if ( pPrinter )
            {
                for (sal_uInt16 i=0, nEnd = pPrinter->GetPaperBinCount(); i < nEnd; i++ )
                {
                    if (sTmp == pPrinter->GetPaperBinName ( i ) )
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

            bDone = true;
            break;
        }
        case  FN_UNO_NUM_RULES: // special handling for a SvxNumRuleItem:
        {
            if(aValue.getValueType() == cppu::UnoType<container::XIndexReplace>::get())
            {
                uno::Reference< container::XIndexReplace > const * pxRulesRef =
                        static_cast<uno::Reference< container::XIndexReplace > const *>(aValue.getValue());

                uno::Reference<lang::XUnoTunnel> xNumberTunnel( *pxRulesRef, uno::UNO_QUERY);

                SwXNumberingRules* pSwXRules = nullptr;
                if(xNumberTunnel.is())
                {
                    pSwXRules = reinterpret_cast< SwXNumberingRules * >(
                            sal::static_int_cast< sal_IntPtr >(xNumberTunnel->getSomething( SwXNumberingRules::getUnoTunnelId()) ));
                }
                if(pSwXRules)
                {
                    const OUString* pCharStyleNames = pSwXRules->GetNewCharStyleNames();
                    const OUString* pBulletFontNames = pSwXRules->GetBulletFontNames();

                    SwNumRule aSetRule(*pSwXRules->GetNumRule());
                    const SwCharFormats* pFormats = pDoc->GetCharFormats();
                    const size_t nChCount = pFormats->size();
                    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                    {

                        const SwNumFormat* pFormat = aSetRule.GetNumFormat( i );
                        if(pFormat)
                        {
                            SwNumFormat aFormat(*pFormat);
                            if (!pCharStyleNames[i].isEmpty() &&
                                !SwXNumberingRules::isInvalidStyle(pCharStyleNames[i]) &&
                                (!pFormat->GetCharFormat() || pFormat->GetCharFormat()->GetName() != pCharStyleNames[i]) )
                            {

                                SwCharFormat* pCharFormat = nullptr;
                                for(size_t j = 0; j< nChCount; ++j)
                                {
                                    SwCharFormat* pTmp = (*pFormats)[j];
                                    if(pTmp->GetName() == pCharStyleNames[i])
                                    {
                                        pCharFormat = pTmp;
                                        break;
                                    }
                                }
                                if(!pCharFormat && pBasePool)
                                {

                                    SfxStyleSheetBase* pBase;
                                    pBase = static_cast<SfxStyleSheetBasePool*>(pBasePool)->Find(pCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                                    if(!pBase)
                                        pBase = &pBasePool->Make(pCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                                    pCharFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();

                                }

                                aFormat.SetCharFormat( pCharFormat );
                            }
                            // same for fonts:
                            if (!pBulletFontNames[i].isEmpty() &&
                                !SwXNumberingRules::isInvalidStyle(pBulletFontNames[i]) &&
                                (!pFormat->GetBulletFont() || pFormat->GetBulletFont()->GetFamilyName() != pBulletFontNames[i]) )
                            {
                                const SvxFontListItem* pFontListItem =
                                        static_cast<const SvxFontListItem*>(pDoc->GetDocShell()
                                                            ->GetItem( SID_ATTR_CHAR_FONTLIST ));
                                const FontList*  pList = pFontListItem->GetFontList();
                                vcl::FontInfo aInfo = pList->Get(
                                    pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                                vcl::Font aFont(aInfo);
                                aFormat.SetBulletFont(&aFont);
                            }
                            aSetRule.Set( i, &aFormat );
                        }
                    }
                    rBase.getNewBase()->SetNumRule(aSetRule);
                }
            }
            else
                throw lang::IllegalArgumentException();

            bDone = true;
            break;
        }

        case RES_PARATR_OUTLINELEVEL:
        {
            sal_Int16 nLevel = 0;
            aValue >>= nLevel;
            if( 0 <= nLevel && nLevel <= MAXLEVEL)
                rBase.getNewBase()->GetCollection()->SetAttrOutlineLevel( nLevel );

            bDone = true;
            break;
        }

        case FN_UNO_FOLLOW_STYLE:
        {
            OUString sTmp;
            aValue >>= sTmp;
            OUString aString;
            SwStyleNameMapper::FillUIName(sTmp, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true ) ;
            rBase.getNewBase()->SetFollow( aString );

            bDone = true;
            break;
        }
        case RES_PAGEDESC :
        {
            if (MID_PAGEDESC_PAGEDESCNAME != nMemberId)
                break;
            // special handling for RES_PAGEDESC
            if(aValue.getValueType() != ::cppu::UnoType<OUString>::get())
                throw lang::IllegalArgumentException();
            SfxItemSet& rStyleSet = rBase.GetItemSet();

            std::unique_ptr<SwFormatPageDesc> pNewDesc;
            const SfxPoolItem* pItem;
            if(SfxItemState::SET == rStyleSet.GetItemState( RES_PAGEDESC, true, &pItem ) )
            {
                pNewDesc.reset( new SwFormatPageDesc(*static_cast<const SwFormatPageDesc*>(pItem)) );
            }
            else
                pNewDesc.reset( new SwFormatPageDesc );
            OUString uDescName;
            aValue >>= uDescName;
            OUString sDescName;
            SwStyleNameMapper::FillUIName(uDescName, sDescName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
            if(!pNewDesc->GetPageDesc() || pNewDesc->GetPageDesc()->GetName() != sDescName)
            {
                bool bPut = false;
                if (!sDescName.isEmpty())
                {
                    SwPageDesc* pPageDesc = SwPageDesc::GetByName(*pDoc, sDescName);
                    if(pPageDesc)
                    {
                        pNewDesc->RegisterToPageDesc( *pPageDesc );
                        bPut = true;
                    }
                    else
                    {
                        throw lang::IllegalArgumentException();
                    }
                }
                if(!bPut)
                {
                    rStyleSet.ClearItem(RES_BREAK);
                    rStyleSet.Put(SwFormatPageDesc());
                }
                else
                    rStyleSet.Put(*pNewDesc);

            }
            bDone = true;
            break;
        }
        case RES_TEXT_VERT_ADJUST:
        {
            if( pDoc )
            {
                const SwPageDesc* pOldPageDesc = rBase.GetOldPageDesc();
                SwPageDesc* pPageDesc = pOldPageDesc ? pDoc->FindPageDesc(pOldPageDesc->GetName()) : nullptr;
                if (pPageDesc)
                {
                    drawing::TextVerticalAdjust nVA;
                    rValue >>= nVA;
                    pPageDesc->SetVerticalAdjustment( nVA );
                }
            }
            bDone = true;
            break;
        }
        case FN_UNO_IS_AUTO_UPDATE:
        {
            bool bAuto = *static_cast<sal_Bool const *>(aValue.getValue());
            if(SFX_STYLE_FAMILY_PARA == eFamily)
                rBase.getNewBase()->GetCollection()->SetAutoUpdateFormat(bAuto);
            else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                rBase.getNewBase()->GetFrameFormat()->SetAutoUpdateFormat(bAuto);

            bDone = true;
            break;
        }
        case FN_UNO_PARA_STYLE_CONDITIONS:
        {
            uno::Sequence< beans::NamedValue > aSeq;
            if (!(aValue >>= aSeq))
                throw lang::IllegalArgumentException();

            OSL_ENSURE(COND_COMMAND_COUNT == 28,
                    "invalid size of command count?");
            const beans::NamedValue *pSeq = aSeq.getConstArray();
            const sal_Int32 nLen = aSeq.getLength();

            bool bFailed = false;
            SwCondCollItem aCondItem;
            for(sal_Int32 i = 0; i < nLen; ++i)
            {
                OUString aTmp;
                if ((pSeq[i].Value >>= aTmp))
                {
                    // get UI style name from programmatic style name
                    OUString aStyleName;
                    SwStyleNameMapper::FillUIName(aTmp, aStyleName,
                            lcl_GetSwEnumFromSfxEnum(eFamily), true);

                    // check for correct context and style name

                    sal_Int16 nIdx = GetCommandContextIndex( pSeq[i].Name );

                    bool bStyleFound = false;
                    if (pBasePool)
                    {
                        pBasePool->SetSearchMask( SFX_STYLE_FAMILY_PARA);
                        const SfxStyleSheetBase* pBase = pBasePool->First();
                        while (pBase && !bStyleFound)
                        {
                            if(pBase->GetName() == aStyleName)
                                bStyleFound = true;
                            pBase = pBasePool->Next();
                        }
                    }

                    if (nIdx == -1 || !bStyleFound)
                    {
                        bFailed = true;
                        break;
                    }

                    aCondItem.SetStyle(&aStyleName, nIdx);
                }
                else
                    bFailed = true;
            }
            if (bFailed)
                throw lang::IllegalArgumentException();
            rBase.GetItemSet().Put( aCondItem );
            bDone = true;
            break;
        }
        case FN_UNO_CATEGORY:
        {
            if(!rBase.getNewBase()->IsUserDefined())
                throw lang::IllegalArgumentException();
            short nSet = 0;
            aValue >>= nSet;

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

            rBase.getNewBase()->SetMask( nId|SFXSTYLEBIT_USERDEF );
            bDone = true;
            break;
        }
        case SID_SWREGISTER_COLLECTION:
        {
            OUString sName;
            aValue >>= sName;
            SwRegisterItem aReg( !sName.isEmpty() );
            aReg.SetWhich(SID_SWREGISTER_MODE);
            rBase.GetItemSet().Put(aReg);
            OUString aString;
            SwStyleNameMapper::FillUIName(sName, aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true);

            rBase.GetItemSet().Put(SfxStringItem(SID_SWREGISTER_COLLECTION, aString ) );
            bDone = true;
            break;
        }
        case RES_TXTATR_CJK_RUBY:
        {
            if(MID_RUBY_CHARSTYLE == nMemberId )
            {
                OUString sTmp;
                if(aValue >>= sTmp)
                {
                    SfxItemSet& rStyleSet = rBase.GetItemSet();
                    std::unique_ptr<SwFormatRuby> pRuby;
                    const SfxPoolItem* pItem;
                    if(SfxItemState::SET == rStyleSet.GetItemState( RES_TXTATR_CJK_RUBY, true, &pItem ) )
                        pRuby.reset(new SwFormatRuby(*static_cast<const SwFormatRuby*>(pItem)));
                    else
                        pRuby.reset(new SwFormatRuby(OUString()));
                    OUString sStyle;
                    SwStyleNameMapper::FillUIName(sTmp, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                    pRuby->SetCharFormatName( sTmp );
                    pRuby->SetCharFormatId( 0 );
                    if(!sTmp.isEmpty())
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sTmp, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
                        pRuby->SetCharFormatId(nId);
                    }
                    rStyleSet.Put(*pRuby);
                }
                else
                    throw lang::IllegalArgumentException();
            }
            break;
        }
        case RES_PARATR_DROP:
        {
            if( MID_DROPCAP_CHAR_STYLE_NAME == nMemberId)
            {
                if(aValue.getValueType() == ::cppu::UnoType<OUString>::get())
                {
                    SfxItemSet& rStyleSet = rBase.GetItemSet();

                    std::unique_ptr<SwFormatDrop> pDrop;
                    const SfxPoolItem* pItem;
                    if(SfxItemState::SET == rStyleSet.GetItemState( RES_PARATR_DROP, true, &pItem ) )
                        pDrop.reset(new SwFormatDrop(*static_cast<const SwFormatDrop*>(pItem)));
                    else
                        pDrop.reset( new SwFormatDrop );
                    OUString uStyle;
                    aValue >>= uStyle;
                    OUString sStyle;
                    SwStyleNameMapper::FillUIName(uStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                    SwDocStyleSheet* pStyle =
                        static_cast<SwDocStyleSheet*>(pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR));
                    if(pStyle)
                        pDrop->SetCharFormat(pStyle->GetCharFormat());
                    else
                        throw lang::IllegalArgumentException();
                    rStyleSet.Put(*pDrop);
                }
                else
                    throw lang::IllegalArgumentException();

                bDone = true;
            }
            break;
        }
        default:
        {
            // nothing to do
            break;
        }
    }

    if(!bDone)
    {
        // default ItemSet handling
        SfxItemSet& rStyleSet = rBase.GetItemSet();
        SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
        aSet.SetParent(&rStyleSet);
        rPropSet.setPropertyValue(rEntry, aValue, aSet);
        rStyleSet.Put(aSet);

        // --> OD 2006-10-18 #i70223#
        if ( SFX_STYLE_FAMILY_PARA == eFamily &&
                rEntry.nWID == RES_PARATR_NUMRULE &&
                rBase.getNewBase().is() && rBase.getNewBase()->GetCollection() &&
                //rBase.getNewBase()->GetCollection()->GetOutlineLevel() < MAXLEVEL /* assigned to list level of outline style */) //#outline level,removed by zhaojianwei
                rBase.getNewBase()->GetCollection()->IsAssignedToListLevelOfOutlineStyle() )       ////<-end,add by zhaojianwei
        {
            OUString sNewNumberingRuleName;
            aValue >>= sNewNumberingRuleName;
            if ( sNewNumberingRuleName.isEmpty() ||
                 sNewNumberingRuleName != pDoc->GetOutlineNumRule()->GetName() )
            {
                rBase.getNewBase()->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
            }
        }
    }
}

void SAL_CALL SwXStyle::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
           lang::IllegalArgumentException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    if ( !m_pDoc )
        throw uno::RuntimeException();

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;

    switch(m_eFamily)
    {
        case SFX_STYLE_FAMILY_PARA  : nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
        case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE   ;break;
        default: ;
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();

    if(rPropertyNames.getLength() != rValues.getLength())
    {
        throw lang::IllegalArgumentException();
    }

    const OUString* pNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();

    SwStyleBase_Impl aBaseImpl(*m_pDoc, m_sStyleName, &GetDoc()->GetDfltTextFormatColl()->GetAttrSet()); //UUUU add pDfltTextFormatColl as parent
    if(m_pBasePool)
    {
        const sal_uInt16 nSaveMask = m_pBasePool->GetSearchMask();
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        m_pBasePool->SetSearchMask(m_eFamily, nSaveMask );
        OSL_ENSURE(pBase, "where is the style?" );
        if(pBase)
            aBaseImpl.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
        else
            throw uno::RuntimeException();
    }

    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pNames[nProp]);

        if(!pEntry ||
           (!m_bIsConditional && pNames[nProp] == UNO_NAME_PARA_STYLE_CONDITIONS))
            throw beans::UnknownPropertyException("Unknown property: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if(aBaseImpl.getNewBase().is())
        {
            lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl, m_pBasePool, m_pDoc, m_eFamily);
        }
        else if(m_bIsDescriptor)
        {
            if(!m_pPropertiesImpl->SetProperty(pNames[nProp], pValues[nProp]))
            {
                throw lang::IllegalArgumentException();
            }
        }
        else
        {
            throw uno::RuntimeException();
        }
    }

    if(aBaseImpl.HasItemSet())
    {
        aBaseImpl.getNewBase()->SetItemSet(aBaseImpl.GetItemSet());
    }
}

void SwXStyle::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException,
                lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
                        SwDoc *pDoc) throw(uno::RuntimeException, std::exception)
{
    uno::Any aRet;

    if(FN_UNO_IS_PHYSICAL == rEntry.nWID)
    {
        bool bPhys = pBase != nullptr;
        if(pBase)
        {
            bPhys = static_cast<SwDocStyleSheet*>(pBase)->IsPhysical();
            // The standard character format is not existing physically
            if( bPhys && SFX_STYLE_FAMILY_CHAR == eFamily &&
                static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat() &&
                static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat()->IsDefault() )
                bPhys = false;
        }
        aRet <<= bPhys;
    }
    else if (FN_UNO_HIDDEN == rEntry.nWID)
    {
        bool bHidden = false;
        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xBase( new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)) );
            bHidden = xBase->IsHidden();
        }
        aRet <<= bHidden;
    }
    else if (FN_UNO_STYLE_INTEROP_GRAB_BAG == rEntry.nWID)
    {
        if (pBase)
        {
            rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
            xBase->GetGrabBagItem(aRet);
        }
    }
    else if(pBase)
    {
        if(!rBase.getNewBase().is())
        {
            rBase.setNewBase(new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ));
        }

        //UUUU
        const sal_uInt8 nMemberId(rEntry.nMemberId & (~SFX_METRIC_ITEM));

        //UUUU adapted switch logic to a more readable state; removed goto's and made
        // execution of standard setting of proerty in ItemSet dependent of this variable
        bool bDone(false);

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
                    SfxPrinter *pPrinter = pDoc->getIDocumentDeviceAccess().getPrinter( false );
                    OUString sTmp;
                    if (pPrinter )
                        sTmp = pPrinter->GetPaperBinName ( nBin );
                    aRet <<= sTmp;
                }

                bDone = true;
                break;
            }
            case  FN_UNO_NUM_RULES: // special handling for a SvxNumRuleItem:
            {
                const SwNumRule* pRule = rBase.getNewBase()->GetNumRule();
                OSL_ENSURE(pRule, "Where is the NumRule?");
                uno::Reference< container::XIndexReplace >  xRules = new SwXNumberingRules(*pRule, pDoc);

                aRet.setValue(&xRules, cppu::UnoType<container::XIndexReplace>::get());
                bDone = true;
                break;
            }
            break;
            case RES_PARATR_OUTLINELEVEL:
            {
                OSL_ENSURE( SFX_STYLE_FAMILY_PARA == eFamily, "only paras" );
                int nLevel = rBase.getNewBase()->GetCollection()->GetAttrOutlineLevel();
                aRet <<= static_cast<sal_Int16>( nLevel );
                bDone = true;
                break;
            }
            case FN_UNO_FOLLOW_STYLE:
            {
                OUString aString;
                SwStyleNameMapper::FillProgName(rBase.getNewBase()->GetFollow(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), true);
                aRet <<= aString;
                bDone = true;
                break;
            }
            case RES_PAGEDESC :
            {
                if (MID_PAGEDESC_PAGEDESCNAME != nMemberId)
                    break;
                // special handling for RES_PAGEDESC
                const SfxPoolItem* pItem;
                if (SfxItemState::SET == rBase.GetItemSet().GetItemState(RES_PAGEDESC, true, &pItem))
                {
                    const SwPageDesc* pDesc = static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc();
                    if(pDesc)
                    {
                        OUString aString;
                        SwStyleNameMapper::FillProgName(pDesc->GetName(), aString,  nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
                        aRet <<= aString;
                    }

                }
                bDone = true;
                break;
            }
            case FN_UNO_IS_AUTO_UPDATE:
            {
                bool bAuto = false;
                if(SFX_STYLE_FAMILY_PARA == eFamily)
                    bAuto = rBase.getNewBase()->GetCollection()->IsAutoUpdateFormat();
                else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                    bAuto = rBase.getNewBase()->GetFrameFormat()->IsAutoUpdateFormat();
                aRet <<= bAuto;

                bDone = true;
                break;
            }
            case FN_UNO_DISPLAY_NAME:
            {
                aRet <<= rBase.getNewBase()->GetDisplayName();

                bDone = true;
                break;
            }
            case FN_UNO_PARA_STYLE_CONDITIONS:
            {
                OSL_ENSURE(COND_COMMAND_COUNT == 28,
                        "invalid size of command count?");
                uno::Sequence< beans::NamedValue > aSeq(COND_COMMAND_COUNT);
                beans::NamedValue *pSeq = aSeq.getArray();

                SwFormat *pFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCollection();
                const CommandStruct *pCmds = SwCondCollItem::GetCmds();
                for (sal_uInt16 n = 0;  n < COND_COMMAND_COUNT;  ++n)
                {
                    OUString aStyleName;

                    const SwCollCondition* pCond = nullptr;
                    if( pFormat && RES_CONDTXTFMTCOLL == pFormat->Which() &&
                        nullptr != ( pCond = static_cast<SwConditionTextFormatColl*>(pFormat)->
                        HasCondition( SwCollCondition( nullptr, pCmds[n].nCnd, pCmds[n].nSubCond ) ) )
                        && pCond->GetTextFormatColl() )
                    {
                        // get programmatic style name from UI style name
                        aStyleName = pCond->GetTextFormatColl()->GetName();
                        SwStyleNameMapper::FillProgName(aStyleName, aStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), true);
                    }

                    pSeq[n].Name  = GetCommandContextByIndex(n);
                    pSeq[n].Value <<= aStyleName;
                }
                aRet <<= aSeq;

                bDone = true;
                break;
            }
            case FN_UNO_CATEGORY:
            {
                const sal_uInt16 nPoolId = rBase.getNewBase()->GetCollection()->GetPoolFormatId();
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
                bDone = true;
                break;
            }
            case SID_SWREGISTER_COLLECTION:
            {
                const SwPageDesc *pPageDesc = rBase.getNewBase()->GetPageDesc();
                const SwTextFormatColl* pCol = nullptr;
                OUString aString;
                if( pPageDesc )
                    pCol = pPageDesc->GetRegisterFormatColl();
                if( pCol )
                    SwStyleNameMapper::FillProgName(
                                pCol->GetName(), aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
                aRet <<= aString;
                bDone = true;
                break;
            }
            case RES_BACKGROUND:
            {
                //UUUU
                const SfxItemSet& rSet = rBase.GetItemSet();
                const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));

                if(!aOriginalBrushItem.QueryValue(aRet, nMemberId))
                {
                    OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
                }

                bDone = true;
                break;
            }
            case OWN_ATTR_FILLBMP_MODE:
            {
                //UUUU
                const SfxItemSet& rSet = rBase.GetItemSet();
                const XFillBmpStretchItem* pStretchItem = dynamic_cast< const XFillBmpStretchItem* >(&rSet.Get(XATTR_FILLBMP_STRETCH));
                const XFillBmpTileItem* pTileItem = dynamic_cast< const XFillBmpTileItem* >(&rSet.Get(XATTR_FILLBMP_TILE));

                if( pTileItem && pTileItem->GetValue() )
                {
                    aRet <<= drawing::BitmapMode_REPEAT;
                }
                else if( pStretchItem && pStretchItem->GetValue() )
                {
                    aRet <<= drawing::BitmapMode_STRETCH;
                }
                else
                {
                    aRet <<= drawing::BitmapMode_NO_REPEAT;
                }

                bDone = true;
                break;
            }
            default:
            {
                // nothing to do as default
                break;
            }
        }

        if(!bDone)
        {
            SfxItemSet& rSet = rBase.GetItemSet();
            rPropSet.getPropertyValue(rEntry, rSet, aRet);

            //UUUU
            if(rEntry.aType == ::cppu::UnoType<sal_Int16>::get() && rEntry.aType != aRet.getValueType())
            {
                // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                sal_Int32 nValue = 0;
                if (aRet >>= nValue)
                    aRet <<= (sal_Int16)nValue;
            }

            //UUUU check for needed metric translation
            if(rEntry.nMemberId & SFX_METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aRet >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt && pDoc)
                {
                    const SfxItemPool& rPool = pDoc->GetAttrPool();
                    const SfxMapUnit eMapUnit(rPool.GetMetric(rEntry.nWID));

                    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
                    {
                        SvxUnoConvertToMM(eMapUnit, aRet);
                    }
                }
            }
        }
    }
    else
    {
        throw uno::RuntimeException();
    }

    return aRet;
}

uno::Sequence< uno::Any > SAL_CALL SwXStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    if ( !m_pDoc )
        throw uno::RuntimeException();

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;

    switch(m_eFamily)
    {
        case SFX_STYLE_FAMILY_PARA  : nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
        case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE   ;break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    const OUString* pNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aRet(rPropertyNames.getLength());
    uno::Any* pRet = aRet.getArray();
    SwStyleBase_Impl aBase(*m_pDoc, m_sStyleName, &GetDoc()->GetDfltTextFormatColl()->GetAttrSet()); //UUUU add pDfltTextFormatColl as parent
    SfxStyleSheetBase* pBase = nullptr;

    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName( pNames[nProp]);
        if(!pEntry ||
           (!m_bIsConditional && pNames[nProp] == UNO_NAME_PARA_STYLE_CONDITIONS))
            throw beans::UnknownPropertyException("Unknown property: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

        if(m_pBasePool)
        {
            if(!pBase)
            {
                const sal_uInt16 nSaveMask = m_pBasePool->GetSearchMask();
                m_pBasePool->SetSearchMask(m_eFamily);
                pBase = m_pBasePool->Find(m_sStyleName);
                m_pBasePool->SetSearchMask(m_eFamily, nSaveMask );
            }

            pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, m_eFamily, GetDoc() );
        }
        else if(m_bIsDescriptor)
        {
            uno::Any *pAny = nullptr;
            m_pPropertiesImpl->GetProperty ( pNames[nProp], pAny );
            if( !pAny )
            {
                bool bExcept = false;
                switch( m_eFamily )
                {
                    case SFX_STYLE_FAMILY_PSEUDO:
                        bExcept = true;
                    break;
                    case SFX_STYLE_FAMILY_PARA:
                    case SFX_STYLE_FAMILY_PAGE:
                        SwStyleProperties_Impl::GetProperty ( pNames[nProp], mxStyleData, pRet[ nProp ] );
                    break;
                    case SFX_STYLE_FAMILY_CHAR:
                    case SFX_STYLE_FAMILY_FRAME :
                    {
                        if (pEntry->nWID >= POOLATTR_BEGIN && pEntry->nWID < RES_UNKNOWNATR_END )
                        {
                            SwFormat * pFormat;
                            if ( m_eFamily == SFX_STYLE_FAMILY_CHAR )
                                pFormat = m_pDoc->GetDfltCharFormat();
                            else
                                pFormat = m_pDoc->GetDfltFrameFormat();
                            const SwAttrPool * pPool = pFormat->GetAttrSet().GetPool();
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
                    aExcept.Message = "No default value for: " + pNames[nProp];
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
    const uno::Sequence< OUString >& rPropertyNames ) throw(uno::RuntimeException, std::exception)
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
        throw(uno::RuntimeException, std::exception)
{
}

void SwXStyle::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException, std::exception)
{
}

void SwXStyle::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException, std::exception)
{
}

void SwXStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException,
        lang::IllegalArgumentException,
        lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);

    SetPropertyValues_Impl( aProperties, aValues );
}

uno::Any SwXStyle::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];

}

void SwXStyle::addPropertyChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXStyle::removePropertyChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXStyle::addVetoableChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXStyle::removeVetoableChangeListener(const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXStyle::getPropertyState(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXStyle::getPropertyStates(
    const uno::Sequence< OUString >& rPropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();

    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily );
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "where is the style?" );

        if(pBase)
        {
            const OUString* pNames = rPropertyNames.getConstArray();
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
            sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;

            switch(m_eFamily)
            {
                case SFX_STYLE_FAMILY_PARA  : nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
                case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE ;break;
                case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE;   break;
                case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE   ;break;
                default: ;
            }

            const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
            const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
            const SfxItemSet& rSet = xStyle->GetItemSet();

            for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
            {
                const OUString sPropName = pNames[i];
                const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
                bool bDone(false);

                if(!pEntry)
                {
                    throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
                }

                if( FN_UNO_NUM_RULES == pEntry->nWID || FN_UNO_FOLLOW_STYLE == pEntry->nWID )
                {
                    // handle NumRules first, done
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    bDone = true;
                }

                // allow to retarget the SfxItemSet working on, default correctly. Only
                // use pSourceSet below this point (except in header/footer processing)
                const SfxItemSet* pSourceSet = &rSet;

                if(!bDone)
                {
                    // check for Header/Footer entry
                    const bool bHeader(SFX_STYLE_FAMILY_PAGE == m_eFamily && sPropName.startsWith("Header"));
                    const bool bFooter(SFX_STYLE_FAMILY_PAGE == m_eFamily && sPropName.startsWith("Footer"));

                    if(bHeader || bFooter || sPropName == UNO_NAME_FIRST_IS_SHARED)
                    {
                        const SvxSetItem* pSetItem;

                        if (lcl_GetHeaderFooterItem(rSet, sPropName, bFooter, pSetItem))
                        {
                            // retarget the SfxItemSet to the HeaderFooter SfxSetItem's SfxItenSet
                            pSourceSet = &pSetItem->GetItemSet();
                        }
                        else
                        {
                            // if no SetItem, value is ambigous and we are done
                            pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                            bDone = true;
                        }
                    }
                }

                if(!bDone && OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
                {
                    //UUUU
                    if(SfxItemState::SET == pSourceSet->GetItemState(XATTR_FILLBMP_STRETCH, false)
                        || SfxItemState::SET == pSourceSet->GetItemState(XATTR_FILLBMP_TILE, false))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                    }

                    bDone = true;
                }

                //UUUU for FlyFrames we need to mark the used properties from type RES_BACKGROUND
                // as beans::PropertyState_DIRECT_VALUE to let users of this property call
                // getPropertyValue where the member properties will be mapped from the
                // fill attributes to the according SvxBrushItem entries
                if (!bDone && RES_BACKGROUND == pEntry->nWID)
                {
                    if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(*pSourceSet, pEntry->nMemberId))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                    bDone = true;
                }

                if(!bDone)
                {
                    pStates[i] = pPropSet->getPropertyState(*pEntry, *pSourceSet);

                    if(SFX_STYLE_FAMILY_PAGE == m_eFamily && SID_ATTR_PAGE_SIZE == pEntry->nWID && beans::PropertyState_DIRECT_VALUE == pStates[i])
                    {
                        const SvxSizeItem& rSize = static_cast <const SvxSizeItem&>( rSet.Get(SID_ATTR_PAGE_SIZE));
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
        {
            throw uno::RuntimeException();
        }
    }
    else
    {
        throw uno::RuntimeException();
    }

    return aRet;
}

void SwXStyle::setPropertyToDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, uno::RuntimeException, std::exception )
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    setPropertiesToDefault ( aSequence );
}

void SAL_CALL SwXStyle::setPropertiesToDefault( const uno::Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFormat *pTargetFormat = nullptr;

    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "Where is the style?");

        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
            switch(m_eFamily)
            {
                case SFX_STYLE_FAMILY_CHAR:
                    pTargetFormat = xStyle->GetCharFormat();
                    break;

                case SFX_STYLE_FAMILY_PARA:
                    pTargetFormat = xStyle->GetCollection();
                    break;

                case SFX_STYLE_FAMILY_FRAME:
                    pTargetFormat = xStyle->GetFrameFormat();
                    break;

                case SFX_STYLE_FAMILY_PAGE:
                {
                    SwPageDesc *pDesc = m_pDoc->FindPageDesc(xStyle->GetPageDesc()->GetName());
                    if( pDesc )
                        pTargetFormat = &pDesc->GetMaster();
                    break;
                }
                case SFX_STYLE_FAMILY_PSEUDO:
                    break;
                default: ;
            }
        }
    }

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;

    switch(m_eFamily)
    {
        case SFX_STYLE_FAMILY_PARA  : nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
        case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE; break;
        case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE; break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE; break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    const OUString* pNames = aPropertyNames.getConstArray();

    if(pTargetFormat)
    {
        for(sal_Int32 nProp = 0, nEnd = aPropertyNames.getLength(); nProp < nEnd; nProp++)
        {
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pNames[nProp]);

            if( !pEntry )
            {
                throw beans::UnknownPropertyException ( "Unknown property: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            }

            if ( pEntry->nWID == FN_UNO_FOLLOW_STYLE || pEntry->nWID == FN_UNO_NUM_RULES )
            {
                throw uno::RuntimeException ("Cannot reset: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            }

            if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
            {
                throw uno::RuntimeException("setPropertiesToDefault: property is read-only: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            }

            if( pEntry->nWID == RES_PARATR_OUTLINELEVEL )
            {
                static_cast<SwTextFormatColl*>(pTargetFormat)->DeleteAssignmentToListLevelOfOutlineStyle();
            }
            else
            {
                pTargetFormat->ResetFormatAttr( pEntry->nWID );
            }

            if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                //UUUU
                SwDoc* pDoc = pTargetFormat->GetDoc();
                SfxItemSet aSet(pDoc->GetAttrPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);
                aSet.SetParent(&pTargetFormat->GetAttrSet());

                aSet.ClearItem(XATTR_FILLBMP_STRETCH);
                aSet.ClearItem(XATTR_FILLBMP_TILE);

                pTargetFormat->SetFormatAttr(aSet);
            }
        }
    }
    else if(m_bIsDescriptor)
    {
        for(sal_Int32 nProp = 0, nEnd = aPropertyNames.getLength(); nProp < nEnd; nProp++)
        {
            m_pPropertiesImpl->ClearProperty(pNames[nProp]);
        }
    }
}

void SAL_CALL SwXStyle::setAllPropertiesToDefault(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(m_pBasePool)
    {
        m_pBasePool->SetSearchMask(m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        OSL_ENSURE(pBase, "Where is the style?");

        if(pBase)
        {
            rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
            SwFormat *pTargetFormat = nullptr;
            size_t nPgDscPos = SIZE_MAX;
            switch(m_eFamily)
            {
                case SFX_STYLE_FAMILY_CHAR:
                    pTargetFormat = xStyle->GetCharFormat();
                    break;

                case SFX_STYLE_FAMILY_PARA:
                {
                    pTargetFormat = xStyle->GetCollection();
                    if(xStyle->GetCollection())
                    {
                        xStyle->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
                    }
                    break;
                }

                case SFX_STYLE_FAMILY_FRAME:
                    pTargetFormat = xStyle->GetFrameFormat();
                    break;

                case SFX_STYLE_FAMILY_PAGE:
                {
                    SwPageDesc *pDesc = m_pDoc->FindPageDesc(xStyle->GetPageDesc()->GetName(), &nPgDscPos);
                    if( pDesc )
                    {
                        pTargetFormat = &pDesc->GetMaster();
                        pDesc->SetUseOn(nsUseOnPage::PD_ALL);
                    }
                    break;
                }

                case SFX_STYLE_FAMILY_PSEUDO:
                    break;

                default: ;
            }

            if(pTargetFormat)
            {
                if(SIZE_MAX != nPgDscPos)
                {
                    SwPageDesc& rPageDesc = m_pDoc->GetPageDesc(nPgDscPos);
                    rPageDesc.ResetAllMasterAttr();

                    SvxLRSpaceItem aLR(RES_LR_SPACE);
                    sal_Int32 nSize = GetMetricVal(CM_1) * 2;
                    aLR.SetLeft(nSize);
                    aLR.SetLeft(nSize);
                    SvxULSpaceItem aUL(RES_UL_SPACE);
                    aUL.SetUpper(static_cast <sal_uInt16> (nSize));
                    aUL.SetLower(static_cast <sal_uInt16> (nSize));
                    pTargetFormat->SetFormatAttr(aLR);
                    pTargetFormat->SetFormatAttr(aUL);
                    SwPageDesc* pStdPgDsc = m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD);
                    SwFormatFrameSize aFrameSz(ATT_FIX_SIZE);

                    if(RES_POOLPAGE_STANDARD == rPageDesc.GetPoolFormatId())
                    {
                        if(m_pDoc->getIDocumentDeviceAccess().getPrinter(false))
                        {
                            const Size aPhysSize( SvxPaperInfo::GetPaperSize(
                                static_cast<Printer*>(m_pDoc->getIDocumentDeviceAccess().getPrinter(false))));
                            aFrameSz.SetSize(aPhysSize);
                        }
                        else
                        {
                            aFrameSz.SetSize(SvxPaperInfo::GetDefaultPaperSize());
                        }

                    }
                    else
                    {
                        aFrameSz = pStdPgDsc->GetMaster().GetFrameSize();
                    }

                    if(pStdPgDsc->GetLandscape())
                    {
                        SwTwips nTmp = aFrameSz.GetHeight();
                        aFrameSz.SetHeight(aFrameSz.GetWidth());
                        aFrameSz.SetWidth(nTmp);
                    }

                    pTargetFormat->SetFormatAttr( aFrameSz );
                }
                else
                {
                    pTargetFormat->ResetAllFormatAttr();
                }

                if(SIZE_MAX != nPgDscPos)
                {
                    m_pDoc->ChgPageDesc(nPgDscPos, m_pDoc->GetPageDesc(nPgDscPos));
                }
            }
        }
        else
        {
            throw uno::RuntimeException();
        }
    }
    else if ( m_bIsDescriptor )
    {
        m_pPropertiesImpl->ClearAllProperties();
    }
    else
    {
        throw uno::RuntimeException();
    }
}

uno::Sequence< uno::Any > SAL_CALL SwXStyle::getPropertyDefaults( const uno::Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = aPropertyNames.getLength();

    uno::Sequence < uno::Any > aRet(nCount);

    if(nCount)
    {
        if(m_pBasePool)
        {
            m_pBasePool->SetSearchMask(m_eFamily);
            SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
            OSL_ENSURE(pBase, "Doesn't seem to be a style!");

            if(pBase)
            {
                rtl::Reference< SwDocStyleSheet > xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
                sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_STYLE;
                switch(m_eFamily)
                {
                    case SFX_STYLE_FAMILY_PARA  : nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : PROPERTY_MAP_PARA_STYLE; break;
                    case SFX_STYLE_FAMILY_FRAME : nPropSetId = PROPERTY_MAP_FRAME_STYLE; break;
                    case SFX_STYLE_FAMILY_PAGE  : nPropSetId = PROPERTY_MAP_PAGE_STYLE; break;
                    case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_MAP_NUM_STYLE; break;
                    default: ;
                }
                const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
                const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

                const SfxItemSet &rSet = xStyle->GetItemSet(), *pParentSet = rSet.GetParent();
                const OUString *pNames = aPropertyNames.getConstArray();
                uno::Any *pRet = aRet.getArray();

                for(sal_Int32 i = 0; i < nCount; i++)
                {
                    const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pNames[i]);

                    if(!pEntry)
                    {
                        throw beans::UnknownPropertyException("Unknown property: " + pNames[i], static_cast < cppu::OWeakObject * >(this));
                    }

                    if (pEntry->nWID >= RES_UNKNOWNATR_END)
                    {
                        // these cannot be in an item set, especially not the
                        // parent set, so the default value is void
                        continue;
                    }

                    if(pParentSet)
                    {
                        aSwMapProvider.GetPropertySet(nPropSetId)->getPropertyValue(pNames[i], *pParentSet, pRet[i]);
                    }
                    else if(pEntry->nWID != rSet.GetPool()->GetSlotId(pEntry->nWID))
                    {
                        const SfxPoolItem& rItem = rSet.GetPool()->GetDefaultItem(pEntry->nWID);

                        rItem.QueryValue(pRet[i], pEntry->nMemberId);
                    }
                }
            }
            else
            {
                throw uno::RuntimeException();
            }
        }
        else
        {
            throw uno::RuntimeException();
        }
    }
    return aRet;
}

uno::Any SwXStyle::getPropertyDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

void SwXStyle::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxSimpleHint* pHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if( pHint )
    {
        if(( pHint->GetId() & SFX_HINT_DYING ) || ( pHint->GetId() & SfxStyleSheetHintId::ERASED))
        {
            m_pBasePool = nullptr;
            EndListening(rBC);
        }
        else if( pHint->GetId() &(SfxStyleSheetHintId::CHANGED|SfxStyleSheetHintId::ERASED) )
        {
            static_cast<SfxStyleSheetBasePool&>(rBC).SetSearchMask(m_eFamily);
            SfxStyleSheetBase* pOwnBase = static_cast<SfxStyleSheetBasePool&>(rBC).Find(m_sStyleName);
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
    m_sStyleName.clear();
    m_pBasePool = nullptr;
    m_pDoc = nullptr;
    mxStyleData.clear();
    mxStyleFamily.clear();
}

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

static void lcl_putItemToSet(const SvxSetItem* pSetItem, const SfxItemPropertySet& rPropSet, const SfxItemPropertySimpleEntry& rEntry, const uno::Any& rVal, SwStyleBase_Impl& rBaseImpl, SfxStyleSheetBasePool* pPool, SwDoc *pDoc, SfxStyleFamily eFamily)
{
    // create a new SvxSetItem and get it's ItemSet as new target
    SvxSetItem* pNewSetItem = static_cast< SvxSetItem* >(pSetItem->Clone());
    SfxItemSet& rSetSet = pNewSetItem->GetItemSet();

    // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
    rSetSet.SetParent(&pDoc->GetDfltFrameFormat()->GetAttrSet());

    // replace the used SfxItemSet at the SwStyleBase_Impl temporarily and use the
    // default method to set the property
    SfxItemSet* pRememberItemSet = rBaseImpl.replaceItemSet(&rSetSet);
    lcl_SetStyleProperty(rEntry, rPropSet, rVal, rBaseImpl, pPool, pDoc, eFamily);
    rBaseImpl.replaceItemSet(pRememberItemSet);

    // reset paret at ItemSet from SetItem
    rSetSet.SetParent(nullptr);

    // set the new SvxSetItem at the real target and delete it
    rBaseImpl.GetItemSet().Put(*pNewSetItem);
    delete pNewSetItem;
}

void SAL_CALL SwXPageStyle::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
           lang::IllegalArgumentException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    if(!GetDoc())
        throw uno::RuntimeException();

    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBaseImpl(*GetDoc(), GetStyleName(), &GetDoc()->GetDfltFrameFormat()->GetAttrSet()); //UUUU add pDfltFrameFormat as parent
    if(GetBasePool())
    {
        const sal_uInt16 nSaveMask = GetBasePool()->GetSearchMask();
        GetBasePool()->SetSearchMask(GetFamily());
        SfxStyleSheetBase* pBase = GetBasePool()->Find(GetStyleName());
        GetBasePool()->SetSearchMask(GetFamily(), nSaveMask );
        OSL_ENSURE(pBase, "where is the style?" );
        if(pBase)
        {
            aBaseImpl.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
        }
        else
        {
            throw uno::RuntimeException();
        }
    }

    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        const OUString& rPropName = pNames[nProp];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropName);

        if (!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }

        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
        {
            throw beans::PropertyVetoException ("Property is read-only: " + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }

        if(GetBasePool())
        {
            switch(pEntry->nWID)
            {
                case SID_ATTR_PAGE_ON:
                case RES_BACKGROUND:
                case RES_BOX:
                case RES_LR_SPACE:
                case RES_SHADOW:
                case RES_UL_SPACE:
                case SID_ATTR_PAGE_DYNAMIC:
                case SID_ATTR_PAGE_SHARED:
                case SID_ATTR_PAGE_SHARED_FIRST:
                case SID_ATTR_PAGE_SIZE:
                case RES_HEADER_FOOTER_EAT_SPACING:
                {
                    // these entries are used in Header, Footer and (partially) in the PageStyle itself.
                    // Check for Header/Footer entry
                    const bool bHeader(rPropName.startsWith("Header"));
                    const bool bFooter(rPropName.startsWith("Footer"));

                    if(bHeader || bFooter || rPropName == UNO_NAME_FIRST_IS_SHARED)
                    {
                        // it is a Header/Footer entry, access the SvxSetItem containing it's information
                        const SvxSetItem* pSetItem = nullptr;
                        if (lcl_GetHeaderFooterItem(aBaseImpl.GetItemSet(),
                                    rPropName, bFooter, pSetItem))
                        {
                            lcl_putItemToSet(pSetItem, *pPropSet, *pEntry, pValues[nProp], aBaseImpl, GetBasePool(), GetDoc(), GetFamily());

                            if (pEntry->nWID == SID_ATTR_PAGE_SHARED_FIRST)
                            {
                                // Need to add this to the other as well
                                if (SfxItemState::SET == aBaseImpl.GetItemSet().GetItemState(
                                            bFooter ? SID_ATTR_PAGE_HEADERSET : SID_ATTR_PAGE_FOOTERSET,
                                            false, reinterpret_cast<const SfxPoolItem**>(&pSetItem)))
                                {
                                    lcl_putItemToSet(pSetItem, *pPropSet, *pEntry, pValues[nProp], aBaseImpl, GetBasePool(), GetDoc(), GetFamily());
                                }
                            }
                        }
                        else if(pEntry->nWID == SID_ATTR_PAGE_ON)
                        {
                            bool bVal = *static_cast<sal_Bool const *>(pValues[nProp].getValue());

                            if(bVal)
                            {
                                // Header/footer gets switched on, create defaults and the needed SfxSetItem
                                SfxItemSet aTempSet(*aBaseImpl.GetItemSet().GetPool(),
                                    RES_FRMATR_BEGIN,RES_FRMATR_END - 1,            // [82

                                    //UUUU FillAttribute support
                                    XATTR_FILL_FIRST, XATTR_FILL_LAST,              // [1014

                                    SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,    // [10023
                                    SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,          // [10051
                                    SID_ATTR_PAGE_ON,SID_ATTR_PAGE_SHARED,          // [10060
                                    SID_ATTR_PAGE_SHARED_FIRST,SID_ATTR_PAGE_SHARED_FIRST,
                                    0);

                                //UUUU set correct parent to get the XFILL_NONE FillStyle as needed
                                aTempSet.SetParent(&GetDoc()->GetDfltFrameFormat()->GetAttrSet());

                                aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_ON, true));
                                aTempSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(MM50, MM50)));
                                aTempSet.Put(SvxLRSpaceItem(RES_LR_SPACE));
                                aTempSet.Put(SvxULSpaceItem(RES_UL_SPACE));
                                aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED, true));
                                aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED_FIRST, true));
                                aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_DYNAMIC, true));

                                SvxSetItem aNewSetItem(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, aTempSet);
                                aBaseImpl.GetItemSet().Put(aNewSetItem);
                            }
                        }
                    }
                    else
                    {
                        switch(pEntry->nWID)
                        {
                            case SID_ATTR_PAGE_DYNAMIC:
                            case SID_ATTR_PAGE_SHARED:
                            case SID_ATTR_PAGE_SHARED_FIRST:
                            case SID_ATTR_PAGE_ON:
                            case RES_HEADER_FOOTER_EAT_SPACING:
                            {
                                // these slots are exclusive to Header/Footer, thus this is an error
                                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropName, static_cast < cppu::OWeakObject * > ( this ) );
                            }
                            default:
                            {
                                // part of PageStyle, fallback to default
                                lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl, GetBasePool(), GetDoc(), GetFamily());
                            }
                        }
                    }
                    break;
                }

                case XATTR_FILLBMP_SIZELOG:
                case XATTR_FILLBMP_TILEOFFSETX:
                case XATTR_FILLBMP_TILEOFFSETY:
                case XATTR_FILLBMP_POSOFFSETX:
                case XATTR_FILLBMP_POSOFFSETY:
                case XATTR_FILLBMP_POS:
                case XATTR_FILLBMP_SIZEX:
                case XATTR_FILLBMP_SIZEY:
                case XATTR_FILLBMP_STRETCH:
                case XATTR_FILLBMP_TILE:
                case OWN_ATTR_FILLBMP_MODE:
                case XATTR_FILLCOLOR:
                case XATTR_FILLBACKGROUND:
                case XATTR_FILLBITMAP:
                case XATTR_GRADIENTSTEPCOUNT:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLSTYLE:
                case XATTR_FILLTRANSPARENCE:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_SECONDARYFILLCOLOR:
                {
                    // This DrawingLayer FillStyle attributes can be part of Header, Footer and PageStyle
                    // itself, so decide what to do using the name
                    const bool bHeader(rPropName.startsWith("Header"));
                    const bool bFooter(rPropName.startsWith("Footer"));

                    if (bHeader || bFooter)
                    {
                        const SvxSetItem* pSetItem = nullptr;

                        if(SfxItemState::SET == aBaseImpl.GetItemSet().GetItemState(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, false, reinterpret_cast<const SfxPoolItem**>(&pSetItem)))
                        {
                            // create a new SvxSetItem and get it's ItemSet as new target
                            SvxSetItem* pNewSetItem = static_cast< SvxSetItem* >(pSetItem->Clone());
                            SfxItemSet& rSetSet = pNewSetItem->GetItemSet();

                            // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
                            rSetSet.SetParent(&GetDoc()->GetDfltFrameFormat()->GetAttrSet());

                            // replace the used SfxItemSet at the SwStyleBase_Impl temporarily and use the
                            // default method to set the property
                            SfxItemSet* pRememberItemSet = aBaseImpl.replaceItemSet(&rSetSet);
                            lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl, GetBasePool(), GetDoc(), GetFamily());
                            aBaseImpl.replaceItemSet(pRememberItemSet);

                            // reset paret at ItemSet from SetItem
                            rSetSet.SetParent(nullptr);

                            // set the new SvxSetItem at the real target and delete it
                            aBaseImpl.GetItemSet().Put(*pNewSetItem);
                            delete pNewSetItem;
                        }
                    }
                    else
                    {
                        // part of PageStyle, fallback to default
                        lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl, GetBasePool(), GetDoc(), GetFamily());
                    }

                    break;
                }
                case FN_PARAM_FTN_INFO :
                {
                    const SfxPoolItem& rItem = aBaseImpl.GetItemSet().Get(FN_PARAM_FTN_INFO);
                    SfxPoolItem* pNewFootnoteItem = rItem.Clone();
                    bool bPut = pNewFootnoteItem->PutValue(pValues[nProp], pEntry->nMemberId);
                    aBaseImpl.GetItemSet().Put(*pNewFootnoteItem);
                    delete pNewFootnoteItem;
                    if(!bPut)
                        throw lang::IllegalArgumentException();
                    break;
                }
                case  FN_UNO_HEADER       :
                case  FN_UNO_HEADER_LEFT  :
                case  FN_UNO_HEADER_RIGHT :
                case  FN_UNO_HEADER_FIRST :
                case  FN_UNO_FOOTER       :
                case  FN_UNO_FOOTER_LEFT  :
                case  FN_UNO_FOOTER_RIGHT :
                case  FN_UNO_FOOTER_FIRST :
                {
                    throw lang::IllegalArgumentException();
                }
                default:
                {
                    //UUUU
                    lcl_SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl, GetBasePool(), GetDoc(), GetFamily());
                    break;
                }
            }
        }
        else if(IsDescriptor())
        {
            if(!GetPropImpl()->SetProperty(rPropName, pValues[nProp]))
                throw lang::IllegalArgumentException();
        }
        else
        {
            throw uno::RuntimeException();
        }
    }

    if(aBaseImpl.HasItemSet())
    {
        ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

        if (undoGuard.UndoWasEnabled())
        {
            // Fix i64460: as long as Undo of page styles with header/footer causes trouble...
            GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
        }

        aBaseImpl.getNewBase()->SetItemSet(aBaseImpl.GetItemSet());
    }
}

void SwXPageStyle::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
    const sal_uInt16 nRes, const bool bHeader, SwFrameFormat const*const pFrameFormat)
{
    if (!pFrameFormat) { return nullptr; }

    const SfxItemSet& rSet = pFrameFormat->GetAttrSet();
    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rSet.GetItemState(nRes, true, &pItem))
    {
        SwFrameFormat *const pHeadFootFormat = (bHeader)
            ? static_cast<SwFormatHeader*>(const_cast<SfxPoolItem*>(pItem))->
                    GetHeaderFormat()
            : static_cast<SwFormatFooter*>(const_cast<SfxPoolItem*>(pItem))->
                    GetFooterFormat();
        if (pHeadFootFormat)
        {
            return SwXHeadFootText::CreateXHeadFootText(*pHeadFootFormat, bHeader);
        }
    }
    return nullptr;
}

uno::Sequence< uno::Any > SAL_CALL SwXPageStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString >& rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    if(!GetDoc())
        throw uno::RuntimeException();

    sal_Int32 nLength = rPropertyNames.getLength();
    const OUString* pNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aRet ( nLength );

    uno::Any* pRet = aRet.getArray();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBase(*GetDoc(), GetStyleName(), &GetDoc()->GetDfltFrameFormat()->GetAttrSet()); //UUUU add pDfltFrameFormat as parent
    SfxStyleSheetBase* pBase = nullptr;

    for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
    {
        const OUString& rPropName = pNames[nProp];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropName);

        if (!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + rPropName, static_cast < cppu::OWeakObject * > ( this ) );
        }

        if(GetBasePool())
        {
            if(!pBase)
            {
                const sal_uInt16 nSaveMask = GetBasePool()->GetSearchMask();
                GetBasePool()->SetSearchMask(GetFamily());
                pBase = GetBasePool()->Find(GetStyleName());
                GetBasePool()->SetSearchMask(GetFamily(), nSaveMask );
            }

            sal_uInt16 nRes = 0;
            const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));

            switch(pEntry->nWID)
            {
                case SID_ATTR_PAGE_ON:
                case RES_BACKGROUND:
                case RES_BOX:
                case RES_LR_SPACE:
                case RES_SHADOW:
                case RES_UL_SPACE:
                case SID_ATTR_PAGE_DYNAMIC:
                case SID_ATTR_PAGE_SHARED:
                case SID_ATTR_PAGE_SHARED_FIRST:
                case SID_ATTR_PAGE_SIZE:
                case RES_HEADER_FOOTER_EAT_SPACING:
                {
                    // These slots are used for Header, Footer and (partially) for PageStyle directly.
                    // Check for Header/Footer entry
                    const bool bHeader(rPropName.startsWith("Header"));
                    const bool bFooter(rPropName.startsWith("Footer"));

                    if(bHeader || bFooter || rPropName == UNO_NAME_FIRST_IS_SHARED)
                    {
                        // slot is a Header/Footer slot
                        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                        const SfxItemSet& rSet = xStyle->GetItemSet();
                        const SvxSetItem* pSetItem;

                        if (lcl_GetHeaderFooterItem(rSet, rPropName, bFooter, pSetItem))
                        {
                            // get from SfxItemSet of the corresponding SfxSetItem
                            const SfxItemSet& rSetSet = pSetItem->GetItemSet();
                            SfxItemSet* pRememberItemSet = aBase.replaceItemSet(&const_cast< SfxItemSet& >(rSetSet));
                            pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, GetFamily(), GetDoc() );
                            aBase.replaceItemSet(pRememberItemSet);
                        }
                        else if(pEntry->nWID == SID_ATTR_PAGE_ON)
                        {
                            // header/footer is not available, thus off. Default is <false>, though
                            pRet[nProp] <<= false;
                        }
                    }
                    else
                    {
                        switch(pEntry->nWID)
                        {
                            case SID_ATTR_PAGE_DYNAMIC:
                            case SID_ATTR_PAGE_SHARED:
                            case SID_ATTR_PAGE_SHARED_FIRST:
                            case SID_ATTR_PAGE_ON:
                            case RES_HEADER_FOOTER_EAT_SPACING:
                            {
                                // these slots are exclusive to Header/Footer, thus this is an error
                                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropName, static_cast < cppu::OWeakObject * > ( this ) );
                            }
                            default:
                            {
                                // part of PageStyle, fallback to default
                                pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, GetFamily(), GetDoc() );
                            }
                        }
                    }

                    break;
                }

                case XATTR_FILLBMP_SIZELOG:
                case XATTR_FILLBMP_TILEOFFSETX:
                case XATTR_FILLBMP_TILEOFFSETY:
                case XATTR_FILLBMP_POSOFFSETX:
                case XATTR_FILLBMP_POSOFFSETY:
                case XATTR_FILLBMP_POS:
                case XATTR_FILLBMP_SIZEX:
                case XATTR_FILLBMP_SIZEY:
                case XATTR_FILLBMP_STRETCH:
                case XATTR_FILLBMP_TILE:
                case OWN_ATTR_FILLBMP_MODE:
                case XATTR_FILLCOLOR:
                case XATTR_FILLBACKGROUND:
                case XATTR_FILLBITMAP:
                case XATTR_GRADIENTSTEPCOUNT:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLSTYLE:
                case XATTR_FILLTRANSPARENCE:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_SECONDARYFILLCOLOR:
                {
                    // This DrawingLayer FillStyle attributes can be part of Header, Footer and PageStyle
                    // itself, so decide what to do using the name
                    const bool bHeader(rPropName.startsWith("Header"));
                    const bool bFooter(rPropName.startsWith("Footer"));

                    if (bHeader || bFooter)
                    {
                        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                        const SfxItemSet& rSet = xStyle->GetItemSet();
                        const SvxSetItem* pSetItem;
                        if(SfxItemState::SET == rSet.GetItemState(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, false, reinterpret_cast<const SfxPoolItem**>(&pSetItem)))
                        {
                            // set at SfxItemSet of the corresponding SfxSetItem
                            const SfxItemSet& rSetSet = pSetItem->GetItemSet();
                            SfxItemSet* pRememberItemSet = aBase.replaceItemSet(&const_cast< SfxItemSet& >(rSetSet));
                            pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, GetFamily(), GetDoc() );
                            aBase.replaceItemSet(pRememberItemSet);
                        }
                    }
                    else
                    {
                        // part of PageStyle, fallback to default
                        pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, GetFamily(), GetDoc() );
                    }

                    break;
                }

                case FN_UNO_HEADER:
                case FN_UNO_HEADER_LEFT:
                case FN_UNO_HEADER_FIRST:
                case FN_UNO_HEADER_RIGHT:
                case FN_UNO_FOOTER:
                case FN_UNO_FOOTER_LEFT:
                case FN_UNO_FOOTER_FIRST:
                case FN_UNO_FOOTER_RIGHT:
                {
                    //UUUU cleanups for readability (undos removed, rearranged)
                    bool bHeader(false);
                    bool bLeft(false);
                    bool bFirst(false);

                    switch(pEntry->nWID)
                    {
                        case FN_UNO_HEADER:       bHeader = true;  nRes = RES_HEADER; break;
                        case FN_UNO_HEADER_LEFT:  bHeader = true;  nRes = RES_HEADER; bLeft = true;  break;
                        case FN_UNO_HEADER_FIRST: bHeader = true;  nRes = RES_HEADER; bFirst = true; break;
                        case FN_UNO_HEADER_RIGHT: bHeader = true;  nRes = RES_HEADER; break;
                        case FN_UNO_FOOTER:       bHeader = false; nRes = RES_FOOTER; break;
                        case FN_UNO_FOOTER_LEFT:  bHeader = false; nRes = RES_FOOTER; bLeft = true;  break;
                        case FN_UNO_FOOTER_FIRST: bHeader = false; nRes = RES_FOOTER; bFirst = true; break;
                        case FN_UNO_FOOTER_RIGHT: bHeader = false; nRes = RES_FOOTER; break;
                        default: break;
                    }

                    const SwPageDesc* pDesc = aBase.GetOldPageDesc();
                    assert(pDesc);
                    const SwFrameFormat* pFrameFormat = nullptr;
                    bool bShare = (bHeader && pDesc->IsHeaderShared()) || (!bHeader && pDesc->IsFooterShared());
                    bool bShareFirst = pDesc->IsFirstShared();
                    // TextLeft returns the left content if there is one,
                    // Text and TextRight return the master content.
                    // TextRight does the same as Text and is for
                    // compatability only.
                    if( bLeft && !bShare )
                    {
                        pFrameFormat = &pDesc->GetLeft();
                    }
                    else if (bFirst && !bShareFirst)
                    {
                        pFrameFormat = &pDesc->GetFirstMaster();
                        // no need to make GetFirstLeft() accessible
                        // since it is always shared
                    }
                    else
                    {
                        pFrameFormat = &pDesc->GetMaster();
                    }
                    const uno::Reference< text::XText > xRet =
                        lcl_makeHeaderFooter(nRes, bHeader, pFrameFormat);
                    if (xRet.is())
                    {
                        pRet[nProp] <<= xRet;
                    }
                    break;
                }

                case FN_PARAM_FTN_INFO :
                {
                    rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                    const SfxItemSet& rSet = xStyle->GetItemSet();
                    const SfxPoolItem& rItem = rSet.Get(FN_PARAM_FTN_INFO);
                    rItem.QueryValue(pRet[nProp], nMemberId);
                    break;
                }
                default:
                {
                    //UUUU
                    pRet[nProp] = lcl_GetStyleProperty(*pEntry, *pPropSet, aBase, pBase, GetFamily(), GetDoc() );
                    break;
                }
            }
        }
        else if(IsDescriptor())
        {
            uno::Any* pAny = nullptr;
            GetPropImpl()->GetProperty(rPropName, pAny);

            if ( !pAny )
            {
                SwStyleProperties_Impl::GetProperty(rPropName, mxStyleData, pRet[nProp]);
            }
            else
            {
                pRet[nProp] = *pAny;
            }
        }
        else
        {
            throw uno::RuntimeException();
        }
    }

    return aRet;
}

uno::Sequence< uno::Any > SwXPageStyle::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames )
        throw(uno::RuntimeException, std::exception)
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
    beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);
    SetPropertyValues_Impl( aProperties, aValues );
}

SwXFrameStyle::SwXFrameStyle ( SwDoc *pDoc )
: SwXStyle ( pDoc, SFX_STYLE_FAMILY_FRAME, false)
{
}

SwXFrameStyle::~SwXFrameStyle()
{
}

uno::Sequence< uno::Type > SwXFrameStyle::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTypes = SwXStyle::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 1);
    aTypes.getArray()[nLen] = cppu::UnoType<XEventsSupplier>::get();
    return aTypes;
}

uno::Any SwXFrameStyle::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if(rType == cppu::UnoType<XEventsSupplier>::get())
        aRet <<= uno::Reference<XEventsSupplier>(this);
    else
        aRet = SwXStyle::queryInterface(rType);
    return aRet;
}

uno::Reference< container::XNameReplace > SwXFrameStyle::getEvents(  ) throw(uno::RuntimeException, std::exception)
{
    return new SwFrameStyleEventDescriptor( *this );
}

SwXAutoStyles::SwXAutoStyles(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()), m_pDocShell( &rDocShell )
{
}

SwXAutoStyles::~SwXAutoStyles()
{
}

sal_Int32 SwXAutoStyles::getCount() throw( uno::RuntimeException, std::exception )
{
    return AUTOSTYLE_FAMILY_COUNT;
}

uno::Any SwXAutoStyles::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
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
                if(!m_xAutoCharStyles.is())
                    m_xAutoCharStyles = new SwXAutoStyleFamily(m_pDocShell, nType);
                aRef = m_xAutoCharStyles;
            }
            break;
            case IStyleAccess::AUTO_STYLE_RUBY:
            {
                if(!m_xAutoRubyStyles.is())
                    m_xAutoRubyStyles = new SwXAutoStyleFamily(m_pDocShell, nType );
                aRef = m_xAutoRubyStyles;
            }
            break;
            case IStyleAccess::AUTO_STYLE_PARA:
            {
                if(!m_xAutoParaStyles.is())
                    m_xAutoParaStyles = new SwXAutoStyleFamily(m_pDocShell, nType );
                aRef = m_xAutoParaStyles;
            }
            break;

            default:
                ;
        }
        aRet.setValue(&aRef, cppu::UnoType<style::XAutoStyleFamily>::get());
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SwXAutoStyles::getElementType(  ) throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<style::XAutoStyleFamily>::get();
}

sal_Bool SwXAutoStyles::hasElements(  ) throw(uno::RuntimeException, std::exception)
{
    return sal_True;
}

uno::Any SwXAutoStyles::getByName(const OUString& Name)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    uno::Any aRet;
    if(Name == "CharacterStyles")
        aRet = getByIndex(0);
    else if(Name == "RubyStyles")
        aRet = getByIndex(1);
    else if(Name == "ParagraphStyles")
        aRet = getByIndex(2);
    else
        throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXAutoStyles::getElementNames()
            throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aNames(AUTOSTYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = "CharacterStyles";
    pNames[1] = "RubyStyles";
    pNames[2] = "ParagraphStyles";
    return aNames;
}

sal_Bool SwXAutoStyles::hasByName(const OUString& Name)
            throw( uno::RuntimeException, std::exception )
{
    if( Name == "CharacterStyles" ||
        Name == "RubyStyles" ||
        Name == "ParagraphStyles" )
        return sal_True;
    else
        return sal_False;
}

SwXAutoStyleFamily::SwXAutoStyleFamily(SwDocShell* pDocSh, IStyleAccess::SwAutoStyleFamily nFamily) :
    m_pDocShell( pDocSh ), m_eFamily(nFamily)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDocSh->GetDoc()->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStyleFamily::~SwXAutoStyleFamily()
{
}

void SwXAutoStyleFamily::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        m_pDocShell = nullptr;
}

uno::Reference< style::XAutoStyle > SwXAutoStyleFamily::insertStyle(
    const uno::Sequence< beans::PropertyValue >& Values )
        throw (uno::RuntimeException, std::exception)
{
    if (!m_pDocShell)
    {
        throw uno::RuntimeException();
    }

    const sal_uInt16* pRange = nullptr;
    const SfxItemPropertySet* pPropSet = nullptr;
    switch( m_eFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            pRange = aCharAutoFormatSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE);
            break;
        }
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            pRange = nullptr;//aTextNodeSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_RUBY_AUTO_STYLE);
            break;
        }
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            pRange = aTextNodeSetRange; //UUUU checked, already added support for [XATTR_FILL_FIRST, XATTR_FILL_LAST]
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARA_AUTO_STYLE);
            break;
        }
        default: ;
    }

    if( !pPropSet)
        throw uno::RuntimeException();

    SwAttrSet aSet( m_pDocShell->GetDoc()->GetAttrPool(), pRange );
    const beans::PropertyValue* pSeq = Values.getConstArray();
    sal_Int32 nLen = Values.getLength();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == m_eFamily);

    if(!bTakeCareOfDrawingLayerFillStyle)
    {
        for( sal_Int32 i = 0; i < nLen; ++i )
        {
            try
            {
                pPropSet->setPropertyValue( pSeq[i].Name, pSeq[i].Value, aSet );
            }
            catch (const beans::UnknownPropertyException &)
            {
                OSL_FAIL( "Unknown property" );
            }
            catch (const lang::IllegalArgumentException &)
            {
                OSL_FAIL( "Illegal argument" );
            }
        }
    }
    else
    {
        //UUUU set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
        // to make cases in RES_BACKGROUND work correct; target *is* a style
        // where this is the case
        aSet.SetParent(&m_pDocShell->GetDoc()->GetDfltTextFormatColl()->GetAttrSet());

        //UUUU here the used DrawingLayer FillStyles are imported when family is
        // equal to IStyleAccess::AUTO_STYLE_PARA, thus we will need to serve the
        // used slots functionality here to do this correctly
        const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

        for( sal_Int32 i = 0; i < nLen; ++i )
        {
            const OUString& rPropName = pSeq[i].Name;
            uno::Any aValue(pSeq[i].Value);
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropName);

            if (!pEntry)
            {
                SAL_WARN("sw.core", "SwXAutoStyleFamily::insertStyle: Unknown property: " << rPropName);
                continue;
            }

            const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));
            bool bDone(false);

            // check for needed metric translation
            if(pEntry->nMemberId & SFX_METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aValue >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt)
                {
                    const SfxItemPool& rPool = m_pDocShell->GetDoc()->GetAttrPool();
                    const SfxMapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

                    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
                    {
                        SvxUnoConvertFromMM(eMapUnit, aValue);
                    }
                }
            }

            switch(pEntry->nWID)
            {
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLBITMAP:
                case XATTR_FILLFLOATTRANSPARENCE:
                // not yet needed; activate when LineStyle support may be added
                // case XATTR_LINESTART:
                // case XATTR_LINEEND:
                // case XATTR_LINEDASH:
                {
                    if(MID_NAME == nMemberId)
                    {
                        //UUUU add set commands for FillName items
                        OUString aTempName;

                        if(!(aValue >>= aTempName))
                        {
                            throw lang::IllegalArgumentException();
                        }

                        SvxShape::SetFillAttribute(pEntry->nWID, aTempName, aSet);
                        bDone = true;
                    }
                    else if(MID_GRAFURL == nMemberId)
                    {
                        if(XATTR_FILLBITMAP == pEntry->nWID)
                        {
                            //UUUU Bitmap also has the MID_GRAFURL mode where a Bitmap URL is used
                            const Graphic aNullGraphic;
                            XFillBitmapItem aXFillBitmapItem(aSet.GetPool(), aNullGraphic);

                            aXFillBitmapItem.PutValue(aValue, nMemberId);
                            aSet.Put(aXFillBitmapItem);
                            bDone = true;
                        }
                    }

                    break;
                }
                case RES_BACKGROUND:
                {
                    //UUUU
                    const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(aSet, RES_BACKGROUND, true, m_pDocShell->GetDoc()->IsInXMLImport()));
                    SvxBrushItem aChangedBrushItem(aOriginalBrushItem);

                    aChangedBrushItem.PutValue(aValue, nMemberId);

                    if(!(aChangedBrushItem == aOriginalBrushItem))
                    {
                        setSvxBrushItemAsFillAttributesToTargetSet(aChangedBrushItem, aSet);
                    }

                    bDone = true;
                    break;
                }
                case OWN_ATTR_FILLBMP_MODE:
                {
                    //UUUU
                    drawing::BitmapMode eMode;

                    if(!(aValue >>= eMode))
                    {
                        sal_Int32 nMode = 0;

                        if(!(aValue >>= nMode))
                        {
                            throw lang::IllegalArgumentException();
                        }

                        eMode = (drawing::BitmapMode)nMode;
                    }

                    aSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
                    aSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));

                    bDone = true;
                    break;
                }
                default: break;
            }

            if(!bDone)
            {
                try
                {
                    pPropSet->setPropertyValue( rPropName, aValue, aSet );
                }
                catch (const beans::UnknownPropertyException &)
                {
                    OSL_FAIL( "Unknown property" );
                }
                catch (const lang::IllegalArgumentException &)
                {
                    OSL_FAIL( "Illegal argument" );
                }
            }
         }

        //UUUU clear parent again
        aSet.SetParent(nullptr);
    }

    //UUUU need to ensure uniqueness of evtl. added NameOrIndex items
    // currently in principle only needed when bTakeCareOfDrawingLayerFillStyle,
    // but does not hurt and is easily forgotten later eventually, so keep it
    // as common case
    m_pDocShell->GetDoc()->CheckForUniqueItemForLineFillNameOrIndex(aSet);

    // AutomaticStyle creation
    SfxItemSet_Pointer_t pSet = m_pDocShell->GetDoc()->GetIStyleAccess().cacheAutomaticStyle( aSet, m_eFamily );
    uno::Reference<style::XAutoStyle> xRet = new SwXAutoStyle(m_pDocShell->GetDoc(), pSet, m_eFamily);

    return xRet;
}

uno::Reference< container::XEnumeration > SwXAutoStyleFamily::createEnumeration(  )
        throw (uno::RuntimeException, std::exception)
{
    if( !m_pDocShell )
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >
        (new SwXAutoStylesEnumerator( m_pDocShell->GetDoc(), m_eFamily ));
}

uno::Type SwXAutoStyleFamily::getElementType(  ) throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<style::XAutoStyle>::get();
}

sal_Bool SwXAutoStyleFamily::hasElements(  ) throw(uno::RuntimeException, std::exception)
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
            const SwFormatRuby* pItem = static_cast<const SwFormatRuby*>(rAttrPool.GetItem2( RES_TXTATR_CJK_RUBY, nI ));
            if ( pItem && pItem->GetTextRuby() )
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
: m_pImpl( new SwAutoStylesEnumImpl( pDoc, eFam ) )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStylesEnumerator::~SwXAutoStylesEnumerator()
{
    delete m_pImpl;
}

void SwXAutoStylesEnumerator::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

sal_Bool SwXAutoStylesEnumerator::hasMoreElements(  )
    throw (uno::RuntimeException, std::exception)
{
    if( !m_pImpl )
        throw uno::RuntimeException();
    return m_pImpl->hasMoreElements();
}

uno::Any SwXAutoStylesEnumerator::nextElement(  )
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( !m_pImpl )
        throw uno::RuntimeException();
    uno::Any aRet;
    if( m_pImpl->hasMoreElements() )
    {
        SfxItemSet_Pointer_t pNextSet = m_pImpl->nextElement();
        uno::Reference< style::XAutoStyle > xAutoStyle = new SwXAutoStyle(m_pImpl->getDoc(),
                                                        pNextSet, m_pImpl->getFamily());
        aRet.setValue(&xAutoStyle, cppu::UnoType<style::XAutoStyle>::get());
    }
    return aRet;
}

//UUUU SwXAutoStyle with the family IStyleAccess::AUTO_STYLE_PARA (or
// PROPERTY_MAP_PARA_AUTO_STYLE) now uses DrawingLayer FillStyles to allow
// unified paragraph background fill, thus the UNO API implementation has to
// support the needed slots for these. This seems to be used only for reading
// (no setPropertyValue implementation here), so maybe specialized for saving
// the Writer Doc to ODF

SwXAutoStyle::SwXAutoStyle(
    SwDoc* pDoc,
    SfxItemSet_Pointer_t pInitSet,
    IStyleAccess::SwAutoStyleFamily eFam)
:   mpSet(pInitSet),
    meFamily(eFam),
    mrDoc(*pDoc)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    mrDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStyle::~SwXAutoStyle()
{
}

void SwXAutoStyle::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        mpSet.reset();
    }
}

uno::Reference< beans::XPropertySetInfo > SwXAutoStyle::getPropertySetInfo(  )
                throw (uno::RuntimeException, std::exception)
{
    uno::Reference< beans::XPropertySetInfo >  xRet;
    switch( meFamily )
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
                const sal_uInt16 nMapId = PROPERTY_MAP_RUBY_AUTO_STYLE;
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
                const sal_uInt16 nMapId = PROPERTY_MAP_PARA_AUTO_STYLE;
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
            uno::RuntimeException, std::exception)
{
}

uno::Any SwXAutoStyle::getPropertyValue( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];
}

void SwXAutoStyle::addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                              const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                 const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::addVetoableChangeListener( const OUString& /*PropertyName*/,
                                              const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::removeVetoableChangeListener( const OUString& /*PropertyName*/,
                                                 const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::setPropertyValues(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Sequence< uno::Any >& /*aValues*/ )
            throw (beans::PropertyVetoException, lang::IllegalArgumentException,
                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
}

uno::Sequence< uno::Any > SwXAutoStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( !mpSet.get() )
    {
        throw uno::RuntimeException();
    }

    // query_item
    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const OUString* pNames = rPropertyNames.getConstArray();

    const sal_Int32 nLen(rPropertyNames.getLength());
    uno::Sequence< uno::Any > aRet( nLen );
    uno::Any* pValues = aRet.getArray();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == meFamily);

    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
        }

        uno::Any aTarget;
        bool bDone(false);

        if ( RES_TXTATR_AUTOFMT == pEntry->nWID || RES_AUTO_STYLE == pEntry->nWID )
        {
            OUString sName(StylePool::nameOf( mpSet ));
            aTarget <<= sName;
            bDone = true;
        }
        else if(bTakeCareOfDrawingLayerFillStyle)
        {
            //UUUU add support for DrawingLayer FillStyle slots
            switch(pEntry->nWID)
            {
                case RES_BACKGROUND:
                {
                    const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(*mpSet, RES_BACKGROUND));
                    const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));

                    if(!aOriginalBrushItem.QueryValue(aTarget, nMemberId))
                    {
                        OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
                    }

                    bDone = true;
                    break;
                }
                case OWN_ATTR_FILLBMP_MODE:
                {
                    const XFillBmpStretchItem* pStretchItem = dynamic_cast< const XFillBmpStretchItem* >(&mpSet->Get(XATTR_FILLBMP_STRETCH));
                    const XFillBmpTileItem* pTileItem = dynamic_cast< const XFillBmpTileItem* >(&mpSet->Get(XATTR_FILLBMP_TILE));

                    if( pTileItem && pTileItem->GetValue() )
                    {
                        aTarget <<= drawing::BitmapMode_REPEAT;
                    }
                    else if( pStretchItem && pStretchItem->GetValue() )
                    {
                        aTarget <<= drawing::BitmapMode_STRETCH;
                    }
                    else
                    {
                        aTarget <<= drawing::BitmapMode_NO_REPEAT;
                    }

                    bDone = true;
                    break;
                }
            }
        }

        if(!bDone)
        {
            pPropSet->getPropertyValue( *pEntry, *mpSet, aTarget );
        }

        if(bTakeCareOfDrawingLayerFillStyle)
        {
            if(pEntry->aType == cppu::UnoType<sal_Int16>::get() && pEntry->aType != aTarget.getValueType())
            {
                // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                sal_Int32 nValue = 0;
                aTarget >>= nValue;
                aTarget <<= (sal_Int16)nValue;
            }

            // check for needed metric translation
            if(pEntry->nMemberId & SFX_METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aTarget >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt)
                {
                    const SfxItemPool& rPool = mrDoc.GetAttrPool();
                    const SfxMapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

                    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
                    {
                        SvxUnoConvertToMM(eMapUnit, aTarget);
                    }
                }
            }
        }

        // add value
        pValues[i] = aTarget;
    }

    return aRet;
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyValues (
        const uno::Sequence< OUString >& rPropertyNames )
            throw (uno::RuntimeException, std::exception)
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
            throw (uno::RuntimeException, std::exception)
{
}

void SwXAutoStyle::removePropertiesChangeListener(
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException, std::exception)
{
}

void SwXAutoStyle::firePropertiesChangeEvent(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException, std::exception)
{
}

beans::PropertyState SwXAutoStyle::getPropertyState( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

void SwXAutoStyle::setPropertyToDefault( const OUString& /*PropertyName*/ )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException, std::exception )
{
}

uno::Any SwXAutoStyle::getPropertyDefault( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXAutoStyle::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames )
            throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    if (!mpSet.get())
    {
        throw uno::RuntimeException();
    }

    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == meFamily);

    for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
        }

        bool bDone(false);

        if(bTakeCareOfDrawingLayerFillStyle)
        {
            //UUUU DrawingLayer PropertyStyle support
            switch(pEntry->nWID)
            {
                case OWN_ATTR_FILLBMP_MODE:
                {
                    if(SfxItemState::SET == mpSet->GetItemState(XATTR_FILLBMP_STRETCH, false)
                        || SfxItemState::SET == mpSet->GetItemState(XATTR_FILLBMP_TILE, false))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                    }

                    bDone = true;
                    break;
                }
                case RES_BACKGROUND:
                {
                    if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(*mpSet,
                            pEntry->nMemberId))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                    bDone = true;

                    break;
                }
            }
        }

        if(!bDone)
        {
            pStates[i] = pPropSet->getPropertyState(*pEntry, *mpSet );
        }
    }

    return aRet;
}

void SwXAutoStyle::setAllPropertiesToDefault(  )
            throw (uno::RuntimeException, std::exception)
{
}

void SwXAutoStyle::setPropertiesToDefault(
        const uno::Sequence< OUString >& /*rPropertyNames*/ )
            throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyDefaults(
        const uno::Sequence< OUString >& /*aPropertyNames*/ )
            throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                    uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Any > aRet(0);
    return aRet;
}

uno::Sequence< beans::PropertyValue > SwXAutoStyle::getProperties() throw (uno::RuntimeException, std::exception)
{
    if( !mpSet.get() )
        throw uno::RuntimeException();
    SolarMutexGuard aGuard;
    std::vector< beans::PropertyValue > aPropertyVector;

    sal_Int8 nPropSetId = 0;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    PropertyEntryVector_t aPropVector = rMap.getPropertyEntries();

    SfxItemSet& rSet = *mpSet.get();
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
