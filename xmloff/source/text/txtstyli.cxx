/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtstyli.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:44:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHSTYLECATEGORY_HPP_
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTPROPERTYSETCONTEXT_HXX
#include "XMLTextPropertySetContext.hxx"
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_TXTSTYLI_HXX
#include <xmloff/txtstyli.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include <xmloff/XMLEventsImportContext.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

// STL includes
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
//using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

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
            // --> OD 2006-09-21 #i69523#
            mbListStyleSet = sal_True;
            // <--
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
            if( SvXMLUnitConverter::convertNumber( nTmp, rValue ) &&
                nTmp > 0 && nTmp < 256 )
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
,   sIsAutoUpdate( RTL_CONSTASCII_USTRINGPARAM( "IsAutoUpdate" ) )
,   sCategory( RTL_CONSTASCII_USTRINGPARAM( "Category" ) )
,   sNumberingStyleName( RTL_CONSTASCII_USTRINGPARAM( "NumberingStyleName" ) )
,   sDropCapCharStyleName( RTL_CONSTASCII_USTRINGPARAM( "DropCapCharStyleName" ) )
,   sPageDescName( RTL_CONSTASCII_USTRINGPARAM( "PageDescName" ) )
,   nOutlineLevel( 0 )
,   bAutoUpdate( sal_False )
,   bHasMasterPageName( sal_False )
,   bHasCombinedCharactersLetter( sal_False )
// --> OD 2006-09-21 #i69523#
,   mbListStyleSet( sal_False )
// <--
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

    // --> OD 2006-10-12 #i69629#
    if ( nOutlineLevel > 0 )
    {
        GetImport().GetTextImport()->AddOutlineStyleCandidate( nOutlineLevel,
                                                      GetDisplayName() );
    }
    // <--
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
    // --> OD 2006-09-21 #i69523#
    // consider set empty list style
//    if ( !( sListStyleName.getLength() ||
    if ( !( mbListStyleSet ||
            sDropCapTextStyleName.getLength() ||
            bHasMasterPageName ) ||
         !xStyle.is() ||
         !( bOverwrite || IsNew() ) )
        return;
    // <--

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();

    // --> OD 2006-09-21 #i69523#
    // consider set empty list style
//    if( sListStyleName.getLength() )
    if ( mbListStyleSet &&
         xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
    {
        // --> OD 2006-10-12 #i70223#
        // Only for text document from version prior OOo 2.1 resp. SO 8 PU5:
        // - Do not apply list style, if paragraph style has a default outline
        //   level > 0 and thus, will be assigned to the corresponding list
        //   level of the outline style.
        bool bApplyListStyle( true );
        if ( nOutlineLevel > 0 )
        {
            // --> OD 2007-12-19 #152540#
            if ( GetImport().IsTextDocInOOoFileFormat() )
            {
                bApplyListStyle = false;
            }
            else
            {
                sal_Int32 nUPD( 0 );
                sal_Int32 nBuild( 0 );
                // --> OD 2008-03-19 #i86058#
                // check explicitly on certain versions
                if ( GetImport().getBuildIds( nUPD, nBuild ) &&
                     ( ( nUPD == 641 ) || ( nUPD == 645 ) || // prior OOo 2.0
                       ( nUPD == 680 && nBuild <= 9073 ) ) ) // OOo 2.0 - OOo 2.0.4
                {
                    bApplyListStyle = false;
                }
                // <--
            }
            // <--
        }

        if ( bApplyListStyle )
        {
            if ( !sListStyleName.getLength() )
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
        // <--
    }
    // <--

    if( sDropCapTextStyleName.getLength() )
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
        if( ( !sDisplayName.getLength() ||
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
            { CTF_FONTFAMILYNAME, -1 },
            { CTF_FONTFAMILYNAME_CJK, -1 },
            { CTF_FONTFAMILYNAME_CTL, -1 },
            { -1, -1 }
        };

        bool bAutomatic = false;
        if( ((SvXMLStylesContext *)GetStyles())->IsAutomaticStyle() &&
            ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT || GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ) )
        {
            bAutomatic = true;
            if( GetAutoName().getLength() )
            {
                OUString sAutoProp = ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT ) ?
                    OUString( RTL_CONSTASCII_USTRINGPARAM("CharAutoStyleName") ):
                    OUString( RTL_CONSTASCII_USTRINGPARAM("ParaAutoStyleName") );
                try
                {
                    rPropSet->setPropertyValue( sAutoProp, makeAny(GetAutoName()) );
                }
                catch( UnknownPropertyException )
                {
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

        // get property set info
        Reference< XPropertySetInfo > xInfo;

        // keep-together: the application default is different from
        // the file format default. Hence, if we always set this
        // value; if we didn't find one, we'll set to false, the file
        // format default.
        // border-model: same
        if( IsDefaultStyle() && GetFamily() == XML_STYLE_FAMILY_TABLE_ROW )
        {
            OUString sIsSplitAllowed =
                OUString( RTL_CONSTASCII_USTRINGPARAM( "IsSplitAllowed" ) );
            DBG_ASSERT( rPropSet->getPropertySetInfo()->hasPropertyByName( sIsSplitAllowed ),
                        "property missing?" );
            rPropSet->setPropertyValue( sIsSplitAllowed,
                (aContextIDs[1].nIndex == -1)
                ? makeAny( false )
                : GetProperties()[aContextIDs[1].nIndex].maValue );
        }

        if( IsDefaultStyle() && GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE )
        {
            OUString sCollapsingBorders(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "CollapsingBorders" ) ) );
            DBG_ASSERT( rPropSet->getPropertySetInfo()->hasPropertyByName( sCollapsingBorders ),
                        "property missing?" );
            rPropSet->setPropertyValue( sCollapsingBorders,
                (aContextIDs[2].nIndex == -1)
                ? makeAny( false )
                : GetProperties()[aContextIDs[2].nIndex].maValue );
        }


        // check for StarBats and StarMath fonts

        if (!xInfo.is())
            xInfo.set(rPropSet->getPropertySetInfo());

        // iterate over aContextIDs entries 1..3
        for ( sal_Int32 i = 2; i < 5; i++ )
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

                        // get property set mapper
                        UniReference<XMLPropertySetMapper> rPropMapper =
                            xImpPrMap->getPropertySetMapper();

                        // set property
                        OUString rPropertyName(
                            rPropMapper->GetEntryAPIName(nMapperIndex) );
                        if( !xInfo.is() )
                            xInfo = rPropSet->getPropertySetInfo();
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
