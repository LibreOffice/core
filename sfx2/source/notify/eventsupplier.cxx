/*************************************************************************
 *
 *  $RCSfile: eventsupplier.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:38:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//--------------------------------------------------------------------------------------------------------

#ifndef  _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef  _COM_SUN_STAR_UTL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef  _COM_SUN_STAR_UTL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif

#ifndef _SFX_APPUNO_HXX
#include <appuno.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <objsh.hxx>
#endif
#ifndef _SFX_SFXBASEMODEL_HXX_
#include <sfxbasemodel.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX
#include <evntconf.hxx>
#endif

#include <svtools/securityoptions.hxx>
#include <comphelper/processfactory.hxx>

#ifndef _SFX_EVENTSUPPLIER_HXX_
#include "eventsupplier.hxx"
#endif

#include "app.hxx"
#include "sfxresid.hxx"

#include "sfxsids.hrc"
#include "sfxlocal.hrc"
#include "docfile.hxx"
#include "viewfrm.hxx"
#include "frame.hxx"

//--------------------------------------------------------------------------------------------------------

#define MACRO_PRFIX         "macro://"
#define MACRO_POSTFIX       "()"

//--------------------------------------------------------------------------------------------------------

#define PROPERTYVALUE       ::com::sun::star::beans::PropertyValue
#define UNO_QUERY           ::com::sun::star::uno::UNO_QUERY

//--------------------------------------------------------------------------------------------------------
    //  --- XNameReplace ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::replaceByName( const OUSTRING & aName, const ANY & rElement )
                                throw( ILLEGALARGUMENTEXCEPTION, NOSUCHELEMENTEXCEPTION,
                                       WRAPPEDTARGETEXCEPTION, RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and replace the data
    long nCount = maEventNames.getLength();
    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
        {
            // check for correct type of the element
            if ( ::getCppuType( (const SEQUENCE < PROPERTYVALUE > *)0 ) == rElement.getValueType() )
            {
                // create Configuration at first, creation might call this method also and that would overwrite everything
                // we might have stored before!
                USHORT nID = (USHORT) SfxEventConfiguration::GetEventId_Impl( aName );
                if ( nID )
                {
                    SfxEventConfigItem_Impl* pConfig =
                        mpObjShell ? mpObjShell->GetEventConfig_Impl(TRUE) : SFX_APP()->GetEventConfig()->GetAppEventConfig_Impl();

                    ANY aValue;
                    BlowUpMacro( rElement, aValue, mpObjShell );

                    // pConfig becomes the owner of the new SvxMacro
                    SvxMacro *pMacro = ConvertToMacro( aValue, mpObjShell, FALSE );
                     pConfig->ConfigureEvent( nID, pMacro );
                    maEventData[i] = aValue;

                    SEQUENCE < PROPERTYVALUE > aProperties;
                    if ( aValue >>= aProperties )
                    {
                        long nCount = aProperties.getLength();
                        for ( long nIndex = 0; nIndex < nCount; nIndex++ )
                        {
                            if ( aProperties[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
                            {
                                ::rtl::OUString aType;
                                aProperties[ nIndex ].Value >>= aType;
                                break;
                            }
                        }
                    }
                }
            }
            else
                throw ILLEGALARGUMENTEXCEPTION();

            return;
        }
    }

    throw NOSUCHELEMENTEXCEPTION();
}

//--------------------------------------------------------------------------------------------------------
//  --- XNameAccess ---
//--------------------------------------------------------------------------------------------------------
ANY SAL_CALL SfxEvents_Impl::getByName( const OUSTRING& aName )
                                throw( NOSUCHELEMENTEXCEPTION, WRAPPEDTARGETEXCEPTION,
                                       RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and return the data

    long nCount = maEventNames.getLength();

    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
            return maEventData[i];
    }

    throw NOSUCHELEMENTEXCEPTION();

    return ANY();
}

//--------------------------------------------------------------------------------------------------------
SEQUENCE< OUSTRING > SAL_CALL SfxEvents_Impl::getElementNames() throw ( RUNTIMEEXCEPTION )
{
    return maEventNames;
}

//--------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL SfxEvents_Impl::hasByName( const OUSTRING& aName ) throw ( RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and return the data

    long nCount = maEventNames.getLength();

    for ( long i=0; i<nCount; i++ )
    {
        if ( maEventNames[i] == aName )
            return sal_True;
    }

    return sal_False;
}

//--------------------------------------------------------------------------------------------------------
//  --- XElementAccess ( parent of XNameAccess ) ---
//--------------------------------------------------------------------------------------------------------
UNOTYPE SAL_CALL SfxEvents_Impl::getElementType() throw ( RUNTIMEEXCEPTION )
{
    UNOTYPE aElementType = ::getCppuType( (const SEQUENCE < PROPERTYVALUE > *)0 );
    return aElementType;
}

//--------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL SfxEvents_Impl::hasElements() throw ( RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( maEventNames.getLength() )
        return sal_True;
    else
        return sal_False;
}

//--------------------------------------------------------------------------------------------------------
// --- ::document::XEventListener ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::notifyEvent( const DOCEVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );

    // get the event name, find the coresponding data, execute the data

    OUSTRING    aName   = aEvent.EventName;
    long        nCount  = maEventNames.getLength();
    long        nIndex  = 0;
    sal_Bool    bFound  = sal_False;

    while ( !bFound && ( nIndex < nCount ) )
    {
        if ( maEventNames[nIndex] == aName )
            bFound = sal_True;
        else
            nIndex += 1;
    }

    if ( !bFound )
        return;

    SEQUENCE < PROPERTYVALUE > aProperties;
    ANY aEventData = maEventData[ nIndex ];

    if ( aEventData >>= aProperties )
    {
        OUSTRING        aPrefix = OUSTRING( RTL_CONSTASCII_USTRINGPARAM( MACRO_PRFIX ) );
        OUSTRING        aType;
        OUSTRING        aScript;
        OUSTRING        aLibrary;
        OUSTRING        aMacroName;

        nCount = aProperties.getLength();

        if ( !nCount )
            return;

        nIndex = 0;
        while ( nIndex < nCount )
        {
            if ( aProperties[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
                aProperties[ nIndex ].Value >>= aType;
            else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
                aProperties[ nIndex ].Value >>= aScript;
            else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
                aProperties[ nIndex ].Value >>= aLibrary;
            else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
                aProperties[ nIndex ].Value >>= aMacroName;
            else
                DBG_ERROR("Unknown property value!");
            nIndex += 1;
        }

        if ( aType.compareToAscii( STAR_BASIC ) == 0 && aScript.getLength() )
        {
            aGuard.clear();
            SfxMacroLoader::loadMacro( aScript, mpObjShell );
        }
        else if ( aType.compareToAscii( "Service" ) == 0  || ( aType.compareToAscii( "Script" ) == 0 ) )
        {
            if ( aScript.getLength() )
            {
                SfxViewFrame* pView = mpObjShell ? SfxViewFrame::GetFirst( mpObjShell ) : SfxViewFrame::Current();
                ::com::sun::star::util::URL aURL;
                aURL.Complete = aScript;
                ::com::sun::star::uno::Reference < ::com::sun::star::util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
                xTrans->parseStrict( aURL );

                ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchProvider > xProv( pView->GetFrame()->GetFrameInterface(), UNO_QUERY );
                ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > xDisp;
                if ( xProv.is() )
                    xDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
                if ( xDisp.is() )
                {
                    //::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aArgs(1);
                    //aArgs[0].Name = rtl::OUString::createFromAscii("Referer");
                    //aArs[0].Value <<= ::rtl::OUString( mpObjShell->GetMedium()->GetName() );
                    //xDisp->dispatch( aURL, aArgs );
                    xDisp->dispatch( aURL, ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >() );
                }
            }
        }
        else
        {
            DBG_ERRORFILE( "notifyEvent(): Unsupported event type" );
        }
    }
}

//--------------------------------------------------------------------------------------------------------
// --- ::lang::XEventListener ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::disposing( const EVENTOBJECT& Source ) throw( RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mxBroadcaster.is() )
    {
        mxBroadcaster->removeEventListener( this );
        mxBroadcaster = NULL;
    }
}

//--------------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------------
SfxEvents_Impl::SfxEvents_Impl( SfxObjectShell* pShell,
                                REFERENCE< XEVENTBROADCASTER > xBroadcaster )
{
    // get the list of supported events and store it
    if ( pShell )
        maEventNames = pShell->GetEventNames();
    else
        maEventNames = SfxObjectShell::GetEventNames_Impl();

    maEventData = SEQUENCE < ANY > ( maEventNames.getLength() );

    mpObjShell      = pShell;
    mxBroadcaster   = xBroadcaster;

    if ( mxBroadcaster.is() )
        mxBroadcaster->addEventListener( this );
}

//--------------------------------------------------------------------------------------------------------
SfxEvents_Impl::~SfxEvents_Impl()
{
    if ( mxBroadcaster.is() )
        mxBroadcaster->removeEventListener( this );
}

//--------------------------------------------------------------------------------------------------------
SvxMacro* SfxEvents_Impl::ConvertToMacro( const ANY& rElement, SfxObjectShell* pObjShell, BOOL bBlowUp )
{
    SvxMacro* pMacro = NULL;
    SEQUENCE < PROPERTYVALUE > aProperties;
    ANY aAny;
    if ( bBlowUp )
        BlowUpMacro( rElement, aAny, pObjShell );
    else
        aAny = rElement;

    if ( aAny >>= aProperties )
    {
        OUSTRING        aType;
        OUSTRING        aScriptURL;
        OUSTRING        aLibrary;
        OUSTRING        aMacroName;

        long nCount = aProperties.getLength();
        long nIndex = 0;

        if ( !nCount )
            return pMacro;

        while ( nIndex < nCount )
        {
            if ( aProperties[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
                aProperties[ nIndex ].Value >>= aType;
            else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
                aProperties[ nIndex ].Value >>= aScriptURL;
            else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
                aProperties[ nIndex ].Value >>= aLibrary;
            else if ( aProperties[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
                aProperties[ nIndex ].Value >>= aMacroName;
            else
                DBG_ERROR("Unknown propery value!");
            nIndex += 1;
        }

        // Get the type
        ScriptType  eType( STARBASIC );
        if ( aType.compareToAscii( STAR_BASIC ) == COMPARE_EQUAL )
            eType = STARBASIC;
        else if ( aType.compareToAscii( "Script" ) == COMPARE_EQUAL && aScriptURL.getLength() )
            eType = EXTENDED_STYPE;
        else if ( aType.compareToAscii( SVX_MACRO_LANGUAGE_JAVASCRIPT ) == COMPARE_EQUAL )
            eType = JAVASCRIPT;
        else
            DBG_ERRORFILE( "ConvertToMacro: Unknown macro type" );

        if ( aMacroName.getLength() )
            pMacro = new SvxMacro( aMacroName, aLibrary, eType );
        else if ( eType == EXTENDED_STYPE )
            pMacro = new SvxMacro( aScriptURL, aType );
    }

    return pMacro;
}

void SfxEvents_Impl::BlowUpMacro( const ANY& rEvent, ANY& rRet, SfxObjectShell* pDoc )
{
    SEQUENCE < PROPERTYVALUE > aInProps;
    SEQUENCE < PROPERTYVALUE > aOutProps(2);

    if ( !( rEvent >>= aInProps ) )
        return;

    sal_Int32 nCount = aInProps.getLength();

    if ( !nCount )
        return;

    OUSTRING aType;
    OUSTRING aScript;
    OUSTRING aLibrary;
    OUSTRING aMacroName;

    sal_Int32 nIndex = 0;

    while ( nIndex < nCount )
    {
        if ( aInProps[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
        {
            aInProps[nIndex].Value >>= aType;
            aOutProps[0] = aInProps[nIndex];
        }
        else if ( aInProps[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
        {
            aInProps[nIndex].Value >>= aScript;
            aOutProps[1] = aInProps[nIndex];
        }
        else if ( aInProps[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
        {
            aInProps[ nIndex ].Value >>= aLibrary;
        }
        else if ( aInProps[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
        {
            aInProps[ nIndex ].Value >>= aMacroName;
        }
        nIndex += 1;
    }

    if ( aType.compareToAscii( STAR_BASIC ) == 0 )
    {
        aOutProps.realloc(4);
        if ( aScript.getLength() && ( ! aMacroName.getLength() || ! aLibrary.getLength() ) )
        {
            sal_Int32 nHashPos = aScript.indexOf( '/', 8 );
            sal_Int32 nArgsPos = aScript.indexOf( '(' );

            if ( ( nHashPos != STRING_NOTFOUND ) && ( nHashPos < nArgsPos ) )
            {
                OUSTRING aBasMgrName( INetURLObject::decode( aScript.copy( 8, nHashPos-8 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET ) );
                if ( !pDoc )
                    pDoc = SfxObjectShell::Current();

                if ( aBasMgrName.compareToAscii(".") == 0 )
                    aLibrary = pDoc->GetTitle( SFX_TITLE_APINAME );
                else if ( aBasMgrName.getLength() )
                    aLibrary = aBasMgrName;
                else
                    aLibrary = SFX_APP()->GetName();

                // Get the macro name
                aMacroName = aScript.copy( nHashPos+1, nArgsPos - nHashPos - 1 );
            }
            else
            {
                DBG_ERRORFILE( "ConvertToMacro: Unknown macro url format" );
            }
        }
        else if ( !aScript.getLength() && aMacroName.getLength() && aLibrary.getLength() )
        {
            aScript = OUSTRING( RTL_CONSTASCII_USTRINGPARAM( MACRO_PRFIX ) );
            if ( aLibrary.compareTo( SFX_APP()->GetName() ) != 0
                    && aLibrary.compareToAscii("StarDesktop") != 0 )
            {
                aScript += OUSTRING('.');
            }

            aScript += OUSTRING('/');
            aScript += aMacroName;
            aScript += OUSTRING( RTL_CONSTASCII_USTRINGPARAM( MACRO_POSTFIX ) );
        }
        else
            // wrong properties
            return;

        aOutProps[1].Name = OUSTRING::createFromAscii( PROP_SCRIPT );
        aOutProps[1].Value <<= aScript;
        aOutProps[2].Name = OUSTRING::createFromAscii( PROP_LIBRARY );
        aOutProps[2].Value <<= aLibrary;
        aOutProps[3].Name = OUSTRING::createFromAscii( PROP_MACRO_NAME );
        aOutProps[3].Value <<= aMacroName;
        rRet <<= aOutProps;
    }
}

SFX_IMPL_XSERVICEINFO( SfxGlobalEvents_Impl, "com.sun.star.frame.GlobalEventBroadcaster", "com.sun.star.comp.sfx2.GlobalEventBroadcaster" )
SFX_IMPL_ONEINSTANCEFACTORY( SfxGlobalEvents_Impl );

SfxGlobalEvents_Impl::SfxGlobalEvents_Impl( const com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& )
    : m_aInterfaceContainer( m_aMutex )
{
    m_refCount++;
    pImp = new SfxEvents_Impl( NULL, this );
    m_xEvents = pImp;
    m_refCount--;
    StartListening(*SFX_APP());
}

SfxGlobalEvents_Impl::~SfxGlobalEvents_Impl()
{
}

REFERENCE< XNAMEREPLACE > SAL_CALL SfxGlobalEvents_Impl::getEvents() throw( RUNTIMEEXCEPTION )
{
    return m_xEvents;
}

void SAL_CALL SfxGlobalEvents_Impl::addEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
{
    m_aInterfaceContainer.addInterface( xListener );
}

void SAL_CALL SfxGlobalEvents_Impl::removeEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION )
{
    m_aInterfaceContainer.removeInterface( xListener );
}

void SfxGlobalEvents_Impl::Notify( SfxBroadcaster& aBC, const SfxHint& aHint )
{
    SfxEventHint* pNamedHint = PTR_CAST( SfxEventHint, &aHint );
    if ( pNamedHint )
    {
        OUSTRING aName = SfxEventConfiguration::GetEventName_Impl( pNamedHint->GetEventId() );
        REFERENCE < XEVENTSSUPPLIER > xSup;
        if ( pNamedHint->GetObjShell() )
            xSup = REFERENCE < XEVENTSSUPPLIER >( pNamedHint->GetObjShell()->GetModel(), UNO_QUERY );
//      else
//          xSup = (XEVENTSSUPPLIER*) this;

        DOCEVENTOBJECT aEvent( xSup, aName );
        ::cppu::OInterfaceIteratorHelper aIt( m_aInterfaceContainer );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
            }
            catch( RUNTIMEEXCEPTION& )
            {
                aIt.remove();
            }
        }
    }
}
