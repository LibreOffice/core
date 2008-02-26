/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appmain.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:01:27 $
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
#include "precompiled_sfx2.hxx"

//#define TF_NEWDESKTOP

#define _SDINTERN_HXX

#include <stdio.h>

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _CSTITEM_HXX //autogen
#include <svtools/cstitem.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <svtools/startoptions.hxx>
#endif
#include <svtools/itempool.hxx>
#include <svtools/urihelper.hxx>
#include <svtools/helpopt.hxx>
#include <vos/process.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "sfxtypes.hxx"
#include "appdata.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/app.hxx>
#include "arrdecl.hxx"
#include <sfx2/dispatch.hxx>
#include "sfxresid.hxx"
#include <sfx2/fcontnr.hxx>
#include <sfx2/viewsh.hxx>
#include "intro.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/appuno.hxx>
#include "app.hrc"
#include <sfx2/docfile.hxx>
#include "workwin.hxx"

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif


//===================================================================

DBG_NAME(SfxAppMainNewMenu)
DBG_NAME(SfxAppMainBmkMenu)
DBG_NAME(SfxAppMainWizMenu)
DBG_NAME(SfxAppMainOLEReg)
DBG_NAME(SfxAppMainCHAOSReg)

//===================================================================

#define SFX_TEMPNAMEBASE_DIR    "soffice.tmp"
#define SFX_KEY_TEMPNAMEBASE    "Temp-Dir"

//===================================================================

#ifdef TF_POOLABLE
static SfxItemInfo __READONLY_DATA aItemInfos[] =
{
    { 0, 0 }
};
#endif

//===================================================================

typedef Link* LinkPtr;
SV_DECL_PTRARR(SfxInitLinkList, LinkPtr, 4, 4)

TYPEINIT2(SfxApplication,SfxShell,SfxBroadcaster);

//--------------------------------------------------------------------
void SfxApplication::Init
(
)

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx aus Application:a:Main() gerufen,
    bevor Execute() ausgef"uhrt wird und
    - das Intro bereits angezeigt ist,
    - das Applikationsfenster exisitiert, aber noch hidden ist,
    - die Bindings bereits existieren (Controller sind anmeldbar),
    - der Ini- und Config-Manager bereits existiert,
    - die Standard-Controller bereits exisitieren,
    - die SFx-Shells ihre Interfaces bereits registriert haben.

    [Querverweise]
    <SfxApplication::Exit()>
    <SfxApplication::OpenClients()>
*/
{
#ifdef DDE_AVAILABLE
#ifdef PRODUCT
    InitializeDde();
#else
    if( !InitializeDde() )
    {
        ByteString aStr( "Kein DDE-Service moeglich. Fehler: " );
        if( GetDdeService() )
            aStr += GetDdeService()->GetError();
        else
            aStr += '?';
        DBG_ASSERT( sal_False, aStr.GetBuffer() )
    }
#endif
#endif
}

//--------------------------------------------------------------------

void SfxApplication::Exit()

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx aus Application::Main() gerufen,
    nachdem Execute() beendet ist und
    - die Konfiguration (SfxConfigManager) bereits gespeichert wurde,
    - die Fensterpostionen etc. in den SfxIniManager geschrieben wurden,
    - das Applikationsfenster noch existiert, aber hidden ist
    - s"amtliche Dokumente und deren Views bereits geschlossen sind.
    - Dispatcher, Bindings etc. bereits zerst"ort sind

    [Querverweise]
    <SfxApplication::Init(int,char*[])>
*/

{
}

//---------------------------------------------------------------------------

void SfxApplication::PreInit( )
{
}

//---------------------------------------------------------------------------
bool SfxApplication::InitLabelResMgr( const char* _pLabelPrefix, bool _bException )
{
    bool bRet = false;
    // Label-DLL mit diversen Resourcen fuer OEM-Ver. etc. (Intro, Titel, About)
    DBG_ASSERT( _pLabelPrefix, "Wrong initialisation!" );
    if ( _pLabelPrefix )
    {
        // versuchen, die Label-DLL zu erzeugen
        pAppData_Impl->pLabelResMgr = CreateResManager( _pLabelPrefix );

        // keine separate Label-DLL vorhanden?
        if ( !pAppData_Impl->pLabelResMgr )
        {
            if ( _bException )
            {
                // maybe corrupted installation
                throw (::com::sun::star::uno::RuntimeException(
                    ::rtl::OUString::createFromAscii("iso resource could not be loaded by SfxApplication"),
                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >()));
            }
        }
        else
            bRet = true;
    }

    return bRet;
}

void SfxApplication::Main( )
{
}

//-------------------------------------------------------------------------

SfxFilterMatcher& SfxApplication::GetFilterMatcher()
{
    if( !pAppData_Impl->pMatcher )
    {
        pAppData_Impl->pMatcher = new SfxFilterMatcher();
        URIHelper::SetMaybeFileHdl( STATIC_LINK(
            pAppData_Impl->pMatcher, SfxFilterMatcher, MaybeFileHdl_Impl ) );
    }
    return *pAppData_Impl->pMatcher;
}
