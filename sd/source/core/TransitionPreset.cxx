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

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>

#include <rtl/uri.hxx>
#include <rtl/instance.hxx>
#include <tools/debug.hxx>

#include <TransitionPreset.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <algorithm>

#include "sdpage.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;

using ::com::sun::star::uno::UNO_QUERY;
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

extern Reference< XAnimationNode > implImportEffects( const Reference< XMultiServiceFactory >& xServiceFactory, const OUString& rPath );
extern void implImportLabels( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, UStringMap& rStringMap );

TransitionPreset::TransitionPreset( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
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
                                              Reference< XMultiServiceFactory >& xServiceFactory,
                                              UStringMap& rTransitionNameMape,
                                              OUString aURL )
{
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
                // create it
                TransitionPresetPtr pPreset( new TransitionPreset( xChildNode ) );

                // name it
                OUString aPresetId( pPreset->getPresetId() );
                if( !aPresetId.isEmpty() )
                {
                    UStringMap::const_iterator aIter( rTransitionNameMape.find( aPresetId ) );
                    if( aIter != rTransitionNameMape.end() )
                        pPreset->maUIName = (*aIter).second;

                                // add it
                    rList.push_back( pPreset );
                }
            }
            else
                {
                    OSL_FAIL( "sd::TransitionPreset::importTransitionPresetList(), misformed xml configuration file, giving up!" );
                    break;
                }
        }
    } catch( Exception& ) {
        return false;
    }

    return true;
}

#define EXPAND_PROTOCOL "vnd.sun.star.expand:"

bool TransitionPreset::importTransitionPresetList( TransitionPresetList& rList )
{
    bool bRet = false;

    try
    {
        uno::Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        Reference< XMultiServiceFactory > xServiceFactory(
            xContext->getServiceManager(), UNO_QUERY_THROW );

        uno::Reference< util::XMacroExpander > xMacroExpander =
            util::theMacroExpander::get(xContext);

        // import ui strings
        Reference< XMultiServiceFactory > xConfigProvider =
            configuration::theDefaultProvider::get( xContext );

        UStringMap aTransitionNameMape;
        const OUString aTransitionPath("/org.openoffice.Office.UI.Effects/UserInterface/Transitions" );
        implImportLabels( xConfigProvider, aTransitionPath, aTransitionNameMape );

        // read path to transition effects files from config
        Any propValue = uno::makeAny(
            beans::PropertyValue("nodepath", -1,
                uno::makeAny( OUString("/org.openoffice.Office.Impress/Misc")),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                Sequence<Any>( &propValue, 1 ) ),
                UNO_QUERY_THROW );
        uno::Sequence< OUString > aFiles;
        xNameAccess->getByName("TransitionFiles") >>= aFiles;

        for( sal_Int32 i=0; i<aFiles.getLength(); ++i )
        {
            OUString aURL = aFiles[i];
            if( aURL.startsWith( EXPAND_PROTOCOL ) )
            {
                // cut protocol
                OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
                // decode uric class chars
                aMacro = rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                // expand macro string
                aURL = xMacroExpander->expandMacros( aMacro );
            }

            bRet |= importTransitionsFile( rList,
                                           xServiceFactory,
                                           aTransitionNameMape,
                                           aURL );
        }

        return bRet;
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::TransitionPreset::importResources(), Exception cought!" );
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

void TransitionPreset::apply( SdPage* pSlide ) const
{
    if( pSlide )
    {
        pSlide->setTransitionType( mnTransition );
        pSlide->setTransitionSubtype( mnSubtype );
        pSlide->setTransitionDirection( mbDirection );
        pSlide->setTransitionFadeColor( mnFadeColor );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
