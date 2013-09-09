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
#include "xmloff/xmlnmspe.hxx"
#include "xmloff/XMLEventsImportContext.hxx"
#include "xmloff/attrlist.hxx"
#include "xmloff/families.hxx"
#include "xmloff/txtprmap.hxx"
#include "xmloff/txtstyli.hxx"
#include "xmloff/xmlimp.hxx"
#include "xmloff/xmltkmap.hxx"
#include "xmloff/xmltoken.hxx"
#include "xmloff/xmluconv.hxx"

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

// STL includes
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>


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
                bAutoUpdate = sal_True;
        }
        else if( IsXMLToken( rLocalName, XML_LIST_STYLE_NAME ) )
        {
            sListStyleName = rValue;
            // Inherited paragraph style lost information about unset numbering (#i69523#)
            mbListStyleSet = sal_True;
        }
        else if( IsXMLToken( rLocalName, XML_MASTER_PAGE_NAME ) )
        {
            sMasterPageName = rValue;
            bHasMasterPageName = sal_True;
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

TYPEINIT1( XMLTextStyleContext, XMLPropStyleContext );

XMLTextStyleContext::XMLTextStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        sal_Bool bDefaultStyle )
:   XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle )
,   sIsAutoUpdate( "IsAutoUpdate" )
,   sCategory( "Category" )
,   sNumberingStyleName( "NumberingStyleName" )
,       sOutlineLevel("OutlineLevel" )
,   sDropCapCharStyleName( "DropCapCharStyleName" )
,   sPageDescName( "PageDescName" )
,   nOutlineLevel( -1 )
,   bAutoUpdate( sal_False )
,   bHasMasterPageName( sal_False )
,   bHasCombinedCharactersLetter( sal_False )
// Inherited paragraph style lost information about unset numbering (#i69523#)
,   mbListStyleSet( sal_False )
,   pEventContext( NULL )
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
    SvXMLImportContext *pContext = 0;

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
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
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
        pEventContext->AddRef();
        pContext = pEventContext;
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLTextStyleContext::CreateAndInsert( sal_Bool bOverwrite )
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
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsAutoUpdate, aAny );
    }

    sal_uInt16 nCategory = ParagraphStyleCategory::TEXT;
    if(  XML_STYLE_FAMILY_TEXT_PARAGRAPH == GetFamily() &&
         !sCategoryVal.isEmpty() && xStyle->isUserDefined() &&
         xPropSetInfo->hasPropertyByName( sCategory ) &&
          SvXMLUnitConverter::convertEnum( nCategory, sCategoryVal, aCategoryMap ) )
    {
        Any aAny;
        aAny <<= (sal_Int16)nCategory;
        xPropSet->setPropertyValue( sCategory, aAny );
    }

    // tell the style about it's events (if applicable)
    if (NULL != pEventContext)
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
            Reference < XInterface > xInt = xFactory->createInstance (
                OUString ( "com.sun.star.text.Defaults" ) );
            Reference < XPropertySet > xProperties ( xInt, UNO_QUERY );
            if ( xProperties.is() )
                FillPropertySet ( xProperties );
        }
    }
}

void XMLTextStyleContext::Finish( sal_Bool bOverwrite )
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
        Any aAny;
        if( nOutlineLevel >= 0 )
        {
            aAny <<= nOutlineLevel;
            xPropSet->setPropertyValue( sOutlineLevel, aAny );
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
                Any aAny;
                aAny <<= sListStyleName /* empty string */;
                xPropSet->setPropertyValue( sNumberingStyleName, aAny );
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
                    Any aAny;
                    aAny <<= sDisplayListStyleName;
                    xPropSet->setPropertyValue( sNumberingStyleName, aAny );
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
            Any aAny;
            aAny <<= sDisplayDropCapTextStyleName;
            xPropSet->setPropertyValue( sDropCapCharStyleName, aAny );
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
            Any aAny;
            aAny <<= sDisplayName;
            xPropSet->setPropertyValue( sPageDescName, aAny );
        }
    }
}

void XMLTextStyleContext::FillPropertySet(
    const Reference<XPropertySet > & rPropSet )
{
    // imitate the FillPropertySet of the super class, so we get a chance to
    // catch the combined characters attribute

    // imitate XMLPropStyleContext::FillPropertySet(...)
    UniReference < SvXMLImportPropertyMapper > xImpPrMap =
        ((SvXMLStylesContext *)GetStyles())->GetImportPropertyMapper(GetFamily());
    DBG_ASSERT( xImpPrMap.is(), "Where is the import prop mapper?" );
    if( xImpPrMap.is() )
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
            { -1, -1 }
        };

        // get property set info
        Reference< XPropertySetInfo > xInfo( rPropSet->getPropertySetInfo(), UNO_SET_THROW );

        bool bAutomatic = false;
        if( ((SvXMLStylesContext *)GetStyles())->IsAutomaticStyle() &&
            ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT || GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ) )
        {
            bAutomatic = true;
            if( !GetAutoName().isEmpty() )
            {
                OUString sAutoProp = ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT ) ?
                    OUString( "CharAutoStyleName" ):
                    OUString( "ParaAutoStyleName" );
                try
                {
                    if ( xInfo->hasPropertyByName( sAutoProp ) )
                        rPropSet->setPropertyValue( sAutoProp, makeAny(GetAutoName()) );
                    else
                        bAutomatic = false;
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
            xImpPrMap->CheckSpecialContext( GetProperties(), rPropSet, aContextIDs );
        else
            xImpPrMap->FillPropertySet( GetProperties(), rPropSet, aContextIDs );

        // have we found a combined characters
        sal_Int32 nIndex = aContextIDs[0].nIndex;
        if ( nIndex != -1 )
        {
            Any& rAny = GetProperties()[nIndex].maValue;
            sal_Bool bVal = *(sal_Bool*)rAny.getValue();
            bHasCombinedCharactersLetter = bVal;
        }

        // keep-together: the application default is different from
        // the file format default. Hence, if we always set this
        // value; if we didn't find one, we'll set to false, the file
        // format default.
        // border-model: same
        if( IsDefaultStyle() && GetFamily() == XML_STYLE_FAMILY_TABLE_ROW )
        {
            OUString sIsSplitAllowed( "IsSplitAllowed" );
            DBG_ASSERT( rPropSet->getPropertySetInfo()->hasPropertyByName( sIsSplitAllowed ),
                        "property missing?" );
            rPropSet->setPropertyValue( sIsSplitAllowed,
                (aContextIDs[1].nIndex == -1)
                ? makeAny( false )
                : GetProperties()[aContextIDs[1].nIndex].maValue );
        }

        if( IsDefaultStyle() && GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE )
        {
            OUString sCollapsingBorders("CollapsingBorders");
            DBG_ASSERT( rPropSet->getPropertySetInfo()->hasPropertyByName( sCollapsingBorders ),
                        "property missing?" );
            rPropSet->setPropertyValue( sCollapsingBorders,
                (aContextIDs[2].nIndex == -1)
                ? makeAny( false )
                : GetProperties()[aContextIDs[2].nIndex].maValue );
        }


        // check for StarBats and StarMath fonts

        // iterate over aContextIDs entries 3..6
        for ( sal_Int32 i = 3; i < 7; i++ )
        {
            nIndex = aContextIDs[i].nIndex;
            if ( nIndex != -1 )
            {
                // Found!
                struct XMLPropertyState& rState = GetProperties()[nIndex];
                Any rAny = rState.maValue;
                sal_Int32 nMapperIndex = rState.mnIndex;

                // Now check for font name in rState and set corrected value,
                // if necessary.
                OUString sFontName;
                rAny >>= sFontName;
                if ( !sFontName.isEmpty() )
                {
                    OUString sStarBats( "StarBats" );
                    OUString sStarMath( "StarMath" );
                    if ( sFontName.equalsIgnoreAsciiCase( sStarBats ) ||
                         sFontName.equalsIgnoreAsciiCase( sStarMath ) )
                    {
                        // construct new value
                        sFontName = OUString("StarSymbol");
                        Any aAny( rAny );
                        aAny <<= sFontName;

                        // get property set mapper
                        UniReference<XMLPropertySetMapper> rPropMapper =
                            xImpPrMap->getPropertySetMapper();

                        // set property
                        OUString rPropertyName(
                            rPropMapper->GetEntryAPIName(nMapperIndex) );
                        if ( xInfo->hasPropertyByName( rPropertyName ) )
                        {
                            rPropSet->setPropertyValue( rPropertyName, aAny );
                        }
                    }
                    // else: "normal" style name -> no correction is necessary
                }
                // else: no style name found -> illegal value -> ignore
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
