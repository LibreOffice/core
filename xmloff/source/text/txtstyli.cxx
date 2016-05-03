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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/families.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/txtstyli.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/maptype.hxx>

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/style/XStyle.hpp>

#include <sax/tools/converter.hxx>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

//UUUU
#include <xmlsdtypes.hxx>
#include <xmloff/xmlerror.hxx>


using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;

static const SvXMLEnumMapEntry aCategoryMap[] =
{
    { XML_TEXT,     ParagraphStyleCategory::TEXT },
    { XML_CHAPTER,  ParagraphStyleCategory::CHAPTER },
    { XML_LIST,     ParagraphStyleCategory::LIST },
    { XML_INDEX,    ParagraphStyleCategory::INDEX },
    { XML_EXTRA,    ParagraphStyleCategory::EXTRA },
    { XML_HTML,     ParagraphStyleCategory::HTML },
    { XML_TOKEN_INVALID, 0 }
};

void XMLTextStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey )
    {
        // TODO: use a map here
        if( IsXMLToken( rLocalName, XML_AUTO_UPDATE ) )
        {
            if( IsXMLToken( rValue, XML_TRUE ) )
                bAutoUpdate = true;
        }
        else if( IsXMLToken( rLocalName, XML_LIST_STYLE_NAME ) )
        {
            sListStyleName = rValue;
            // Inherited paragraph style lost information about unset numbering (#i69523#)
            mbListStyleSet = true;
        }
        else if( IsXMLToken( rLocalName, XML_MASTER_PAGE_NAME ) )
        {
            sMasterPageName = rValue;
            bHasMasterPageName = true;
        }
        else if( IsXMLToken( rLocalName, XML_DATA_STYLE_NAME ) )
        {
            sDataStyleName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_CLASS ) )
        {
            sCategoryVal = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_DEFAULT_OUTLINE_LEVEL ) )
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber( nTmp, rValue ) &&
                0 <= nTmp && nTmp <= 10 )
                nOutlineLevel = static_cast< sal_Int8 >( nTmp );
        }
        else
        {
            XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
        }
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}


XMLTextStyleContext::XMLTextStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        bool bDefaultStyle )
:   XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle )
,   sIsAutoUpdate( "IsAutoUpdate" )
,   sCategory( "Category" )
,   sNumberingStyleName( "NumberingStyleName" )
,       sOutlineLevel("OutlineLevel" )
,   sDropCapCharStyleName( "DropCapCharStyleName" )
,   sPageDescName( "PageDescName" )
,   nOutlineLevel( -1 )
,   bAutoUpdate( false )
,   bHasMasterPageName( false )
,   bHasCombinedCharactersLetter( false )
// Inherited paragraph style lost information about unset numbering (#i69523#)
,   mbListStyleSet( false )
,   pEventContext( nullptr )
{
}

XMLTextStyleContext::~XMLTextStyleContext()
{
}

SvXMLImportContext *XMLTextStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        sal_uInt32 nFamily = 0;
        if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_SECTION_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_SECTION;
        else if( IsDefaultStyle() && IsXMLToken( rLocalName, XML_TABLE_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TABLE;
        else if( IsDefaultStyle() && IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TABLE_ROW;
        if( nFamily )
        {
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                pContext = new XMLTextPropertySetContext( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        nFamily,
                                                        GetProperties(),
                                                        xImpPrMap,
                                                        sDropCapTextStyleName );
        }
    }
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        // create and remember events import context
        // (for delayed processing of events)
        pEventContext = new XMLEventsImportContext( GetImport(), nPrefix,
                                                   rLocalName);
        pEventContext->AddFirstRef();
        pContext = pEventContext;
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
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
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        Any aAny;
        sal_Bool bTmp = bAutoUpdate;
        aAny.setValue( &bTmp, cppu::UnoType<bool>::get() );
        xPropSet->setPropertyValue( sIsAutoUpdate, aAny );
    }

    sal_uInt16 nCategory = ParagraphStyleCategory::TEXT;
    if(  XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() &&
         !sCategoryVal.isEmpty() && xStyle->isUserDefined() &&
         xPropSetInfo->hasPropertyByName( sCategory ) &&
          SvXMLUnitConverter::convertEnum( nCategory, sCategoryVal, aCategoryMap ) )
    {
        xPropSet->setPropertyValue( sCategory, Any((sal_Int16)nCategory) );
    }

    // tell the style about it's events (if applicable)
    if (nullptr != pEventContext)
    {
        // set event suppplier and release reference to context
        Reference<document::XEventsSupplier> xEventsSupplier(xStyle,UNO_QUERY);
        pEventContext->SetEvents(xEventsSupplier);
        pEventContext->ReleaseRef();
    }

    // XML import: reconstrution of assignment of paragraph style to outline levels (#i69629#)
    if ( nOutlineLevel > 0 )
    {
        GetImport().GetTextImport()->AddOutlineStyleCandidate( nOutlineLevel,
                                                      GetDisplayName() );
    }
}

void XMLTextStyleContext::SetDefaults( )
{
    if( ( GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ) ||
        ( GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE ) ||
        ( GetFamily() == XML_STYLE_FAMILY_TABLE_ROW ) )
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
    if ( !( mbListStyleSet ||
            nOutlineLevel >= 0 ||
            !sDropCapTextStyleName.isEmpty() ||
            bHasMasterPageName ) ||
         !xStyle.is() ||
         !( bOverwrite || IsNew() ) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();

    if( xPropSetInfo->hasPropertyByName( sOutlineLevel ))
    {
        if( nOutlineLevel >= 0 )
        {
            xPropSet->setPropertyValue( sOutlineLevel, Any(nOutlineLevel) );
        }
    }

    // Consider set empty list style (#i69523#)
    if ( mbListStyleSet &&
         xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
    {
        /* Only for text document from version prior OOo 2.1 resp. SO 8 PU5:
           - Do not apply list style, if paragraph style has a default outline
             level > 0 and thus, will be assigned to the corresponding list
             level of the outline style. (#i70223#)
        */
        bool bApplyListStyle( true );
        if ( nOutlineLevel > 0 )
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
            if ( sListStyleName.isEmpty() )
            {
                xPropSet->setPropertyValue( sNumberingStyleName, Any(sListStyleName) ); /* empty string */;
            }
            else
            {
                // change list style name to display name
                OUString sDisplayListStyleName(
                    GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_LIST,
                                                  sListStyleName ) );
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
        }
    }

    if( !sDropCapTextStyleName.isEmpty() )
    {
        // change list style name to display name
        OUString sDisplayDropCapTextStyleName(
            GetImport().GetStyleDisplayName( XML_STYLE_FAMILY_TEXT_TEXT,
                                          sDropCapTextStyleName ) );
        // The families cointaner must exist
        const Reference < XNameContainer >& rTextStyles =
            GetImport().GetTextImport()->GetTextStyles();
        if( rTextStyles.is() &&
            rTextStyles->hasByName( sDisplayDropCapTextStyleName ) &&
            xPropSetInfo->hasPropertyByName( sDropCapCharStyleName ) )
        {
            xPropSet->setPropertyValue( sDropCapCharStyleName, Any(sDisplayDropCapTextStyleName) );
        }
    }

    if( bHasMasterPageName )
    {
        OUString sDisplayName(
            GetImport().GetStyleDisplayName(
                            XML_STYLE_FAMILY_MASTER_PAGE, sMasterPageName ) );
        // The families cointaner must exist
        const Reference < XNameContainer >& rPageStyles =
            GetImport().GetTextImport()->GetPageStyles();
        if( ( sDisplayName.isEmpty() ||
              (rPageStyles.is() &&
               rPageStyles->hasByName( sDisplayName )) ) &&
            xPropSetInfo->hasPropertyByName( sPageDescName ) )
        {
            xPropSet->setPropertyValue( sPageDescName, Any(sDisplayName) );
        }
    }
}

void XMLTextStyleContext::FillPropertySet(
    const Reference<XPropertySet > & rPropSet )
{
    // imitate the FillPropertySet of the super class, so we get a chance to
    // catch the combined characters attribute

    // imitate XMLPropStyleContext::FillPropertySet(...)
    SvXMLStylesContext* pSvXMLStylesContext = static_cast< SvXMLStylesContext* >(GetStyles());
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = pSvXMLStylesContext->GetImportPropertyMapper(GetFamily());
    DBG_ASSERT(xImpPrMap.is(),"Where is the import prop mapper?");

    if(xImpPrMap.is())
    {
        // imitate SvXMLImportPropertyMapper::FillPropertySet(...)
        // The reason for this is that we have no other way to
        // efficiently intercept the value of combined characters. To
        // get that value, we could iterate through the map once more,
        // but instead we chose to insert the code into this
        // iteration. I haven't been able to come up with a much more
        // intelligent solution.
        struct _ContextID_Index_Pair aContextIDs[] =
        {
            { CTF_COMBINED_CHARACTERS_FIELD, -1 },
            { CTF_KEEP_TOGETHER, -1 },
            { CTF_BORDER_MODEL, -1 },
            { CTF_TEXT_DISPLAY, -1 },
            { CTF_FONTFAMILYNAME, -1 },
            { CTF_FONTFAMILYNAME_CJK, -1 },
            { CTF_FONTFAMILYNAME_CTL, -1 },

            //UUU need special handling for DrawingLayer FillStyle names
            { CTF_FILLGRADIENTNAME, -1 },
            { CTF_FILLTRANSNAME, -1 },
            { CTF_FILLHATCHNAME, -1 },
            { CTF_FILLBITMAPNAME, -1 },

            { -1, -1 }
        };

        // the style families associated with the same index modulo 4
        static sal_uInt16 aFamilies[] =
        {
            XML_STYLE_FAMILY_SD_GRADIENT_ID,
            XML_STYLE_FAMILY_SD_GRADIENT_ID,
            XML_STYLE_FAMILY_SD_HATCH_ID,
            XML_STYLE_FAMILY_SD_FILL_IMAGE_ID
        };

        // get property set info
        Reference< XPropertySetInfo > xInfo;
        rtl::Reference< XMLPropertySetMapper > rPropMapper;
        bool bAutomatic = false;

        if(pSvXMLStylesContext->IsAutomaticStyle() &&
            (XML_STYLE_FAMILY_TEXT_TEXT == GetFamily() || XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily()))
        {
            bAutomatic = true;

            if( !GetAutoName().isEmpty() )
            {
                OUString sAutoProp = ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT ) ?
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
                    DBG_UNHANDLED_EXCEPTION();
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
            bool bVal = *static_cast<sal_Bool const *>(rAny.getValue());
            bHasCombinedCharactersLetter = bVal;
        }

        // keep-together: the application default is different from
        // the file format default. Hence, if we always set this
        // value; if we didn't find one, we'll set to false, the file
        // format default.
        // border-model: same
        if(IsDefaultStyle() && XML_STYLE_FAMILY_TABLE_ROW == GetFamily())
        {
            OUString sIsSplitAllowed("IsSplitAllowed");
            DBG_ASSERT( rPropSet->getPropertySetInfo()->hasPropertyByName( sIsSplitAllowed ), "property missing?" );
            rPropSet->setPropertyValue(
                sIsSplitAllowed,
                (aContextIDs[1].nIndex == -1) ? makeAny( false ) : GetProperties()[aContextIDs[1].nIndex].maValue );
        }

        if(IsDefaultStyle() && XML_STYLE_FAMILY_TABLE_TABLE == GetFamily())
        {
            OUString sCollapsingBorders("CollapsingBorders");
            DBG_ASSERT( rPropSet->getPropertySetInfo()->hasPropertyByName( sCollapsingBorders ), "property missing?" );
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
                        rtl::OUString sStyleName;
                        rState.maValue >>= sStyleName;

                        //UUUU translate the used name from ODF intern to the name used in the Model
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
                            // converted there (its called first) and not here (see
                            // translateNameBasedDrawingLayerFillStyleDefinitionsToStyleDisplayNames)
                            // For convenience, still Write back the corrected value to the XMLPropertyState entry
                            rState.maValue <<= sStyleName;
                            break;
                        }

                        // Still needed if it's not an AutomaticStyle (!)
                        try
                        {
                            if(!rPropMapper.is())
                            {
                                rPropMapper = xImpPrMap->getPropertySetMapper();
                            }

                            // set property
                            const rtl::OUString& rPropertyName = rPropMapper->GetEntryAPIName(rState.mnIndex);

                            if(!xInfo.is())
                            {
                                xInfo = rPropSet->getPropertySetInfo();
                            }

                            if(xInfo->hasPropertyByName(rPropertyName))
                            {
                                rPropSet->setPropertyValue(rPropertyName,Any(sStyleName));
                            }
                        }
                        catch(::com::sun::star::lang::IllegalArgumentException& e)
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
