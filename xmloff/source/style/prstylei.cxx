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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <set>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlprcon.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;
using namespace com::sun::star::drawing;

void XMLPropStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey && IsXMLToken( rLocalName, XML_FAMILY ) )
    {
        SAL_WARN_IF( GetFamily() != SvXMLStylesContext::GetFamily( rValue ), "xmloff", "unexpected style family" );
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}


namespace
{
    struct theStandardSet :
        public rtl::StaticWithInit<OldFillStyleDefinitionSet, theStandardSet>
    {
        OldFillStyleDefinitionSet operator () ()
        {
            OldFillStyleDefinitionSet aSet;
            aSet.insert("BackColorRGB");
            aSet.insert("BackTransparent");
            aSet.insert("BackColorTransparency");
            aSet.insert("BackGraphic");
            aSet.insert("BackGraphicFilter");
            aSet.insert("BackGraphicLocation");
            aSet.insert("BackGraphicTransparency");
            return aSet;
        }
    };
    struct theHeaderSet :
        public rtl::StaticWithInit<OldFillStyleDefinitionSet, theHeaderSet>
    {
        OldFillStyleDefinitionSet operator () ()
        {
            OldFillStyleDefinitionSet aSet;
            aSet.insert("HeaderBackColorRGB");
            aSet.insert("HeaderBackTransparent");
            aSet.insert("HeaderBackColorTransparency");
            aSet.insert("HeaderBackGraphic");
            aSet.insert("HeaderBackGraphicFilter");
            aSet.insert("HeaderBackGraphicLocation");
            aSet.insert("HeaderBackGraphicTransparency");
            return aSet;
        }
    };
    struct theFooterSet :
        public rtl::StaticWithInit<OldFillStyleDefinitionSet, theFooterSet>
    {
        OldFillStyleDefinitionSet operator () ()
        {
            OldFillStyleDefinitionSet aSet;
            aSet.insert("FooterBackColorRGB");
            aSet.insert("FooterBackTransparent");
            aSet.insert("FooterBackColorTransparency");
            aSet.insert("FooterBackGraphic");
            aSet.insert("FooterBackGraphicFilter");
            aSet.insert("FooterBackGraphicLocation");
            aSet.insert("FooterBackGraphicTransparency");
            return aSet;
        }
    };
    struct theParaSet :
        public rtl::StaticWithInit<OldFillStyleDefinitionSet, theParaSet>
    {
        OldFillStyleDefinitionSet operator () ()
        {
            OldFillStyleDefinitionSet aSet;
            // Caution: here it is *not* 'ParaBackColorRGB' as it should be, but indeed
            // 'ParaBackColor' is used, see aXMLParaPropMap definition (line 313)
            aSet.insert("ParaBackColor");
            aSet.insert("ParaBackTransparent");
            aSet.insert("ParaBackGraphicLocation");
            aSet.insert("ParaBackGraphicFilter");
            aSet.insert("ParaBackGraphic");

            // These are not used in aXMLParaPropMap definition, thus not needed here
            // aSet.insert("ParaBackColorTransparency");
            // aSet.insert("ParaBackGraphicTransparency");
            return aSet;
        }
    };
}



static const OUStringLiteral gsIsPhysical(  "IsPhysical"  );
static const OUStringLiteral gsFollowStyle(  "FollowStyle"  );

XMLPropStyleContext::XMLPropStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        bool bDefault )
:   SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, nFamily, bDefault )
,   mxStyles( &rStyles )
{
}

XMLPropStyleContext::~XMLPropStyleContext()
{
}

const OldFillStyleDefinitionSet& XMLPropStyleContext::getStandardSet()
{
    return theStandardSet::get();
}

const OldFillStyleDefinitionSet& XMLPropStyleContext::getHeaderSet()
{
    return theHeaderSet::get();
}

const OldFillStyleDefinitionSet& XMLPropStyleContext::getFooterSet()
{
    return theFooterSet::get();
}

SvXMLImportContextRef XMLPropStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    sal_uInt32 nFamily = 0;
    if( XML_NAMESPACE_STYLE == nPrefix || XML_NAMESPACE_LO_EXT == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( IsXMLToken( rLocalName, XML_DRAWING_PAGE_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_DRAWING_PAGE;
        else if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_RUBY_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_RUBY;
        else if( IsXMLToken( rLocalName, XML_SECTION_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_SECTION;
        else if( IsXMLToken( rLocalName, XML_TABLE_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE;
        else if( IsXMLToken( rLocalName, XML_TABLE_COLUMN_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_COLUMN;
        else if( IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_ROW;
        else if( IsXMLToken( rLocalName, XML_TABLE_CELL_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_CELL;
        else if( IsXMLToken( rLocalName, XML_CHART_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_CHART;
    }
    if( nFamily )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
            static_cast<SvXMLStylesContext *>(mxStyles.get())->GetImportPropertyMapper(
                                                        GetFamily() );
        if( xImpPrMap.is() )
            xContext = new SvXMLPropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    nFamily,
                                                    maProperties,
                                                    xImpPrMap );
    }

    if (!xContext)
        xContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return xContext;
}

void XMLPropStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
        static_cast<SvXMLStylesContext *>(mxStyles.get())->GetImportPropertyMapper(
                                                                GetFamily() );
    SAL_WARN_IF( !xImpPrMap.is(), "xmloff", "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->FillPropertySet( maProperties, rPropSet );
}

void XMLPropStyleContext::SetDefaults()
{
}

Reference < XStyle > XMLPropStyleContext::Create()
{
    Reference < XStyle > xNewStyle;

    OUString sServiceName(
        static_cast<SvXMLStylesContext *>(mxStyles.get())->GetServiceName( GetFamily() ) );
    if( !sServiceName.isEmpty() )
    {
        Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
        if( xFactory.is() )
        {
            Reference < XInterface > xIfc =
                xFactory->createInstance( sServiceName );
            if( xIfc.is() )
                xNewStyle.set( xIfc, UNO_QUERY );
        }
    }

    return xNewStyle;
}

void XMLPropStyleContext::CreateAndInsert( bool bOverwrite )
{
    SvXMLStylesContext* pSvXMLStylesContext = static_cast< SvXMLStylesContext* >(mxStyles.get());
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = pSvXMLStylesContext->GetImportPropertyMapper(GetFamily());
    OSL_ENSURE(xImpPrMap.is(), "There is no import prop mapper");

    // need to filter out old fill definitions when the new ones are used. The new
    // ones are used when a FillStyle is defined
    const bool bTakeCareOfDrawingLayerFillStyle(xImpPrMap.is() && GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH);
    bool bDrawingLayerFillStylesUsed(false);

    if(bTakeCareOfDrawingLayerFillStyle)
    {
        // check if new FillStyles are used and if so mark old ones with -1
        static OUString s_FillStyle("FillStyle");

        if(doNewDrawingLayerFillStyleDefinitionsExist(s_FillStyle))
        {
            deactivateOldFillStyleDefinitions(theParaSet::get());
            bDrawingLayerFillStylesUsed = true;
        }
    }

    if( pSvXMLStylesContext->IsAutomaticStyle()
        && ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT || GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ) )
    {
        // Need to translate StyleName from temp MapNames to names
        // used in already imported items (already exist in the pool). This
        // is required for AutomaticStyles since these do *not* use FillPropertySet
        // and thus just trigger CheckSpecialContext in XMLTextStyleContext::FillPropertySet
        // (which may be double action anyways). The mechanism there to use _ContextID_Index_Pair
        // is not working for AutomaticStyles and is already too late, too (this
        // method is already called before XMLTextStyleContext::FillPropertySet gets called)
        if(bDrawingLayerFillStylesUsed)
        {
            translateNameBasedDrawingLayerFillStyleDefinitionsToStyleDisplayNames();
        }

        Reference < XAutoStyleFamily > xAutoFamily = pSvXMLStylesContext->GetAutoStyles( GetFamily() );
        if( !xAutoFamily.is() )
            return;
        if( xImpPrMap.is() )
        {
            Sequence< PropertyValue > aValues;
            xImpPrMap->FillPropertySequence( maProperties, aValues );

            sal_Int32 nLen = aValues.getLength();
            if( nLen )
            {
                if( GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH )
                {
                    aValues.realloc( nLen + 2 );
                    PropertyValue *pProps = aValues.getArray() + nLen;
                    pProps->Name = "ParaStyleName";
                    OUString sParent( GetParentName() );
                    if( !sParent.isEmpty() )
                    {
                        sParent = GetImport().GetStyleDisplayName( GetFamily(), sParent );
                        Reference < XNameContainer > xFamilies = pSvXMLStylesContext->GetStylesContainer( GetFamily() );
                        if(xFamilies.is() && xFamilies->hasByName( sParent ) )
                        {
                            css::uno::Reference< css::style::XStyle > xStyle;
                            Any aAny = xFamilies->getByName( sParent );
                            aAny >>= xStyle;
                            sParent = xStyle->getName() ;
                        }
                    }
                    else
                        sParent = "Standard";
                    pProps->Value <<= sParent;
                    ++pProps;
                    pProps->Name = "ParaConditionalStyleName";
                    pProps->Value <<= sParent;
                }

                Reference < XAutoStyle > xAutoStyle = xAutoFamily->insertStyle( aValues );
                if( xAutoStyle.is() )
                {
                    Sequence< OUString > aPropNames(1);
                    aPropNames[0] = GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ?
                        OUString("ParaAutoStyleName") :
                        OUString("CharAutoStyleName");
                    Sequence< Any > aAny = xAutoStyle->getPropertyValues( aPropNames );
                    if( aAny.hasElements() )
                    {
                        OUString aName;
                        aAny[0] >>= aName;
                        SetAutoName( aName );
                    }
                }
            }
        }
    }
    else
    {
        const OUString& rName = GetDisplayName();
        if( rName.isEmpty() || IsDefaultStyle() )
            return;

        Reference < XNameContainer > xFamilies = pSvXMLStylesContext->GetStylesContainer( GetFamily() );
        if( !xFamilies.is() )
        {
            SAL_WARN("xmloff", "no styles container for family " << GetFamily());
            return;
        }

        bool bNew = false;
        if( xFamilies->hasByName( rName ) )
        {
            Any aAny = xFamilies->getByName( rName );
            aAny >>= mxStyle;
        }
        else
        {
            mxStyle = Create();
            if( !mxStyle.is() )
                return;

            xFamilies->insertByName( rName, Any(mxStyle) );
            bNew = true;
        }

        Reference < XPropertySet > xPropSet( mxStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
                    xPropSet->getPropertySetInfo();
        if( !bNew && xPropSetInfo->hasPropertyByName( gsIsPhysical ) )
        {
            Any aAny = xPropSet->getPropertyValue( gsIsPhysical );
            bNew = !*o3tl::doAccess<bool>(aAny);
        }
        SetNew( bNew );
        if( rName != GetName() )
            GetImport().AddStyleDisplayName( GetFamily(), GetName(), rName );


        if( bOverwrite || bNew )
        {
            rtl::Reference < XMLPropertySetMapper > xPrMap;
            if( xImpPrMap.is() )
                xPrMap = xImpPrMap->getPropertySetMapper();
            if( xPrMap.is() )
            {
                Reference < XMultiPropertyStates > xMultiStates( xPropSet,
                                                                 UNO_QUERY );
                if( xMultiStates.is() )
                {
                    xMultiStates->setAllPropertiesToDefault();
                }
                else
                {
                    std::set < OUString > aNameSet;
                    sal_Int32 nCount = xPrMap->GetEntryCount();
                    sal_Int32 i;
                    for( i = 0; i < nCount; i++ )
                    {
                        const OUString& rPrName = xPrMap->GetEntryAPIName( i );
                        if( xPropSetInfo->hasPropertyByName( rPrName ) )
                            aNameSet.insert( rPrName );
                    }
                    Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
                    if (xPropState.is())
                    {
                        nCount = aNameSet.size();
                        Sequence<OUString> aNames( comphelper::containerToSequence(aNameSet) );
                        Sequence < PropertyState > aStates( xPropState->getPropertyStates(aNames) );
                        const PropertyState *pStates = aStates.getConstArray();
                        OUString* pNames = aNames.getArray();

                        for( i = 0; i < nCount; i++ )
                        {
                            if( PropertyState_DIRECT_VALUE == *pStates++ )
                                xPropState->setPropertyToDefault( pNames[i] );
                        }
                    }
                }
            }

            if (mxStyle.is())
                mxStyle->setParentStyle(OUString());

            FillPropertySet( xPropSet );
        }
        else
        {
            SetValid( false );
        }
    }
}

void XMLPropStyleContext::Finish( bool bOverwrite )
{
    if( !mxStyle.is() || !(IsNew() || bOverwrite) )
        return;

    // The families container must exist
    Reference < XNameContainer > xFamilies =
        static_cast<SvXMLStylesContext *>(mxStyles.get())->GetStylesContainer( GetFamily() );
    SAL_WARN_IF( !xFamilies.is(), "xmloff", "Families lost" );
    if( !xFamilies.is() )
        return;

    // connect parent
    OUString sParent( GetParentName() );
    if( !sParent.isEmpty() )
        sParent = GetImport().GetStyleDisplayName( GetFamily(), sParent );
    if( !sParent.isEmpty() && !xFamilies->hasByName( sParent ) )
        sParent.clear();

    if( sParent != mxStyle->getParentStyle() )
    {
        // this may except if setting the parent style forms a
        // circle in the style dependencies; especially if the parent
        // style is the same as the current style
        try
        {
            mxStyle->setParentStyle( sParent );
        }
        catch(const uno::Exception& e)
        {
            // according to the API definition, I would expect a
            // container::NoSuchElementException. But it throws an
            // uno::RuntimeException instead. I catch
            // uno::Exception in order to process both of them.

            // We can't set the parent style. For a proper
            // Error-Message, we should pass in the name of the
            // style, as well as the desired parent style.
            Sequence<OUString> aSequence(2);

            // getName() throws no non-Runtime exception:
            aSequence[0] = mxStyle->getName();
            aSequence[1] = sParent;

            GetImport().SetError(
                XMLERROR_FLAG_ERROR | XMLERROR_PARENT_STYLE_NOT_ALLOWED,
                aSequence, e.Message, nullptr );
        }
    }

    // connect follow
    OUString sFollow( GetFollow() );
    if( !sFollow.isEmpty() )
        sFollow = GetImport().GetStyleDisplayName( GetFamily(), sFollow );
    if( sFollow.isEmpty() || !xFamilies->hasByName( sFollow ) )
        sFollow = mxStyle->getName();

    Reference < XPropertySet > xPropSet( mxStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( gsFollowStyle ) )
    {
        Any aAny = xPropSet->getPropertyValue( gsFollowStyle );
        OUString sCurrFollow;
        aAny >>= sCurrFollow;
        if( sCurrFollow != sFollow )
        {
            xPropSet->setPropertyValue( gsFollowStyle, Any(sFollow) );
        }
    }

    if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
    {
        xPropSet->setPropertyValue( "Hidden", uno::makeAny( IsHidden( ) ) );
    }

}

bool XMLPropStyleContext::doNewDrawingLayerFillStyleDefinitionsExist(
    const OUString& rFillStyleTag) const
{
    if(!maProperties.empty() && rFillStyleTag.getLength())
    {
        const rtl::Reference< XMLPropertySetMapper >& rMapper = GetStyles()->GetImportPropertyMapper(GetFamily())->getPropertySetMapper();

        if(rMapper.is())
        {
            for(const auto& a : maProperties)
            {
                if(a.mnIndex != -1)
                {
                    const OUString& rPropName = rMapper->GetEntryAPIName(a.mnIndex);

                    if(rPropName == rFillStyleTag)
                    {
                        FillStyle eFillStyle(FillStyle_NONE);

                        if(a.maValue >>= eFillStyle)
                        {
                            // okay, type was good, FillStyle is set
                        }
                        else
                        {
                            // also try an int (see XFillStyleItem::PutValue)
                            sal_Int32 nFillStyle(0);

                            if(a.maValue >>= nFillStyle)
                            {
                                eFillStyle = static_cast< FillStyle >(nFillStyle);
                            }
                        }

                        // we found the entry, check it
                        return FillStyle_NONE != eFillStyle;
                    }
                }
            }
        }
    }

    return false;
}

void XMLPropStyleContext::deactivateOldFillStyleDefinitions(
    const OldFillStyleDefinitionSet& rHashSetOfTags)
{
    if(!rHashSetOfTags.empty() && !maProperties.empty())
    {
        const rtl::Reference< XMLPropertySetMapper >& rMapper = GetStyles()->GetImportPropertyMapper(GetFamily())->getPropertySetMapper();

        if(rMapper.is())
        {
            for(auto& a : maProperties)
            {
                if(a.mnIndex != -1)
                {
                    const OUString& rPropName = rMapper->GetEntryAPIName(a.mnIndex);

                    if(rHashSetOfTags.find(rPropName) != rHashSetOfTags.end())
                    {
                        // mark entry as inactive
                        a.mnIndex = -1;
                    }
                }
            }
        }
    }
}

void XMLPropStyleContext::translateNameBasedDrawingLayerFillStyleDefinitionsToStyleDisplayNames()
{
    if(!maProperties.empty())
    {
        const rtl::Reference< XMLPropertySetMapper >& rMapper = GetStyles()->GetImportPropertyMapper(GetFamily())->getPropertySetMapper();

        if(rMapper.is())
        {
            static OUString s_FillGradientName("FillGradientName");
            static OUString s_FillHatchName("FillHatchName");
            static OUString s_FillBitmapName("FillBitmapName");
            static OUString s_FillTransparenceGradientName("FillTransparenceGradientName");

            for(auto& a : maProperties)
            {
                if(a.mnIndex != -1)
                {
                    const OUString& rPropName = rMapper->GetEntryAPIName(a.mnIndex);
                    sal_uInt16 aStyleFamily(0);

                    if(rPropName == s_FillGradientName || rPropName == s_FillTransparenceGradientName)
                    {
                        aStyleFamily = XML_STYLE_FAMILY_SD_GRADIENT_ID;
                    }
                    else if(rPropName == s_FillHatchName)
                    {
                        aStyleFamily = XML_STYLE_FAMILY_SD_HATCH_ID;
                    }
                    else if(rPropName == s_FillBitmapName)
                    {
                        aStyleFamily = XML_STYLE_FAMILY_SD_FILL_IMAGE_ID;
                    }

                    if(aStyleFamily)
                    {
                        OUString sStyleName;

                        a.maValue >>= sStyleName;
                        sStyleName = GetImport().GetStyleDisplayName( aStyleFamily, sStyleName );
                        a.maValue <<= sStyleName;
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
