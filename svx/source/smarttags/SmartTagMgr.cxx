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

// SMARTTAGS

#include <svx/SmartTagMgr.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/smarttags/XSmartTagRecognizer.hpp>
#include <com/sun/star/smarttags/XRangeBasedSmartTagRecognizer.hpp>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/text/XTextMarkup.hpp>
#include <com/sun/star/smarttags/SmartTagRecognizerMode.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/text/XTextRange.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::i18n;


SmartTagMgr::SmartTagMgr( const OUString& rApplicationName )
    : maApplicationName( rApplicationName ),
      maRecognizerList(),
      maActionList(),
      maDisabledSmartTagTypes(),
      maSmartTagMap(),
      mxContext( ::comphelper::getProcessComponentContext() ),
      mbLabelTextWithSmartTags(true)
{
}

SmartTagMgr::~SmartTagMgr()
{
}

void SmartTagMgr::Init( const OUString& rConfigurationGroupName )
{
    PrepareConfiguration( rConfigurationGroupName );
    ReadConfiguration( true, true );
    RegisterListener();
    LoadLibraries();
}
void SmartTagMgr::CreateBreakIterator() const
{
    if ( !mxBreakIter.is() )
    {
        // get the break iterator
        mxBreakIter.set( BreakIterator::create(mxContext) );
    }
}

/** Dispatches the recognize call to all installed smart tag recognizers
*/
void SmartTagMgr::RecognizeString( const OUString& rText,
                             const Reference< text::XTextMarkup >& xMarkup,
                             const Reference< frame::XController >& xController,
                             const lang::Locale& rLocale,
                             sal_uInt32 nStart, sal_uInt32 nLen ) const
{
    for ( size_t i = 0; i < maRecognizerList.size(); i++ )
    {
        Reference < smarttags::XSmartTagRecognizer > xRecognizer = maRecognizerList[i];

        // if all smart tag types supported by this recognizer have been
        // disabled, we do not have to call the recognizer:
        bool bCallRecognizer = false;
        const sal_uInt32 nSmartTagCount = xRecognizer->getSmartTagCount();
        for ( sal_uInt32 j = 0; j < nSmartTagCount && !bCallRecognizer; ++j )
        {
            const OUString aSmartTagName = xRecognizer->getSmartTagName(j);
            if ( IsSmartTagTypeEnabled( aSmartTagName ) )
                bCallRecognizer = true;
        }

        if ( bCallRecognizer )
        {
            CreateBreakIterator();
            maRecognizerList[i]->recognize( rText, nStart, nLen,
                                            smarttags::SmartTagRecognizerMode_PARAGRAPH,
                                            rLocale, xMarkup, maApplicationName, xController,
                                            mxBreakIter );
        }
    }
}

void SmartTagMgr::RecognizeTextRange(const Reference< text::XTextRange>& xRange,
                             const Reference< text::XTextMarkup >& xMarkup,
                             const Reference< frame::XController >& xController) const
{
    for ( size_t i = 0; i < maRecognizerList.size(); i++ )
    {
        Reference < smarttags::XSmartTagRecognizer > xRecognizer = maRecognizerList[i];

        Reference< smarttags::XRangeBasedSmartTagRecognizer > xRangeBasedRecognizer( xRecognizer, UNO_QUERY);

        if (!xRangeBasedRecognizer.is()) continue;

        // if all smart tag types supported by this recognizer have been
        // disabled, we do not have to call the recognizer:
        bool bCallRecognizer = false;
        const sal_uInt32 nSmartTagCount = xRecognizer->getSmartTagCount();
        for ( sal_uInt32 j = 0; j < nSmartTagCount && !bCallRecognizer; ++j )
        {
            const rtl::OUString aSmartTagName = xRecognizer->getSmartTagName(j);
            if ( IsSmartTagTypeEnabled( aSmartTagName ) )
                bCallRecognizer = true;
        }

        if ( bCallRecognizer )
        {
            xRangeBasedRecognizer->recognizeTextRange( xRange,
                                                       smarttags::SmartTagRecognizerMode_PARAGRAPH,
                                                       xMarkup, maApplicationName, xController);
        }
    }

}

typedef std::multimap < OUString, ActionReference >::const_iterator SmartTagMapIter;

void SmartTagMgr::GetActionSequences( Sequence < OUString >& rSmartTagTypes,
                                      Sequence < Sequence< Reference< smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                                      Sequence < Sequence< sal_Int32 > >& rActionIndicesSequence ) const
{
    rActionComponentsSequence.realloc( rSmartTagTypes.getLength() );
    rActionIndicesSequence.realloc( rSmartTagTypes.getLength() );

    for ( sal_Int32 j = 0; j < rSmartTagTypes.getLength(); ++j )
    {
        const OUString& rSmartTagType = rSmartTagTypes[j];

        const sal_Int32 nNumberOfActionRefs = maSmartTagMap.count( rSmartTagType );

        Sequence< Reference< smarttags::XSmartTagAction > > aActions( nNumberOfActionRefs );
        Sequence< sal_Int32 > aIndices( nNumberOfActionRefs );

        sal_uInt16 i = 0;
        SmartTagMapIter aActionsIter;
        SmartTagMapIter aEnd = maSmartTagMap.upper_bound( rSmartTagType );

        for ( aActionsIter = maSmartTagMap.lower_bound( rSmartTagType ); aActionsIter != aEnd; ++aActionsIter )
        {
            aActions[ i ] = (*aActionsIter).second.mxSmartTagAction;
            aIndices[ i++ ] = (*aActionsIter).second.mnSmartTagIndex;
        }

        rActionComponentsSequence[ j ] = aActions;
        rActionIndicesSequence[ j ]  = aIndices;
    }
}

/** Returns the caption for a smart tag type.
*/
OUString SmartTagMgr::GetSmartTagCaption( const OUString& rSmartTagType, const css::lang::Locale& rLocale ) const
{
    OUString aRet;

    SmartTagMapIter aLower = maSmartTagMap.lower_bound( rSmartTagType );

    if ( aLower != maSmartTagMap.end() )
    {
        const ActionReference& rActionRef = (*aLower).second;
        Reference< smarttags::XSmartTagAction > xAction = rActionRef.mxSmartTagAction;

        if ( xAction.is() )
        {
            const sal_Int32 nSmartTagIndex = rActionRef.mnSmartTagIndex;
            aRet = xAction->getSmartTagCaption( nSmartTagIndex, rLocale );
        }
    }

    return aRet;
}


/** Returns true if the given smart tag type is enabled.
*/
bool SmartTagMgr::IsSmartTagTypeEnabled( const OUString& rSmartTagType ) const
{
    return maDisabledSmartTagTypes.end() == maDisabledSmartTagTypes.find( rSmartTagType );
}

/** Writes currently disabled smart tag types to configuration.
*/
void SmartTagMgr::WriteConfiguration( const bool* pIsLabelTextWithSmartTags,
                                      const std::vector< OUString >* pDisabledTypes ) const
{
    if ( mxConfigurationSettings.is() )
    {
        bool bCommit = false;

        if ( pIsLabelTextWithSmartTags )
        {
            const Any aEnabled = makeAny( *pIsLabelTextWithSmartTags );

            try
            {
                mxConfigurationSettings->setPropertyValue( "RecognizeSmartTags", aEnabled );
                bCommit = true;
            }
            catch ( css::uno::Exception& )
            {
            }
        }

        if ( pDisabledTypes )
        {
            const sal_Int32 nNumberOfDisabledSmartTagTypes = pDisabledTypes->size();
            Sequence< OUString > aTypes( nNumberOfDisabledSmartTagTypes );

            std::vector< OUString >::const_iterator aIter;
            sal_Int32 nCount = 0;
            for ( aIter = pDisabledTypes->begin(); aIter != pDisabledTypes->end(); ++aIter )
                aTypes[ nCount++ ] = *aIter;

            const Any aNewTypes = makeAny( aTypes );

            try
            {
                mxConfigurationSettings->setPropertyValue( "ExcludedSmartTagTypes", aNewTypes );
                bCommit = true;
            }
            catch ( css::uno::Exception& )
            {
            }
        }

        if ( bCommit )
        {
            try
            {
                Reference< util::XChangesBatch >( mxConfigurationSettings, UNO_QUERY_THROW )->commitChanges();
            }
            catch ( css::uno::Exception& )
            {
            }
        }
    }
}

// css::util::XModifyListener
void SmartTagMgr::modified( const lang::EventObject& )  throw( RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    maRecognizerList.clear();
    maActionList.clear();
    maSmartTagMap.clear();

    LoadLibraries();
}

// css::lang::XEventListener
void SmartTagMgr::disposing( const lang::EventObject& rEvent ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    uno::Reference< frame::XModel >  xModel( rEvent.Source, uno::UNO_QUERY );
    uno::Reference< util::XModifyBroadcaster >  xMB(xModel, uno::UNO_QUERY);
    uno::Reference< util::XChangesNotifier >  xCN(xModel, uno::UNO_QUERY);

    try
    {
        if( xMB.is() )
        {
            uno::Reference< util::XModifyListener >  xListener( this );
            xMB->removeModifyListener( xListener );
        }
        else if ( xCN.is() )
        {
            uno::Reference< util::XChangesListener >  xListener( this );
            xCN->removeChangesListener( xListener );
        }
    }
    catch(Exception& )
    {
    }
}

// css::util::XChangesListener
void SmartTagMgr::changesOccurred( const util::ChangesEvent& rEvent ) throw( RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const util::ElementChange* pElementChanges = rEvent.Changes.getConstArray();
    const sal_Int32 nNumberOfChanges = rEvent.Changes.getLength();
    bool bExcludedTypes = false;
    bool bRecognize = false;

    for( sal_Int32 i = 0; i < nNumberOfChanges; ++i)
    {
        OUString sTemp;
        pElementChanges[i].Accessor >>= sTemp;

        if ( sTemp == "ExcludedSmartTagTypes" )
            bExcludedTypes = true;
        else if ( sTemp == "RecognizeSmartTags" )
            bRecognize = true;
    }

    ReadConfiguration( bExcludedTypes, bRecognize );
}

/**
*/
void SmartTagMgr::LoadLibraries()
{
    Reference< container::XContentEnumerationAccess > rContent( mxContext->getServiceManager(), UNO_QUERY_THROW );

    // load recognizers: No recognizers -> nothing to do.
    Reference < container::XEnumeration > rEnum = rContent->createContentEnumeration( "com.sun.star.smarttags.SmartTagRecognizer");
    if ( !rEnum.is() || !rEnum->hasMoreElements() )
        return;

    // iterate over all implementations of the smart tag recognizer service:
    while( rEnum->hasMoreElements())
    {
        const Any a = rEnum->nextElement();
        Reference< lang::XSingleComponentFactory > xSCF;
        Reference< lang::XServiceInfo > xsInfo;

        if (a >>= xsInfo)
            xSCF.set(xsInfo, UNO_QUERY);
        else
            continue;

        Reference< smarttags::XSmartTagRecognizer > xLib ( xSCF->
                   createInstanceWithContext(mxContext), UNO_QUERY );

        if (!xLib.is())
            continue;

        xLib->initialize( Sequence< Any >() );
        maRecognizerList.push_back(xLib);
    }

    // load actions: No actions -> nothing to do.
    rEnum = rContent->createContentEnumeration( "com.sun.star.smarttags.SmartTagAction");
    if ( !rEnum.is() )
        return;

    // iterate over all implementations of the smart tag action service:
    while( rEnum->hasMoreElements())
    {
        const Any a = rEnum->nextElement();
        Reference< lang::XServiceInfo > xsInfo;
        Reference< lang::XSingleComponentFactory > xSCF;

        if (a >>= xsInfo)
            xSCF.set(xsInfo, UNO_QUERY);
        else
            continue;

        Reference< smarttags::XSmartTagAction > xLib ( xSCF->
                    createInstanceWithContext(mxContext), UNO_QUERY );

        if (!xLib.is())
            continue;

        xLib->initialize( Sequence< Any >() );
        maActionList.push_back(xLib);
    }

    AssociateActionsWithRecognizers();

}

/**
*/
void SmartTagMgr::PrepareConfiguration( const OUString& rConfigurationGroupName )
{
    Any aAny = makeAny( "/org.openoffice.Office.Common/SmartTags/" + rConfigurationGroupName );
    beans::PropertyValue aPathArgument;
    aPathArgument.Name = "nodepath";
    aPathArgument.Value = aAny;
    Sequence< Any > aArguments( 1 );
    aArguments[ 0 ] <<= aPathArgument;
    Reference< lang::XMultiServiceFactory > xConfProv = configuration::theDefaultProvider::get( mxContext );

    // try to get read-write access to configuration:
    Reference< XInterface > xConfigurationAccess;
    try
    {
        xConfigurationAccess = xConfProv->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess", aArguments );
    }
    catch ( uno::Exception& )
    {
    }

    // fallback: try read-only access to configuration:
    if ( !xConfigurationAccess.is() )
    {
        try
        {
            xConfigurationAccess = xConfProv->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess", aArguments );
        }
        catch ( uno::Exception& )
        {
        }
    }

    if ( xConfigurationAccess.is() )
    {
        mxConfigurationSettings.set( xConfigurationAccess, UNO_QUERY );
    }
}


void SmartTagMgr::ReadConfiguration( bool bExcludedTypes, bool bRecognize )
{
    if ( mxConfigurationSettings.is() )
    {
        if ( bExcludedTypes )
        {
            maDisabledSmartTagTypes.clear();

            Any aAny = mxConfigurationSettings->getPropertyValue( "ExcludedSmartTagTypes" );
            Sequence< OUString > aValues;
            aAny >>= aValues;

            const sal_Int32 nValues = aValues.getLength();

            for ( sal_Int32 nI = 0; nI < nValues; ++nI )
                maDisabledSmartTagTypes.insert( aValues[nI] );
        }

        if ( bRecognize )
        {
            Any aAny = mxConfigurationSettings->getPropertyValue( "RecognizeSmartTags" );
            bool bValue = true;
            aAny >>= bValue;

            mbLabelTextWithSmartTags = bValue;
        }
    }
}

/**
*/
void SmartTagMgr::RegisterListener()
{
    // register as listener at package manager
    try
    {
        Reference<deployment::XExtensionManager> xExtensionManager(
                deployment::ExtensionManager::get( mxContext ) );
        Reference< util::XModifyBroadcaster > xMB ( xExtensionManager, UNO_QUERY_THROW );

        Reference< util::XModifyListener > xListener( this );
        xMB->addModifyListener( xListener );
    }
    catch ( uno::Exception& )
    {
    }

    // register as listener at configuration
    try
    {
        Reference<util::XChangesNotifier> xCN( mxConfigurationSettings, UNO_QUERY_THROW );
        Reference< util::XChangesListener > xListener( this );
        xCN->addChangesListener( xListener );
    }
    catch ( uno::Exception& )
    {
    }
}

typedef std::pair < const OUString, ActionReference > SmartTagMapElement;

/** Sets up a map that maps smart tag type names to actions references.
*/
void SmartTagMgr::AssociateActionsWithRecognizers()
{
    const sal_uInt32 nActionLibCount = maActionList.size();
    const sal_uInt32 nRecognizerCount = maRecognizerList.size();

    for ( sal_uInt32 i = 0; i < nRecognizerCount; ++i )
    {
        Reference < smarttags::XSmartTagRecognizer > xRecognizer = maRecognizerList[i];
        const sal_uInt32 nSmartTagCount = xRecognizer->getSmartTagCount();
        for ( sal_uInt32 j = 0; j < nSmartTagCount; ++j )
        {
            const OUString aSmartTagName = xRecognizer->getSmartTagName(j);

            // check if smart tag type has already been processed:
            if ( maSmartTagMap.find( aSmartTagName ) != maSmartTagMap.end() )
                continue;

            bool bFound = false;
            for ( sal_uInt32 k = 0; k < nActionLibCount; ++k )
            {
                Reference< smarttags::XSmartTagAction > xActionLib = maActionList[k];
                const sal_uInt32 nSmartTagCountInActionLib = xActionLib->getSmartTagCount();
                for ( sal_uInt32 l = 0; l < nSmartTagCountInActionLib; ++l )
                {
                    const OUString aSmartTagNameInActionLib = xActionLib->getSmartTagName(l);
                    if ( aSmartTagName.equals( aSmartTagNameInActionLib ) )
                    {
                        // found actions and recognizer for same smarttag
                        ActionReference aActionRef( xActionLib, l );

                        // add recognizer/action pair to map
                        maSmartTagMap.insert( SmartTagMapElement( aSmartTagName, aActionRef ));

                        bFound = true;
                    }
                }
            }

            if ( !bFound )
            {
                // insert 'empty' action reference if there is no action associated with
                // the current smart tag type:
                Reference< smarttags::XSmartTagAction > xActionLib;
                ActionReference aActionRef( xActionLib, 0 );

                // add recognizer/action pair to map
                maSmartTagMap.insert( SmartTagMapElement( aSmartTagName, aActionRef ));
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
