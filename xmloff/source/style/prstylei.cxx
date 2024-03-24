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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include "StylePropertiesContext.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;
using namespace com::sun::star::drawing;

void XMLPropStyleContext::SetAttribute( sal_Int32 nElement,
                                        const OUString& rValue )
{
    if( nElement == XML_ELEMENT(STYLE, XML_FAMILY) )
    {
        SAL_WARN_IF( GetFamily() != SvXMLStylesContext::GetFamily( rValue ), "xmloff", "unexpected style family" );
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nElement, rValue );
    }
}


namespace
{
    const OldFillStyleDefinitionSet & theStandardSet()
    {
        static const OldFillStyleDefinitionSet theSet = []()
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
            }();
        return theSet;
    };
    const OldFillStyleDefinitionSet & theHeaderSet()
    {
        static const OldFillStyleDefinitionSet theSet = []()
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
            }();
        return theSet;
    };
    const OldFillStyleDefinitionSet & theFooterSet()
    {
        static const OldFillStyleDefinitionSet theSet = []()
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
            }();
        return theSet;
    };
    const OldFillStyleDefinitionSet & theParaSet()
    {
        static const OldFillStyleDefinitionSet theSet = []()
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
            }();
        return theSet;
    };
}



constexpr OUString gsIsPhysical(  u"IsPhysical"_ustr  );
constexpr OUString gsFollowStyle(  u"FollowStyle"_ustr  );

XMLPropStyleContext::XMLPropStyleContext( SvXMLImport& rImport,
        SvXMLStylesContext& rStyles, XmlStyleFamily nFamily,
        bool bDefault )
:   SvXMLStyleContext( rImport, nFamily, bDefault )
,   mxStyles( &rStyles )
{
}

XMLPropStyleContext::~XMLPropStyleContext()
{
}

const OldFillStyleDefinitionSet& XMLPropStyleContext::getStandardSet()
{
    return theStandardSet();
}

const OldFillStyleDefinitionSet& XMLPropStyleContext::getHeaderSet()
{
    return theHeaderSet();
}

const OldFillStyleDefinitionSet& XMLPropStyleContext::getFooterSet()
{
    return theFooterSet();
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLPropStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    sal_uInt32 nFamily = 0;
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_STYLE) ||
        IsTokenInNamespace(nElement, XML_NAMESPACE_LO_EXT) )
    {
        sal_Int32 nLocalName = nElement & TOKEN_MASK;
        if( nLocalName == XML_GRAPHIC_PROPERTIES )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( nLocalName == XML_DRAWING_PAGE_PROPERTIES )
            nFamily = XML_TYPE_PROP_DRAWING_PAGE;
        else if( nLocalName == XML_TEXT_PROPERTIES )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( nLocalName == XML_PARAGRAPH_PROPERTIES )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( nLocalName == XML_RUBY_PROPERTIES )
            nFamily = XML_TYPE_PROP_RUBY;
        else if( nLocalName == XML_SECTION_PROPERTIES )
            nFamily = XML_TYPE_PROP_SECTION;
        else if( nLocalName == XML_TABLE_PROPERTIES )
            nFamily = XML_TYPE_PROP_TABLE;
        else if( nLocalName == XML_TABLE_COLUMN_PROPERTIES  )
            nFamily = XML_TYPE_PROP_TABLE_COLUMN;
        else if( nLocalName ==XML_TABLE_ROW_PROPERTIES  )
            nFamily = XML_TYPE_PROP_TABLE_ROW;
        else if( nLocalName == XML_TABLE_CELL_PROPERTIES  )
            nFamily = XML_TYPE_PROP_TABLE_CELL;
        else if( nLocalName == XML_CHART_PROPERTIES )
            nFamily = XML_TYPE_PROP_CHART;
    }
    if( nFamily )
    {
        rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
            mxStyles->GetImportPropertyMapper( GetFamily() );
        if (xImpPrMap.is())
        {
            return new StylePropertiesContext(GetImport(), nElement, xAttrList, nFamily, maProperties, xImpPrMap);
        }
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void XMLPropStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
        mxStyles->GetImportPropertyMapper( GetFamily() );
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

    OUString sServiceName = mxStyles->GetServiceName( GetFamily() );
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
    SvXMLStylesContext* pSvXMLStylesContext = mxStyles.get();
    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = pSvXMLStylesContext->GetImportPropertyMapper(GetFamily());
    OSL_ENSURE(xImpPrMap.is(), "There is no import prop mapper");

    // need to filter out old fill definitions when the new ones are used. The new
    // ones are used when a FillStyle is defined
    const bool bTakeCareOfDrawingLayerFillStyle(xImpPrMap.is() && GetFamily() == XmlStyleFamily::TEXT_PARAGRAPH);
    bool bDrawingLayerFillStylesUsed(false);

    if(bTakeCareOfDrawingLayerFillStyle)
    {
        // check if new FillStyles are used and if so mark old ones with -1
        static OUString s_FillStyle("FillStyle");

        if(doNewDrawingLayerFillStyleDefinitionsExist(s_FillStyle))
        {
            deactivateOldFillStyleDefinitions(theParaSet());
            bDrawingLayerFillStylesUsed = true;
        }
    }

    if( pSvXMLStylesContext->IsAutomaticStyle()
        && ( GetFamily() == XmlStyleFamily::TEXT_TEXT || GetFamily() == XmlStyleFamily::TEXT_PARAGRAPH ) )
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
                if( GetFamily() == XmlStyleFamily::TEXT_PARAGRAPH )
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
                    Sequence< OUString > aPropNames
                    {
                        (GetFamily() == XmlStyleFamily::TEXT_PARAGRAPH)?
                        OUString("ParaAutoStyleName"):
                        OUString("CharAutoStyleName")
                    };
                    Sequence< Any > aAny = xAutoStyle->getPropertyValues( aPropNames );
                    if( aAny.hasElements() )
                    {
                        SetAutoName(aAny[0]);
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
            SAL_WARN("xmloff", "no styles container for family " << static_cast<int>(GetFamily()));
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
    Reference < XNameContainer > xFamilies = mxStyles->GetStylesContainer( GetFamily() );
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

            // getName() throws no non-Runtime exception:
            GetImport().SetError(
                XMLERROR_FLAG_ERROR | XMLERROR_PARENT_STYLE_NOT_ALLOWED,
                { mxStyle->getName(), sParent }, e.Message, nullptr );
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

    // Connect linked style.
    OUString aLinked(GetLinked());
    if (!aLinked.isEmpty())
    {
        if (GetFamily() == XmlStyleFamily::TEXT_PARAGRAPH)
        {
            aLinked = GetImport().GetStyleDisplayName(XmlStyleFamily::TEXT_TEXT, aLinked);
        }
        else if (GetFamily() == XmlStyleFamily::TEXT_TEXT)
        {
            aLinked = GetImport().GetStyleDisplayName(XmlStyleFamily::TEXT_PARAGRAPH, aLinked);
        }
    }
    if (!aLinked.isEmpty() && xPropSetInfo->hasPropertyByName("LinkStyle"))
    {
        uno::Any aAny = xPropSet->getPropertyValue("LinkStyle");
        OUString aCurrentLinked;
        aAny >>= aCurrentLinked;
        if (aCurrentLinked != aLinked)
        {
            xPropSet->setPropertyValue("LinkStyle", uno::Any(aLinked));
        }
    }

    if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
    {
        xPropSet->setPropertyValue( "Hidden", uno::Any( IsHidden( ) ) );
    }

}

bool XMLPropStyleContext::doNewDrawingLayerFillStyleDefinitionsExist(
    std::u16string_view rFillStyleTag) const
{
    if(!maProperties.empty() && !rFillStyleTag.empty())
    {
        // no & to avoid non-obvious UAF due to the 2nd temp Reference
        const rtl::Reference<XMLPropertySetMapper> rMapper = GetStyles()->GetImportPropertyMapper(GetFamily())->getPropertySetMapper();

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
    if(rHashSetOfTags.empty() || maProperties.empty())
        return;

    const rtl::Reference< XMLPropertySetMapper >& rMapper = GetStyles()->GetImportPropertyMapper(GetFamily())->getPropertySetMapper();

    if(!rMapper.is())
        return;

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

void XMLPropStyleContext::translateNameBasedDrawingLayerFillStyleDefinitionsToStyleDisplayNames()
{
    if(maProperties.empty())
        return;

    const rtl::Reference< XMLPropertySetMapper >& rMapper = GetStyles()->GetImportPropertyMapper(GetFamily())->getPropertySetMapper();

    if(!rMapper.is())
        return;

    for(auto& a : maProperties)
    {
        if(a.mnIndex != -1)
        {
            const OUString& rPropName = rMapper->GetEntryAPIName(a.mnIndex);
            XmlStyleFamily aStyleFamily(XmlStyleFamily::DATA_STYLE);

            if(rPropName == u"FillGradientName"
               || rPropName == u"FillTransparenceGradientName")
            {
                aStyleFamily = XmlStyleFamily::SD_GRADIENT_ID;
            }
            else if(rPropName == u"FillHatchName")
            {
                aStyleFamily = XmlStyleFamily::SD_HATCH_ID;
            }
            else if(rPropName == u"FillBitmapName")
            {
                aStyleFamily = XmlStyleFamily::SD_FILL_IMAGE_ID;
            }

            if(aStyleFamily != XmlStyleFamily::DATA_STYLE)
            {
                OUString sStyleName;

                a.maValue >>= sStyleName;
                sStyleName = GetImport().GetStyleDisplayName( aStyleFamily, sStyleName );
                a.maValue <<= sStyleName;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
