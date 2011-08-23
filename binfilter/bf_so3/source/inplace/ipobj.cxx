/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <stdio.h>

#include <vcl/bitmap.hxx>
#include <bf_so3/ipobj.hxx>
#include <tools/debug.hxx>
#include "bf_so3/ipclient.hxx"
#include <svuidlg.hrc>
#include "bf_so3/ipwin.hxx"
#include <ipmenu.hxx>
#include <bf_so3/ipenv.hxx>
#include "bf_so3/soerr.hxx"
#include "bf_so3/outplace.hxx"

namespace binfilter {

/************** class SvInPlaceObject ***********************************/
SV_IMPL_FACTORY(SvInPlaceObjectFactory)
    {
    }
};
TYPEINIT1(SvInPlaceObjectFactory,SvFactory);

SO2_IMPL_STANDARD_CLASS1_DLL(SvInPlaceObject,SvInPlaceObjectFactory,SvEmbeddedObject,
                             0x5D4C00E0L, 0x7959, 0x101B,
                             0x80,0x4C,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD )

::IUnknown * SvInPlaceObject::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

void SvInPlaceObject::TestMemberObjRef( BOOL bFree )
{
    (void)bFree;

#ifdef DBG_UTIL
    if( GetIPClient() )
    {
        ByteString aTest = "\t\tGetIPClient() == ";
        aTest.Append( ByteString::CreateFromInt32( (ULONG)(SvObject *)GetIPClient() ) );
        DBG_TRACE( aTest.GetBuffer() );
    }
#endif
}

#ifdef TEST_INVARIANT
void SvInPlaceObject::TestMemberInvariant( BOOL bPrint )
{
#ifdef DBG_UTIL
    if( bPrint )
    {
        if( pIPEnv )
        {
            ByteString aTest( "\t\tSvInPlaceEnvironment == " );
            aTest.Append( ByteString::CreateFromInt32( (ULONG)pIPEnv ) );
            DBG_TRACE( aTest.GetBuffer() );
        }
    }
#else
    (void)bPrint;
#endif
}
#endif

/************************************************************************
|*    SvInPlaceObject::SvInPlaceObject()
|*
|*    Beschreibung
*************************************************************************/
#define INIT_CTOR()         \
    : pObjI( NULL )         \
    , pActiveObj( NULL )    \
    , pIPEnv( NULL )        \
    , bIsUndoable( FALSE )	\
    , bDeleteIPEnv( FALSE )

SvInPlaceObject::SvInPlaceObject()
    INIT_CTOR()
{
}

SvInPlaceObject::~SvInPlaceObject()
{
    DBG_ASSERT( LIST_ENTRY_NOTFOUND == GetIPActiveObjectList().GetPos( this ),
                "ip-object in ip-object-list" );
}


/************************************************************************
|*    SvInPlaceObject::SetIPEnv()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceObject::SetIPEnv( SvInPlaceEnvironment * pFrm )
{
    DBG_ASSERTWARNING( pIPEnv || pFrm, "already NULL" );
    DBG_ASSERT( !pIPEnv || !pFrm, "IPEnv exist" );
    pIPEnv = pFrm;
}

/************************************************************************
|*    SvInPlaceObject::GetIPActiveObjectList()
|*
|*    Beschreibung
*************************************************************************/
SvInPlaceObjectList & SvInPlaceObject::GetIPActiveObjectList()
{
    SoDll * pSoApp = SOAPP;
    if( !pSoApp->pIPActiveObjectList )
        pSoApp->pIPActiveObjectList = new SvInPlaceObjectList();
    return *pSoApp->pIPActiveObjectList;
}

/*************************************************************************
|*    SvInPlaceObject::Verb()
|*
|*    Beschreibung
*************************************************************************/
ErrCode SvInPlaceObject::Verb
(
    long nVerb,
    SvEmbeddedClient * pCl,
    Window * pWin,
    const Rectangle * pWorkRectPixel
)
{
    SvInPlaceClient * pICl = GetIPClient();
    if( !pICl )
        return SvEmbeddedObject::Verb( nVerb, pCl, pWin, pWorkRectPixel );

    ErrCode nRet = ERRCODE_NONE;
    /*
    BOOL bGroesseNachTreten = TRUE;
    switch ( nVerb )
    {
        case SVVERB_OPEN:
        case SVVERB_HIDE:
            bGroesseNachTreten = FALSE;
            break;
    }
    */

    if( Owner() )
    {
        // sonst funktioniert WordPad als Container nicht
        //bGroesseNachTreten = FALSE;
        switch ( nVerb )
        {
            case SVVERB_SHOW:
                nRet = GetProtocol().UIProtocol();
                break;
            case SVVERB_OPEN:
                nRet = GetProtocol().EmbedProtocol();
                break;
            case SVVERB_HIDE:
                nRet = DoInPlaceActivate( FALSE );
                break;
            case SVVERB_UIACTIVATE:
                nRet = GetProtocol().UIProtocol();
                break;
            case SVVERB_IPACTIVATE:
                nRet = GetProtocol().IPProtocol();
                break;
            default:
                if( nVerb >= 0 )
                    nRet = GetProtocol().UIProtocol();
        }
    }
    else
    {
        nRet = SvEmbeddedObject::Verb( nVerb, pCl, pWin, pWorkRectPixel );
    }
    /*
    if( bRet && bGroesseNachTreten && pWorkRectPixel
      && GetProtocol().IsInPlaceActive() )
        pICl->GetEnv()->RequestObjAreaPixel( *pWorkRectPixel );
    */
    return nRet;
}

/************************************************************************
|*    SvInPlaceObject::Open()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceObject::Open( BOOL bOpen )
{
    SvEmbeddedObject::Open( bOpen );
}

/************************************************************************
|*    SvInPlaceObject::DoInPlaceActivate()
|*
|*    Beschreibung
*************************************************************************/
ErrCode SvInPlaceObject::DoInPlaceActivate( BOOL bActivate )
{
    if( aProt.IsInPlaceActive() == bActivate )
        return ERRCODE_NONE;

    SvInPlaceObjectRef aAlive( this ); // wegen Reset2Connect
    if( !bActivate )
        aProt.Reset2InPlaceActive();
    if( Owner() )
        aProt.InPlaceActivate( bActivate );
    return (aProt.IsInPlaceActive() == bActivate) ?
            ERRCODE_NONE : ERRCODE_SO_NOT_INPLACEACTIVE;
}

//=========================================================================
void SvInPlaceObject::InPlaceActivate
(
    BOOL bActivate	/* Zeigt an, ob aktiviert oder deaktiviert wird. */
)
/*	[Beschreibung]

    Diese Methode zeigt das IP-Window an. Wenn bActive == TRUE ist,
    wird die Methode <SvInPLaceObject::DoMergePalette()> gerufen, die
    daf"ur sorgt, dass die Palette zwischen Container und Objekt
    abgestimmt werden kann.
    Ist bActive == FALSE, dann wird das Container-Menu freigegeben
    und das in <SvInPlaceObject::SetIPEnv> gemerkte
    <SvInPlaceEnvironment> vergessen.

    [Anmerkung]

    Sollte die IP-Aktivierung scheitern, muss das <SvInPlaceEnvironment>
    mit "SetIPEnv( NULL )" zur"uckgesetzt werden.

    [Beispiel]

    ...::InPlaceActivate( BOOL bActivate )
    {
        if( bActivate )
        {
            SvContainerEnvironment * pEnv;
            pEnv = GetIPClient()->GetEnv();
            pMyEnv = new MyInPlaceEnvironment( pEnv, this );

            // wird in die Verwaltung eingesetzt
            SetIPEnv( pMyEnv );

            if( "Kann nicht aktiviert werden" )
            {
                SetIPEnv( NULL );
                DELETEZ( pMyEnv );
                DoClose();
                return;
            }
        }
        SvInPlaceObject::InPlaceActivate( bActivate );
        if( bActivate )
        {
            DELETEZ( pMyEnv );
        }
    }


    [Querverweise]

    <SvEditProtocol>
*/
{
    if( Owner() )
    {
        if( bActivate )
        {
            DBG_ASSERT( pIPEnv, "set SvInPlaceEnvironment in InPlaceActivate bevor call to superclass" );

            if( GetClient()->Owner() )
                // sonst nur bei Ole-SetInPlaceActiveObj
                DoMergePalette();
        }
        else
        {
            SendViewChanged();
        }
        pIPEnv->DoShowIPObj( bActivate );
    }
    else
    {
        if( bActivate )
        {
            // Weil SetActiveObject zu spaet kommt
            if( !pIPEnv )
            {
                pIPEnv = new SvInPlaceEnvironment( GetIPClient()->GetEnv(), this );
                bDeleteIPEnv = TRUE;
            }
        }
    }

    if( Owner() && !bActivate )
    { // Client-Items aus OleMenu entfernen
        if( pIPEnv )
            pIPEnv->ReleaseClientMenu();
    }
    if( !bActivate && pIPEnv )
    {
        if( bDeleteIPEnv )
        {
            delete pIPEnv;
            bDeleteIPEnv = FALSE;
        }
        pIPEnv = NULL;
    }
}

/************************************************************************
|*    SvInPlaceObject::DoUIActivate()
|*
|*    Beschreibung
*************************************************************************/
ErrCode SvInPlaceObject::DoUIActivate( BOOL bActivate )
{
    SvInPlaceObjectRef aAlive( this ); // wegen Reset2Connect
    if( aProt.IsUIActive() == bActivate )
        return ERRCODE_NONE;
    if( !bActivate )
        aProt.Reset2UIActive();
    if( Owner() )
    {
        aProt.UIActivate( bActivate );
    }
    return (aProt.IsUIActive() == bActivate) ?
            ERRCODE_NONE : ERRCODE_SO_NOT_INPLACEACTIVE;
}

/************************************************************************
|*    SvInPlaceObject::UIActivate()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceObject::UIActivate( BOOL bActivate )
{
    if( Owner() )
    {
        if( bActivate )
            pIPEnv->MergeMenus();
    }
    // bei !bActivate siehe IPClient::UIActivate
    if( bActivate )
        pIPEnv->ShowIPObj( bActivate );
    pIPEnv->DoShowUITools( bActivate );
}

/************************************************************************
|*    SvInPlaceObject::TopWinActivate()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceObject::TopWinActivate( BOOL )
{
}


/************************************************************************
|*    SvInPlaceObject::DocWinActivate()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceObject::DocWinActivate( BOOL bActivate )
{
    if( pIPEnv )
    {
        if( GetProtocol().IsUIActive() )
            pIPEnv->DoShowUITools( bActivate );
    }
}

/************************************************************************
|*    SvInPlaceObject::DoMergePalette()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvInPlaceObject::DoMergePalette()
{
    /*if( GetClient()->Owner() )
    {
        WorkWindow * pWW = pIPEnv->GetContainerEnv()->GetDocWin();
        if( pWW )
        {
            Palette aPal = pWW->GetPalette();
            return MergePalette( aPal );
        }
        else
        {
            pWW = pIPEnv->GetContainerEnv()->GetTopWin();
            Palette aPal = pWW->GetPalette();
            return MergePalette( aPal );
        }
    }*/
    return FALSE;
}

/************************************************************************
|*    SvInPlaceObject::MergePalette()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvInPlaceObject::MergePalette( const Palette & )
{
    return FALSE;
}

/*************************************************************************
|*    SvInPlaceObject::SetVisArea()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceObject::SetVisArea( const Rectangle & rVisArea )
{
    SvEmbeddedObject::SetVisArea( rVisArea );
}

/*************************************************************************
|*    SvInPlaceObject::ReactivateAndUndo()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvInPlaceObject::ReactivateAndUndo()
{
    return FALSE;
}

//=========================================================================
//==============class SvDeathObject========================================
//=========================================================================
SvDeathObject::SvDeathObject
(
    const Rectangle & rVisArea	/* Die VisArea wird sofort eingesetzt */
)
/*	[Beschreibung]

    Das Objekt wird mit der VisArea erzeugt. Es ruft im Konstruktor
    <SvPersist::DoInitNew>. Es darf also nicht von aussen gerufen werden.
*/
{
    AddNextRef();
    DoInitNew( NULL );
    SetVisArea( rVisArea );
    RestoreNoDelete();
    ReleaseRef();
}

//=========================================================================
ErrCode SvDeathObject::Verb
(
    long,
    SvEmbeddedClient *,
    Window *,
    const Rectangle *
)
/*	[Beschreibung]

    Lehnt alle Verben ab und gibt immer FALSE zur"uck.
*/
{
    return ERRCODE_SO_GENERALERROR;
}

//=========================================================================
#define SoResId( id )	ResId( id, *SOAPP->GetResMgr() )
void SvDeathObject::Draw
(
    OutputDevice * pOut,
    const JobSetup &,
    USHORT
)
/*	[Beschreibung]

    Gibt ein Symbol aus, dass das Objekt nicht geladen werden konnte.
*/
{
    Rectangle aR = GetVisArea();
    Bitmap aBmp( SoResId( BMP_OLEOBJ ) );
    pOut->DrawBitmap( aR.TopLeft(), aR.GetSize(), aBmp );
}

}
