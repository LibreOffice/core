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

#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <unotools/configmgr.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <officecfg/Office/UI/Effects.hxx>

#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

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
    Sequence< NamedValue > aUserData( xNode->getUserData() );
    sal_Int32 nLength = aUserData.getLength();
    const NamedValue* p = aUserData.getConstArray();
    while( nLength-- )
    {
        if ( p->Name == "preset-id" )
        {
            p->Value >>= maPresetId;
            break;
        }
    }

    // second, locate transition filter element
    Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
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

    try {
        xAnimationNode = implImportEffects( xServiceFactory, aURL );
        Reference< XEnumerationAccess > xEnumerationAccess( xAnimationNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );

        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            if( xChildNode->getType() == AnimationNodeType::PAR )
            {
                TransitionPresetPtr pPreset( new TransitionPreset( xChildNode ) );

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
    if (utl::ConfigManager::IsFuzzing())
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
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(OUString("/org.openoffice.Office.Impress/Misc"))}
        }));
        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                aArgs),
                UNO_QUERY_THROW );
        uno::Sequence< OUString > aFiles;
        xNameAccess->getByName("TransitionFiles") >>= aFiles;

        for( sal_Int32 i=0; i<aFiles.getLength(); ++i )
        {
            OUString aURL = comphelper::getExpandedUri(xContext, aFiles[i]);

            bRet |= importTransitionsFile( rList,
                                           xServiceFactory,
                                           aURL );
        }

        return bRet;
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::TransitionPreset::importResources(), exception caught!" );
    }

    return bRet;
}

namespace
{
    class ImportedTransitionPresetList
    {
    private:
        sd::TransitionPresetList m_aTransitionPresetList;
    public:
        ImportedTransitionPresetList()
        {
            sd::TransitionPreset::importTransitionPresetList(
                m_aTransitionPresetList);
        }
        const sd::TransitionPresetList& getList() const
        {
            return m_aTransitionPresetList;
        }
    };

    class theTransitionPresetList :
        public rtl::Static<ImportedTransitionPresetList,
                           theTransitionPresetList>
    {
    };
}

const TransitionPresetList& TransitionPreset::getTransitionPresetList()
{
    return theTransitionPresetList::get().getList();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
