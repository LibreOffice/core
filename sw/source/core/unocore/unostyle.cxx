/*************************************************************************
 *
 *  $RCSfile: unostyle.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-10-24 10:01:18 $
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

#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#include <cmdid.h>
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOSETT_HXX
#include <unosett.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SWSTYLE_H
#include <swstyle.h>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SHELLIO_HXX //autogen
#include <shellio.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#define ITEMID_SETITEM
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#define ITEMID_SIZE SID_ATTR_PAGE_SIZE
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen wg. SwFmtPageDesc
#include <fmtpdsc.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHSTYLECATEGORY_HPP_
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#endif


#define STYLE_FAMILY_COUNT 5            // wir habe fuenf Familien

#define TYPE_BOOL       0
#define TYPE_SIZE       1
#define TYPE_BRUSH      2
#define TYPE_ULSPACE    3
#define TYPE_SHADOW     4
#define TYPE_LRSPACE    5
#define TYPE_BOX        6

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::rtl;
/******************************************************************************
 *
 ******************************************************************************/

const unsigned short aStyleByIndex[] =
{
    SFX_STYLE_FAMILY_CHAR,
    SFX_STYLE_FAMILY_PARA,
    SFX_STYLE_FAMILY_PAGE     ,
    SFX_STYLE_FAMILY_FRAME    ,
    SFX_STYLE_FAMILY_PSEUDO
};

struct Programmatic2UIName
{
    String sProgrammaticName;
    String sUIName;
};
const Programmatic2UIName* lcl_GetStyleNameTable(SfxStyleFamily eFamily)
{
    const Programmatic2UIName* pRet = 0;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            static BOOL bInitialized = FALSE;
            static Programmatic2UIName aCharFamilyNames[(STR_POOLCHR_PRGM_CURRENT_END - RC_POOLCHRFMT_PRGM_BEGIN) +
                                                        (STR_POOLCHR_PRGM_HTML_CURRENT_END - RC_POOLCHRFMT_PRGM_HTML_BEGIN) + 3];
            if(!bInitialized)
            {
                bInitialized = TRUE;
                int nUIResId;
                int nProgrammaticResId;
                int nName = 0;
                for(nUIResId = RC_POOLCHRFMT_BEGIN, nProgrammaticResId = RC_POOLCHRFMT_PRGM_BEGIN;
                    nProgrammaticResId <= STR_POOLCHR_PRGM_CURRENT_END; nUIResId++, nProgrammaticResId++)
                {
                    aCharFamilyNames[nName].sUIName = String(SW_RES(nUIResId));
                    aCharFamilyNames[nName++].sProgrammaticName = String(SW_RES(nProgrammaticResId));
                }
                for(nUIResId = RC_POOLCHRFMT_HTML_BEGIN, nProgrammaticResId = RC_POOLCHRFMT_PRGM_HTML_BEGIN;
                    nProgrammaticResId <= STR_POOLCHR_PRGM_HTML_CURRENT_END; nUIResId++, nProgrammaticResId++)
                {
                    aCharFamilyNames[nName].sUIName = String(SW_RES(nUIResId));
                    aCharFamilyNames[nName++].sProgrammaticName = String(SW_RES(nProgrammaticResId));
                }
            }
            pRet = &aCharFamilyNames[0];
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            static BOOL bInitialized = FALSE;
            static Programmatic2UIName aParaFamilyNames[
                    ( STR_POCO_PRGM_HEADLINE10    -  STR_POCO_PRGM_STANDARD     ) +
                    ( STR_POCO_PRGM_BUL_NONUM5    -  STR_POCO_PRGM_NUMBUL_BASE   )+
                    ( STR_POCO_PRGM_LABEL_DRAWING -  STR_POCO_PRGM_HEADER        )+
                    ( STR_POCO_PRGM_TOX_USER10    -  STR_POCO_PRGM_REGISTER_BASE  )+
                    ( STR_POCO_PRGM_DOC_SUBTITEL  -  STR_POCO_PRGM_DOC_TITEL     )+
                    ( STR_POCO_PRGM_HTML_DT       -  STR_POCO_PRGM_HTML_BLOCKQUOTE)+
                    + 7 ];


            if(!bInitialized)
            {
                struct ParaIds
                {
                    USHORT nUIStart, nUIEnd, nProgStart;
                };
                ParaIds aParaIds[] =
                {
                    {STR_POOLCOLL_STANDARD,         STR_POOLCOLL_HEADLINE10,    STR_POCO_PRGM_STANDARD       },
                    {STR_POOLCOLL_NUMBUL_BASE,      STR_POOLCOLL_BUL_NONUM5,    STR_POCO_PRGM_NUMBUL_BASE    },
                    {STR_POOLCOLL_HEADER,           STR_POOLCOLL_LABEL_DRAWING, STR_POCO_PRGM_HEADER             },
                    {STR_POOLCOLL_REGISTER_BASE,    STR_POOLCOLL_TOX_USER10,    STR_POCO_PRGM_REGISTER_BASE   },
                    {STR_POOLCOLL_DOC_TITEL,        STR_POOLCOLL_DOC_SUBTITEL,  STR_POCO_PRGM_DOC_TITEL      },
                    {STR_POOLCOLL_HTML_BLOCKQUOTE,  STR_POOLCOLL_HTML_DT,       STR_POCO_PRGM_HTML_BLOCKQUOTE },
                };
                bInitialized = TRUE;
                int nUIResId;
                int nProgrammaticResId;
                int nName = 0;
                for(USHORT nPart = 0; nPart < 6; nPart++)
                    for(nUIResId = aParaIds[nPart].nUIStart, nProgrammaticResId = aParaIds[nPart].nProgStart;
                        nUIResId <= aParaIds[nPart].nUIEnd; nUIResId++, nProgrammaticResId++)
                    {
                        aParaFamilyNames[nName].sUIName = String(SW_RES(nUIResId));
                        aParaFamilyNames[nName++].sProgrammaticName = String(SW_RES(nProgrammaticResId));
                    }
            }
            pRet = &aParaFamilyNames[0];
        }
        break;
        case SFX_STYLE_FAMILY_FRAME:
        {
            static BOOL bInitialized = FALSE;
            static Programmatic2UIName aFrameFamilyNames[(STR_POOLFRM_PRGM_LABEL - STR_POOLFRM_PRGM_FRAME) + 2];
            if(!bInitialized)
            {
                bInitialized = TRUE;
                int nUIResId;
                int nProgrammaticResId;
                int nName = 0;
                for(nUIResId = STR_POOLFRM_FRAME, nProgrammaticResId = STR_POOLFRM_PRGM_FRAME;
                    nProgrammaticResId <= STR_POOLFRM_PRGM_LABEL; nUIResId++, nProgrammaticResId++)
                {
                    aFrameFamilyNames[nName].sUIName = String(SW_RES(nUIResId));
                    aFrameFamilyNames[nName++].sProgrammaticName = String(SW_RES(nProgrammaticResId));
                }
            }
            pRet = &aFrameFamilyNames[0];
        }
        break;
        case SFX_STYLE_FAMILY_PAGE:
        {
            static BOOL bInitialized = FALSE;
            static Programmatic2UIName aPageFamilyNames[(STR_POOLPAGE_PRGM_ENDNOTE - STR_POOLPAGE_PRGM_STANDARD) + 2];
            if(!bInitialized)
            {
                bInitialized = TRUE;
                int nUIResId;
                int nProgrammaticResId;
                int nName = 0;
                for(nUIResId = STR_POOLPAGE_STANDARD, nProgrammaticResId = STR_POOLPAGE_PRGM_STANDARD;
                    nProgrammaticResId <= STR_POOLPAGE_PRGM_ENDNOTE; nUIResId++, nProgrammaticResId++)
                {
                    aPageFamilyNames[nName].sUIName = String(SW_RES(nUIResId));
                    aPageFamilyNames[nName++].sProgrammaticName = String(SW_RES(nProgrammaticResId));
                }
            }
            pRet = &aPageFamilyNames[0];
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            static BOOL bInitialized = FALSE;
            static Programmatic2UIName aNumFamilyNames[(STR_POOLNUMRULE_PRGM_BUL5 - STR_POOLNUMRULE_PRGM_NUM1) + 2];
            if(!bInitialized)
            {
                bInitialized = TRUE;
                int nUIResId;
                int nProgrammaticResId;
                int nName = 0;
                for(nUIResId = STR_POOLNUMRULE_NUM1, nProgrammaticResId = STR_POOLNUMRULE_PRGM_NUM1;
                    nProgrammaticResId <= STR_POOLNUMRULE_PRGM_BUL5; nUIResId++, nProgrammaticResId++)
                {
                    aNumFamilyNames[nName].sUIName = String(SW_RES(nUIResId));
                    aNumFamilyNames[nName++].sProgrammaticName = String(SW_RES(nProgrammaticResId));
                }
            }
            pRet = &aNumFamilyNames[0];
        }
        break;
    }
    return pRet;
}
const String&   SwXStyleFamilies::GetProgrammaticName(const String& rUIName, SfxStyleFamily eFamily)
{
    const Programmatic2UIName* pNames =lcl_GetStyleNameTable(eFamily);
    DBG_ASSERT(pNames, "no mapping found!!!")
    if(pNames)
    do
    {
        if(pNames->sUIName == rUIName)
            return pNames->sProgrammaticName;
    }
    while((++pNames)->sUIName.Len());
    return rUIName;
}
const String&   SwXStyleFamilies::GetUIName(const String& rProgrammaticName, SfxStyleFamily eFamily)
{
    const Programmatic2UIName* pNames = lcl_GetStyleNameTable(eFamily);
    do
    {
        if(pNames->sProgrammaticName == rProgrammaticName)
            return pNames->sUIName;
    }
    while((++pNames)->sProgrammaticName.Len());
    return rProgrammaticName;
}

/******************************************************************
 * SwXStyleFamilies
 ******************************************************************/
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXStyleFamilies::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXStyleFamilies");
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXStyleFamilies::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.style.StyleFamilies") == rServiceName;
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamilies::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.style.StyleFamilies");
    return aRet;
}
/*-- 16.12.98 15:13:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamilies::SwXStyleFamilies(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()),
    pxCharStyles(0),
    pxParaStyles(0),
    pxFrameStyles(0),
    pxPageStyles(0),
    pxNumberingStyles(0),
    pDocShell(&rDocShell),
    bLoadStyleText(sal_True),
    bLoadStyleFrame(sal_True),
    bLoadStylePage(sal_True),
    bLoadStyleOverwrite(sal_False),
    bLoadStyleNumbering(sal_True)
{

}
/*-- 16.12.98 15:13:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamilies::~SwXStyleFamilies()
{
    delete pxCharStyles;
    delete pxParaStyles;
    delete pxFrameStyles;
    delete pxPageStyles;
    delete pxNumberingStyles;
}
/*-- 21.12.98 12:05:22---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SAL_CALL SwXStyleFamilies::getByName(const OUString& Name)
    throw(
        container::NoSuchElementException,
        lang::WrappedTargetException,
        RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
// der Index kommt aus const unsigned short aStyleByIndex[] =
    Any aRet;
    if(!IsValid())
        throw RuntimeException();
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
/*-- 21.12.98 12:05:22---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamilies::getElementNames(void) throw( RuntimeException )
{
    Sequence< OUString > aNames(STYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = C2U("CharacterStyles");
    pNames[1] = C2U("ParagraphStyles");
    pNames[2] = C2U("FrameStyles");
    pNames[3] = C2U("PageStyles");
    pNames[4] = C2U("NumberingStyles");
    return aNames;
}
/*-- 21.12.98 12:05:22---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamilies::hasByName(const OUString& Name) throw( RuntimeException )
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
/*-- 16.12.98 15:13:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXStyleFamilies::getCount(void) throw( RuntimeException )
{
    return STYLE_FAMILY_COUNT;
}
/*-- 16.12.98 15:13:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyleFamilies::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    if(nIndex >= STYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(IsValid())
    {
        Reference< container::XNameContainer >  aRef;
        sal_uInt16 nType = aStyleByIndex[nIndex];
        switch( nType )
        {
            case SFX_STYLE_FAMILY_CHAR:
            {
                if(!pxCharStyles)
                {
                    ((SwXStyleFamilies*)this)->pxCharStyles = new Reference< container::XNameContainer > ();
                    *pxCharStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxCharStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PARA:
            {
                if(!pxParaStyles)
                {
                    ((SwXStyleFamilies*)this)->pxParaStyles = new Reference< container::XNameContainer > ();
                    *pxParaStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxParaStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PAGE     :
            {
                if(!pxPageStyles)
                {
                    ((SwXStyleFamilies*)this)->pxPageStyles = new Reference< container::XNameContainer > ();
                    *pxPageStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxPageStyles;
            }
            break;
            case SFX_STYLE_FAMILY_FRAME    :
            {
                if(!pxFrameStyles)
                {
                    ((SwXStyleFamilies*)this)->pxFrameStyles = new Reference< container::XNameContainer > ();
                    *pxFrameStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxFrameStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PSEUDO:
            {
                if(!pxNumberingStyles)
                {
                    ((SwXStyleFamilies*)this)->pxNumberingStyles = new Reference< container::XNameContainer > ();
                    *pxNumberingStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxNumberingStyles;
            }
            break;
        }
        aRet.setValue(&aRef, ::getCppuType((const Reference<container::XNameContainer>*)0));
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 16.12.98 15:13:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Type SwXStyleFamilies::getElementType(void)
    throw( RuntimeException )
{
    return ::getCppuType((const Reference<container::XNameContainer>*)0);

}
/*-- 16.12.98 15:13:28---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamilies::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}
/*-- 16.12.98 15:13:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamilies::loadStylesFromURL(const OUString& rURL,
    const Sequence< beans::PropertyValue >& aOptions)
    throw( io::IOException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid() && rURL.len())
    {
        int nCount = aOptions.getLength();
        const beans::PropertyValue* pArray = aOptions.getConstArray();
        for(int i = 0; i < nCount; i++)
        {
            String sName = pArray[i].Name;
            const Any& rVal = pArray[i].Value;
            if(rVal.getValueType() != ::getBooleanCppuType())
                continue;
            sal_Bool bVal = *(sal_Bool*)rVal.getValue();
            if( sName.EqualsAscii(UNO_NAME_OVERWRITE_STYLES     ))
                bLoadStyleOverwrite = bVal;
            else if( sName.EqualsAscii(UNO_NAME_LOAD_NUMBERING_STYLES ))
                bLoadStyleNumbering = bVal;
            else if( sName.EqualsAscii(UNO_NAME_LOAD_PAGE_STYLES   ))
                bLoadStylePage = bVal;
            else if( sName.EqualsAscii(UNO_NAME_LOAD_FRAME_STYLES     ))
                bLoadStyleFrame = bVal;
            else if( sName.EqualsAscii(UNO_NAME_LOAD_TEXT_STYLES      ))
                bLoadStyleText = bVal;
        }

        String aFileName(rURL);
        INetURLObject aObj(aFileName);
        aFileName = aObj.GetFull();
        //bug SB
        aFileName.SearchAndReplace('|', ':');
        SvStorageRef pStor;
        SvFileStream* pStream = 0;
        SwRead pRead;
        SwReader* pReader;
        if( SvStorage::IsStorageFile( aFileName ))
        {
            pStor = new SvStorage( aFileName, STREAM_STD_READ );
            pRead = ReadSw3;
            SwNodeIndex aIdx(GetDoc()->GetNodes().GetEndOfContent(), -1);
            SwPaM aPam(aIdx);
            pReader = new SwReader(*pStor, aFileName,
                                    aPam );
        }
        else
        {
            pStream = new SvFileStream(aFileName, STREAM_STD_READ);
            pRead = ReadSwg;
            pReader = new SwReader(*pStream, aFileName, GetDoc());
        }
        pRead->GetReaderOpt().SetAllFmtsOnly();
        pRead->GetReaderOpt().SetTxtFmts(bLoadStyleText);
        pRead->GetReaderOpt().SetFrmFmts(bLoadStyleFrame);
        pRead->GetReaderOpt().SetPageDescs(bLoadStylePage);
        pRead->GetReaderOpt().SetNumRules(bLoadStyleNumbering);
        pRead->GetReaderOpt().SetMerge(!bLoadStyleOverwrite);

        {
            UnoActionContext aAction(GetDoc());
            if( 0 != pReader->Read( *pRead ))
                throw io::IOException();
        }
        delete pReader;
        delete pStream;
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 15:13:28---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< beans::PropertyValue > SwXStyleFamilies::getStyleLoaderOptions(void)
        throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< beans::PropertyValue > aSeq(5);
    beans::PropertyValue* pArray = aSeq.getArray();
    Any aVal;
    sal_Bool bTemp = bLoadStyleText;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[0] = beans::PropertyValue(C2U(UNO_NAME_LOAD_TEXT_STYLES), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    bTemp = bLoadStyleFrame;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[1] = beans::PropertyValue(C2U(UNO_NAME_LOAD_FRAME_STYLES), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    bTemp = bLoadStylePage;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[2] = beans::PropertyValue(C2U(UNO_NAME_LOAD_PAGE_STYLES), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    bTemp = bLoadStyleNumbering;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[3] = beans::PropertyValue(C2U(UNO_NAME_LOAD_NUMBERING_STYLES), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    bTemp = bLoadStyleOverwrite;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[4] = beans::PropertyValue(C2U(UNO_NAME_OVERWRITE_STYLES), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    return aSeq;
}

/******************************************************************
 * SwXStyleFamily
 ******************************************************************/
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXStyleFamily::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXStyleFamily");
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXStyleFamily::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.style.StyleFamily") == rServiceName;
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamily::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.style.StyleFamily");
    return aRet;
}
/*-- 16.12.98 16:03:56---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamily::SwXStyleFamily(SwDocShell* pDocSh, sal_uInt16 nFamily) :
        pBasePool(pDocSh->GetStyleSheetPool()),
        pDocShell(pDocSh),
        eFamily((SfxStyleFamily)nFamily)
{
/*  switch( nFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_CHAR_STYLE);
        break;
        case SFX_STYLE_FAMILY_PARA:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARA_STYLE);
        break;
        case SFX_STYLE_FAMILY_PAGE:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PAGE_STYLE);
        break;
        case SFX_STYLE_FAMILY_FRAME:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_FRAME_STYLE);
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_NUM_STYLE);
        break;
    }*/
    StartListening(*pBasePool);
}
/*-- 16.12.98 16:03:56---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamily::~SwXStyleFamily()
{

}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXStyleFamily::getCount(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nRet = 0;
    if(pBasePool)
    {
        SfxStyleSheetIterator* pIterator = pBasePool->CreateIterator(eFamily, 0xffff);
        nRet = pIterator->Count();
        delete pIterator;
    }
    return nRet;
}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyleFamily::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    if(pBasePool)
    {
        SfxStyleSheetIterator* pIterator = pBasePool->CreateIterator(eFamily, 0xffff);
        sal_uInt16 nCount= pIterator->Count();
        if(nIndex < nCount)
        {
            SfxStyleSheetBase* pBase = (*pIterator)[(sal_uInt16) nIndex];
            Reference< style::XStyle >  xStyle = _FindStyle(pBase->GetName());
            if(!xStyle.is())
            {
                xStyle =
                    eFamily == SFX_STYLE_FAMILY_PAGE ?
                    new SwXPageStyle(*pBasePool, pDocShell, eFamily, pBase->GetName()):
                    new SwXStyle(*pBasePool, eFamily, pDocShell->GetDoc(), pBase->GetName());
            }
            aRet.setValue(&xStyle, ::getCppuType((Reference<style::XStyle>*)0));
        }
        else
            throw lang::IndexOutOfBoundsException();
        delete pIterator;
    }
    else
        throw RuntimeException();
    return aRet;

}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyleFamily::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    String sStyleName = SwXStyleFamilies::GetUIName(rName, eFamily);
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
        {
            Reference< style::XStyle >  xStyle;
            xStyle = _FindStyle(sStyleName);
            if(!xStyle.is())
            {
                xStyle = eFamily == SFX_STYLE_FAMILY_PAGE ?
                    new SwXPageStyle(*pBasePool, pDocShell, eFamily, sStyleName) ://, _pPropMap) :
                    new SwXStyle(*pBasePool, eFamily, pDocShell->GetDoc(), sStyleName);//, _pPropMap);
            }
            aRet.setValue(&xStyle, ::getCppuType((Reference<style::XStyle>*)0));
        }
        else
            throw container::NoSuchElementException();
    }
    else
        throw RuntimeException();
    return aRet;

}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamily::getElementNames(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aRet;
    if(pBasePool)
    {
        SfxStyleSheetIterator* pIterator = pBasePool->CreateIterator(eFamily, 0xffff);
        sal_uInt16 nCount = pIterator->Count();
        aRet.realloc(nCount);
        OUString* pArray = aRet.getArray();
        for(sal_uInt16 i = 0; i < nCount; i++)
            pArray[i] = SwXStyleFamilies::GetProgrammaticName((*pIterator)[i]->GetName(), eFamily);
        delete pIterator;
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamily::hasByName(const OUString& rName) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        String sStyleName(SwXStyleFamilies::GetUIName(rName, eFamily));
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        bRet = 0 != pBase;
    }
    else
        throw RuntimeException();
    return bRet;

}
/*-- 16.12.98 16:03:58---------------------------------------------------

  -----------------------------------------------------------------------*/
Type SwXStyleFamily::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((const Reference<style::XStyle>*)0);

}
/*-- 16.12.98 16:03:58---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamily::hasElements(void) throw( RuntimeException )
{
    if(!pBasePool)
        throw RuntimeException();
    return sal_True;
}
/*-- 16.12.98 16:03:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::insertByName(const OUString& rName, const Any& rElement)
        throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pBasePool)
    {
        String sStyleName(rName);
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        SfxStyleSheetBase* pUINameBase = pBasePool->Find(SwXStyleFamilies::GetUIName(sStyleName, eFamily));
        if(pBase || pUINameBase)
            throw container::ElementExistException();
        else
        {
            if(rElement.getValueType().getTypeClass() ==
                                            TypeClass_INTERFACE)
            {
                Reference< XInterface > * pxRef =
                    (Reference< XInterface > *)rElement.getValue();

                Reference<lang::XUnoTunnel> xStyleTunnel( *pxRef, UNO_QUERY);

                SwXStyle* pNewStyle = 0;
                if(xStyleTunnel.is())
                {
                    pNewStyle = (SwXStyle*)xStyleTunnel->getSomething(
                                            SwXStyle::getUnoTunnelId());
                }
                if(!pNewStyle || !pNewStyle->IsDescriptor() ||
                    pNewStyle->GetFamily() != eFamily)
                        throw lang::IllegalArgumentException();
                if(pNewStyle)
                {
                    USHORT nMask = 0xffff;
                    if(eFamily == SFX_STYLE_FAMILY_PARA && !pNewStyle->IsConditional())
                        nMask &= ~SWSTYLEBIT_CONDCOLL;
                    SfxStyleSheetBase& rNewBase = pBasePool->Make(sStyleName, eFamily, nMask);
                    pNewStyle->SetDoc(pDocShell->GetDoc(), pBasePool);
                    pNewStyle->SetStyleName(sStyleName);
                    String sParentStyleName(pNewStyle->GetParentStyleName());
                    if(sParentStyleName.Len())
                    {
                        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
                        SfxStyleSheetBase* pParentBase = pBasePool->Find(sParentStyleName);
                        if(pParentBase && pParentBase->GetFamily() == eFamily &&
                            &pParentBase->GetPool() == pBasePool)
                            pBasePool->SetParent( eFamily, sStyleName,  sParentStyleName );

                    }
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
        throw RuntimeException();
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::replaceByName(const OUString& rName, const Any& rElement)
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
        Reference< style::XStyle >  xStyle = _FindStyle(pBase->GetName());
        if(xStyle.is())
        {
            Reference<XUnoTunnel> xTunnel( xStyle, UNO_QUERY);
            if(xTunnel.is())
            {
                SwXStyle* pStyle = (SwXStyle*)xTunnel->getSomething(
                                                    SwXStyle::getUnoTunnelId());
                pStyle->Invalidate();
            }
        }

        pBasePool->Erase(pBase);
        insertByName(rName, rElement);
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::removeByName(const OUString& rName) throw( container::NoSuchElementException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(SwXStyleFamilies::GetUIName(rName, eFamily));
        if(pBase)
            pBasePool->Erase(pBase);
        else
            throw container::NoSuchElementException();
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle*   SwXStyleFamily::_FindStyle(const String& rStyleName)const
{
    sal_uInt16  nLCount = pBasePool->GetListenerCount();
    SfxListener* pListener = 0;
    for( sal_uInt16 i = 0; i < nLCount; i++)
    {
        pListener = pBasePool->GetListener( i );
        if(PTR_CAST(SwXStyle, pListener) && ((SwXStyle*)pListener)->GetFamily() == eFamily && ((SwXStyle*)pListener)->GetStyleName() == rStyleName)
        {
            return (SwXStyle*)pListener;
        }
    }
    return 0;
}
/******************************************************************
 *
 ******************************************************************/
class SwStyleProperties_Impl
{
    const SfxItemPropertyMap*   _pMap;
    Any**                   pAnyArr;
    sal_uInt16                      nArrLen;

public:
    SwStyleProperties_Impl(const SfxItemPropertyMap* _pMap);
    ~SwStyleProperties_Impl();

    sal_Bool    SetProperty(const String& rName, Any aVal);
    sal_Bool    GetProperty(const String& rName, Any*& rpAny);

    const SfxItemPropertyMap*   GetPropertyMap() const {return _pMap;}
};
//--------------------------------------------------------------------
//--------------------------------------------------------------------
SwStyleProperties_Impl::SwStyleProperties_Impl(const SfxItemPropertyMap* pMap) :
    _pMap(pMap),
    nArrLen(0)
{
    const SfxItemPropertyMap* pTmp = _pMap;
    while(pTmp[nArrLen].nWID)
    {
        nArrLen++;
    }

    pAnyArr = new Any*[nArrLen];
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pAnyArr[i] = 0;

}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
SwStyleProperties_Impl::~SwStyleProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pAnyArr[i];
    delete pAnyArr;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
sal_Bool SwStyleProperties_Impl::SetProperty(const String& rName, Any aVal)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    while( pTemp->pName )
    {
        if(rName.EqualsAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
    {
        delete pAnyArr[nPos];
        pAnyArr[nPos] = new Any(aVal);
    }
    return nPos < nArrLen;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
sal_Bool SwStyleProperties_Impl::GetProperty(const String& rName, Any*& rpAny )
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    while( pTemp->pName )
    {
        if(rName.EqualsAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
    {
        rpAny = pAnyArr[nPos];
    }
    return rpAny && nPos < nArrLen;
}

/******************************************************************
 *
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence< sal_Int8 > & SwXStyle::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXStyle::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXStyle, SfxListener);
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXStyle::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXStyle");
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXStyle::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    BOOL bRet = C2U("com.sun.star.style.Style") == rServiceName;
    if(!bRet && SFX_STYLE_FAMILY_CHAR == eFamily)
        bRet = C2U("com.sun.star.style.CharacterProperties") == rServiceName;
    if(!bRet && SFX_STYLE_FAMILY_PARA == eFamily)
        bRet = (C2U("com.sun.star.style.ParagraphStyle") == rServiceName)||
            (C2U("com.sun.star.style.ParagraphProperties") == rServiceName);

    return  bRet;
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXStyle::getSupportedServiceNames(void) throw( RuntimeException )
{
    long nCount = 1;
    if(SFX_STYLE_FAMILY_PARA == eFamily)
    {
        nCount = 3;
        if(bIsConditional)
            nCount++;
    }
    else if(SFX_STYLE_FAMILY_CHAR == eFamily)
        nCount = 2;
    Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.style.Style");
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:     pArray[1] = C2U("com.sun.star.style.CharacterProperties"); break;
        case SFX_STYLE_FAMILY_PARA:
            pArray[1] = C2U("com.sun.star.style.ParagraphStyle");
            pArray[2] = C2U("com.sun.star.style.ParagraphProperties");
        if(bIsConditional)
            pArray[3] = C2U("com.sun.star.style.ConditionalParagraphStyle");
        break;
    }
    return aRet;
}
/*-- 17.12.98 08:26:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle::SwXStyle(SfxStyleFamily eFam, BOOL bConditional) :
    m_pDoc(0),
    bIsDescriptor(sal_True),
    bIsConditional(bConditional),
    eFamily(eFam),
    pBasePool(0)
{
    sal_uInt16 nMapId = PROPERTY_MAP_CHAR_STYLE;
    switch( eFamily )
    {
        //case SFX_STYLE_FAMILY_CHAR:   nMapId = PROPERTY_MAP_CHAR_STYLE;       break;
        case SFX_STYLE_FAMILY_PARA: nMapId = PROPERTY_MAP_PARA_STYLE;       break;
        case SFX_STYLE_FAMILY_PAGE: nMapId = PROPERTY_MAP_PAGE_STYLE;       break;
        case SFX_STYLE_FAMILY_FRAME :   nMapId = PROPERTY_MAP_FRAME_STYLE;  break;
        case SFX_STYLE_FAMILY_PSEUDO:   nMapId = PROPERTY_MAP_NUM_STYLE;    break;
    }
    pPropImpl = new SwStyleProperties_Impl(aSwMapProvider.GetPropertyMap(nMapId));
}
/*-- 17.12.98 08:26:50---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle::SwXStyle(SfxStyleSheetBasePool& rPool, SfxStyleFamily eFam,
        SwDoc*  pDoc,   const String& rStyleName) :
    sStyleName(rStyleName),
    pBasePool(&rPool),
    eFamily(eFam),
    m_pDoc(pDoc),
    bIsDescriptor(sal_False),
    bIsConditional(sal_False),
    pPropImpl(0)
{
    StartListening(rPool);
    if(eFam == SFX_STYLE_FAMILY_PARA)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")
        if(pBase)
        {
            const USHORT nId = pDoc->GetPoolId(sStyleName, GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                ::IsConditionalByPoolId( nId );
            else
                bIsConditional = RES_CONDTXTFMTCOLL == ((SwDocStyleSheet*)pBase)->GetCollection()->Which();
        }
    }
}
/*-- 17.12.98 08:26:50---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle::~SwXStyle()
{
    if(pBasePool)
        EndListening(*pBasePool);
    delete pPropImpl;
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXStyle::getName(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sRet;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")
        if(!pBase)
            throw RuntimeException();
        sRet = SwXStyleFamilies::GetProgrammaticName(pBase->GetName(), eFamily);
    }
    else
        sRet = sStyleName;
    return sRet;
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setName(const OUString& rName) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")
        sal_Bool bExcept = sal_True;
        if(pBase && pBase->IsUserDefined())
        {
            SwDocStyleSheet aTmp( *(SwDocStyleSheet*)pBase );
             bExcept = !aTmp.SetName(rName);
            if(!bExcept)
                sStyleName = String(rName);
        }
        if(bExcept)
            throw RuntimeException();
    }
    else
        sStyleName = String(rName);
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyle::isUserDefined(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        //if it is not found it must be non user defined
        if(pBase)
            bRet = pBase->IsUserDefined();
    }
    else
        throw RuntimeException();
    return bRet;
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyle::isInUse(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_USED);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
            bRet = pBase->IsUsed();
    }
    else
        throw RuntimeException();
    return bRet;
}
/*-- 17.12.98 08:26:52---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXStyle::getParentStyle(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sRet;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
            sRet = pBase->GetParent();
    }
    else if(bIsDescriptor)
        sRet = sParentStyleName;
    else
        throw RuntimeException();
    return SwXStyleFamilies::GetProgrammaticName(sRet, eFamily);
}
/*-- 17.12.98 08:26:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setParentStyle(const OUString& rParentStyle)
            throw( container::NoSuchElementException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sParentStyle = SwXStyleFamilies::GetUIName(rParentStyle, eFamily);
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        BOOL bExcept = FALSE;
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
        {
            SwDocStyleSheet aBase(*(SwDocStyleSheet*)pBase);
            if(aBase.GetParent() != sParentStyle)
            {
                bExcept = !aBase.SetParent(sParentStyle);
            }
        }
        else
            bExcept = TRUE;
        if(bExcept)
            throw RuntimeException();
    }
    else if(bIsDescriptor)
    {
        sParentStyleName = String(sParentStyle);
    }
    else
        throw RuntimeException();
}
/*-- 17.12.98 08:26:52---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySetInfo >  SwXStyle::getPropertySetInfo(void)
    throw( RuntimeException )
{
    Reference< beans::XPropertySetInfo >  xRet;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            static Reference< beans::XPropertySetInfo >  xCharRef;
            if(!xCharRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_CHAR_STYLE));
                xCharRef = aPropSet.getPropertySetInfo();
            }
            xRet = xCharRef;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            static Reference< beans::XPropertySetInfo >  xParaRef;
            if(!xParaRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARA_STYLE));
                xParaRef = aPropSet.getPropertySetInfo();
            }
            xRet = xParaRef;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE     :
        {
            static Reference< beans::XPropertySetInfo >  xPageRef;
            if(!xPageRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PAGE_STYLE) );
                xPageRef = aPropSet.getPropertySetInfo();
            }
            xRet = xPageRef;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME    :
        {
            static Reference< beans::XPropertySetInfo >  xFrameRef;
            if(!xFrameRef.is())
            {
                SfxItemPropertySet aPropSet(
                aSwMapProvider.GetPropertyMap(PROPERTY_MAP_FRAME_STYLE));
                xFrameRef = aPropSet.getPropertySetInfo();
            }
            xRet = xFrameRef;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            static Reference< beans::XPropertySetInfo >  xNumRef;
            if(!xNumRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_NUM_STYLE));
                xNumRef = aPropSet.getPropertySetInfo();
            }
            xRet = xNumRef;
        }
        break;
    }
    return xRet;
}
/* -----------------23.04.99 13:28-------------------
 *
 * --------------------------------------------------*/
void    SwXStyle::ApplyDescriptorProperties()
{
    bIsDescriptor = sal_False;
    const SfxItemPropertyMap* pTemp = pPropImpl->GetPropertyMap();
    while(pTemp->nWID)
    {
        Any* pAny;
        String sPropName(C2S(pTemp->pName));
        OUString sUPropName(sPropName);
        pPropImpl->GetProperty(sPropName, pAny);
        if(pAny)
            setPropertyValue(sUPropName, *pAny);
        pTemp++;
    }
}

/*-- 17.12.98 08:26:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException,
        lang::IllegalArgumentException,
         lang::WrappedTargetException,
        RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
        case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
    }
    SfxItemPropertySet& aPropSet = aSwMapProvider.GetPropertySet(nPropSetId);

    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
    if(!pMap)
        throw beans::UnknownPropertyException();
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")
        if(pBase)
        {
            SwDocStyleSheet aBase(*(SwDocStyleSheet*)pBase);
            switch(pMap->nWID)
            {
                case  FN_UNO_NUM_RULES: //Sonderbehandlung fuer das SvxNumRuleItem:
                {
                    if(aValue.getValueType() == ::getCppuType((Reference< container::XIndexReplace>*)0) )
                    {
                        Reference< container::XIndexReplace > * pxRulesRef =
                                (Reference< container::XIndexReplace > *)aValue.getValue();

                        Reference<lang::XUnoTunnel> xNumberTunnel( *pxRulesRef, UNO_QUERY);

                        SwXNumberingRules* pSwXRules = 0;
                        if(xNumberTunnel.is())
                        {
                            pSwXRules = (SwXNumberingRules*)
                                xNumberTunnel->getSomething(
                                                    SwXNumberingRules::getUnoTunnelId());
                        }
                        if(pSwXRules)
                        {
                            const String* pCharStyleNames = pSwXRules->GetNewCharStyleNames();
                            const String* pBulletFontNames = pSwXRules->GetBulletFontNames();

                            SwNumRule aSetRule(*pSwXRules->GetNumRule());
                            const SwCharFmts* pFmts = m_pDoc->GetCharFmts();
                            sal_uInt16 nChCount = pFmts->Count();
                            for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                            {

                                const SwNumFmt* pFmt = aSetRule.GetNumFmt( i );
                                if(pFmt)
                                {
                                    SwNumFmt aFmt(*pFmt);
                                    if(
                                        pCharStyleNames[i] != SwXNumberingRules::GetInvalidStyle() &&
                                        ((pCharStyleNames[i].Len() && !pFmt->GetCharFmt()) ||
                                        pCharStyleNames[i].Len() &&
                                                    pFmt->GetCharFmt()->GetName() != pCharStyleNames[i] ))
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
                                    if(pBulletFontNames[i] != SwXNumberingRules::GetInvalidStyle() &&
                                        ((pBulletFontNames[i].Len() && !pFmt->GetBulletFont()) ||
                                        pBulletFontNames[i].Len() &&
                                                pFmt->GetBulletFont()->GetName() != pBulletFontNames[i] ))
                                    {
                                        const SvxFontListItem* pFontListItem =
                                                (const SvxFontListItem* )m_pDoc->GetDocShell()
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
                            aBase.SetNumRule(aSetRule);
                        }
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                break;
                case FN_UNO_FOLLOW_STYLE:
                {
                    OUString sTmp;
                    aValue >>= sTmp;
                    aBase.SetFollow( SwXStyleFamilies::GetUIName(sTmp, eFamily)) ;
                }
                break;
                case RES_PAGEDESC :
                if( MID_PAGEDESC_PAGEDESCNAME != pMap->nMemberId)
                    goto put_itemset;
                {
                    // Sonderbehandlung RES_PAGEDESC
                    if(aValue.getValueType() != ::getCppuType((const OUString*)0))
                        throw lang::IllegalArgumentException();
                    const SfxItemSet& rStyleSet = aBase.GetItemSet();
                    SfxItemSet aSet(*rStyleSet.GetPool(), RES_PAGEDESC, RES_PAGEDESC);
                    aSet.Put(rStyleSet);

                    SwFmtPageDesc* pNewDesc = 0;
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == aSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
                    {
                        pNewDesc = new SwFmtPageDesc(*((SwFmtPageDesc*)pItem));
                    }
                    if(!pNewDesc)
                        pNewDesc = new SwFmtPageDesc();
                    OUString uDescName;
                    aValue >>= uDescName;
                    String sDescName(SwXStyleFamilies::GetUIName(uDescName, SFX_STYLE_FAMILY_PAGE));
                    if(!pNewDesc->GetPageDesc() || pNewDesc->GetPageDesc()->GetName() != sDescName)
                    {
                        sal_uInt16 nCount = m_pDoc->GetPageDescCnt();
                        sal_Bool bPut = sal_False;
                        if(sDescName.Len())
                        {
                            SwPageDesc* pPageDesc = ::GetPageDescByName_Impl(*m_pDoc, sDescName);
                            if(pPageDesc)
                            {
                                pPageDesc->Add( pNewDesc );
                                bPut = sal_True;
                            }
                            else
                            {
                                throw lang::IllegalArgumentException();
                            }
                        }
                        if(!bPut)
                        {
                            aSet.ClearItem(RES_BREAK);
                            aSet.Put(SwFmtPageDesc());
                        }
                        else
                            aSet.Put(*pNewDesc);
                    }
                    aBase.SetItemSet(aSet);
                    delete pNewDesc;
                }
                break;
                case FN_UNO_IS_AUTO_UPDATE:
                {
                    BOOL bAuto = *(sal_Bool*)aValue.getValue();
                    if(SFX_STYLE_FAMILY_PARA == eFamily)
                        aBase.GetCollection()->SetAutoUpdateFmt(bAuto);
                    else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                        aBase.GetFrmFmt()->SetAutoUpdateFmt(bAuto);
                }
                break;
                case FN_UNO_CATEGORY:
                {
                    if(!aBase.IsUserDefined())
                        throw lang::IllegalArgumentException();
                    short nSet;
                    aValue >>= nSet;

                    USHORT nId = aBase.GetCollection()->GetPoolFmtId() &
                                    ~ ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID );
                    switch( nSet )
                    {
                        case ParagraphStyleCategory::TEXT:
                            nId |= COLL_TEXT_BITS;
                            break;
                        case ParagraphStyleCategory::CHAPTER:
                            nId |= COLL_DOC_BITS;
                            break;
                        case ParagraphStyleCategory::LIST:
                            nId |= COLL_LISTS_BITS;
                            break;
                        case ParagraphStyleCategory::INDEX:
                            nId |= COLL_REGISTER_BITS;
                            break;
                        case ParagraphStyleCategory::EXTRA:
                            nId |= COLL_EXTRA_BITS;
                            break;
                        case ParagraphStyleCategory::HTML:
                            nId |= COLL_HTML_BITS;
                            break;
                        default: throw lang::IllegalArgumentException();
                    }
                    aBase.GetCollection()->SetPoolFmtId( nId );
                }
                break;
                case RES_PARATR_DROP:
                {
                    if( MID_DROPCAP_CHAR_STYLE_NAME == pMap->nMemberId)
                    {
                        if(aValue.getValueType() == ::getCppuType((const OUString*)0))
                        {
                            const SfxItemSet& rStyleSet = aBase.GetItemSet();
                            SfxItemSet aSet(*rStyleSet.GetPool(), RES_PARATR_DROP, RES_PARATR_DROP);
                            aSet.Put(rStyleSet);

                            SwFmtDrop* pDrop = 0;
                            const SfxPoolItem* pItem;
                            if(SFX_ITEM_SET == aSet.GetItemState( RES_PARATR_DROP, sal_True, &pItem ) )
                                pDrop = new SwFmtDrop(*((SwFmtDrop*)pItem));
                            if(!pDrop)
                                pDrop = new SwFmtDrop();
                            OUString uStyle;
                            aValue >>= uStyle;
                            String sStyle(SwXStyleFamilies::GetUIName(uStyle, SFX_STYLE_FAMILY_CHAR));
                            SwDocStyleSheet* pStyle =
                                (SwDocStyleSheet*)m_pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
                            if(pStyle)
                                pDrop->SetCharFmt(pStyle->GetCharFmt());
                            else
                                 throw lang::IllegalArgumentException();
                            aSet.Put(*pDrop);
                            aBase.SetItemSet(aSet);
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
                    SfxItemSet& rStyleSet = aBase.GetItemSet();
                    SfxItemSet aSet(*rStyleSet.GetPool(), pMap->nWID, pMap->nWID);
                    aSet.Put(rStyleSet);
                    aPropSet.setPropertyValue(rPropertyName, aValue, aSet);

                    aBase.SetItemSet(aSet);
                }
            }
        }
        else
            throw RuntimeException();
    }
    else if(bIsDescriptor)
    {
        if(!pPropImpl->SetProperty(rPropertyName, aValue))
            throw lang::IllegalArgumentException();
    }
    else
        throw RuntimeException();
}
/*-- 17.12.98 08:26:53---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyle::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;

    sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
        case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
    }
    SfxItemPropertySet& aPropSet = aSwMapProvider.GetPropertySet(nPropSetId);

    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
    if(!pMap)
        throw beans::UnknownPropertyException();
    if(pBasePool)
    {
        USHORT nSaveMask = pBasePool->GetSearchMask();
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        pBasePool->SetSearchMask(eFamily, nSaveMask );
        if(FN_UNO_IS_PHYSICAL == pMap->nWID)
        {
            BOOL bPhys = pBase != 0;
            if(pBase)
            {
                bPhys = ((SwDocStyleSheet*)pBase)->IsPhysical();
                // The standard character format is not existing physically
                if( bPhys && SFX_STYLE_FAMILY_CHAR == eFamily &&
                    ((SwDocStyleSheet*)pBase)->GetCharFmt() &&
                    ((SwDocStyleSheet*)pBase)->GetCharFmt()->IsDefault() )
                    bPhys = FALSE;
            }
            aRet.setValue(&bPhys, ::getBooleanCppuType());
        }
        else if(pBase)
        {
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
            switch(pMap->nWID)
            {
                case  FN_UNO_NUM_RULES: //Sonderbehandlung fuer das SvxNumRuleItem:
                {
                    const SwNumRule* pRule = aStyle.GetNumRule();
                    DBG_ASSERT(pRule, "Wo ist die NumRule?")
                    Reference< container::XIndexReplace >  xRules = new SwXNumberingRules(*pRule);
                    aRet.setValue(&xRules, ::getCppuType((Reference<container::XIndexReplace>*)0));
                }
                break;
                case FN_UNO_FOLLOW_STYLE:
                    aRet <<= OUString(SwXStyleFamilies::GetProgrammaticName(aStyle.GetFollow(), eFamily));
                break;
                case RES_PAGEDESC :
                if( MID_PAGEDESC_PAGEDESCNAME != pMap->nMemberId)
                    goto query_itemset;
                {
                    // Sonderbehandlung RES_PAGEDESC
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == aStyle.GetItemSet().GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
                    {
                        const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                        if(pDesc)
                            aRet <<= OUString( SwXStyleFamilies::GetProgrammaticName(pDesc->GetName(), SFX_STYLE_FAMILY_PAGE) );
                    }
                }
                break;
                case FN_UNO_IS_AUTO_UPDATE:
                {
                    BOOL bAuto = FALSE;
                    if(SFX_STYLE_FAMILY_PARA == eFamily)
                        bAuto = aStyle.GetCollection()->IsAutoUpdateFmt();
                    else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                        bAuto = aStyle.GetFrmFmt()->IsAutoUpdateFmt();
                    aRet.setValue(&bAuto, ::getBooleanCppuType());
                }
                break;
                case FN_UNO_DISPLAY_NAME:
                {
                    OUString sName(aStyle.GetName());
                    aRet <<= sName;
                }
                break;
                case FN_UNO_CATEGORY:
                {
                    USHORT nPoolId = aStyle.GetCollection()->GetPoolFmtId();
                    short nRet = -1;
                    switch ( COLL_GET_RANGE_BITS & nPoolId )
                    {
                        case COLL_TEXT_BITS:
                            nRet = ParagraphStyleCategory::TEXT;
                            break;
                        case COLL_DOC_BITS:
                            nRet = ParagraphStyleCategory::CHAPTER;
                            break;
                        case COLL_LISTS_BITS:
                            nRet = ParagraphStyleCategory::LIST;
                            break;
                        case COLL_REGISTER_BITS:
                            nRet = ParagraphStyleCategory::INDEX;
                            break;
                        case COLL_EXTRA_BITS:
                            nRet = ParagraphStyleCategory::EXTRA;
                            break;
                        case COLL_HTML_BITS:
                            nRet = ParagraphStyleCategory::HTML;
                            break;
                    }
                    aRet <<= nRet;
                }
                break;
                default:
query_itemset:
                {
                    SfxItemSet& rSet = aStyle.GetItemSet();
                    aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
                }
            }
        }
        else
            throw RuntimeException();
    }
    else if(bIsDescriptor)
    {
        Any* pAny = 0;
        if(!pPropImpl->GetProperty(rPropertyName, pAny))
            throw lang::IllegalArgumentException();
        else if(pAny)
            aRet = *pAny;
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 17.12.98 08:26:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::addPropertyChangeListener(const OUString& PropertyName,
    const Reference< beans::XPropertyChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 08:26:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::removePropertyChangeListener(const OUString& PropertyName,
    const Reference< beans::XPropertyChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 08:26:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::addVetoableChangeListener(const OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 17.12.98 08:26:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::removeVetoableChangeListener(const OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}

/*-- 08.03.99 10:50:26---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXStyle::getPropertyState(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")

        if(pBase)
        {
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
            String sPropName(rPropertyName);
            //Sonderbehandlung fuer das SvxNumRuleItem:
            if(sPropName.EqualsAscii(UNO_NAME_NUMBERING_RULES))
            {
                eRet = beans::PropertyState_DIRECT_VALUE;
            }
            else if(sPropName.EqualsAscii(UNO_NAME_FOLLOW_STYLE))
            {
                eRet = beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
                switch(eFamily)
                {
                    case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
                    case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
                    case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
                    case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
                }

                SfxItemSet aSet = aStyle.GetItemSet();
                eRet = aSwMapProvider.GetPropertySet(nPropSetId).
                                    getPropertyState(sPropName, aSet);
            }
        }
        else
            throw RuntimeException();
    }
    else
        throw RuntimeException();
    return eRet;
}
/*-- 08.03.99 10:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< beans::PropertyState > SwXStyle::getPropertyStates(
    const Sequence< OUString >& PropertyNames)
    throw( beans::UnknownPropertyException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const OUString* pNames = PropertyNames.getConstArray();
    Sequence< beans::PropertyState > aRet(PropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < PropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}
/*-- 08.03.99 10:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setPropertyToDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")

        if(pBase)
        {
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
            String sPropName(rPropertyName);
            //Sonderbehandlung fuer das SvxNumRuleItem:
            if(sPropName.EqualsAscii(UNO_NAME_NUMBERING_RULES))
            {
                throw RuntimeException();
            }
            else if(sPropName.EqualsAscii(UNO_NAME_FOLLOW_STYLE))
            {
                throw RuntimeException();
            }
            else
            {
                sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
                switch(eFamily)
                {
                    case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
                    case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
                    case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
                    case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
                }

                SfxItemSet aSet(aStyle.GetItemSet());
                const SfxItemPropertyMap* _pMap = aSwMapProvider.GetPropertyMap(nPropSetId);
                const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
                                                            _pMap, rPropertyName);

                aSet.InvalidateItem( pMap->nWID);
                aStyle.SetItemSet(aSet);
            }
        }
        else
            throw RuntimeException();
    }
    else
        throw RuntimeException();
}
/*-- 08.03.99 10:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyle::getPropertyDefault(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "wo ist der Style?")

        if(pBase)
        {
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
            String sPropName(rPropertyName);
            //Sonderbehandlung fuer das SvxNumRuleItem:
            if(!sPropName.EqualsAscii(UNO_NAME_NUMBERING_RULES) &&
                !sPropName.EqualsAscii(UNO_NAME_FOLLOW_STYLE))
            {
                sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
                switch(eFamily)
                {
                    case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
                    case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
                    case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
                    case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
                }

                SfxItemSet aSet = aStyle.GetItemSet();
                const SfxItemPropertyMap* _pMap = aSwMapProvider.GetPropertyMap(nPropSetId);
                const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(
                                                            _pMap, sPropName);
                const SfxItemSet* pParentSet = aSet.GetParent();
                if(pParentSet)
                    aRet = aSwMapProvider.GetPropertySet(nPropSetId).
                                    getPropertyValue(sPropName, *pParentSet);
                else if(pMap->nWID != aSet.GetPool()->GetSlotId(pMap->nWID))
                {
                    const SfxPoolItem& rItem = aSet.GetPool()->GetDefaultItem(pMap->nWID);
                    rItem.QueryValue(aRet, pMap->nMemberId);
                }
            }
        }
        else
            throw RuntimeException();
    }
    else
        throw RuntimeException();
    return aRet;
}
/* -----------------21.01.99 13:08-------------------
 *
 * --------------------------------------------------*/
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
            ((SfxStyleSheetPool&)rBC).SetSearchMask(eFamily);
            SfxStyleSheetBase* pOwnBase = ((SfxStyleSheetPool&)rBC).Find(sStyleName);
            if(!pOwnBase)
            {
                EndListening(rBC);
                Invalidate();
            }
        }
    }
}
/* -----------------------------15.08.00 11:35--------------------------------

 ---------------------------------------------------------------------------*/
void SwXStyle::Invalidate()
{
    sStyleName.Erase();
    pBasePool = 0;
    m_pDoc = 0;
}
/******************************************************************
 * SwXPageStyle
 ******************************************************************/
/*-- 17.12.98 08:43:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPageStyle::SwXPageStyle(SfxStyleSheetBasePool& rPool,
        SwDocShell* pDocSh, SfxStyleFamily eFam,
        const String& rStyleName)://, const SfxItemPropertyMap* _pMap) :
    SwXStyle(rPool, eFam, pDocSh->GetDoc(), rStyleName),//, _pMap),
    pDocShell(pDocSh)
{

}
/* -----------------23.08.99 15:52-------------------

 --------------------------------------------------*/
SwXPageStyle::SwXPageStyle(SwDocShell* pDocSh) :
    SwXStyle(SFX_STYLE_FAMILY_PAGE),
    pDocShell(pDocSh)
{
}

/*-- 17.12.98 08:43:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPageStyle::~SwXPageStyle()
{

}
/*-- 17.12.98 08:43:36---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXPageStyle::getPropertyValue(const OUString& rPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PAGE_STYLE),
                    rPropertyName);
    if(!pMap)
        throw beans::UnknownPropertyException();
    if(GetBasePool())
    {
        sal_uInt16 nRes;
        sal_Bool bHeader = sal_False, bAll = sal_False, bLeft = sal_False, bRight = sal_False;
        switch(pMap->nWID)
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

            case FN_UNO_FOOTER_ON:
            case FN_UNO_FOOTER_BACKGROUND:
            case FN_UNO_FOOTER_BOX:
            case FN_UNO_FOOTER_LR_SPACE:
            case FN_UNO_FOOTER_SHADOW:
            case FN_UNO_FOOTER_BODY_DISTANCE:
            case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
            case FN_UNO_FOOTER_SHARE_CONTENT:
            case FN_UNO_FOOTER_HEIGHT:
            {
                SfxStyleSheetBasePool* pBasePool = ((SwXPageStyle*)this)->GetBasePool();
                pBasePool->SetSearchMask(GetFamily());
                SfxStyleSheetBase* pBase = pBasePool->Find(GetStyleName());
                if(pBase)
                {
                    SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
                    const SfxItemSet& rSet = aStyle.GetItemSet();
                    sal_Bool bFooter = sal_False;
                    sal_uInt16 nRes = 0;
                    switch(pMap->nWID)
                    {
                        case FN_UNO_FOOTER_ON:
                            bFooter = sal_True;
                        // kein break!
                        case FN_UNO_HEADER_ON:
                        {
                            //falls das SetItem nicht da ist, dann ist der Wert sal_False
                            BOOL bRet = sal_False;
                            aRet.setValue(&bRet, ::getCppuBooleanType());
                            nRes = SID_ATTR_PAGE_ON;
                        }
                        break;
                        case FN_UNO_FOOTER_BACKGROUND:      bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BACKGROUND:      nRes = RES_BACKGROUND;
                        break;
                        case FN_UNO_FOOTER_BOX:             bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BOX:             nRes = RES_BOX;
                        break;
                        case FN_UNO_FOOTER_LR_SPACE:        bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_LR_SPACE:        nRes = RES_LR_SPACE;
                        break;
                        case FN_UNO_FOOTER_SHADOW:          bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_SHADOW:          nRes = RES_SHADOW;
                        break;
                        case FN_UNO_FOOTER_BODY_DISTANCE:   bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BODY_DISTANCE:   nRes = RES_UL_SPACE;
                        break;
                        case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE: bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
                        break;
                        case FN_UNO_FOOTER_SHARE_CONTENT:   bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_SHARE_CONTENT:   nRes = SID_ATTR_PAGE_SHARED;
                        break;
                        case FN_UNO_FOOTER_HEIGHT:          bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_HEIGHT:          nRes = SID_ATTR_PAGE_SIZE;
                        break;
                    }
                    const SvxSetItem* pSetItem;
                    if(SFX_ITEM_SET == rSet.GetItemState(
                            bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                            sal_False, (const SfxPoolItem**)&pSetItem))
                    {
                        const SfxItemSet& rSet = pSetItem->GetItemSet();
                        const SfxPoolItem* pItem = 0;
                        SfxItemState eState = rSet.GetItemState(nRes, sal_True, &pItem);
                        if(!pItem && nRes != rSet.GetPool()->GetSlotId(nRes))
                            pItem = &rSet.GetPool()->GetDefaultItem(nRes);
                        if(pItem)
                            pItem->QueryValue(aRet, pMap->nMemberId);
                    }
                }
            }
            break;
            case  FN_UNO_HEADER       :
                bAll = sal_True; goto Header;
            case  FN_UNO_HEADER_LEFT  :
                bLeft = sal_True; goto Header;
            case  FN_UNO_HEADER_RIGHT :
                bRight = sal_True; goto Header;
Header:
                bHeader = sal_True;
                nRes = RES_HEADER; goto MakeObject;
            case  FN_UNO_FOOTER       :
                bAll = sal_True; goto Footer;
            case  FN_UNO_FOOTER_LEFT  :
                bLeft = sal_True; goto Footer;
            case  FN_UNO_FOOTER_RIGHT :
                bRight = sal_True;
Footer:
                nRes = RES_FOOTER;
MakeObject:
            {
                SwDoc* pDoc = pDocShell->GetDoc();
                sal_uInt16 nPDescCount = pDoc->GetPageDescCnt();
                for(sal_uInt16 i = 0; i < nPDescCount; i++)
                {
                    const SwPageDesc& rDesc = pDoc->GetPageDesc( i );
                    if(rDesc.GetName() == GetStyleName())
                    {
                        const SwFrmFmt* pFrmFmt = 0;
                        sal_Bool bShare = bHeader && rDesc.IsHeaderShared()||
                                        !bHeader && rDesc.IsFooterShared();
                        // TextLeft returns the left content if there is one,
                        // Text and TextRight return the master content.
                        // TextRight does the same as Text and is for
                        // comptability only.
                        if( bLeft && !bShare )
                            pFrmFmt = &rDesc.GetLeft();
                        else
                            pFrmFmt = &rDesc.GetMaster();
                        if(pFrmFmt)
                        {
                            const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
                            const SfxPoolItem* pItem;
                            SwFrmFmt* pHeadFootFmt;
                            if(SFX_ITEM_SET == rSet.GetItemState(nRes, sal_True, &pItem) &&
                             0 != (pHeadFootFmt = bHeader ?
                                        ((SwFmtHeader*)pItem)->GetHeaderFmt() :
                                            ((SwFmtFooter*)pItem)->GetFooterFmt()))
                            {
                                // gibt es schon ein Objekt dafuer?
                                SwXHeadFootText* pxHdFt = (SwXHeadFootText*)SwClientIter( *pHeadFootFmt ).
                                                First( TYPE( SwXHeadFootText ));
                                Reference< text::XText >  xRet = pxHdFt;
                                if(!pxHdFt)
                                    xRet = new SwXHeadFootText(*pHeadFootFmt, bHeader);
                                aRet.setValue(&xRet, ::getCppuType((Reference<text::XText>*)0));
                            }
                        }
                        break;
                    }
                }

            }
            break;
            default:
                aRet = SwXStyle::getPropertyValue(rPropertyName);
        }
    }
    return aRet;
}
/*-- 17.12.98 08:43:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPageStyle::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException,
        lang::IllegalArgumentException,
         lang::WrappedTargetException,
        RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                        aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PAGE_STYLE),
                        rPropertyName);
    if(!pMap)
        throw beans::UnknownPropertyException();
    if(GetBasePool())
    {
        switch(pMap->nWID)
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
            case FN_UNO_FOOTER_ON:
            case FN_UNO_FOOTER_BACKGROUND:
            case FN_UNO_FOOTER_BOX:
            case FN_UNO_FOOTER_LR_SPACE:
            case FN_UNO_FOOTER_SHADOW:
            case FN_UNO_FOOTER_BODY_DISTANCE:
            case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
            case FN_UNO_FOOTER_SHARE_CONTENT:
            case FN_UNO_FOOTER_HEIGHT:
            {
                SfxStyleSheetBasePool* pBasePool = ((SwXPageStyle*)this)->GetBasePool();
                pBasePool->SetSearchMask(GetFamily());
                SfxStyleSheetBase* pBase = pBasePool->Find(GetStyleName());
                if(pBase)
                {
                    SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
                    SfxItemSet aSet(aStyle.GetItemSet());
                    sal_Bool bSetItem = sal_False;
                    sal_Bool bFooter = sal_False;
                    sal_uInt16 nItemType = TYPE_BOOL;
                    sal_uInt16 nRes = 0;
                    switch(pMap->nWID)
                    {
                        case FN_UNO_FOOTER_ON:                  bFooter = sal_True;
                        //kein break;
                        case FN_UNO_HEADER_ON:                  nRes = SID_ATTR_PAGE_ON;
                        break;
                        case FN_UNO_FOOTER_BACKGROUND:          bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BACKGROUND:          nRes = RES_BACKGROUND; nItemType = TYPE_BRUSH;
                        break;
                        case FN_UNO_FOOTER_BOX:                 bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BOX:                 nRes = RES_BOX; nItemType = TYPE_BOX;
                        break;
                        case FN_UNO_FOOTER_LR_SPACE:            bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_LR_SPACE:            nRes = RES_LR_SPACE;nItemType = TYPE_LRSPACE;
                        break;
                        case FN_UNO_FOOTER_SHADOW:              bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_SHADOW:              nRes = RES_SHADOW;nItemType = TYPE_SHADOW;
                        break;
                        case FN_UNO_FOOTER_BODY_DISTANCE:       bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BODY_DISTANCE:       nRes = RES_UL_SPACE;nItemType = TYPE_ULSPACE;
                        break;
                        case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE: bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
                        break;
                        case FN_UNO_FOOTER_SHARE_CONTENT:       bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_SHARE_CONTENT:       nRes = SID_ATTR_PAGE_SHARED;
                        break;
                        case FN_UNO_FOOTER_HEIGHT:              bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_HEIGHT:              nRes = SID_ATTR_PAGE_SIZE;nItemType = TYPE_SIZE;
                        break;

                    }
                    const SvxSetItem* pSetItem;
                    if(SFX_ITEM_SET == aSet.GetItemState(
                            bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                            sal_False, (const SfxPoolItem**)&pSetItem))
                    {
                        SvxSetItem* pNewSetItem = (SvxSetItem*)pSetItem->Clone();
                        SfxItemSet& rSetSet = pNewSetItem->GetItemSet();
                        const SfxPoolItem* pItem = 0;
                        SfxPoolItem* pNewItem = 0;
                        SfxItemState eState = rSetSet.GetItemState(nRes, sal_True, &pItem);
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
                        bSetItem = pNewItem->PutValue(aValue, pMap->nMemberId);
                        rSetSet.Put(*pNewItem);
                        aSet.Put(*pNewSetItem);
                        aStyle.SetItemSet(aSet);
                        delete pNewItem;
                        delete pNewSetItem;
                    }
                    else if(SID_ATTR_PAGE_ON == nRes )
                    {
                        sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                        if(bVal)
                        {
                            SfxItemSet aTempSet(*aSet.GetPool(),
                                RES_BACKGROUND, RES_SHADOW,
                                RES_LR_SPACE, RES_UL_SPACE,
                                nRes, nRes,
                                SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                                SID_ATTR_PAGE_DYNAMIC, SID_ATTR_PAGE_DYNAMIC,
                                SID_ATTR_PAGE_SHARED, SID_ATTR_PAGE_SHARED,
                                0 );
                            aTempSet.Put(SfxBoolItem(nRes, sal_True));
                            aTempSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(MM50, MM50)));
                            aTempSet.Put(SvxLRSpaceItem(RES_LR_SPACE));
                            aTempSet.Put(SvxULSpaceItem(RES_UL_SPACE));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED, sal_True));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_DYNAMIC, sal_True));

                            SvxSetItem aNewSetItem( bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                                    aTempSet);
                            aSet.Put(aNewSetItem);
                            aStyle.SetItemSet(aSet);
                        }
                    }
                }
            }
            break;
            case  FN_UNO_HEADER       :
            case  FN_UNO_HEADER_LEFT  :
            case  FN_UNO_HEADER_RIGHT :
            case  FN_UNO_FOOTER       :
            case  FN_UNO_FOOTER_LEFT  :
            case  FN_UNO_FOOTER_RIGHT :
                throw lang::IllegalArgumentException();
            break;
            default:
                SwXStyle::setPropertyValue(rPropertyName, aValue);
        }
    }
}
/* -----------------12.01.99 15:31-------------------
 * Liefert den StartNode des Headers oder Footers der
 * linken oder rechten Seite zurueck
 * Kann auch Null liefern, wenn es keinen gibt
 * --------------------------------------------------*/
const SwStartNode* SwXPageStyle::GetStartNode(sal_Bool bHeader, sal_Bool bLeft)
{
    const SwStartNode* pRet = 0;
    if(GetBasePool())
    {
        sal_uInt16 nRes = bHeader ? RES_HEADER : RES_FOOTER;
        SwDoc* pDoc = pDocShell->GetDoc();
        sal_uInt16 nPDescCount = pDoc->GetPageDescCnt();
        for(sal_uInt16 i = 0; i < nPDescCount; i++)
        {
            const SwPageDesc& rDesc = pDoc->GetPageDesc( i );
            if(rDesc.GetName() == GetStyleName())
            {
                const SwFrmFmt* pFrmFmt = 0;
                sal_Bool bShare = bHeader && rDesc.IsHeaderShared()||
                                        !bHeader && rDesc.IsFooterShared();
                UseOnPage eUse = rDesc.GetUseOn();
                if(bShare || !bLeft && PD_RIGHT == eUse && PD_LEFT != eUse)
                    pFrmFmt = &rDesc.GetMaster();
                else if(bLeft && PD_RIGHT != eUse)
                        pFrmFmt = &rDesc.GetLeft();
                if(pFrmFmt)
                {
                    const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
                    const SfxPoolItem* pItem;
                    SwFrmFmt* pHeadFootFmt;
                    if(SFX_ITEM_SET == rSet.GetItemState(nRes, sal_True, &pItem) &&
                     0 != (pHeadFootFmt = bHeader ?
                            ((SwFmtHeader*)pItem)->GetHeaderFmt() :
                                ((SwFmtFooter*)pItem)->GetFooterFmt()))
                    {
                        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
                        const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
                        pRet = rNode.FindStartNode();
                    }
                }
                break;
            }
        }
    }
    return pRet;
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.3  2000/10/18 11:20:20  mib
    Setting/Getting PageNumber now works correctly for paragraph styles

    Revision 1.2  2000/10/12 17:15:50  mib
    (Header|Footer)Text(Left)? now don't respect UsedOn property

    Revision 1.1.1.1  2000/09/19 00:08:29  hr
    initial import

    Revision 1.96  2000/09/18 16:04:35  willem.vandorp
    OpenOffice header added.

    Revision 1.95  2000/09/01 14:23:40  os
    #78380# Header and footer separated

    Revision 1.94  2000/08/31 13:42:36  os
    DropCap: WhichId corrected

    Revision 1.93  2000/08/31 10:04:03  os
    Detect conditional style by PoolId

    Revision 1.92  2000/08/25 08:49:07  os
    +property DropCapCharStyleName

    Revision 1.91  2000/08/23 10:49:07  os
    new service: ...style.ConditionalParagraphStyle

    Revision 1.90  2000/08/15 10:06:51  os
    #76785# invalidate style after removing via replaceByName

    Revision 1.89  2000/08/10 10:55:15  os
    solaris compiler bug

    Revision 1.88  2000/08/02 14:53:57  mib
    Standard char style now isn't present physically

    Revision 1.87  2000/07/21 13:13:12  os
    #76644# Category property at paragraph styles

    Revision 1.86  2000/07/10 10:37:38  os
    #76667# isUserDefined: search mask corrected

    Revision 1.85  2000/07/10 07:47:41  os
    #76697# create object of SwDocStyleSheet

    Revision 1.84  2000/07/06 08:01:51  os
    ::setParent: no exception if the parent is already set

    Revision 1.83  2000/06/29 13:06:33  os
    service ParagraphStyle

    Revision 1.82  2000/06/28 14:05:06  os
    ::getPropertyValue: search always for all styles

    Revision 1.81  2000/06/28 12:29:32  os
    #76403# SwXStyleFamilies::getByIndex: exception condition corrected

    Revision 1.80  2000/06/28 09:24:00  os
    IsPhysical: prevent style from creating

    Revision 1.79  2000/06/27 13:37:22  os
    new Properties: IsAutoUpdate + DisplayName

    Revision 1.78  2000/06/27 12:10:33  os
    #76423# programmatic style names

    Revision 1.77  2000/06/26 12:56:42  os
    new Property: IsPhysical

    Revision 1.76  2000/05/25 09:55:12  hr
    exception specification

    Revision 1.75  2000/05/16 17:21:36  jp
    Changes for Unicode

    Revision 1.74  2000/05/16 09:14:55  os
    project usr removed

    Revision 1.73  2000/04/26 11:35:20  os
    GetName() returns String&

    Revision 1.72  2000/04/25 13:23:11  os
    SwXPageStyle::setPropertyValue: throw exception for readonly properties

    Revision 1.71  2000/04/19 13:35:31  os
    UNICODE

    Revision 1.70  2000/04/11 08:31:04  os
    UNICODE

    Revision 1.69  2000/03/27 10:21:10  os
    UNO III

    Revision 1.68  2000/03/21 15:42:25  os
    UNOIII

    Revision 1.67  2000/03/07 18:38:25  os
    #73638# ::loadStyles.. : create Reader with PaM instead of Doc

    Revision 1.66  2000/03/07 18:03:30  os
    #73638# ::loadStyles.. : only file names are valid

    Revision 1.65  2000/02/11 14:36:04  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.64  2000/02/04 08:56:48  jp
    #72424# Chg: PropertyValue

    Revision 1.63  2000/02/02 16:58:07  jp
    Task #72579#: interface of SwReader is changed

    Revision 1.62  2000/01/28 15:20:25  os
    #69861# Chg: beans::Xbeans::PropertyState; #72424# Chg: PropertyValue

    Revision 1.61  1999/11/25 15:31:53  os
    #70144# ::setName - create a copy style sheet

    Revision 1.60  1999/11/22 10:37:57  os
    missing headers added

    Revision 1.59  1999/11/19 16:40:19  os
    modules renamed

    Revision 1.58  1999/10/28 09:19:23  os
    #69199# set/getName corrected

    Revision 1.57  1999/08/23 14:17:54  OS
    #68353# create SwXPageStyle in ServiceProvider too


      Rev 1.56   23 Aug 1999 16:17:54   OS
   #68353# create SwXPageStyle in ServiceProvider too

      Rev 1.55   20 Jul 1999 14:55:08   OS
   #67495# PageDescName property in paragraph styles

      Rev 1.54   09 Jul 1999 09:22:52   OS
   #67525#

      Rev 1.53   07 Jul 1999 17:50:22   OS
   #67343# re-enable setPropertyValue

      Rev 1.52   18 Jun 1999 08:56:40   OS
   #67037# SetItemSet: dont apply on full range

      Rev 1.51   20 May 1999 08:51:40   OS
   #65130# replaceByName impl.

      Rev 1.50   12 May 1999 15:39:32   OS
   #66150# nArrLen initialisiert!

      Rev 1.49   07 May 1999 14:08:48   KZ
   #include <svtools/smplhint.hxx> eingefuegt

      Rev 1.48   23 Apr 1999 15:29:28   OS
   #65134# Properties schon am Style als Descriptor

      Rev 1.47   23 Apr 1999 08:26:38   OS
   #65194# Semikolon muss weg

      Rev 1.46   22 Apr 1999 16:13:56   OS
   #65194# throw -> throw; #65124# not impl. nur noch warning; EventListener

      Rev 1.45   30 Mar 1999 12:47:54   OS
   #64019# HeaderTextLeft/Right - neuer Versuch

      Rev 1.44   15 Mar 1999 14:37:52   OS
   #62845# Makro fuer ServiceInfo jetzt auch fuer OS/2

      Rev 1.43   12 Mar 1999 09:41:28   OS
   #62845# lang::XServiceInfo impl.

      Rev 1.42   09 Mar 1999 12:41:28   OS
    vos::OGuard aGuard(Application::GetSolarMutex());

      Rev 1.41   08 Mar 1999 13:49:36   OS
   #62461##62543 beans::Xbeans::PropertyState

      Rev 1.40   26 Feb 1999 09:38:02   OS
   #62454# SetItemSet immer rufen

      Rev 1.39   23 Feb 1999 10:22:06   OS
   #61767# Kapitelnumerierung funktioniert wieder

      Rev 1.38   22 Feb 1999 13:23:42   OS
   #60606# Numerierungsregel incl. Font richtig setzen

      Rev 1.37   19 Feb 1999 16:34:48   OS
   #62083# StyleName beim insert setzen

      Rev 1.36   19 Feb 1999 09:35:10   OS
   #62083# Styles per ServiceProvider anlegen

      Rev 1.35   04 Feb 1999 16:23:40   OS
   #56371# richtigen rechten Header/FooterText finden

      Rev 1.34   03 Feb 1999 11:48:12   OS
   #56371# ItemSet des StyleSheets kopieren

      Rev 1.33   02 Feb 1999 12:52:22   OS
   #56371# richtige IdlClasses

      Rev 1.32   22 Jan 1999 15:09:24   OS
   #56371# Draw wieder verfuegbar

      Rev 1.31   21 Jan 1999 13:45:32   OS
   #56371##52654# NumberingRule und FollowStyle

      Rev 1.30   21 Jan 1999 09:57:06   OS
   #60971# Zeichenvorlagen der Numerierung an Absatzvorlagen setzen

      Rev 1.29   14 Jan 1999 16:21:46   OS
   #56371# TF_ONE51

      Rev 1.28   13 Jan 1999 10:49:04   OS
   #52654# Start/Endnote fuer Kopf-/Fusszeile zurueckgeben

      Rev 1.27   05 Jan 1999 10:10:54   OS
   #60606# #52654# Numerierungsregel an Styles wieder korrekt

      Rev 1.26   21 Dec 1998 16:22:30   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.25   17 Dec 1998 09:21:02   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.24   10 Dec 1998 15:53:38   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.23   30 Oct 1998 16:32:06   OS
   #52654# fehlende Initialisierung

      Rev 1.22   23 Sep 1998 07:43:08   OS
   #52654# syntax

      Rev 1.21   18 Sep 1998 09:31:06   OS
   52654# style::XStyleLoader

      Rev 1.20   31 Aug 1998 14:56:32   OS
   #52654# addStyle: richtige Parameter fuer 504

      Rev 1.19   31 Aug 1998 14:40:00   OS
   #52654# addStyle: nur noch mit dem Namen des parents aufrufen

      Rev 1.18   27 Aug 1998 09:36:40   OS
   #52654# Kopf- und Fusszeilen impl.

      Rev 1.17   12 Aug 1998 19:43:48   HR
   #54781#: ::::getCppuType((const Reference< XInterface >*)0)() -> ( XInterface*)0)->getSmartUik()

      Rev 1.16   21 Jul 1998 17:00:32   OS
   SwXStyle::getPropertySetInfo - abhaengigg von eFamily unterschiedliche Infos liefern #52654#

      Rev 1.15   10 Jul 1998 18:08:28   OS
   PropertySetInfo und IdlClass static

      Rev 1.14   10 Jul 1998 14:43:58   HR
   .is()

      Rev 1.13   09 Jul 1998 09:13:56   OS
   getElementBy* - queryInterface auf XInterface


      Rev 1.12   25 Jun 1998 11:14:58   OS
   PreopertyMaps nur noch vom PropertyMapProvider

      Rev 1.11   23 Jun 1998 10:25:26   OS
   CONVERT_TWIPS

      Rev 1.10   19 Jun 1998 08:47:26   OS
   Twip-Konvertierung

      Rev 1.9   18 Jun 1998 18:09:58   OS
// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//   queryInterface fuer style::StyleFamily/ies berichtigt


      Rev 1.8   18 Jun 1998 08:11:10   OS
   Property-Namen veraendert

      Rev 1.7   16 Jun 1998 17:25:18   OS
   Bold/Italic/Underlined ersetzt durch Weight, Posture, Underline

      Rev 1.6   15 Jun 1998 12:46:12   OS
   SwXContainer:: vergessen

      Rev 1.5   10 Jun 1998 09:54:46   OS
   Package-Umstellung

      Rev 1.4   04 Jun 1998 09:40:04   OS
// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//   getIdlClasses


      Rev 1.3   29 May 1998 13:48:34   OS
   Properties erweitern

      Rev 1.2   27 May 1998 13:45:10   OS
   grundsaetzlich vollstaendig

      Rev 1.1   26 May 1998 16:59:08   OS
   teilweise funktionstuechtig

      Rev 1.0   26 May 1998 12:29:40   OS
   Initial revision.

------------------------------------------------------------------------*/

