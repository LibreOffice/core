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


#include "XMLTextPropertySetContext.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <xmloff/families.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlement.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/style/XStyle.hpp>

#include <o3tl/any.hxx>

#include <sax/tools/converter.hxx>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <vector>

#include <xmlsdtypes.hxx>
#include <xmloff/xmlerror.hxx>


using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;

const SvXMLEnumMapEntry<sal_uInt16> aCategoryMap[] =
{
    { XML_TEXT,     ParagraphStyleCategory::TEXT },
    { XML_CHAPTER,  ParagraphStyleCategory::CHAPTER },
    { XML_LIST,     ParagraphStyleCategory::LIST },
    { XML_INDEX,    ParagraphStyleCategory::INDEX },
    { XML_EXTRA,    ParagraphStyleCategory::EXTRA },
    { XML_HTML,     ParagraphStyleCategory::HTML },
    { XML_TOKEN_INVALID, 0 }
};

void XMLTextStyleContext::SetAttribute( sal_Int32 nElement,
                                        const OUString& rValue )
{
    switch (nElement)
    {
        case XML_ELEMENT(STYLE, XML_AUTO_UPDATE):
        {
            if( IsXMLToken( rValue, XML_TRUE ) )
                m_isAutoUpdate = true;
            break;
        }
        case XML_ELEMENT(STYLE, XML_LIST_STYLE_NAME):
        {
            m_sListStyleName = rValue;
            // Inherited paragraph style lost information about unset numbering (#i69523#)
            m_bListStyleSet = true;
            break;
        }
        case XML_ELEMENT(STYLE, XML_MASTER_PAGE_NAME):
        {
            m_sMasterPageName = rValue;
            m_bHasMasterPageName = true;
            break;
        }
        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
            m_sDataStyleName = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_CLASS):
            m_sCategoryVal = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_DEFAULT_OUTLINE_LEVEL):
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber( nTmp, rValue ) &&
                0 <= nTmp && nTmp <= 10 )
            {
                m_nOutlineLevel = static_cast<sal_Int8>(nTmp);
            }
            break;
        }
        case XML_ELEMENT(STYLE, XML_LIST_LEVEL):
        {
            sal_Int32 nTmp;
            // The spec is positiveInteger (1-based), but the implementation is 0-based.
            if (sax::Converter::convertNumber(nTmp, rValue) && nTmp > 0 && nTmp <= 10)
            {
                m_aListLevel.emplace(--nTmp);
            }
            break;
        }
        default:
            XMLPropStyleContext::SetAttribute( nElement, rValue );
    }
}

XMLTextStyleContext::XMLTextStyleContext( SvXMLImport& rImport,
        SvXMLStylesContext& rStyles, XmlStyleFamily nFamily,
        bool bDefaultStyle )
:   XMLPropStyleContext( rImport, rStyles, nFamily, bDefaultStyle )
,   m_nOutlineLevel( -1 )
,   m_isAutoUpdate( false )
,   m_bHasMasterPageName( false )
,   m_bHasCombinedCharactersLetter( false )
// Inherited paragraph style lost information about unset numbering (#i69523#)
,   m_bListStyleSet( false )
{
}

XMLTextStyleContext::~XMLTextStyleContext()
{}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_STYLE) )
    {
        sal_Int32 nLocalName = nElement & TOKEN_MASK;
        sal_uInt32 nFamily = 0;
        if( nLocalName == XML_TEXT_PROPERTIES )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( nLocalName == XML_PARAGRAPH_PROPERTIES )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( nLocalName == XML_SECTION_PROPERTIES )
            nFamily = XML_TYPE_PROP_SECTION;
        else if( IsDefaultStyle() && nLocalName == XML_TABLE_PROPERTIES )
            nFamily = XML_TYPE_PROP_TABLE;
        else if( IsDefaultStyle() && nLocalName == XML_TABLE_ROW_PROPERTIES )
            nFamily = XML_TYPE_PROP_TABLE_ROW;
        if( nFamily )
        {
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                return new XMLTextPropertySetContext( GetImport(), nElement, xAttrList,
                                                        nFamily,
                                                        GetProperties(),
                                                        xImpPrMap,
                                                        m_sDropCapTextStyleName);
        }
    }
    else if ( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        // create and remember events import context
        // (for delayed processing of events)
        m_xEventContext.set(new XMLEventsImportContext( GetImport() ));
        return m_xEventContext;
    }

    return XMLPropStyleContext::createFastChildContext( nElement, xAttrList );
}

void XMLTextStyleContext::CreateAndInsert( bool bOverwrite )
{
    XMLPropStyleContext::CreateAndInsert( bOverwrite );
    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() || !(bOverwrite || IsNew()) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();

    OUString const sIsAutoUpdate("IsAutoUpdate");
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        xPropSet->setPropertyValue( sIsAutoUpdate, Any(m_isAutoUpdate) );
    }

    sal_uInt16 nCategory = ParagraphStyleCategory::TEXT;
    if(  XmlStyleFamily::TEXT_PARAGRAPH == GetFamily() &&
         !m_sCategoryVal.isEmpty() && xStyle->isUserDefined() &&
         xPropSetInfo->hasPropertyByName("Category") &&
        SvXMLUnitConverter::convertEnum( nCategory, m_sCategoryVal, aCategoryMap))
    {
        xPropSet->setPropertyValue("Category", Any(static_cast<sal_Int16>(nCategory)));
    }

    // tell the style about it's events (if applicable)
    if (m_xEventContext.is())
    {
        // pass events into event supplier
        Reference<document::XEventsSupplier> xEventsSupplier(xStyle,UNO_QUERY);
        m_xEventContext->SetEvents(xEventsSupplier);
        m_xEventContext.clear();
    }

    // XML import: reconstruction of assignment of paragraph style to outline levels (#i69629#)
    if (m_nOutlineLevel > 0)
    {
        GetImport().GetTextImport()->AddOutlineStyleCandidate(m_nOutlineLevel,
                                                      GetDisplayName() );
    }
}

void XMLTextStyleContext::SetDefaults( )
{
    if( ( GetFamily() == XmlStyleFamily::TEXT_PARAGRAPH ) ||
        ( GetFamily() == XmlStyleFamily::TABLE_TABLE ) ||
        ( GetFamily() == XmlStyleFamily::TABLE_ROW ) )
    {
        Reference < XMultiServiceFactory > xFactory ( GetImport().GetModel(), UNO_QUERY);
        if (xFactory.is())
        {
            Reference < XInterface > xInt = xFactory->createInstance( "com.sun.star.text.Defaults" );
            Reference < XPropertySet > xProperties ( xInt, UNO_QUERY );
            if ( xProperties.is() )
                FillPropertySet ( xProperties );
        }
    }
}

void XMLTextStyleContext::Finish( bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );

    Reference < XStyle > xStyle = GetStyle();
    // Consider set empty list style (#i69523#)
    if ( !( m_bListStyleSet ||
            m_nOutlineLevel >= 0 ||
            !m_sDropCapTextStyleName.isEmpty() ||
            m_bHasMasterPageName ) ||
         !xStyle.is() ||
         !( bOverwrite || IsNew() ) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();

    OUString const sOutlineLevel("OutlineLevel");
    if( xPropSetInfo->hasPropertyByName( sOutlineLevel ))
    {
        if (m_nOutlineLevel >= 0)
        {
            xPropSet->setPropertyValue( sOutlineLevel, Any(m_nOutlineLevel) );
        }
    }

    // Consider set empty list style (#i69523#)
    OUString const sNumberingStyleName("NumberingStyleName");
    if (m_bListStyleSet &&
         xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
    {
        /* Only for text document from version prior OOo 2.1 resp. SO 8 PU5:
           - Do not apply list style, if paragraph style has a default outline
             level > 0 and thus, will be assigned to the corresponding list
             level of the outline style. (#i70223#)
        */
        bool bApplyListStyle( true );
        if (m_nOutlineLevel > 0)
        {
            if ( GetImport().IsTextDocInOOoFileFormat() )
            {
                bApplyListStyle = false;
            }
            else
            {
                sal_Int32 nUPD( 0 );
                sal_Int32 nBuild( 0 );
                // Check explicitly on certain versions (#i86058#)
                if ( GetImport().getBuildIds( nUPD, nBuild ) &&
                     ( ( nUPD == 641 ) || ( nUPD == 645 ) || // prior OOo 2.0
                       ( nUPD == 680 && nBuild <= 9073 ) ) ) // OOo 2.0 - OOo 2.0.4
                {
                    bApplyListStyle = false;
                }
            }
        }

        if ( bApplyListStyle )
        {
            if (m_sListStyleName.isEmpty())
            {
                xPropSet->setPropertyValue(sNumberingStyleName, Any(m_sListStyleName)); /* empty string */
            }
            else
            {
                // change list style name to display name
                OUString sDisplayListStyleName(
                    GetImport().GetStyleDisplayName(XmlStyleFamily::TEXT_LIST,
                                                  m_sListStyleName));
                // The families container must exist
                const Reference < XNameContainer >& rNumStyles =
                    GetImport().GetTextImport()->GetNumberingStyles();
    //            if( rNumStyles.is() && rNumStyles->hasByName( sDisplayListStyleName ) &&
    //                xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
                if ( rNumStyles.is() &&
                     rNumStyles->hasByName( sDisplayListStyleName ) )
                {
                    xPropSet->setPropertyValue( sNumberingStyleName, Any(sDisplayListStyleName) );
                }
            }

            if (m_aListLevel.has_value())
            {
                xPropSet->setPropertyValue("NumberingLevel", uno::Any(*m_aListLevel));
            }
        }
    }

    if (!m_sDropCapTextStyleName.isEmpty())
    {
        // change list style name to display name
        OUString sDisplayDropCapTextStyleName(
            GetImport().GetStyleDisplayName( XmlStyleFamily::TEXT_TEXT,
                                          m_sDropCapTextStyleName));
        // The families container must exist
        const Reference < XNameContainer >& rTextStyles =
            GetImport().GetTextImport()->GetTextStyles();
        if( rTextStyles.is() &&
            rTextStyles->hasByName( sDisplayDropCapTextStyleName ) &&
            xPropSetInfo->hasPropertyByName("DropCapCharStyleName"))
        {
            xPropSet->setPropertyValue("DropCapCharStyleName", Any(sDisplayDropCapTextStyleName));
        }
    }

    if (!m_bHasMasterPageName)
        return;

    OUString sDisplayName(
        GetImport().GetStyleDisplayName(
                        XmlStyleFamily::MASTER_PAGE, m_sMasterPageName));
    // The families container must exist
    const Reference < XNameContainer >& rPageStyles =
        GetImport().GetTextImport()->GetPageStyles();

    OUString const sPageDescName("PageDescName");
    if( ( sDisplayName.isEmpty() ||
          (rPageStyles.is() &&
           rPageStyles->hasByName( sDisplayName )) ) &&
        xPropSetInfo->hasPropertyByName( sPageDescName ) )
    {
        xPropSet->setPropertyValue( sPageDescName, Any(sDisplayName) );
    }
}

void XMLTextStyleContext::FillPropertySet(
    const Reference<XPropertySet > & rPropSet )
{
    // imitate the FillPropertySet of the super class, so we get a chance to
    // catch the combined characters attribute

    // imitate XMLPropStyleContext::FillPropertySet(...)
    SvXMLStylesContext* pSvXMLStylesContext = GetStyles();
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = pSvXMLStylesContext->GetImportPropertyMapper(GetFamily());
    DBG_ASSERT(xImpPrMap.is(),"Where is the import prop mapper?");

    if(!xImpPrMap.is())
        return;

    // imitate SvXMLImportPropertyMapper::FillPropertySet(...)
    // The reason for this is that we have no other way to
    // efficiently intercept the value of combined characters. To
    // get that value, we could iterate through the map once more,
    // but instead we chose to insert the code into this
    // iteration. I haven't been able to come up with a much more
    // intelligent solution.
    struct ContextID_Index_Pair aContextIDs[] =
    {
        { CTF_COMBINED_CHARACTERS_FIELD, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_KEEP_TOGETHER, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_BORDER_MODEL, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_TEXT_DISPLAY, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_FONTFAMILYNAME, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_FONTFAMILYNAME_CJK, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_FONTFAMILYNAME_CTL, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },

        //UUU need special handling for DrawingLayer FillStyle names
        { CTF_FILLGRADIENTNAME, -1, drawing::FillStyle::FillStyle_GRADIENT },
        { CTF_FILLTRANSNAME, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE },
        { CTF_FILLHATCHNAME, -1, drawing::FillStyle::FillStyle_HATCH },
        { CTF_FILLBITMAPNAME, -1, drawing::FillStyle::FillStyle_BITMAP },

        { -1, -1, drawing::FillStyle::FillStyle_MAKE_FIXED_SIZE }
    };

    // the style families associated with the same index modulo 4
    static const XmlStyleFamily aFamilies[] =
    {
        XmlStyleFamily::SD_GRADIENT_ID,
        XmlStyleFamily::SD_GRADIENT_ID,
        XmlStyleFamily::SD_HATCH_ID,
        XmlStyleFamily::SD_FILL_IMAGE_ID
    };

    // get property set info
    Reference< XPropertySetInfo > xInfo;
    rtl::Reference< XMLPropertySetMapper > rPropMapper;
    bool bAutomatic = false;

    if(pSvXMLStylesContext->IsAutomaticStyle() &&
        (XmlStyleFamily::TEXT_TEXT == GetFamily() || XmlStyleFamily::TEXT_PARAGRAPH == GetFamily()))
    {
        bAutomatic = true;

        if( !GetAutoName().isEmpty() )
        {
            OUString sAutoProp = ( GetFamily() == XmlStyleFamily::TEXT_TEXT ) ?
                OUString( "CharAutoStyleName" ):
                OUString( "ParaAutoStyleName" );

            try
            {
                if(!xInfo.is())
                {
                    xInfo = rPropSet->getPropertySetInfo();
                }

                if ( xInfo->hasPropertyByName( sAutoProp ) )
                {
                    rPropSet->setPropertyValue( sAutoProp, makeAny(GetAutoName()) );
                }
                else
                {
                    bAutomatic = false;
                }
            }
            catch( const RuntimeException& ) { throw; }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.text");
                bAutomatic = false;
            }
        }
    }

    if( bAutomatic )
    {
        xImpPrMap->CheckSpecialContext( GetProperties(), rPropSet, aContextIDs );
    }
    else
    {
        xImpPrMap->FillPropertySet( GetProperties(), rPropSet, aContextIDs );
    }

    sal_Int32 nIndex = aContextIDs[0].nIndex;

    // have we found a combined characters
    if ( nIndex != -1 )
    {
        Any& rAny = GetProperties()[nIndex].maValue;
        bool bVal = *o3tl::doAccess<bool>(rAny);
        m_bHasCombinedCharactersLetter = bVal;
    }

    // keep-together: the application default is different from
    // the file format default. Hence, if we always set this
    // value; if we didn't find one, we'll set to false, the file
    // format default.
    // border-model: same
    if(IsDefaultStyle() && XmlStyleFamily::TABLE_ROW == GetFamily())
    {
        OUString sIsSplitAllowed("IsSplitAllowed");
        SAL_WARN_IF( !rPropSet->getPropertySetInfo()->hasPropertyByName( sIsSplitAllowed ), "xmloff", "property missing?" );
        rPropSet->setPropertyValue(
            sIsSplitAllowed,
            (aContextIDs[1].nIndex == -1) ? makeAny( false ) : GetProperties()[aContextIDs[1].nIndex].maValue );
    }

    if(IsDefaultStyle() && XmlStyleFamily::TABLE_TABLE == GetFamily())
    {
        OUString sCollapsingBorders("CollapsingBorders");
        SAL_WARN_IF( !rPropSet->getPropertySetInfo()->hasPropertyByName( sCollapsingBorders ), "xmloff", "property missing?" );
        rPropSet->setPropertyValue(
            sCollapsingBorders,
            (aContextIDs[2].nIndex == -1)
            ? makeAny( false )
            : GetProperties()[aContextIDs[2].nIndex].maValue );
    }


    // iterate over aContextIDs entries, start with 3, prev ones are already used above
    for(sal_uInt16 i(3); aContextIDs[i].nContextID != -1; i++)
    {
        nIndex = aContextIDs[i].nIndex;

        if ( nIndex != -1 )
        {
            // Found!
            struct XMLPropertyState& rState = GetProperties()[nIndex];

            switch(aContextIDs[i].nContextID)
            {
                case CTF_FILLGRADIENTNAME:
                case CTF_FILLTRANSNAME:
                case CTF_FILLHATCHNAME:
                case CTF_FILLBITMAPNAME:
                {
                    // DrawingLayer FillStyle name needs to be mapped to DisplayName
                    OUString sStyleName;
                    rState.maValue >>= sStyleName;

                    // translate the used name from ODF intern to the name used in the Model
                    sStyleName = GetImport().GetStyleDisplayName(aFamilies[i - 7], sStyleName);

                    if(bAutomatic)
                    {
                        // in this case the rPropSet got not really filled since above the call to
                        // CheckSpecialContext was used and not FillPropertySet, thus the below call to
                        // setPropertyValue can fail/will not be useful (e.g. when the rPropSet
                        // is a SwXTextCursor).
                        // This happens for AutoStyles which are already filled in XMLPropStyleContext::CreateAndInsert,
                        // thus the whole mechanism based on _ContextID_Index_Pair will not work
                        // in that case. Thus the slots which need to be converted already get
                        // converted there (it's called first) and not here (see
                        // translateNameBasedDrawingLayerFillStyleDefinitionsToStyleDisplayNames)
                        // For convenience, still Write back the corrected value to the XMLPropertyState entry
                        rState.maValue <<= sStyleName;
                        break;
                    }

                    if (::xmloff::IsIgnoreFillStyleNamedItem(rPropSet, aContextIDs[i].nExpectedFillStyle))
                    {
                        SAL_INFO("xmloff.style", "XMLTextStyleContext: dropping fill named item: " << sStyleName);
                        break; // ignore it, it's not used
                    }

                    // Still needed if it's not an AutomaticStyle (!)
                    try
                    {
                        if(!rPropMapper.is())
                        {
                            rPropMapper = xImpPrMap->getPropertySetMapper();
                        }

                        // set property
                        const OUString& rPropertyName = rPropMapper->GetEntryAPIName(rState.mnIndex);

                        if(!xInfo.is())
                        {
                            xInfo = rPropSet->getPropertySetInfo();
                        }

                        if(xInfo->hasPropertyByName(rPropertyName))
                        {
                            rPropSet->setPropertyValue(rPropertyName,Any(sStyleName));
                        }
                    }
                    catch(css::lang::IllegalArgumentException& e)
                    {
                        Sequence<OUString> aSeq { sStyleName };
                        GetImport().SetError(XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING, aSeq, e.Message, nullptr);
                    }
                    break;
                }
                default:
                {
                    // check for StarBats and StarMath fonts
                    Any rAny = rState.maValue;
                    sal_Int32 nMapperIndex = rState.mnIndex;

                    // Now check for font name in rState and set corrected value,
                    // if necessary.
                    OUString sFontName;
                    rAny >>= sFontName;

                    if ( !sFontName.isEmpty() )
                    {
                        if ( sFontName.equalsIgnoreAsciiCase( "StarBats" ) ||
                             sFontName.equalsIgnoreAsciiCase( "StarMath" ) )
                        {
                            // construct new value
                            sFontName = "StarSymbol";
                            Any aAny(rAny);
                            aAny <<= sFontName;

                            if(!rPropMapper.is())
                            {
                                rPropMapper = xImpPrMap->getPropertySetMapper();
                            }

                            // set property
                            OUString rPropertyName(rPropMapper->GetEntryAPIName(nMapperIndex));

                            if(!xInfo.is())
                            {
                                xInfo = rPropSet->getPropertySetInfo();
                            }

                            if(xInfo->hasPropertyByName(rPropertyName))
                            {
                                rPropSet->setPropertyValue(rPropertyName,aAny);
                            }
                        }
                        // else: "normal" style name -> no correction is necessary
                    }
                    // else: no style name found -> illegal value -> ignore
                }
            }
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
