/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TransitionPreset.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-23 12:12:54 $
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
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_ANIMATIONS_XTIMECONTAINER_HPP_
#include <com/sun/star/animations/XTimeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XTRANSITIONFILTER_HPP_
#include <com/sun/star/animations/XTransitionFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONNODETYPE_HPP_
#include <com/sun/star/animations/AnimationNodeType.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#include <tools/debug.hxx>

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include <TransitionPreset.hxx>
#endif

#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#include <algorithm>

#include "sdpage.hxx"
#include <hash_map>

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

typedef std::hash_map<rtl::OUString, rtl::OUString, comphelper::UStringHash, comphelper::UStringEqual> UStringMap;
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

bool TransitionPreset::importTransitionPresetList( TransitionPresetList& rList )
{
    try
    {
        // Get service factory
        Reference< XMultiServiceFactory > xServiceFactory( comphelper::getProcessServiceFactory() );
        DBG_ASSERT( xServiceFactory.is(), "sd::CustomAnimationPresets::import(), got no service manager" );
        if( !xServiceFactory.is() )
            return false;

        // import ui strings
        Reference< XMultiServiceFactory > xConfigProvider(
            xServiceFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ))),
            UNO_QUERY_THROW );

        UStringMap aTransitionNameMape;
        const OUString aTransitionPath( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Effects/UserInterface/Transitions" ) );
        implImportLabels( xConfigProvider, aTransitionPath, aTransitionNameMape );

        // import transition presets
        INetURLObject   aURL( SvtPathOptions().GetConfigPath() );
        aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM("soffice.cfg") ) );
        aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM("simpress" ) ) );
        aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM("transitions.xml" ) ) );

        Reference< XEnumerationAccess > xEnumerationAccess( implImportEffects( xServiceFactory, aURL.GetMainURL( INetURLObject::NO_DECODE ) ), UNO_QUERY_THROW );
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
                    UStringMap::const_iterator aIter( aTransitionNameMape.find( aPresetId ) );
                    if( aIter != aTransitionNameMape.end() )
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

        return true;
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR( "sd::TransitionPreset::importResources(), Exception cought!" );
    }

    return false;
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

