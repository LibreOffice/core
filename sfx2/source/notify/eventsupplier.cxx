/*************************************************************************
 *
 *  $RCSfile: eventsupplier.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dv $ $Date: 2001-02-22 14:36:24 $
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

#ifndef _SFX_EVENTSUPPLIER_HXX_
#include "eventsupplier.hxx"
#endif

#include "app.hxx"
#include "sfxresid.hxx"

#include "sfxsids.hrc"
#include "sfxlocal.hrc"

//--------------------------------------------------------------------------------------------------------

#define MACRO_PRFIX         "macro://"
#define MACRO_POSTFIX       "()"

//--------------------------------------------------------------------------------------------------------

#define PROPERTYVALUE       ::com::sun::star::beans::PropertyValue
#define UNO_QUERY           ::com::sun::star::uno::UNO_QUERY

//--------------------------------------------------------------------------------------------------------
    //  --- XNameReplace ---
//--------------------------------------------------------------------------------------------------------
void SAL_CALL SfxEvents_Impl::replaceByName( const OUSTRING & aName, const ANY & aElement )
                                throw( ILLEGALARGUMENTEXCEPTION, NOSUCHELEMENTEXCEPTION,
                                       WRAPPEDTARGETEXCEPTION, RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( maMutex );

    // find the event in the list and replace the data

    long        nCount  = maEventNames.getLength();
    OUSTRING*   pNames  = maEventNames.getArray();

    for ( long i=0; i<nCount; i++ )
    {
        if ( pNames[i] == aName )
        {
            ANY* pElement = maEventData.getArray();
            // check for correct type of the element
            if ( ::getCppuType( (const SEQUENCE < PROPERTYVALUE > *)0 ) ==
                 aElement.getValueType() )
            {
                pElement[i] = aElement;

                SvxMacro   *pMacro = ConvertToMacro( aElement );
                USHORT      nID = (USHORT) SfxEventConfiguration::GetEventId_Impl( aName );

                if ( nID && pMacro )
                    SFX_APP()->GetEventConfig()->ConfigureEvent( nID, *pMacro, mpObjShell );

                if ( pMacro )
                    delete pMacro;
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

    long        nCount  = maEventNames.getLength();
    OUSTRING*   pNames  = maEventNames.getArray();

    for ( long i=0; i<nCount; i++ )
    {
        if ( pNames[i] == aName )
        {
            ANY* pElement = maEventData.getArray();
            return pElement[i];
        }
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

    long        nCount  = maEventNames.getLength();
    OUSTRING*   pNames  = maEventNames.getArray();

    for ( long i=0; i<nCount; i++ )
    {
        if ( pNames[i] == aName )
        {
            return sal_True;
        }
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
    OUSTRING*   pNames  = maEventNames.getArray();
    long        nCount  = maEventNames.getLength();
    long        nIndex  = 0;
    sal_Bool    bFound  = sal_False;

    while ( !bFound && ( nIndex < nCount ) )
    {
        if ( pNames[nIndex] == aName )
            bFound = sal_True;
        else
            nIndex += 1;
    }

    if ( !bFound )
        return;

    SEQUENCE < PROPERTYVALUE > aProperties;
    ANY aEventData = maEventData.getArray()[ nIndex ];

    if ( aEventData >>= aProperties )
    {
        PROPERTYVALUE  *pValues     = aProperties.getArray();
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
            if ( pValues[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
            {
                pValues[ nIndex ].Value >>= aType;
            }
            else if ( pValues[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
            {
                pValues[ nIndex ].Value >>= aScript;
            }
            else if ( pValues[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
            {
                pValues[ nIndex ].Value >>= aLibrary;
            }
            else if ( pValues[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
            {
                pValues[ nIndex ].Value >>= aMacroName;
            }
            nIndex += 1;
        }

        if ( aType.compareToAscii( STAR_BASIC ) == 0 )
        {
            if ( ! aScript.getLength() && aMacroName.getLength() )
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

            if ( aScript.getLength() )
            {
                aGuard.clear();
                ErrCode nErr;
                if ( Warn_Impl() )
                    nErr = SfxMacroLoader::loadMacro( aScript );
                else
                    nErr = 0;
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
    maEventNames = pShell->GetEventNames();
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
SvxMacro* SfxEvents_Impl::ConvertToMacro( ANY aElement )
{
    SvxMacro* pMacro = NULL;

    SEQUENCE < PROPERTYVALUE > aProperties;

    if ( aElement >>= aProperties )
    {
        PROPERTYVALUE  *pValues     = aProperties.getArray();
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
            if ( pValues[ nIndex ].Name.compareToAscii( PROP_EVENT_TYPE ) == 0 )
            {
                pValues[ nIndex ].Value >>= aType;
            }
            else if ( pValues[ nIndex ].Name.compareToAscii( PROP_SCRIPT ) == 0 )
            {
                pValues[ nIndex ].Value >>= aScriptURL;
            }
            else if ( pValues[ nIndex ].Name.compareToAscii( PROP_LIBRARY ) == 0 )
            {
                pValues[ nIndex ].Value >>= aLibrary;
            }
            else if ( pValues[ nIndex ].Name.compareToAscii( PROP_MACRO_NAME ) == 0 )
            {
                pValues[ nIndex ].Value >>= aMacroName;
            }
            nIndex += 1;
        }

        // Get the type
        ScriptType  eType( STARBASIC );

        if ( aType.compareToAscii( STAR_BASIC ) == COMPARE_EQUAL )
            eType = STARBASIC;
        else if ( aType.compareToAscii( JAVA_SCRIPT ) == COMPARE_EQUAL )
            eType = JAVASCRIPT;
        else
            DBG_ERRORFILE( "ConvertToMacro: Unknown macro type" );

        if ( ( eType == STARBASIC ) && ! aMacroName.getLength() && aScriptURL.getLength() )
        {
            sal_Int32 nHashPos = aScriptURL.indexOf( '/', 8 );
            sal_Int32 nArgsPos = aScriptURL.indexOf( '(' );

            if ( ( nHashPos != STRING_NOTFOUND ) && ( nHashPos < nArgsPos ) )
            {
                OUSTRING aBasMgrName( INetURLObject::decode( aScriptURL.copy( 8, nHashPos-8 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET ) );

                if ( aBasMgrName.compareToAscii(".") == 0 )
                    aLibrary = mpObjShell->GetTitle( SFX_TITLE_APINAME );
                else if ( aBasMgrName.getLength() )
                    aLibrary = aBasMgrName;
                else
                    aLibrary = SFX_APP()->GetName();

                // Get the macro name
                aMacroName = aScriptURL.copy( nHashPos+1, nArgsPos - nHashPos - 1 );
            }
            else
            {
                DBG_ERRORFILE( "ConvertToMacro: Unknown macro url format" );
            }
        }

        if ( aMacroName.getLength() )
            pMacro = new SvxMacro( aMacroName, aLibrary, eType );
    }

    return pMacro;
}

//--------------------------------------------------------------------------------------------------------
sal_Bool SfxEvents_Impl::Warn_Impl()
{
    // Wenn das Macro sowieso nicht ausgef"uhrt wird, mu\s auch nicht gefragt werden
    if ( !mpObjShell->IsSecure() )
        return sal_False;

    // Bei dokumentgebundenen Macros WarningStatus checken
    // Wenn "Immer warnen" angeschaltet ist, Warnung ausgeben
    sal_Bool bWarn = SFX_APP()->GetEventConfig()->IsWarningForced();

    if ( bWarn )
    {
        SfxMacroQueryDlg_Impl aBox ( SfxResId( DLG_MACROQUERY ) );
        if ( aBox.Execute() )
            bWarn = sal_False;
    }

    return !bWarn;
}

