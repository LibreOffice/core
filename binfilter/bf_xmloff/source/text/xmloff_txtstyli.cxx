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


#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include "xmlnmspe.hxx"
#include "XMLTextPropertySetContext.hxx"
#include "xmlimp.hxx"
#include "xmluconv.hxx"

#include "txtprmap.hxx"
#include "txtstyli.hxx"

#include "XMLEventsImportContext.hxx"

#include <tools/debug.hxx>

// STL includes
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
namespace binfilter {

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

using rtl::OUString;
//using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

static __FAR_DATA SvXMLEnumMapEntry aCategoryMap[] =
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
        }
        else if( IsXMLToken( rLocalName, XML_MASTER_PAGE_NAME ) )
        {
            sMasterPageName = rValue;
            bHasMasterPageName = sal_True;
        }
        else if( IsXMLToken( rLocalName, XML_CLASS ) )
        {
            sCategoryVal = rValue;
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
        sal_Bool bDefaultStyle ) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles,
                         nFamily, bDefaultStyle ),
    bAutoUpdate( sal_False ),
    bHasMasterPageName( sal_False ),
    bHasCombinedCharactersLetter( sal_False ),
    pEventContext( NULL ),
    sIsAutoUpdate( RTL_CONSTASCII_USTRINGPARAM( "IsAutoUpdate" ) ),
    sCategory( RTL_CONSTASCII_USTRINGPARAM( "Category" ) ),
    sNumberingStyleName( RTL_CONSTASCII_USTRINGPARAM( "NumberingStyleName" ) ),
    sPageDescName( RTL_CONSTASCII_USTRINGPARAM( "PageDescName" ) ),
    sDropCapCharStyleName( RTL_CONSTASCII_USTRINGPARAM( "DropCapCharStyleName" ) )
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

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_PROPERTIES ) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            pContext = new XMLTextPropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    GetProperties(),
                                                    xImpPrMap,
                                                    sDropCapTextStyleName );
    }
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken( rLocalName, XML_EVENTS ) )
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
         sCategoryVal.getLength() && xStyle->isUserDefined() &&
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
}

void XMLTextStyleContext::SetDefaults( )
{
    if (GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH )
    {
        Reference < XMultiServiceFactory > xFactory ( GetImport().GetModel(), UNO_QUERY); 
        if (xFactory.is())
        {
            Reference < XInterface > xInt = xFactory->createInstance ( 
                OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.Defaults" ) ) );
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
    if( !(sListStyleName.getLength() || sDropCapTextStyleName.getLength() ||
        bHasMasterPageName) ||
        !xStyle.is() || !(bOverwrite || IsNew()) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();

    if( sListStyleName.getLength() )
    {
        // The families cointaner must exist
        const Reference < XNameContainer >& rNumStyles =
            GetImport().GetTextImport()->GetNumberingStyles();
        if( rNumStyles.is() && rNumStyles->hasByName( sListStyleName ) &&
            xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
        {
            Any aAny;
            aAny <<= sListStyleName;
            xPropSet->setPropertyValue( sNumberingStyleName, aAny );
        }
    }

    if( sDropCapTextStyleName.getLength() )
    {
        // The families cointaner must exist
        const Reference < XNameContainer >& rTextStyles =
            GetImport().GetTextImport()->GetTextStyles();
        if( rTextStyles.is() &&
            rTextStyles->hasByName( sDropCapTextStyleName ) &&
            xPropSetInfo->hasPropertyByName( sDropCapCharStyleName ) )
        {
            Any aAny;
            aAny <<= sDropCapTextStyleName;
            xPropSet->setPropertyValue( sDropCapCharStyleName, aAny );
        }
    }

    if( bHasMasterPageName )
    {
        // The families cointaner must exist
        const Reference < XNameContainer >& rPageStyles =
            GetImport().GetTextImport()->GetPageStyles();
        if( ( !sMasterPageName.getLength() ||
              (rPageStyles.is() && 
               rPageStyles->hasByName( sMasterPageName )) ) &&
            xPropSetInfo->hasPropertyByName( sPageDescName ) )
        {
            Any aAny;
            aAny <<= sMasterPageName;
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

        // get property set mapper
        UniReference<XMLPropertySetMapper> rPropMapper =
            xImpPrMap->getPropertySetMapper();

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
#ifdef CONV_STAR_FONTS
            { CTF_FONTFAMILYNAME, -1 },
            { CTF_FONTFAMILYNAME_CJK, -1 },
            { CTF_FONTFAMILYNAME_CTL, -1 },
#endif
            { -1, -1 }
        };

        // get property set info
        Reference< XPropertySetInfo > xInfo = rPropSet->getPropertySetInfo();

        // check for multi-property set
        Reference<XMultiPropertySet> xMultiPropSet( rPropSet, UNO_QUERY );
        if ( xMultiPropSet.is() )
        {
            // Try XMultiPropertySet. If that fails, try the regular route.
            sal_Bool bSet = SvXMLImportPropertyMapper::_FillMultiPropertySet( 
                GetProperties(), xMultiPropSet, xInfo, rPropMapper,
                aContextIDs );
            if ( !bSet )
                SvXMLImportPropertyMapper::_FillPropertySet( 
                    GetProperties(), rPropSet, xInfo, rPropMapper, 
                    GetImport(), aContextIDs );
        }
        else
            SvXMLImportPropertyMapper::_FillPropertySet( 
                    GetProperties(), rPropSet, xInfo, rPropMapper,
                    GetImport(), aContextIDs );

        // have we found a combined characters
        sal_Int32 nIndex = aContextIDs[0].nIndex;
        if ( nIndex != -1 )
        {
            Any& rAny = GetProperties()[nIndex].maValue;
            sal_Bool bVal = *(sal_Bool*)rAny.getValue();
            bHasCombinedCharactersLetter = bVal;
        }

#ifdef CONV_STAR_FONTS
        // check for StarBats and StarMath fonts

        // iterate over aContextIDs entries 1..3
        for ( sal_Int32 i = 1; i < 4; i++ )
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
                if ( sFontName.getLength() > 0 )
                {
                    OUString sStarBats( RTL_CONSTASCII_USTRINGPARAM("StarBats" ) );
                    OUString sStarMath( RTL_CONSTASCII_USTRINGPARAM("StarMath" ) );
                    if ( sFontName.equalsIgnoreAsciiCase( sStarBats ) ||
                         sFontName.equalsIgnoreAsciiCase( sStarMath ) )
                    {
                        // construct new value
                        sFontName = OUString( 
                            RTL_CONSTASCII_USTRINGPARAM("StarSymbol") );
                        Any aAny( rAny );
                        aAny <<= sFontName;

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
#endif
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
