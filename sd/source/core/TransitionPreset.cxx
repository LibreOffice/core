/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>

#include <rtl/uri.hxx>
#include <tools/debug.hxx>

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include <TransitionPreset.hxx>
#endif
#include <unotools/ucbstreamhelper.hxx>

#include <algorithm>

#include "sdpage.hxx"

using namespace ::vos;
using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;

using ::rtl::OUString;
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

extern Reference< XAnimationNode > implImportEffects( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rPath );
extern void implImportLabels( const Reference< XMultiServiceFactory >& xConfigProvider, const OUString& rNodePath, UStringMap& rStringMap );

TransitionPreset::TransitionPreset( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
{
    // first locate preset id
    Sequence< NamedValue > aUserData( xNode->getUserData() );
    sal_Int32 nLength = aUserData.getLength();
    const NamedValue* p = aUserData.getConstArray();
    while( nLength-- )
    {
        if( p->Name.equalsAscii( "preset-id" ) )
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
                                              String aURL )
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
                if( aPresetId.getLength() )
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
                    DBG_ERROR( "sd::TransitionPreset::importTransitionPresetList(), missformed xml configuration file, giving up!" );
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
        // Get service factory
        Reference< XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );
        DBG_ASSERT( xServiceFactory.is(), "sd::CustomAnimationPresets::import(), got no service manager" );
        if( !xServiceFactory.is() )
            return false;

        uno::Reference< beans::XPropertySet > xProps( xServiceFactory, UNO_QUERY );
        uno::Reference< uno::XComponentContext > xContext;
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;

        uno::Reference< util::XMacroExpander > xMacroExpander;
        if( xContext.is() )
            xMacroExpander.set( xContext->getValueByName(
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))),
                                UNO_QUERY );

        // import ui strings
        Reference< XMultiServiceFactory > xConfigProvider(
            xServiceFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ))),
            UNO_QUERY_THROW );

        UStringMap aTransitionNameMape;
        const OUString aTransitionPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/UserInterface/Transitions" ) );
        implImportLabels( xConfigProvider, aTransitionPath, aTransitionNameMape );

        // read path to transition effects files from config
        Any propValue = uno::makeAny(
            beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("nodepath")), -1,
                uno::makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Impress/Misc"))),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")),
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
        uno::Sequence< rtl::OUString > aFiles;
        xNameAccess->getByName(
            OUString( RTL_CONSTASCII_USTRINGPARAM("TransitionFiles"))) >>= aFiles;

        for( sal_Int32 i=0; i<aFiles.getLength(); ++i )
        {
            rtl::OUString aURL = aFiles[i];
            if( aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( EXPAND_PROTOCOL )) == 0 )
            {
                // cut protocol
                rtl::OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
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
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR( "sd::TransitionPreset::importResources(), Exception caught!" );
    }

    return bRet;
}

TransitionPresetList* TransitionPreset::mpTransitionPresetList = 0;

const TransitionPresetList& TransitionPreset::getTransitionPresetList()
{
    if( !mpTransitionPresetList )
    {
        OGuard aGuard( Application::GetSolarMutex() );
        if( !mpTransitionPresetList )
        {
            mpTransitionPresetList = new sd::TransitionPresetList();
            sd::TransitionPreset::importTransitionPresetList( *mpTransitionPresetList );
        }
    }

    return *mpTransitionPresetList;
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
