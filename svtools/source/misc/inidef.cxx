/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inidef.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:11:47 $
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
#include "precompiled_svtools.hxx"



#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#include "inetdef.hxx"
#include "inidef.hxx"

//========================================================================

class SfxStdIniDef_Impl: public SfxIniDefaulter
{
public:
                    SfxStdIniDef_Impl( SfxIniDefaultManager *pDefMgr )
                    :   SfxIniDefaulter( pDefMgr )
                    {}
    virtual BOOL    QueryDefault( String &rValue, const SfxIniEntry &rEntry );
};

//-------------------------------------------------------------------------

BOOL SfxStdIniDef_Impl::QueryDefault( String &rValue, const SfxIniEntry &rEntry )
{
    switch ( rEntry.GetKey() )
    {
        case SFX_KEY_BROWSERRESTORE:
        {
            rValue = "1";
            return TRUE;
        }

        case SFX_KEY_INET_HOME:
        {
            if ( System::GetLanguage() == LANGUAGE_GERMAN )
                rValue = "http://www.stardivision.de";
            else
                rValue = "http://www.stardivision.com";
            return TRUE;
        }

        case SFX_KEY_INET_MEMCACHE:
            rValue = "4";
            return TRUE;

        case SFX_KEY_INET_DISKCACHE:
            rValue = "2048";
            return TRUE;

        case SFX_KEY_INET_CACHEEXPIRATION:
            rValue = "3";
            return TRUE;

        case SFX_KEY_INET_MAXHTTPCONS:
            rValue = "4";
            return TRUE;

        case SFX_KEY_INET_MAXFTPCONS:
            rValue = "2";
            return TRUE;

//          case SFX_KEY_INET_JAVAMINHEAP:
//              rValue = "256";
//              return TRUE;

//          case SFX_KEY_INET_JAVAMAXHEAP:
//              rValue = "";
//              return TRUE;

        case SFX_KEY_INET_USERAGENT:
            rValue = INET_DEF_CALLERNAME;
            return TRUE;

        case SFX_KEY_INET_EXE_JAVASCRIPT:
#ifdef SOLAR_JAVA
            rValue = "0"; // noch "0", solange es noch soviel Bugs gibt
#else
            rValue = "0"; // immer "0"
#endif
            return TRUE;

        case SFX_KEY_INET_EXE_PLUGIN:
            rValue = "1";
            return TRUE;

/*      case SFX_KEY_INET_JAVA_ENABLE:
#ifdef SOLAR_JAVA
            rValue = "1";
#else
            rValue = "0";
#endif
            return TRUE; */

//          case SFX_KEY_INET_NETACCESS:
//              rValue = "2";
//              return TRUE;

        case SFX_KEY_INET_CHANNELS:
            rValue = "1";
            return TRUE;

        case SFX_KEY_BASIC_ENABLE:
            rValue = "1";
            return TRUE;

        case SFX_KEY_INET_COOKIES:
            rValue = "1";
            return TRUE;

        case SFX_KEY_ICONGRID:
            rValue = "100;70;0";
            return TRUE;

        case SFX_KEY_METAFILEPRINT:
            rValue = "1";
            return TRUE;
    }

    return SfxIniDefaulter::QueryDefault( rValue, rEntry );
}

//=========================================================================

SfxIniDefaultManager::SfxIniDefaultManager()
:   _pList( new SfxIniDefaulterList )
{
    new SfxStdIniDef_Impl( this );
}

//-------------------------------------------------------------------------

SfxIniDefaultManager::~SfxIniDefaultManager()
{
    if ( _pList )
    {
        for ( USHORT n = _pList->Count(); n--; )
            delete _pList->GetObject(n);
        delete _pList;
    }
}

//-------------------------------------------------------------------------

BOOL SfxIniDefaultManager::QueryDefault
(
    String&             rValue,     /* out: Default-Wert f"ur 'rEntry'
                                       (Default ist Leerstring) */
    const SfxIniEntry&  rEntry      // in:  Beschreibung des Eintrags
)

/*  [Beschreibung]

    "Uber diese interne Methode besorgt sich der <SfxIniManager> den
    Default f"ur einen in 'rEntry' beschriebenen Eintrag.
*/

{
    for ( USHORT n = _pList->Count(); n--; )
        if ( _pList->GetObject(n)->QueryDefault( rValue, rEntry ) )
            return TRUE;
    return FALSE;
}

//=========================================================================

SfxIniDefaulter::SfxIniDefaulter( SfxIniDefaultManager *pManager )

/*  [Beschreibung]

    Der Ctor dieser Klasse meldet die neue Instanz automatisch am
    <SfxiniDefaultManager> 'pManager' an.
*/

:   _pManager( pManager )

{
    pManager->Insert( this );
}

//-------------------------------------------------------------------------

SfxIniDefaulter::~SfxIniDefaulter()

/*  [Beschreibung]

    Der Dtor dieser Klasse meldet die neue Instanz automatisch am
    <SfxiniDefaultManager> ab, der im Ctor angegeben wurde.
*/

{
    _pManager->Remove( this );
}

//-------------------------------------------------------------------------

BOOL SfxIniDefaulter::QueryDefault
(
    String&             rValue,     /* out: Default-Wert f"ur 'rEntry'
                                       (Default ist Leerstring) */
    const SfxIniEntry&  rEntry      // in:  Beschreibung des Eintrags
)

/*  [Beschreibung]

    Diese virtuelle Methode mu\s "uberladen werden. Sie soll dann in
    'rValue' einen Default-Wert f"ur den in 'rEntry' beschriebenen
    ini-Eintrag setzen, falls ihr dieser bekannt ist.


    [Returnwert]

    TRUE            In 'rValue' befindet sich der Default-Wert.

    FALSE           F"ur diesen Eintrag ist kein Default-Wert bekannt.

*/

{
    return FALSE;
};

//========================================================================

SfxIniEntry::SfxIniEntry
(
    const String&   aGroup,
    const String&   aKey,
    SfxIniGroup     eGroup,
    SfxIniKey       eKey,
    USHORT          nIndex
)
:   _aGroup( aGroup ),
    _aKey( aKey ),
    _eGroup( eGroup ),
    _eKey( eKey ),
    _nIndex( nIndex )
{
}


