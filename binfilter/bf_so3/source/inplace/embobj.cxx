/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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



#include <bf_so3/embobj.hxx>

#include <vcl/cvtgrf.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <sot/storinfo.hxx>
#include <sot/absdev.hxx>
#include <tools/debug.hxx>
#include <sot/exchange.hxx>

#include <soimpl.hxx>
#include "bf_so3/outplace.hxx"
#include <bf_so3/client.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_so3/so2dll.hxx>
#include "bf_so3/soerr.hxx"
#include <svuidlg.hrc>

#include <bf_svtools/filter.hxx>
#include <viscache.hxx>
#include <sot/formats.hxx>

namespace binfilter {

/************** class SvEmbeddedInfoObject ***************************************/
/*************************************************************************/
SV_IMPL_PERSIST1(SvEmbeddedInfoObject,SvInfoObject)

/************************************************************************
|*    SvEmbeddedInfoObject::SvEmbeddedInfoObject()
|*    SvEmbeddedInfoObject::~SvEmbeddedInfoObject()
|*
|*    Beschreibung
*************************************************************************/
SvEmbeddedInfoObject::SvEmbeddedInfoObject() :
    nViewAspect( ASPECT_CONTENT )
{
}

SvEmbeddedInfoObject::SvEmbeddedInfoObject( SvEmbeddedObject * pObj,
                                            const String & rName )
    : SvInfoObject( pObj, rName ),
    nViewAspect( ASPECT_CONTENT )

{
}

SvEmbeddedInfoObject::SvEmbeddedInfoObject( const String& rObjName,
                                     const SvGlobalName& rClassName )
    : SvInfoObject( rObjName, rClassName ),
    nViewAspect( ASPECT_CONTENT )

{
}

SvEmbeddedInfoObject::~SvEmbeddedInfoObject()
{
}

/************************************************************************
|*    SvEmbeddedInfoObject::MakeCopy()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedInfoObject::Assign( const SvInfoObject * pObj )
{
    SvInfoObject::Assign( pObj );
    SvEmbeddedInfoObject * pI = PTR_CAST(SvEmbeddedInfoObject, pObj );
    if( pI )
    {
        aVisArea = pI->aVisArea;
    }
}

/************************************************************************
|*    SvEmbeddedInfoObject::Load()
|*    SvEmbeddedInfoObject::Save()
|*
|*    Beschreibung
*************************************************************************/
#define INFO_VERSION    (BYTE)2
void SvEmbeddedInfoObject::Load( SvPersistStream & rStm )
{
    SvInfoObject::Load( rStm );
    BYTE nVers = 0;
    rStm >> nVers;
    DBG_ASSERT( nVers == INFO_VERSION, "SvInfoObject version conflict" );
    if( nVers != INFO_VERSION )
        rStm.SetError( SVSTREAM_WRONGVERSION );
    else
    {
        BOOL bIsLink;
        rStm >> bIsLink;
        rStm >> aVisArea;
    }
}

void SvEmbeddedInfoObject::Save( SvPersistStream & rStm )
{
    SvInfoObject::Save( rStm );
    rStm << (BYTE)INFO_VERSION;
    BOOL bIsLink = FALSE;
    rStm << bIsLink;
    rStm << GetVisArea();
}

/************************************************************************
|*    SvEmbeddedInfoObject::SetObj()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedInfoObject::SetObj( SvPersist * pObjP )
{
    SvInfoObject::SetObj( pObjP );
    SvEmbeddedObject * pObj = GetEmbed();
    if( pObj && !pObj->Owner() )
        pObj->SvEmbeddedObject::SetVisArea( aVisArea );
}

/************************************************************************
|*    SvEmbeddedInfoObject::IsLink()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvEmbeddedInfoObject::IsLink() const
{
    return FALSE;
}

/************************************************************************
|*    SvEmbeddedInfoObject::GetVisArea()
|*
|*    Beschreibung
*************************************************************************/
const Rectangle & SvEmbeddedInfoObject::GetVisArea() const
{
    SvEmbeddedObject * pObj = GetEmbed();
    if( pObj )
        ((SvEmbeddedInfoObject *)this)->aVisArea = pObj->GetVisArea();
    return aVisArea;
}

UINT32 SvEmbeddedInfoObject::GetViewAspect() const
{
    SvEmbeddedObject * pObj = GetEmbed();
    if( pObj )
        ((SvEmbeddedInfoObject *)this)->nViewAspect = pObj->GetViewAspect();
    return nViewAspect;
}

/********************** SvFilterList **************************************
**************************************************************************/

/************** class SvObjectDescriptor ********************************
**************************************************************************/




/*************************************************************************
|*
|*    SvEmbeddedObject::Factory()
|*
*************************************************************************/
SO2_IMPL_CLASS2_DLL(SvEmbeddedObject,SvFactory,SvPersist,SvPseudoObject,
                    SvGlobalName( 0xBB0D2800L, 0x73EE, 0x101B,
                                  0x80,0x4C,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD) )

::IUnknown * SvEmbeddedObject::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

void SvEmbeddedObject::TestMemberObjRef( BOOL /*bFree*/ )
{
#ifdef DBG_UTIL
    if( GetClient() )
    {
        ByteString aTest = "\t\tGetClient() == ";
        aTest.Append( ByteString::CreateFromInt32( (ULONG)(SvObject *)GetClient() ) );
        DBG_TRACE( aTest.GetBuffer() );
    }
#endif
}

#ifdef TEST_INVARIANT
void SvEmbeddedObject::TestMemberInvariant( BOOL bPrint )
{
    (void)bPrint;
}
#endif

/************************************************************************
|*    SvEmbeddedObject::SvEmbeddedObject()
|*
|*    Beschreibung
*************************************************************************/
SvEmbeddedObject::SvEmbeddedObject()
    : bAutoSave       ( TRUE )
    , bAutoHatch      ( TRUE )
    , nMapUnit        ( MAP_100TH_MM )
{
}


SvEmbeddedObject::~SvEmbeddedObject()
{
}

/// Direct sent, when ViewChange is called. So nothing to do
void SvEmbeddedObject::SendViewChanged()
{
}

void SvEmbeddedObject::ViewChanged( USHORT nAspect)
{
    SvEmbeddedClient * pCl = GetClient();
    if( pCl )
        pCl->ViewChanged( nAspect );
}


/*************************************************************************
|*    SvEmbeddedObject::GetGDIMetaFile()
|*
|*    Beschreibung
*************************************************************************/

GDIMetaFile& SvEmbeddedObject::GetGDIMetaFile( GDIMetaFile& rMTF )
{
    rMTF.Clear();
    return rMTF;
}

BOOL SvEmbeddedObject::SetData( const String& )
{
    return FALSE;
}

/*************************************************************************
|*    SvEmbeddedObject::Load()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvEmbeddedObject::Load( SvStorage * pStor )
{
    SvGlobalName aClassName = pStor->GetClassName();

    SvGlobalName aActualClassName =
        SvFactory::GetAutoConvertTo( aClassName );

    if( aActualClassName == *GetSvFactory() )
        return SvPersist::Load( pStor );
    else
    { // Ich bin es nicht selbst
        return SvPersist::Load( pStor );
    }
}

/*************************************************************************
|*    SvEmbeddedObject::Save()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvEmbeddedObject::Save()
{
    return SvPersist::Save();
}

BOOL SvEmbeddedObject::SaveAs( SvStorage * pNewStor )
{
    BOOL bRet = FALSE;
    if( SvPersist::SaveAs( pNewStor ) )
    {
        bRet = TRUE;
        if( bRet && Owner() && GetParent()
          && SOFFICE_FILEFORMAT_31 == pNewStor->GetVersion() )
        {
            ULONG n = pNewStor->GetFormat();
            if( n == SOT_FORMATSTR_ID_STARWRITER_30 || n == SOT_FORMATSTR_ID_STARDRAW
              || n == SOT_FORMATSTR_ID_STARCALC )
            {
                // empty MTF for 3.1 formats
                GDIMetaFile aMtf;
                MakeContentStream( pNewStor, aMtf );
            }
        }
    }
    return bRet;
}

/*************************************************************************
|*    SvEmbeddedObject::LoadContent()
|*
|*    Beschreibung
*************************************************************************/
#define EMBEDDED_OBJECT_VERSION (BYTE)0
void SvEmbeddedObject::LoadContent( SvStream & rStm, BOOL bOwner_ )
{
    SvPersist::LoadContent( rStm, bOwner_ );
    if( rStm.GetError() != SVSTREAM_OK )
        return;

    if( bOwner_ )
    {
        BYTE nVers;
        rStm >> nVers;
        DBG_ASSERT( nVers == EMBEDDED_OBJECT_VERSION, "version conflict" );

        if( nVers != EMBEDDED_OBJECT_VERSION )
            rStm.SetError( SVSTREAM_WRONGVERSION );
        else
        {
            // Sichtbaren Bereich setzten
            rStm >> aVisArea;
            USHORT nUnit;
            rStm >> nUnit;
            SetMapUnit( (MapUnit)nUnit );
        }
    }
}

/*************************************************************************
|*    SvEmbeddedObject::SaveContent()
|*
|*    Beschreibung
*************************************************************************/

static void WriteExtContent( SvStream & rStm, const GDIMetaFile & rMtf,
                             USHORT nAspect, ULONG nAdviseModes )
{
    Impl_OlePres aEle( FORMAT_GDIMETAFILE );
    // Die Groesse in 1/100 mm umrechnen
    // Falls eine nicht anwendbare MapUnit (Device abhaengig) verwendet wird,
    // versucht SV einen BestMatchden richtigen Wert zu raten.
    Size aSize = rMtf.GetPrefSize();
    MapMode aMMSrc = rMtf.GetPrefMapMode();
    MapMode aMMDst( MAP_100TH_MM );
    aSize = OutputDevice::LogicToLogic( aSize, aMMSrc, aMMDst );
    aEle.SetSize( aSize );
     aEle.SetAspect( nAspect );
    aEle.SetAdviseFlags( nAdviseModes );
    aEle.SetMtf( rMtf );
    aEle.Write( rStm );
}


void SvEmbeddedObject::SaveContent( SvStream & rStm, BOOL bOwner_ )
{
    SvPersist::SaveContent( rStm, bOwner_ );
    if( bOwner_ )
    {
        rStm << (BYTE)EMBEDDED_OBJECT_VERSION;
        rStm << GetVisArea();
        rStm << (USHORT)GetMapUnit();
    }
}


BOOL SvEmbeddedObject::MakeContentStream( SotStorage * pStor,
                                          const GDIMetaFile & rMtf )
{
    // MAC MPW mag's sonst nicht ...
    String aPersistStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SVEXT_PERSIST_STREAM ) ) );
    SotStorageStreamRef xStm = pStor->OpenSotStream( aPersistStream );
    xStm->SetVersion( pStor->GetVersion() );

    xStm->SetBufferSize( 8192 );
    WriteExtContent( *xStm, rMtf, ASPECT_CONTENT, 2 );
    xStm->SetBufferSize( 0 );
    return xStm->GetError() == SVSTREAM_OK;
}

BOOL SvEmbeddedObject::MakeContentStream( SvStorage * pStor,
                                          const GDIMetaFile & rMtf )
{
    return MakeContentStream( (SotStorage *)pStor, rMtf );
}

//=========================================================================
void SvEmbeddedObject::FillClass
(
    SvGlobalName * pClassName,	/* Der Typ der Klasse */
    ULONG * pFormat,			/* Das Dateiformat in dem geschrieben wird */
    String * pAppName,			/* Der Applikationsname */
    String * pFullTypeName,     /* Der genaue Name des Typs	*/
    String * pShortTypeName,	/* Der kurze Name des Typs	*/
    long nFileFormat            /* Fuer dieses Office-Format sollen die
                                   Parameter gefuellt werden */
) const
/*  [Beschreibung]

    Die Methoden <SvPersist::FillClass> und <SvPseudoObject::FillClass>
    setzen nicht alle Parameter. <SvPersist::FillClass> setzt die
    Parameter pClassName und pFormat. <SvPseudoObject::FillClass> setzt
    die restlichen Parmeter.
*/
{
    SvGlobalName    aName;
    ULONG           nFormat;

    SvPersist::FillClass( pClassName, pFormat, pAppName, pFullTypeName,
                            pShortTypeName, nFileFormat );
    SvPseudoObject::FillClass( &aName, &nFormat, pAppName, pFullTypeName,
                                pShortTypeName, nFileFormat );
}

/*************************************************************************
|*    SvEmbeddedObject::SetModified()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::SetModified( BOOL bModifiedP )
{
    ASSERT_INIT()
    SvPersist::SetModified( bModifiedP );

    if( IsEnableSetModified() )
    {
        SvEmbeddedObjectRef xPar = this;

        while( xPar.Is() )
        {
            xPar->SetModifyTime( GetModifyTime() );

            SvPersist * pP = xPar->GetParent();
            xPar = pP;

            // Fuer diesen Fall muss der gesammte Ablauf noch
            // einmal geprueft werden
            DBG_ASSERT( xPar.Is() && pP || !pP && !xPar.Is(),
                        "Persist-Parent ist kein EmbeddedObject" );
        }
    }
}


/*************************************************************************
|*    SvEmbeddedObject::GetMiscStatus()
|*
|*    Beschreibung
*************************************************************************/
ULONG SvEmbeddedObject::GetMiscStatus() const
{
    return SvPseudoObject::GetMiscStatus();
}

/*************************************************************************
|*    SvEmbeddedObject::DoOpen()
|*
|*    Beschreibung
*************************************************************************/
ErrCode SvEmbeddedObject::DoOpen( BOOL bOpen )
{
    if( !aProt.IsConnect() )
        return ERRCODE_SO_GENERALERROR;
    if( aProt.IsOpen() == bOpen )
        return ERRCODE_NONE;

    SvEmbeddedObjectRef xHoldAliveDuringCall( this );

    if( !bOpen )
        aProt.Reset2Open();


    // Ein Open kommt nicht von Ole2
    // loest eventuell ein Close aus
    // Saved, wenn AutoSave
    aProt.Opened( bOpen );

    return (aProt.IsOpen() == bOpen) ? ERRCODE_NONE : ERRCODE_SO_GENERALERROR;
}

/*************************************************************************
|*    SvEmbeddedObject::DoEmbed()
|*
|*    Beschreibung
*************************************************************************/
ErrCode SvEmbeddedObject::DoEmbed( BOOL bEmbed )
{
    if( aProt.IsEmbed() == bEmbed )
        return ERRCODE_NONE;

    if( !bEmbed )
        aProt.Reset2Embed();

    ErrCode nRet = ERRCODE_NONE;
    if( Owner() )
    {
        if( bEmbed )
            // falls IP-Active
            aProt.Reset2Open();
        aProt.Embedded( bEmbed );
    }

    if( aProt.IsEmbed() != bEmbed && !ERRCODE_TOERROR( nRet ) )
        nRet = ERRCODE_SO_GENERALERROR;
    return nRet;
}

/*************************************************************************
|*    SvEmbeddedObject::Embed()
|*
|*    Beschreibung
*************************************************************************/
#ifdef DBG_UTIL_PROT
void SvEmbeddedObject::Embed( BOOL bEmbed )
#else
void SvEmbeddedObject::Embed( BOOL )
#endif
{
#ifdef DBG_UTIL_PROT
    String aTest( "Object---Embed---" );
    aTest += Owner() ? "Intern" : "Extern: ";
    aTest += bEmbed ? "TRUE" : "FALSE";
    DBG_TRACE( aTest )
#endif
}

//=========================================================================
ErrCode SvEmbeddedObject::DoPlugIn( BOOL bPlugIn )
/*	[Beschreibung]

    [Rueckgabewert]

    BOOL		TRUE, das Objekt ist im PlugIn-Zustand.
                FALSE, das Objekt ist nicht im PlugIn-Zustand.


    [Querverweise]
*/
{
    if( aProt.IsPlugIn() == bPlugIn )
        return ERRCODE_NONE;

    ErrCode nRet = ERRCODE_NONE;
    if( !bPlugIn )
        aProt.Reset2PlugIn();

    if( Owner() )
    {
        if( bPlugIn )
            // falls IP-Active
            aProt.Reset2Open();
        aProt.PlugIn( bPlugIn );
    }

    if( aProt.IsPlugIn() != bPlugIn && !ERRCODE_TOERROR( nRet ) )
        nRet = ERRCODE_SO_GENERALERROR;
    return nRet;
}

//=========================================================================
#ifdef DBG_UTIL_PROT
void SvEmbeddedObject::PlugIn( BOOL bPlugIn )
#else
void SvEmbeddedObject::PlugIn( BOOL )
#endif
{
#ifdef DBG_UTIL_PROT
    String aTest( "Object---PlugIn---" );
    aTest += Owner() ? "Intern" : "Extern: ";
    aTest += bPlugIn ? "TRUE" : "FALSE";
    DBG_TRACE( aTest )
#endif
}

/*************************************************************************
|*    SvEmbeddedObject::Verb()
|*
|*    Beschreibung
*************************************************************************/
ErrCode SvEmbeddedObject::Verb
(
    long nVerb,
    SvEmbeddedClient * pCl,
    Window * pWin,
    const Rectangle * pWorkRectPixel
)
{
    ErrCode nRet = ERRCODE_NONE;
    if( Owner() )
    {
        if( nVerb == SVVERB_OPEN )
            nRet = GetProtocol().EmbedProtocol();
        else
            nRet = GetProtocol().PlugInProtocol();
    }
    else
    {
        nRet = SvPseudoObject::Verb( nVerb, pCl, pWin, pWorkRectPixel );
    }
    return nRet;
}

/*************************************************************************
|*    SvEmbeddedObject::DocumentNameChanged()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::DocumentNameChanged( const String & /*rDocName*/ )
{
}

//=========================================================================
// locker die Struktur von Windows kopiert
struct MY_GUID
{
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    BYTE   Data4[8];
};

void SvEmbeddedObject::Connect
(
    BOOL bConnect	/* TRUE, connect. FALSE, disconnect */
)
/*	[Beschreibung]

    Ist bConnect == TRUE, dann wurde das Objekt mit einem
    <SvEmbeddedClient> verbunden. Diesen kann man mit
    <SvEmbeddedObject::GetClient> erfragen.
    Ist bConnect == TRUE, dann wurde das Objekt von dem Client getrennt,
    mit dem es vorher verbunden war.

    [Querverweise]

    <SvPseudoObject::DoClose>
*/
{
    if( Owner() )
    {
        SvEmbeddedObjectRef xCont = GetClient()->GetContainer();
        // Owner muss abgefragt werden, da Ole2 sonst streikt
        if( xCont.Is() && xCont->Owner() )
            xCont->Lock( bConnect );
    }
}

/*************************************************************************
|*    SvEmbeddedObject::Close()
|*
|*    Beschreibung: Close darf mehrmals gerufen werden
*************************************************************************/
BOOL SvEmbeddedObject::Close()
{
    const SvInfoObjectMemberList * pChildList_ = GetObjectList();
    if( pChildList_ )
    {
        ULONG nCount = pChildList_->Count();
        for( ULONG i = 0; i < nCount; i++ )
        {
            SvInfoObject * pIO = pChildList_->GetObject( i );
            SvPersist * pPer = pIO->GetPersist();
            SvEmbeddedObjectRef xEO( pPer );
            if( xEO.Is() )
                xEO->DoClose();
        }
    }

    // Unter Ole2 muss Close() vor SetClientSite( NULL ) gerufen werden.
    aProt.Reset2Connect();
    SvPseudoObject::Close();
    // Jetzt SetClientSite( NULL ).
    aProt.Reset();
    return TRUE;
}

/*************************************************************************
|*    SvEmbeddedObject::Open()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::Open( BOOL bOpen )
{
#ifdef DBG_UTIL_PROT
    String aTest( "Object---Open---" );
    aTest += Owner() ? "Intern" : "Extern: ";
    aTest += bOpen ? "TRUE" : "FALSE";
    DBG_TRACE( aTest )
#endif

    SendViewChanged();
    // kein Autosave im HandsOff State
    if( bAutoSave && !bOpen && !IsHandsOff())
    {
        SvEmbeddedClient * pCl = GetClient();
        if( pCl )
            pCl->SaveObject();
    }

}





/*************************************************************************
|*    SvEmbeddedObject::SetGetVisArea()
|*
|*    Beschreibung
*************************************************************************/
const Rectangle & SvEmbeddedObject::SetGetVisArea( const Rectangle & rArea )
{
    // nicht auf Gleichheit optimieren, da der Server sie jederzeit
    // modifizieren darf
    if( Owner() )
        // muss, weil SetVisArea ist virtuell ist
        SetVisArea( rArea );
    return aVisArea;
}

/*************************************************************************
|*    SvEmbeddedObject::SetVisArea()
|*    SvEmbeddedObject::GetVisArea()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::SetVisArea( const Rectangle & rArea )
{
    if( Owner() )
        aVisArea = rArea;
    else
        SetGetVisArea( rArea );
}

void SvEmbeddedObject::SetVisAreaSize( const Size & rSize )
{
    SetVisArea( Rectangle( GetVisArea().TopLeft(), rSize ) );
}

Rectangle SvEmbeddedObject::GetVisArea( USHORT nAspect ) const
{
    if( nAspect == ASPECT_CONTENT )
        return aVisArea;
    else if( nAspect == ASPECT_THUMBNAIL )
    {
        Rectangle aRect;
        aRect.SetSize( OutputDevice::LogicToLogic( Size( 5000, 5000 ),
                                         MAP_100TH_MM, GetMapUnit() ) );
        return aRect;
    }
    return Rectangle();
}

const Rectangle & SvEmbeddedObject::GetVisArea() const
{
    ((SvEmbeddedObject *)this)->aVisArea = GetVisArea( ASPECT_CONTENT );
    return aVisArea;
}

UINT32 SvEmbeddedObject::GetViewAspect() const
{
    return ASPECT_CONTENT;
}

/*************************************************************************
|*    SvEmbeddedObject::DrawHatch()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::DrawHatch( OutputDevice * pDev, const Point & rViewPos,
                               const Size &rSize )
{
    GDIMetaFile * pMtf = pDev->GetConnectMetaFile();
    if( pMtf && pMtf->IsRecord() )
        return;

    SvEmbeddedClient * pCl = GetClient();
    if( pCl && pCl->Owner() && bAutoHatch
      && pDev->GetOutDevType() == OUTDEV_WINDOW
      && GetProtocol().IsEmbed() )
    {
        pDev->Push();
        pDev->SetLineColor( Color( COL_BLACK ) );

        Size aPixSize = pDev->LogicToPixel( rSize );
        aPixSize.Width() -= 1;
        aPixSize.Height() -= 1;
        Point aPixViewPos = pDev->LogicToPixel( rViewPos );
        INT32 nMax = aPixSize.Width() + aPixSize.Height();
        for( INT32 i = 5; i < nMax; i += 5 )
        {
            Point a1( aPixViewPos ), a2( aPixViewPos );
            if( i > aPixSize.Width() )
                a1 += Point( aPixSize.Width(), i - aPixSize.Width() );
            else
                a1 += Point( i, 0 );
            if( i > aPixSize.Height() )
                a2 += Point( i - aPixSize.Height(), aPixSize.Height() );
            else
                a2 += Point( 0, i );

            pDev->DrawLine( pDev->PixelToLogic( a1 ), pDev->PixelToLogic( a2 ) );
        }
        pDev->Pop();
    }
}


/*************************************************************************
|*    SvEmbeddedObject::DoDraw()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::DoDraw( OutputDevice * pDev, const Point & rViewPos,
                               const Size &rSize,
                               const JobSetup & rSetup, USHORT nAspect )
{
    if( Owner() )
    {
        MapMode aMod = pDev->GetMapMode();
        Size aSize = GetVisArea( nAspect ).GetSize();
        MapMode aWilliMode( GetMapUnit() );
        aSize = pDev->LogicToLogic( aSize, &aWilliMode, &aMod );
        if( aSize.Width() && aSize.Height() )
        {
            Fraction aXF( rSize.Width(), aSize.Width() );
            Fraction aYF( rSize.Height(), aSize.Height() );

            Point aOrg = rViewPos;
            aMod.SetMapUnit( MAP_100TH_MM );
            aSize = pDev->LogicToLogic( GetVisArea( nAspect ).GetSize(), &aMod, &aWilliMode );
            DoDraw( pDev, aOrg, aXF, aYF, rSetup, aSize, nAspect );
        }
    }
}

/*************************************************************************
|*    SvEmbeddedObject::DoDraw()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::DoDraw( OutputDevice * pDev, const Point & rViewPos,
                               const Fraction & rScaleX,
                               const Fraction & rScaleY,
                               const JobSetup & rSetup, const Size& rSize,
                               USHORT nAspect )
{
    Rectangle aVisArea_  = GetVisArea( nAspect );
    // MapUnit des Ziels
    MapMode aMapMode( GetMapUnit() );
    aMapMode.SetScaleX( rScaleX );
    aMapMode.SetScaleY( rScaleY );

    if( Owner() )
    {
        // Ziel in Pixel
        Point aOrg	 = pDev->LogicToLogic( rViewPos, NULL, &aMapMode );
        Point aDelta = aOrg - aVisArea_.TopLeft();

        // Origin entsprechend zum sichtbaren Bereich verschieben
        // Origin mit Scale setzen
        aMapMode.SetOrigin( aDelta );

        // Deviceeinstellungen sichern
        pDev->Push();

        Region aRegion;
        if( pDev->IsClipRegion() && pDev->GetOutDevType() != OUTDEV_PRINTER )
        {
            aRegion = pDev->GetClipRegion();
            aRegion = pDev->LogicToPixel( aRegion );
        }
        pDev->SetRelativeMapMode( aMapMode );

        GDIMetaFile * pMtf = pDev->GetConnectMetaFile();
        if( pMtf )
        {
            if( pMtf->IsRecord() && pDev->GetOutDevType() != OUTDEV_PRINTER )
                pMtf->Stop();
            else
                pMtf = NULL;
        }
// #ifndef UNX
        if( pDev->IsClipRegion() && pDev->GetOutDevType() != OUTDEV_PRINTER )
// #endif
        {
            aRegion = pDev->PixelToLogic( aRegion );
            pDev->SetClipRegion( aRegion );
        }
        if( pMtf )
            pMtf->Record( pDev );

        SvOutPlaceObjectRef xOutRef( this );
        if ( xOutRef.Is() )
            xOutRef->DrawObject( pDev, rSetup, rSize, nAspect );
        else
            Draw( pDev, rSetup, nAspect );
        DrawHatch( pDev, aVisArea_.TopLeft(), aVisArea_.GetSize() );

        // Deviceeinstellungen wieder herstellen
        pDev->Pop();
    }
    else
    {
        Size aSize = aVisArea_.GetSize();
        pDev->LogicToLogic( rViewPos, NULL, &aMapMode );
        DoDraw( pDev, rViewPos, aSize, rSetup, nAspect );
    }
}

/*************************************************************************
|*    SvEmbeddedObject::Draw()
|*
|*    Beschreibung
*************************************************************************/
void SvEmbeddedObject::Draw( OutputDevice * /*pDev*/,
                             const JobSetup & /*rSetup*/, USHORT /*nAspect*/ )
{
}

/*************************************************************************
|*    SvEmbeddedObject::GetDocumentPrinter()
|*
|*    Beschreibung:
*************************************************************************/
Printer *	SvEmbeddedObject::GetDocumentPrinter()
{
    SvEmbeddedObjectRef xParent( GetParent() );
    if( xParent.Is() )
        return xParent->GetDocumentPrinter();
    return NULL;
}

/*************************************************************************
|*    SvEmbeddedObject::GetDocumentRefDev()
|*
|*    Beschreibung:
*************************************************************************/
OutputDevice*   SvEmbeddedObject::GetDocumentRefDev()
{
    SvEmbeddedObjectRef xParent( GetParent() );
    if( xParent.Is() )
        return xParent->GetDocumentRefDev();
    return NULL;
}

/*************************************************************************
|*    SvEmbeddedObject::ConvertToOle1()
|*
|*    Beschreibung:
*************************************************************************/
struct ClsIDs {
    UINT32		nId;
    const sal_Char* pSvrName;
    const sal_Char* pDspName;
};

void SvEmbeddedObject::OnDocumentPrinterChanged( Printer * )
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
