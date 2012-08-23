/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <rsc/rscsfx.hxx>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <comphelper/string.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <cnttab.hxx>
#include <poolfmt.hxx>
#include <unoprnms.hxx>
#include <unotools.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>

#include <cmdid.h>
#include <utlui.hrc>
#include <index.hrc>
#include <cnttab.hrc>
#include <globals.hrc>
#include <SwStyleNameMapper.hxx>
#include <swuicnttab.hxx>
#include <unomid.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using ::rtl::OUString;


#ifdef SW_PROP_NAME_STR
#undef SW_PROP_NAME_STR
#endif
#define SW_PROP_NAME_STR(nId) SwGetPropName((nId)).pName

void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
                           uno::Reference< XPropertySet > & xProps,
                         const char* pPropName, const String& rValue)
{
    OUString uPropName(rtl::OUString::createFromAscii(pPropName));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue <<= OUString(rValue);
        xProps->setPropertyValue(uPropName, aValue);
    }
}

void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
                           uno::Reference< XPropertySet > & xProps,
                           sal_uInt16 nId, const String& rValue)
{
    lcl_SetProp( xInfo, xProps, SW_PROP_NAME_STR(nId), rValue);
}

void lcl_SetProp( uno::Reference< XPropertySetInfo > & xInfo,
                           uno::Reference< XPropertySet > & xProps,
                           sal_uInt16 nId, sal_Int16 nValue )
{
    OUString uPropName(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(nId)));
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
                sal_uInt16 nId, sal_Bool bValue )
{
    OUString uPropName(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(nId)));
    if(xInfo->hasPropertyByName(uPropName))
    {
        uno::Any aValue;
        aValue.setValue(&bValue, ::getCppuBooleanType());
        xProps->setPropertyValue(uPropName, aValue);
    }
}

IMPL_LINK_NOARG(SwMultiTOXTabDialog, CreateExample_Hdl)
{
    try
    {
         uno::Reference< frame::XModel > & xModel = pExampleFrame->GetModel();
        uno::Reference< lang::XUnoTunnel > xDocTunnel(xModel, uno::UNO_QUERY);
        SwXTextDocument* pDoc = reinterpret_cast<SwXTextDocument*>(xDocTunnel->getSomething(SwXTextDocument::getUnoTunnelId()));

        if( pDoc )
            pDoc->GetDocShell()->_LoadStyles( *rSh.GetView().GetDocShell(), sal_True );

         uno::Reference< lang::XMultiServiceFactory >  xFact(
                                             xModel, uno::UNO_QUERY);

         uno::Reference< text::XTextSectionsSupplier >  xSectionSupplier(
                                                 xModel, uno::UNO_QUERY);
         uno::Reference< container::XNameAccess >  xSections =
                                        xSectionSupplier->getTextSections();

        rtl::OUString sSectionName("IndexSection_");
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
    catch (const Exception&)
    {
        OSL_FAIL("::CreateExample() - exception caught");
    }
    return 0;
}

void SwMultiTOXTabDialog::CreateOrUpdateExample(
    TOXTypes nTOXIndex, sal_uInt16 nPage, sal_uInt16 nCurrentLevel)
{
    if(!pExampleFrame || !pExampleFrame->IsInitialized())
        return;

    const char* IndexServiceNames[] =
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
        OSL_ENSURE(pxIndexSectionsArr[nTOXIndex] &&
                        pxIndexSectionsArr[nTOXIndex]->xContainerSection.is(),
                            "Section not created");
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
        OUString uIsVisible(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_IS_VISIBLE)));
        for(sal_uInt16 i = 0 ; i <= TOX_AUTHORITIES; i++)
        {
         uno::Reference< beans::XPropertySet >  xSectPr(pxIndexSectionsArr[i]->xContainerSection, uno::UNO_QUERY);
            uno::Any aVal;

            if(xSectPr.is())
            {
                sal_Bool bTemp = i == nTOXIndex;
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
            if(xInfo->hasPropertyByName(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LEVEL_PARAGRAPH_STYLES))))
            {
                sal_Bool bOn = 0!=(nContentOptions&nsSwTOXElement::TOX_TEMPLATE    );
                uno::Any aStyleNames(xIdxProps->getPropertyValue(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LEVEL_PARAGRAPH_STYLES))));
                uno::Reference< container::XIndexReplace >  xAcc;
                aStyleNames >>= xAcc;

                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    String sLevel;
                    if(bOn)
                        sLevel = rDesc.GetStyleNames(i);
                    sal_uInt16 nStyles = comphelper::string::getTokenCount(sLevel, TOX_STYLE_DELIMITER);
                    uno::Sequence<OUString> aStyles(nStyles);
                    OUString* pArr = aStyles.getArray();
                    for(sal_uInt16 nStyle = 0; nStyle < nStyles; nStyle++)
                        pArr[nStyle] = sLevel.GetToken(nStyle, TOX_STYLE_DELIMITER);
                    uno::Any aAny(&aStyles, ::getCppuType((uno::Sequence<OUString>*)0));
                    xAcc->replaceByIndex(i, aAny);
                }
            }
            lcl_SetProp(xInfo, xIdxProps, UNO_NAME_LEVEL, (sal_Int16)rDesc.GetLevel());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_MARKS,           0!=(nContentOptions&nsSwTOXElement::TOX_MARK        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_OUTLINE,         0!=(nContentOptions&nsSwTOXElement::TOX_OUTLINELEVEL));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS,0!=(nContentOptions&nsSwTOXElement::TOX_OLE            ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_TABLES ,         0!=(nContentOptions&nsSwTOXElement::TOX_TABLE          ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS, 0!=(nContentOptions&nsSwTOXElement::TOX_GRAPHIC        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_TEXT_FRAMES,     0!=(nContentOptions&nsSwTOXElement::TOX_FRAME          ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_LABELS,          0!=(nContentOptions&nsSwTOXElement::TOX_SEQUENCE       ));

            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_CHAPTER, rDesc.IsFromChapter());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_PROTECTED, rDesc.IsReadonly());

            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_COMBINED_ENTRIES,        0 != (nIdxOptions&nsSwTOIOptions::TOI_SAME_ENTRY        ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_P_P,                     0 != (nIdxOptions&nsSwTOIOptions::TOI_FF                   ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_CASE_SENSITIVE,           0 != (nIdxOptions&nsSwTOIOptions::TOI_CASE_SENSITIVE     ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_KEY_AS_ENTRY,            0 != (nIdxOptions&nsSwTOIOptions::TOI_KEY_AS_ENTRY     ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_ALPHABETICAL_SEPARATORS, 0 != (nIdxOptions&nsSwTOIOptions::TOI_ALPHA_DELIMITTER));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_DASH,                    0 != (nIdxOptions&nsSwTOIOptions::TOI_DASH             ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_UPPER_CASE,              0 != (nIdxOptions&nsSwTOIOptions::TOI_INITIAL_CAPS     ));

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
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_MATH,   0 != (nsSwTOOElements::TOO_MATH &nOLEOptions           ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_CHART,  0 != (nsSwTOOElements::TOO_CHART    &nOLEOptions       ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_CALC,   0 != (nsSwTOOElements::TOO_CALC &nOLEOptions           ));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_STAR_DRAW,   0 != (nsSwTOOElements::TOO_DRAW_IMPRESS&nOLEOptions));
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS, 0 != (nsSwTOOElements::TOO_OTHER|nOLEOptions       ));
        }
        const SwForm* pForm = GetForm(eCurrentTOXType);
        if(bInitialCreate || !nPage || nPage == TOX_PAGE_ENTRY)
        {
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_IS_COMMA_SEPARATED, pForm->IsCommaSeparated());
            lcl_SetBOOLProp(xInfo, xIdxProps, UNO_NAME_USE_ALPHABETICAL_SEPARATORS, 0 != (nIdxOptions&nsSwTOIOptions::TOI_ALPHA_DELIMITTER));
            sal_uInt16 nStartLevel = USHRT_MAX;
            sal_uInt16 nEndLevel = USHRT_MAX;
            if(nCurrentLevel < pForm->GetFormMax())
                nStartLevel = nEndLevel = nCurrentLevel;
            else
            {
                nStartLevel = 0;
                nEndLevel = pForm->GetFormMax() - 1;
            }

            if(xInfo->hasPropertyByName(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LEVEL_FORMAT))))
            {
                for(sal_uInt16 nCurrLevel = nStartLevel; nCurrLevel <= nEndLevel; nCurrLevel++)
                {
                    String sTokenType;
                    uno::Sequence< beans::PropertyValues> aSequPropVals(10);
                    long nTokenIndex = 0;
                    long nParamCount = 2;

                    // #i24377#
                    SwFormTokens aPattern = pForm->GetPattern(nCurrLevel);
                    SwFormTokens::iterator aIt = aPattern.begin();

                    while(aIt != aPattern.end())
                    {
                        if( aSequPropVals.getLength() <= nTokenIndex)
                            aSequPropVals.realloc(nTokenIndex + 10);

                        SwFormToken aToken = *aIt; // #i24377#
                        switch(aToken.eTokenType)
                        {
                            case TOKEN_ENTRY_NO     :
                                sTokenType.AssignAscii(RTL_CONSTASCII_STRINGPARAM(
                                                        "TokenEntryNumber"));
                                // numbering for content index
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
                            }
                            break;
                            default:; //prevent warning
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
                            sal_Bool bTemp = SVX_TAB_ADJUST_END == aToken.eTabAlign;
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

                        ++aIt; // #i24377#
                    }
                    aSequPropVals.realloc(nTokenIndex);

                    uno::Any aFormatAccess = xIdxProps->getPropertyValue(rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_LEVEL_FORMAT)));
                    OSL_ENSURE(aFormatAccess.getValueType() == ::getCppuType((uno::Reference<container::XIndexReplace>*)0),
                        "wrong property type");


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
                default:; //prevent warning
            }
            for(sal_uInt16 i = 1; i < nEndLevel; i++)
            {
                String sPropName(rtl::OUString("ParaStyleLevel"));
                sPropName += String::CreateFromInt32( i );
                lcl_SetProp(xInfo,
                    xIdxProps,
                    rtl::OUStringToOString(sPropName, RTL_TEXTENCODING_ASCII_US).getStr(),
                    pForm->GetTemplate(i + nOffset));
            }
        }
        pxIndexSectionsArr[nTOXIndex]->xDocumentIndex->update();

    }
    catch (const Exception&)
    {
        OSL_FAIL("::CreateExample() - exception caught");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
