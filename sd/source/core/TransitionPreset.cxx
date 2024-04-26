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

#include <set>

#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <unotools/configmgr.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/lok.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/UI/Effects.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include <CustomAnimationPreset.hxx>
#include <TransitionPreset.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;

using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::beans::NamedValue;

namespace sd {

TransitionPreset::TransitionPreset( const css::uno::Reference< css::animations::XAnimationNode >& xNode )
{
    // first locate preset id
    const Sequence< NamedValue > aUserData( xNode->getUserData() );
    const NamedValue* pProp = std::find_if(aUserData.begin(), aUserData.end(),
        [](const NamedValue& rProp) { return rProp.Name == "preset-id"; });
    if (pProp != aUserData.end())
        pProp->Value >>= maPresetId;

    // second, locate transition filter element
    Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), css::uno::UNO_SET_THROW );
    Reference< XTransitionFilter > xTransition( xEnumeration->nextElement(), UNO_QUERY_THROW );

    mnTransition = xTransition->getTransition();
    mnSubtype = xTransition->getSubtype();
    mbDirection = xTransition->getDirection();
    mnFadeColor = xTransition->getFadeColor();
}

bool TransitionPreset::importTransitionsFile( TransitionPresetList& rList,
                                              Reference< XMultiServiceFactory > const & xServiceFactory,
                                              const OUString& aURL )
{
    SAL_INFO("sd.transitions", "Importing " << aURL);

    Reference< container::XNameAccess > xTransitionSets( officecfg::Office::UI::Effects::UserInterface::TransitionSets::get() );
    Reference< container::XNameAccess > xTransitionGroups( officecfg::Office::UI::Effects::UserInterface::TransitionGroups::get() );
    Reference< container::XNameAccess > xTransitionVariants( officecfg::Office::UI::Effects::UserInterface::TransitionVariants::get() );
    Reference< container::XNameAccess > xTransitions( officecfg::Office::UI::Effects::UserInterface::Transitions::get() );

    // import transition presets
    Reference< XAnimationNode > xAnimationNode;

    const std::set<sal_Int16> LOKSupportedTransitionTypes = {
            TransitionType::BARWIPE,
            TransitionType::BOXWIPE,
            TransitionType::FOURBOXWIPE,
            TransitionType::ELLIPSEWIPE,
            TransitionType::CLOCKWIPE,
            TransitionType::PINWHEELWIPE,
            TransitionType::PUSHWIPE,
            TransitionType::SLIDEWIPE,
            TransitionType::FADE,
            TransitionType::RANDOMBARWIPE,
            TransitionType::CHECKERBOARDWIPE,
            TransitionType::DISSOLVE,
            TransitionType::SNAKEWIPE,
            TransitionType::PARALLELSNAKESWIPE,
            TransitionType::IRISWIPE,
            TransitionType::BARNDOORWIPE,
            TransitionType::VEEWIPE,
            TransitionType::ZIGZAGWIPE,
            TransitionType::BARNZIGZAGWIPE,
            TransitionType::FANWIPE,
            TransitionType::SINGLESWEEPWIPE,
            TransitionType::WATERFALLWIPE,
            TransitionType::SPIRALWIPE,
            TransitionType::MISCDIAGONALWIPE,
            TransitionType::BOXSNAKESWIPE
    };

    const std::set<sal_Int16> LOKSupportedTransitionSubTypes = {
            TransitionSubType::DEFAULT,
            TransitionSubType::LEFTTORIGHT,
            TransitionSubType::TOPTOBOTTOM,
            TransitionSubType::CORNERSIN,
            TransitionSubType::CORNERSOUT,
            TransitionSubType::VERTICAL,
            TransitionSubType::HORIZONTAL,
            TransitionSubType::DOWN,
            TransitionSubType::CIRCLE,
            TransitionSubType::CLOCKWISETWELVE,
            TransitionSubType::CLOCKWISETHREE,
            TransitionSubType::CLOCKWISESIX,
            TransitionSubType::CLOCKWISENINE,
            TransitionSubType::TWOBLADEVERTICAL,
            TransitionSubType::TWOBLADEHORIZONTAL,
            TransitionSubType::FOURBLADE,
            TransitionSubType::FROMLEFT,
            TransitionSubType::FROMTOP,
            TransitionSubType::FROMRIGHT,
            TransitionSubType::FROMBOTTOM,
            TransitionSubType::CROSSFADE,
            TransitionSubType::FADETOCOLOR,
            TransitionSubType::FADEFROMCOLOR,
            TransitionSubType::FADEOVERCOLOR,
            TransitionSubType::THREEBLADE,
            TransitionSubType::EIGHTBLADE,
            TransitionSubType::ONEBLADE,
            TransitionSubType::ACROSS,
            TransitionSubType::TOPLEFTVERTICAL,
            TransitionSubType::TOPLEFTHORIZONTAL,
            TransitionSubType::TOPLEFTDIAGONAL,
            TransitionSubType::TOPRIGHTDIAGONAL,
            TransitionSubType::BOTTOMRIGHTDIAGONAL,
            TransitionSubType::BOTTOMLEFTDIAGONAL,
            TransitionSubType::RECTANGLE,
            TransitionSubType::DIAMOND,
            TransitionSubType::TOPLEFT,
            TransitionSubType::TOPRIGHT,
            TransitionSubType::BOTTOMRIGHT,
            TransitionSubType::BOTTOMLEFT,
            TransitionSubType::TOPCENTER,
            TransitionSubType::RIGHTCENTER,
            TransitionSubType::BOTTOMCENTER,
            TransitionSubType::LEFTCENTER,
            TransitionSubType::LEFT,
            TransitionSubType::UP,
            TransitionSubType::RIGHT,
            TransitionSubType::DIAGONALBOTTOMLEFT,
            TransitionSubType::DIAGONALTOPLEFT,
            TransitionSubType::CENTERTOP,
            TransitionSubType::CENTERRIGHT,
            TransitionSubType::TOP,
            TransitionSubType::BOTTOM,
            TransitionSubType::CLOCKWISETOP,
            TransitionSubType::CLOCKWISERIGHT,
            TransitionSubType::CLOCKWISEBOTTOM,
            TransitionSubType::CLOCKWISELEFT,
            TransitionSubType::CLOCKWISETOPLEFT,
            TransitionSubType::COUNTERCLOCKWISEBOTTOMLEFT,
            TransitionSubType::CLOCKWISEBOTTOMRIGHT,
            TransitionSubType::COUNTERCLOCKWISETOPRIGHT,
            TransitionSubType::VERTICALLEFT,
            TransitionSubType::VERTICALRIGHT,
            TransitionSubType::HORIZONTALLEFT,
            TransitionSubType::HORIZONTALRIGHT,
            TransitionSubType::TOPLEFTCLOCKWISE,
            TransitionSubType::TOPRIGHTCLOCKWISE,
            TransitionSubType::BOTTOMRIGHTCLOCKWISE,
            TransitionSubType::BOTTOMLEFTCLOCKWISE,
            TransitionSubType::TOPLEFTCOUNTERCLOCKWISE,
            TransitionSubType::TOPRIGHTCOUNTERCLOCKWISE,
            TransitionSubType::BOTTOMRIGHTCOUNTERCLOCKWISE,
            TransitionSubType::BOTTOMLEFTCOUNTERCLOCKWISE,
            TransitionSubType::DOUBLEBARNDOOR,
            TransitionSubType::DOUBLEDIAMOND,
            TransitionSubType::VERTICALTOPSAME,
            TransitionSubType::VERTICALBOTTOMSAME,
            TransitionSubType::VERTICALTOPLEFTOPPOSITE,
            TransitionSubType::VERTICALBOTTOMLEFTOPPOSITE,
            TransitionSubType::HORIZONTALLEFTSAME,
            TransitionSubType::HORIZONTALRIGHTSAME,
            TransitionSubType::HORIZONTALTOPLEFTOPPOSITE,
            TransitionSubType::HORIZONTALTOPRIGHTOPPOSITE,
            TransitionSubType::DIAGONALBOTTOMLEFTOPPOSITE,
            TransitionSubType::DIAGONALTOPLEFTOPPOSITE,
            TransitionSubType::TWOBOXTOP,
            TransitionSubType::TWOBOXBOTTOM,
            TransitionSubType::TWOBOXLEFT,
            TransitionSubType::TWOBOXRIGHT,
            TransitionSubType::FOURBOXVERTICAL,
            TransitionSubType::FOURBOXHORIZONTAL
    };

    try {
        xAnimationNode = implImportEffects( xServiceFactory, aURL );
        Reference< XEnumerationAccess > xEnumerationAccess( xAnimationNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), css::uno::UNO_SET_THROW );

        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            if( xChildNode->getType() == AnimationNodeType::PAR )
            {
                TransitionPresetPtr pPreset( new TransitionPreset( xChildNode ) );

                if( comphelper::LibreOfficeKit::isActive() )
                {
                    sal_Int16 eTransitionType = pPreset->getTransition();
                    sal_Int16 eTransitionSubType = pPreset->getSubtype();
                    if( LOKSupportedTransitionTypes.find(eTransitionType) == LOKSupportedTransitionTypes.end()
                            || LOKSupportedTransitionSubTypes.find(eTransitionSubType) == LOKSupportedTransitionSubTypes.end() )
                    {
                        continue;
                    }
                }

                OUString aPresetId( pPreset->getPresetId() );

                if( !aPresetId.isEmpty() )
                {
                    Reference< container::XNameAccess > xTransitionNode;

                    if (xTransitions->hasByName( aPresetId ) &&
                        (xTransitions->getByName( aPresetId ) >>= xTransitionNode) &&
                        xTransitionNode.is() )
                    {
                        OUString sSet;
                        OUString sVariant;

                        xTransitionNode->getByName( "Set" ) >>= sSet;
                        xTransitionNode->getByName( "Variant" ) >>= sVariant;

                        Reference< container::XNameAccess > xSetNode;

                        xTransitionSets->getByName( sSet ) >>= xSetNode;
                        if( xSetNode.is() )
                        {
                            pPreset->maSetId = sSet;
                            xSetNode->getByName( "Label" ) >>= sSet;
                            pPreset->maSetLabel = sSet;

                            OUString sGroup;

                            xSetNode->getByName( "Group" ) >>= sGroup;

                            Reference< container::XNameAccess > xGroupNode;
                            xTransitionGroups->getByName( sGroup ) >>= xGroupNode;

                            if( xGroupNode.is() )
                            {
                                xGroupNode->getByName( "Label" ) >>= sGroup;
                                if( !sVariant.isEmpty() )
                                {
                                    Reference< container::XNameAccess > xVariantNode;
                                    xTransitionVariants->getByName( sVariant ) >>= xVariantNode;
                                    if( xVariantNode.is() )
                                    {
                                        xVariantNode->getByName( "Label" ) >>= sVariant;
                                        pPreset->maVariantLabel = sVariant;
                                    }
                                }

                                pPreset->maSetLabel = sSet;
                                SAL_INFO("sd.transitions", aPresetId << ": " << sGroup << "/" << sSet << (sVariant.isEmpty() ? OUString() : OUString("/" + sVariant)));

                                rList.push_back( pPreset );
                            }
                            else
                                SAL_WARN("sd.transitions", "group node " << sGroup << " not found");
                        }
                        else
                            SAL_WARN("sd.transitions", "set node " << sSet << " not found");
                    }
                    else
                        SAL_WARN("sd.transitions", "transition node " << aPresetId << " not found");
                }
            }
            else
            {
                SAL_WARN("sd.transitions", " malformed xml configuration file " << aURL );
                break;
            }
        }
    } catch( Exception& ) {
        return false;
    }

    return true;
}

bool TransitionPreset::importTransitionPresetList( TransitionPresetList& rList )
{
    if (comphelper::IsFuzzing())
        return false;

    bool bRet = false;

    try
    {
        uno::Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager(), UNO_QUERY_THROW );

        // import ui strings
        Reference< XMultiServiceFactory > xConfigProvider =
            configuration::theDefaultProvider::get( xContext );

        // read path to transition effects files from config
        uno::Sequence< OUString > aFiles;
        aFiles = officecfg::Office::Impress::Misc::TransitionFiles::get();
        for (const auto& rFile : aFiles)
        {
            OUString aURL = comphelper::getExpandedUri(xContext, rFile);

            bRet |= importTransitionsFile( rList,
                                           xServiceFactory,
                                           aURL );
        }

        return bRet;
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::TransitionPreset::importResources()" );
    }

    return bRet;
}

std::map<OUString, TransitionPresetList> sd::TransitionPreset::mPresetsMap;

const TransitionPresetList& TransitionPreset::getTransitionPresetList()
{
    // Support localization per-view. Currently not useful for Desktop
    // but very much critical for LOK. The cache now is per-language.
    const OUString aLang = comphelper::LibreOfficeKit::isActive()
                               ? comphelper::LibreOfficeKit::getLanguageTag().getBcp47()
                               : SvtSysLocaleOptions().GetLanguageTag().getBcp47();

    SolarMutexGuard aGuard;
    const auto it = mPresetsMap.find(aLang);
    if (it != mPresetsMap.end())
        return it->second;

    TransitionPresetList& rList = mPresetsMap[aLang];
    sd::TransitionPreset::importTransitionPresetList(rList);
    return rList;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
