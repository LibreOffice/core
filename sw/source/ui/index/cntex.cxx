/*************************************************************************
 *
 *  $RCSfile: cntex.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:43:50 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <rsc/rscsfx.hxx>

#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEXESSUPPLIER_HPP_
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTIONSSUPPLIER_HPP_
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XPARAGRAPHCURSOR_HPP_
#include <com/sun/star/text/XParagraphCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _CNTTAB_HXX
#include <cnttab.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _INDEX_HRC
#include <index.hrc>
#endif
#ifndef _CNTTAB_HRC
#include <cnttab.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace ::rtl;

#define C2S(cChar) UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))
#define C2U(cChar) OUString::createFromAscii(cChar)

#define INDEX_SECTION_NAME String::CreateFromAscii( \
                            RTL_CONSTASCII_STRINGPARAM( "IndexSection_" ))
/* -----------------04.11.99 11:28-------------------

 --------------------------------------------------*/
void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
                           uno::Reference< XPropertySet > & xProps,
                         const char* pPropName, const String& rValue)
{
    OUString uPropName(C2U(pPropName));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue <<= OUString(rValue);
        xProps->setPropertyValue(uPropName, aValue);
    }
}
void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
                           uno::Reference< XPropertySet > & xProps,
                           USHORT nId, const String& rValue)
{
    lcl_SetProp( xInfo, xProps, SW_PROP_NAME_STR(nId), rValue);
}
void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
                           uno::Reference< XPropertySet > & xProps,
                           USHORT nId, sal_Int16 nValue )
{
    OUString uPropName(C2U(SW_PROP_NAME_STR(nId)));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue <<= nValue;
        xProps->setPropertyValue(uPropName, aValue);
    }
}

void lcl_SetBOOLProp(
                uno::Reference< beans::XPropertySetInfo > & xInfo,
                uno::Reference< beans::XPropertySet > & xProps,
                USHORT nId, sal_Bool bValue )
{
    OUString uPropName(C2U(SW_PROP_NAME_STR(nId)));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue.setValue(&bValue, ::getCppuBooleanType());
        xProps->setPropertyValue(uPropName, aValue);
    }
}
//-----------------------------------------------------------------------------
IMPL_LINK( SwMultiTOXTabDialog, CreateExample_Hdl, void*, EMPTYARG )
{
    try
    {
         uno::Reference< frame::XModel > & xModel = pExampleFrame->GetModel();
        uno::Reference< lang::XUnoTunnel > xDocTunnel(xModel, uno::UNO_QUERY);
        SwXTextDocument* pDoc = (SwXTextDocument*)xDocTunnel->getSomething(SwXTextDocument::getUnoTunnelId());

        if( pDoc )
            pDoc->GetDocShell()->LoadStyles( *rSh.GetView().GetDocShell() );

         uno::Reference< lang::XMultiServiceFactory >  xFact(
                                             xModel, uno::UNO_QUERY);
         uno::Reference< text::XTextCursor > & xCrsr =
                                             pExampleFrame->GetTextCursor();

         uno::Reference< text::XTextSectionsSupplier >  xSectionSupplier(
                                                 xModel, uno::UNO_QUERY);
         uno::Reference< container::XNameAccess >  xSections =
                                        xSectionSupplier->getTextSections();

        String sSectionName( INDEX_SECTION_NAME );
        for(int i = 0; i < 7; ++i )
        {
            String sTmp( sSectionName ); sTmp += String::CreateFromInt32(i);
            uno::Any aSection = xSections->getByName( sTmp );
            aSection >>= pxIndexSectionsArr[i]->xContainerSection;
         }
         uno::Reference< text::XDocumentIndexesSupplier >  xIdxSupp(xModel, uno::UNO_QUERY);
         uno::Reference< container::XIndexAccess >  xIdxs = xIdxSupp->getDocumentIndexes();
        int n = xIdxs->getCount();
        while(n)
        {
            n--;
            uno::Any aIdx = xIdxs->getByIndex(n);
            uno::Reference< text::XDocumentIndex >  xIdx;
            aIdx >>= xIdx;
            xIdx->dispose();
        }
        CreateOrUpdateExample(eCurrentTOXType.eType);
    }
    catch(Exception&)
    {
        DBG_ERROR("::CreateExample() - exception caught")
    }
    return 0;
}

/* --------------------------------------------------

 --------------------------------------------------*/
void SwMultiTOXTabDialog::CreateOrUpdateExample(
    sal_uInt16 nTOXIndex, sal_uInt16 nPage, sal_uInt16 nCurrentLevel)
{
    if(!pExampleFrame || !pExampleFrame->IsInitialized())
        return;

    const char* __FAR_DATA IndexServiceNames[] =
    {
        "com.sun.star.text.DocumentIndex",
        "com.sun.star.text.UserIndex",
        "com.sun.star.text.ContentIndex",
        "com.sun.star.text.IllustrationsIndex",
        "com.sun.star.text.ObjectIndex",
        "com.sun.star.text.TableIndex",
        "com.sun.star.text.Bibliography"
    };

    try
    {
        DBG_ASSERT(pxIndexSectionsArr[nTOXIndex] &&
                        pxIndexSectionsArr[nTOXIndex]->xContainerSection.is(),
                            "Section not created")
         uno::Reference< frame::XModel > & xModel = pExampleFrame->GetModel();
        sal_Bool bInitialCreate = sal_True;
        if(!pxIndexSectionsArr[nTOXIndex]->xDocumentIndex.is())
        {
            bInitialCreate = sal_True;
            if(!pxIndexSectionsArr[nTOXIndex]->xContainerSection.is())
                throw uno::RuntimeException();
            uno::Reference< text::XTextRange >  xAnchor = pxIndexSectionsArr[nTOXIndex]->xContainerSection->getAnchor();
            xAnchor = xAnchor->getStart();
         uno::Reference< text::XTextCursor >  xCrsr = xAnchor->getText()->createTextCursorByRange(xAnchor);

         uno::Reference< lang::XMultiServiceFactory >  xFact(xModel, uno::UNO_QUERY);

            String sIndexTypeName;
            sIndexTypeName.AssignAscii( IndexServiceNames[
                    nTOXIndex <= TOX_AUTHORITIES ? nTOXIndex : TOX_USER] );
            pxIndexSectionsArr[nTOXIndex]->xDocumentIndex = uno::Reference< text::XDocumentIndex > (xFact->createInstance(
                                                    sIndexTypeName), uno::UNO_QUERY);
         uno::Reference< text::XTextContent >  xContent(pxIndexSectionsArr[nTOXIndex]->xDocumentIndex, uno::UNO_QUERY);
         uno::Reference< text::XTextRange >  xRg(xCrsr, uno::UNO_QUERY);
            xCrsr->getText()->insertTextContent(xRg, xContent, sal_False);
        }
        OUString uIsVisible(C2U(SW_PROP_NAME_STR(UNO_NAME_IS_VISIBLE)));
        for(sal_uInt16 i = 0 ; i <= TOX_AUTHORITIES; i++)
        {
         uno::Reference< beans::XPropertySet >  xSectPr(pxIndexSectionsArr[i]->xContainerSection, uno::UNO_QUERY);
            uno::Any aVal;

            if(xSectPr.is())
            {
                BOOL bTemp = i == nTOXIndex;
                aVal.setValue(&bTemp, ::getBooleanCppuType());
                xSectPr->setPropertyValue(uIsVisible, aVal);
            }
        }
        // set properties
     uno::Reference< beans::XPropertySet >  xIdxProps(pxIndexSectionsArr[nTOXIndex]->xDocumentIndex, uno::UNO_QUERY);
     uno::Reference< beans::XPropertySetInfo >  xInfo = xIdxProps->getPropertySetInfo();
        SwTOXDescription& rDesc = GetTOXDescription(eCurrentTOXType);
        sal_uInt16 nIdxOptions = rDesc.GetIndexOptions();
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_SELECT)
        {
            //title
            if(rDesc.GetTitle())
                lcl_SetProp(xInfo, xIdxProps, UNO_NAME_TITLE, *rDesc.GetTitle());

            //stylenames
            sal_uInt16  nContentOptions = rDesc.GetContentOptions();
            if(xInfo->hasPropertyByName(C2U(SW_PROP_NAME_STR(UNO_NAME_LEVEL_PARAGRAPH_STYLES))))
            {
                sal_Bool bOn = 0!=(nContentOptions&TOX_TEMPLATE    );
                uno::Any aStyleNames(xIdxProps->getPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LEVEL_PARAGRAPH_STYLES))));
                uno::Reference< container::XIndexReplace >  xAcc;
                aStyleNames >>= xAcc;

                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    String sLevel;
                    if(bOn)
                        sLevel = rDesc.GetStyleNames(i);
                    sal_uInt16 nStyles = sLevel.GetTokenCount(TOX_STYLE_DELIMITER);
                    uno::Sequence<OUString> aStyles(nStyles);
                    OUString* pArr = aStyles.getArray();
                    for(sal_uInt16 nStyle = 0; nStyle < nStyles; nStyle++)
                        pArr[nStyle] = sLevel.GetToken(nStyle, TOX_STYLE_DELIMITER);
                    uno::Any aAny(&aStyles, ::getCppuType((uno::Sequence<OUString>*)0));
                    xAcc->replaceByIndex(i, aAny);
                }
            }
            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LEVEL, (sal_Int16)rDesc.GetLevel());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_MARKS,           0!=(nContentOptions&TOX_MARK        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_OUTLINE,         0!=(nContentOptions&TOX_OUTLINELEVEL));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS,0!=(nContentOptions&TOX_OLE            ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_TABLES ,         0!=(nContentOptions&TOX_TABLE          ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS, 0!=(nContentOptions&TOX_GRAPHIC        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_TEXT_FRAMES,     0!=(nContentOptions&TOX_FRAME          ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_LABELS,          0!=(nContentOptions&TOX_SEQUENCE       ));

            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_CHAPTER, rDesc.IsFromChapter());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_PROTECTED, rDesc.IsReadonly());

            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_COMBINED_ENTRIES,        0 != (nIdxOptions&TOI_SAME_ENTRY        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_P_P,                     0 != (nIdxOptions&TOI_FF                   ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_CASE_SENSITIVE,           0 != (nIdxOptions&TOI_CASE_SENSITIVE     ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_KEY_AS_ENTRY,            0 != (nIdxOptions&TOI_KEY_AS_ENTRY     ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_ALPHABETICAL_SEPARATORS, 0 != (nIdxOptions&TOI_ALPHA_DELIMITTER));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_DASH,                    0 != (nIdxOptions&TOI_DASH             ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_UPPER_CASE,              0 != (nIdxOptions&TOI_INITIAL_CAPS     ));

            String aTmpName( SwStyleNameMapper::GetSpecialExtraProgName( rDesc.GetSequenceName() ) );
            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LABEL_CATEGORY, aTmpName );
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_LABELS,  !rDesc.IsCreateFromObjectNames());

            sal_Int16 nSet = text::ChapterFormat::NAME_NUMBER;
            switch (rDesc.GetCaptionDisplay())
            {
                case CAPTION_COMPLETE:  nSet = text::ChapterFormat::NAME_NUMBER;break;
                case CAPTION_NUMBER  :  nSet = text::ChapterFormat::NUMBER; break;
                case CAPTION_TEXT    :  nSet = text::ChapterFormat::NAME;      break;
            }
            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LABEL_DISPLAY_TYPE, nSet);

            sal_uInt16  nOLEOptions = rDesc.GetOLEOptions();
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_MATH,   0 != (TOO_MATH  &nOLEOptions           ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_CHART,  0 != (TOO_CHART &nOLEOptions       ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_CALC,   0 != (TOO_CALC  &nOLEOptions           ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_DRAW,   0 != (TOO_DRAW_IMPRESS&nOLEOptions));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS, 0 != (TOO_OTHER|nOLEOptions       ));

            //lcl_SetBOOLProp(xInfo, xIdxProps, , rDesc.IsLevelFromChapter());
        }
        const SwForm* pForm = GetForm(eCurrentTOXType);
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_ENTRY)
        {
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_COMMA_SEPARATED, pForm->IsCommaSeparated());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_ALPHABETICAL_SEPARATORS, 0 != (nIdxOptions&TOI_ALPHA_DELIMITTER));
            sal_uInt16 nStartLevel = USHRT_MAX;
            sal_uInt16 nEndLevel = USHRT_MAX;
            if(nCurrentLevel < pForm->GetFormMax())
                nStartLevel = nEndLevel = nCurrentLevel;
            else
            {
                nStartLevel = 0;
                nEndLevel = pForm->GetFormMax() - 1;
            }

            if(xInfo->hasPropertyByName(C2U(SW_PROP_NAME_STR(UNO_NAME_LEVEL_FORMAT))))
            {
                for(sal_uInt16 nCurrLevel = nStartLevel; nCurrLevel <= nEndLevel; nCurrLevel++)
                {
                    String sTokenType;
                    uno::Sequence< beans::PropertyValues> aSequPropVals(10);
                    long nTokenIndex = 0;
                    long nParamCount = 2;
                    sal_Bool bTabRightAligned = sal_False;
                    SwFormTokenEnumerator aTokenEnum(pForm->GetPattern(nCurrLevel));
                    while(aTokenEnum.HasNextToken())
                    {
                        if( aSequPropVals.getLength() <= nTokenIndex)
                            aSequPropVals.realloc(nTokenIndex + 10);

                        SwFormToken aToken = aTokenEnum.GetNextToken();
                        switch(aToken.eTokenType)
                        {
                            case TOKEN_ENTRY_NO     :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenEntryNumber"));
                                // fuer Inhaltsverzeichnis Numerierung
                            break;
                            case TOKEN_ENTRY_TEXT   :
                            case TOKEN_ENTRY        :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenEntryText"));
                            break;
                            case TOKEN_TAB_STOP     :
                                nParamCount += 3;
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenTabStop"));
                            break;
                            case TOKEN_TEXT         :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenText"));
                                nParamCount += 1;
                            break;
                            case TOKEN_PAGE_NUMS    :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenPageNumber"));
                            break;
                            case TOKEN_CHAPTER_INFO :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenChapterInfo"));
                            {
                                aToken.nChapterFormat;
                            }
                            break;
                            case TOKEN_LINK_START   :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenHyperlinkStart"));
                            break;
                            case TOKEN_LINK_END     :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenHyperlinkEnd"));
                            break;
                            case TOKEN_AUTHORITY :
                            {
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                    "TokenBibliographyDataField"));
                                ToxAuthorityField eField = (ToxAuthorityField)aToken.nAuthorityField;
                            }
                            break;
                        }
                        beans::PropertyValues aPropVals(nParamCount);
                        beans::PropertyValue* pPropValArr = aPropVals.getArray();
                        pPropValArr[0].Name = C2U("TokenType");
                        pPropValArr[0].Value <<= OUString(sTokenType);
                        pPropValArr[1].Name = C2U("CharacterStyleName");
                        pPropValArr[1].Value <<= OUString(aToken.sCharStyleName);
                        if(TOKEN_TAB_STOP == aToken.eTokenType)
                        {
                            pPropValArr[2].Name = C2U("TabStopRightAligned");
                            BOOL bTemp = SVX_TAB_ADJUST_END == aToken.eTabAlign;
                            pPropValArr[2].Value.setValue(&bTemp, ::getBooleanCppuType());
                            pPropValArr[3].Name = C2U("TabStopFillCharacter");
                            pPropValArr[3].Value <<= OUString(aToken.cTabFillChar);
                            pPropValArr[4].Name = C2U("TabStopPosition");
                            SwTwips nTempPos = aToken.nTabStopPosition >= 0 ?
                                                            aToken.nTabStopPosition : 0;
                            nTempPos = TWIP_TO_MM100(nTempPos);
                            pPropValArr[4].Value <<= (sal_Int32)nTempPos;
                        }
                        else if(TOKEN_TEXT == aToken.eTokenType)
                        {
                            pPropValArr[2].Name = C2U("Text");
                            pPropValArr[2].Value <<= OUString(aToken.sText);
                        }
                    beans::PropertyValues* pValues = aSequPropVals.getArray();
                        pValues[nTokenIndex] = aPropVals;
                        nTokenIndex++;
                    }
                    aSequPropVals.realloc(nTokenIndex);

                    uno::Any aFormatAccess = xIdxProps->getPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LEVEL_FORMAT)));
                    DBG_ASSERT(aFormatAccess.getValueType() == ::getCppuType((uno::Reference<container::XIndexReplace>*)0),
                        "wrong property type")


                    uno::Reference< container::XIndexReplace >  xFormatAccess;
                    aFormatAccess >>= xFormatAccess;
                    uno::Any aLevelProp(&aSequPropVals, ::getCppuType((uno::Sequence<beans::PropertyValues>*)0));
                    xFormatAccess->replaceByIndex(nCurrLevel, aLevelProp);
                }
            }
        }
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_STYLES)
        {
            lcl_SetProp(xInfo, xIdxProps, "ParaStyleHeading", pForm->GetTemplate(0));
            sal_uInt16 nOffset = 0;
            sal_uInt16 nEndLevel = 2;
            switch(eCurrentTOXType.eType)
            {
                case  TOX_INDEX:
                {
                    nOffset = 1;
                    nEndLevel = 4;
                    lcl_SetProp(xInfo, xIdxProps, "ParaStyleSeparator", pForm->GetTemplate(1));
                }
                break;
                case TOX_CONTENT :
                    nEndLevel = 11;
                break;

            }
            for(sal_uInt16 i = 1; i < nEndLevel; i++)
            {
                String sPropName(C2S("ParaStyleLevel"));
                sPropName += String::CreateFromInt32( i );
                lcl_SetProp(xInfo,
                    xIdxProps,
                    ByteString(sPropName, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                    pForm->GetTemplate(i + nOffset));
            }
        }
/*
    const String&   GetAutoMarkURL() const { return sAutoMarkURL;}
    const String&   GetMainEntryCharStyle() const {return sMainEntryCharStyle;}

    String          GetAuthBrackets() const {return sAuthBrackets;}
    sal_Bool            IsAuthSequence() const {return bIsAuthSequence;}
    sal_Bool            IsSortByDocument()const {return bSortByDocument ;}

    SwTOXSortKey GetSortKey1() const {return eSortKey1;}
    SwTOXSortKey GetSortKey2() const {return eSortKey2;}
    SwTOXSortKey GetSortKey3() const {return eSortKey3;}
*/
        //
        pxIndexSectionsArr[nTOXIndex]->xDocumentIndex->update();

//#ifdef DEBUG
//      uno::Reference< frame::XStorable >  xStor(xModel, uno::UNO_QUERY);
//      String sURL("file:///e|/temp/sw/idxexample.sdw");
//   uno::Sequence< beans::PropertyValue > aArgs(0);
//      xStor->storeToURL(S2U(sURL), aArgs);
//#endif

    }
    catch(Exception&)
    {
        DBG_ERROR("::CreateExample() - exception caught")
    }
}

