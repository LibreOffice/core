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

#include <bf_so3/persist.hxx>
#include <bf_so3/svstor.hxx>
#include <bf_so3/embobj.hxx>
#include <bf_so3/pseudo.hxx>   // fuer Hack
#include "bf_so3/soerr.hxx"   // fuer Hack
#include "bf_so3/outplace.hxx"   // im- export

#include <sot/storinfo.hxx>
#include <sot/formats.hxx>

#include <tools/debug.hxx>

#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>

namespace binfilter {

/************** class SvInfoObject ***************************************/
/*************************************************************************/
SV_IMPL_PERSIST1(SvInfoObject,SvPersistBase)

class SvInfoObject_Impl
{
public:
    String aRealStorageName;
    const String& GetRealStorageName() {return aRealStorageName;}
    void SetRealStorageName( const String& rName )
    {
        if ( aRealStorageName.Len() )
            ::utl::UCBContentHelper::Kill( aRealStorageName );
        aRealStorageName=rName;
    }
};

/************************************************************************
|*    SvInfoObject::SvInfoObject()
|*    SvInfoObject::~SvInfoObject()
|*
|*    Beschreibung
*************************************************************************/
SvInfoObject::SvInfoObject()
    : pImp( new SvInfoObject_Impl )
    , bDeleted( FALSE )
{
}

SvInfoObject::SvInfoObject( SvPersist * pObj, const String & rName )
    : pImp( new SvInfoObject_Impl )
    , bDeleted( FALSE )
{
    SetObj( pObj );
    aObjName = rName;
}

SvInfoObject::SvInfoObject( const String& rObjName,
                            const SvGlobalName& rClassName ) 
    : pImp( new SvInfoObject_Impl )
    , bDeleted( FALSE )
{
    aObjName = rObjName;
    aSvClassName = rClassName;
}

SvInfoObject::~SvInfoObject()
{
    pImp->SetRealStorageName( String() );
    delete pImp;
}

/************************************************************************
|*    SvInfoObject::CreateCopy()
|*    SvInfoObject::MakeCopy()
|*
|*    Beschreibung
*************************************************************************/
SvInfoObjectRef SvInfoObject::CreateCopy() const
{
    SvCreateInstancePersist pFunc = SOAPP->aInfoClassMgr.Get( GetClassId() );
    SvInfoObject* pI;
    SvPersistBase* pB;
    (*pFunc)(&pB);
//    CreateInstance( &pB );

    pI = PTR_CAST(SvInfoObject,pB);
    DBG_ASSERT( pI, "cannot cast" );
    SvInfoObjectRef xI( pI );
    xI->Assign( this );
    return xI;
}

void SvInfoObject::Assign( const SvInfoObject * pObj )
{
    aObjName     = pObj->GetObjName();
    aStorName    = pObj->GetStorageName();
    pImp->SetRealStorageName( pObj->pImp->GetRealStorageName() );
    aSvClassName = pObj->GetClassName();
}

/************************************************************************
|*    SvInfoObject::Load()
|*    SvInfoObject::Save()
|*
|*    Beschreibung
*************************************************************************/
#define INFO_OBJECT_VER_MIN     (BYTE)0
#define INFO_OBJECT_VER_AKT     (BYTE)1

void SvInfoObject::Load( SvPersistStream & rStm )
{
    BYTE nVers = 0;
    rStm >> nVers;
    // wegen Warning unter OS2 auskommentiert
    DBG_ASSERT( /*nVers >= INFO_OBJECT_VER_MIN &&*/
                nVers <= INFO_OBJECT_VER_AKT,
                "SvInfoObject version conflict" );
    if( nVers > INFO_OBJECT_VER_AKT )
        rStm.SetError( SVSTREAM_WRONGVERSION );
    else
    {
        rStm.ReadByteString( aStorName, gsl_getSystemTextEncoding() );
        rStm.ReadByteString( aObjName, gsl_getSystemTextEncoding() );
        if( !aObjName.Len() )
            aObjName = aStorName;
        rStm >> aSvClassName;
        if( aSvClassName == *SvInPlaceObject::ClassFactory()
          || aSvClassName == *SvEmbeddedObject::ClassFactory() )
        {
            // use SvOutPlaceObject class as ole wrapper
            aSvClassName = *SvOutPlaceObject::ClassFactory();
        }
        if(nVers > 0)
            rStm >> bDeleted;
    }
}

void SvInfoObject::Save( SvPersistStream & rStm )
{
    BYTE    nVer = BYTE(INFO_OBJECT_VER_AKT);
    rStm << nVer;
    String aStgName = GetStorageName();
    DBG_ASSERT( aStgName.Len(), "kein Storage-Name" );
    rStm.WriteByteString( aStgName, gsl_getSystemTextEncoding() );
    String aStr( GetObjName() );
    if( aStgName == aStr )
        aStr = String(); // kleine Optimierung
    rStm.WriteByteString( aStr, gsl_getSystemTextEncoding() );
    SvGlobalName aSvClass = SvFactory::GetSvClass( rStm.GetVersion(), GetClassName() );
    if( rStm.GetVersion() <= SOFFICE_FILEFORMAT_40
      && aSvClass == *SvOutPlaceObject::ClassFactory() )
      //old ole-wrapper
      aSvClass = *SvInPlaceObject::ClassFactory();
    rStm << aSvClass;
    rStm << bDeleted;
}

/************************************************************************
|*    SvInfoObject::SetObj()
|*
|*    Beschreibung
*************************************************************************/
void SvInfoObject::SetObj( SvPersist * pObj )
{
    aObj = pObj;
    if( pObj )
        aSvClassName = *pObj->GetSvFactory();
}

/************************************************************************
|*    SvInfoObject::GetObjName()
|*    SvInfoObject::GetStorageName()
|*
|*    Beschreibung
*************************************************************************/
String SvInfoObject::GetObjName() const
{
    return aObjName;
}

String SvInfoObject::GetStorageName() const
{
    if( aStorName.Len() )
        return aStorName;
    return aObjName;
}

SvGlobalName SvInfoObject::GetClassName() const
{
    if( GetPersist() )
        ((SvInfoObject *)this)->aSvClassName = *GetPersist()->GetSvFactory();
    return aSvClassName;
}

void SvInfoObject::SetDeleted( BOOL bDel )
{
    if( bDel == bDeleted )
        return;

    bDeleted = bDel;

    if( aObj.Is() )
    {
        if ( bDel && !pImp->GetRealStorageName().Len() && !aObj->IsHandsOff() )
        {
            SvStorageRef aStor = aObj->GetStorage();
            String aRealName = ::utl::TempFile().GetURL();
            BOOL bKill = TRUE;
            SvStorageRef aNewStor = new SvStorage( !aStor->IsOLEStorage(), aRealName, STREAM_STD_READWRITE, 0 );
            if( aNewStor->GetError() == SVSTREAM_OK )
            {
                bool bRet;
                if( aObj->IsModified() )
                {
                    bRet = aObj->DoSaveAs( aNewStor );
                }
                else
                {
                    bRet = aStor->CopyTo( aNewStor );
                }

                if( bRet )
                {
                    aObj->DoHandsOff();
                    if( aObj->DoSaveCompleted( aNewStor ) )
                    {
                        pImp->SetRealStorageName( aNewStor->GetName() );
                        bKill = FALSE;
                    }
                    else
                        aObj->DoSaveCompleted();
                }
            }

            if ( bKill )
                ::utl::UCBContentHelper::Kill( aRealName );

        }

        if( aObj->IsEnableSetModified() == bDel )
        {
            aObj->EnableSetModified( !bDel );
        }
    }
}

//=========================================================================
//==================class SvObjectContainer================================
//=========================================================================
SV_IMPL_FACTORY(SvObjectContainerFactory)
    {
    }
};
TYPEINIT1(SvObjectContainerFactory,SvFactory);

SO2_IMPL_STANDARD_CLASS1_DLL(SvObjectContainer,SvObjectContainerFactory,SvObject,
                             0x96dee2a1, 0x62f6, 0x11cf,
                             0x89, 0xca, 0x0, 0x80, 0x29, 0xe4, 0xb0, 0xb1 )

//=========================================================================
::IUnknown * SvObjectContainer::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

//=========================================================================
#ifdef TEST_INVARIANT
void SvObjectContainer::TestMemberObjRef( BOOL /*bFree*/ )
{
}

//=========================================================================
void SvObjectContainer::TestMemberInvariant( BOOL /*bPrint*/ )
{
}
#endif

//=========================================================================
SvObjectContainer::SvObjectContainer()
/*  [Beschreibung]

    Konstruktor der Klasse SvObjectContainer.
*/
{
}

//=========================================================================
SvObjectContainer::~SvObjectContainer()
/*  [Beschreibung]

    Destruktor der Klasse SvObjectContainer.
*/
{
    // Parent haelt immer Referenz, deswegen muss der das Objekt
    // schon Removed sein
}






//=========================================================================
//==============class SvPersist============================================
//=========================================================================
SV_IMPL_FACTORY(SvPersistFactory)
    {
    }
};
TYPEINIT1(SvPersistFactory,SvFactory);

SO2_IMPL_STANDARD_CLASS1_DLL(SvPersist,SvPersistFactory,SvObjectContainer,
                             0xC24CC4E0L, 0x73DF, 0x101B,
                             0x80,0x4C,0xFD,0xFD,0xFD,0xFD,0xFD,0xFD )

::IUnknown * SvPersist::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

//=========================================================================
#ifdef TEST_INVARIANT
void SvPersist::TestMemberObjRef( BOOL /*bFree*/ )
{
    if( pChildList )
    {
        ULONG nCount = pChildList->Count();
        for( ULONG i = 0; i < nCount; i++ )
        {
            SvInfoObject * pEle = pChildList->GetObject( i );
            if( pEle->GetObj() )
            {
                ByteString aTest( "\t\tpChildList[ " );
                aTest += ByteString::CreateFromInt32( i );
                aTest += " ] == ";
                aTest += ByteString::CreateFromInt32( (ULONG)pEle->GetObj() );
                DBG_TRACE( aTest.GetBuffer() );
            }
        }
    }
    if( aStorage.Is() )
    {
        ByteString aTest( "\t\taStorage == " );
        aTest += ByteString::CreateFromInt32( (ULONG)&aStorage );
        DBG_TRACE( aTest.GetBuffer() );
    }
}

//=========================================================================
void SvPersist::TestMemberInvariant( BOOL /*bPrint*/ )
{
#ifdef DBG_UTIL
    DBG_ASSERT( !(bOpSave || bOpSaveAs), "bOpSave && bOpSaveAs" );
    if( !bIsInit )
    {
        if ( !bCreateTempStor )
        {
            DBG_ASSERT( bOpHandsOff && !aStorage.Is(), "failed: bOpHandsOff && !aStorage.Is()" );
            DBG_ASSERT( !bOpHandsOff && aStorage.Is(), "failed: !bOpHandsOff && aStorage.Is()" );
        }
    }
#endif
}
#endif

//=========================================================================
SvPersist::SvPersist()
    : bIsModified     ( FALSE )
    , bIsInit         ( FALSE )
    , bOpSave         ( FALSE )
    , bOpSaveAs       ( FALSE )
    , bSaveExtern     ( FALSE )
    , bOpHandsOff     ( FALSE )
    , bCreateTempStor ( FALSE )
    , bSaveFailed     ( FALSE )
    , bEnableSetModified( TRUE )
    , bIsObjectShell  ( FALSE )
    , nModifyCount    ( 0 )        // Anzahl der modifizierten Childs und sich selbst
    , pParent         ( NULL )     // kein zusaetzlicher RefCount
    , pChildList      ( NULL )
{
}

//=========================================================================
SvPersist::~SvPersist()
{
    // Parent haelt immer Referenz, deswegen muss der das Objekt
    // schon Removed sein
    dtorClear();
}

#ifdef DBG_UTIL
/*************************************************************************
|*    SvPersist::AssertInit()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::AssertInit() const
{
    DBG_ASSERT( bIsInit, "super class SvPersist: call InitNew or Load bevor other calls" );
}
#endif

//=========================================================================
void SvPersist::FillClass
(
    SvGlobalName * pClassName,  /* Der Typ der Klasse */
    ULONG * pFormat,            /* Das Dateiformat in dem geschrieben wird */
    String * pAppName,          /* Der Applikationsname */
    String * pFullTypeName,     /* Der genaue Name des Typs */
    String * pShortTypeName,    /* Der kurze Name des Typs  */
    long /*nFileFormat */       /* F"ur dieses Office-Format sollen die
                                   Parameter gef"ullt werden */
) const
/*  [Beschreibung]

    Diese Methode liefert Informationen "uber den Typ und das Dateiformat
    des Objektes. Alle Parameter werden von der Methode gefuellt.

    [R"uckgabewert]

    *pClassName     Liefert den Typ-Identifier des Objektes.
    *pFormat        Die FormatId des Storages.
    *pAppName       Den sprachabh"angigen Applikationsnamen.
    *pFullTypeName  Den sprachabh"angigen Namen des Typs.
    *pShortTypeName Den kurzen sprachabh"angigen Namen des Typs. Er darf
                    nicht l"anger als 15 Zeichen sein.

    [Anmerkung]

    F"uer externe Objekte ist pAppName und pShortName leer.

    [Beispiel]

    MyClass::FillClass( ... )
    {
        *pClassName     = *SvFactory::GetSvFactory(); // keine emulation
        *pFormat        = nMyDocFormat;
        *pAppName       = "StarDivison Calc 3.0";
        *pFullTypeName  = "StarDivison Calc 3.0 Tabelle";
        *pShortTypeName = "Tabelle";
    }
*/
{
    *pFormat        = 0;
    *pFullTypeName  = *pShortTypeName = *pAppName = String();
    *pClassName     = SvGlobalName();

    if( Owner() )
        *pClassName = *GetSvFactory();
}

/*************************************************************************
|*    SvPersist::SetupStorage()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::SetupStorage( SvStorage * pStor ) const
{
    SvUniqueName    aName;
    String          aFullTypeName, aShortTypeName, aAppName;
    ULONG           nClipFormat;

    // this code is used in the binfilter module only
    // this module will not be made aware of any new file formats
    ULONG nVersion = pStor->GetVersion();
    if ( nVersion > SOFFICE_FILEFORMAT_60 )
    {
        nVersion = SOFFICE_FILEFORMAT_60;
        pStor->SetVersion( nVersion );
    }

    FillClass( &aName, &nClipFormat, &aAppName,
                &aFullTypeName, &aShortTypeName, pStor->GetVersion() );
    pStor->SetClass( aName, nClipFormat, aShortTypeName );
}

/*************************************************************************
|*    SvPersist::GetInfoList()
|*
|*    Beschreibung
*************************************************************************/
SvInfoObjectMemberList * SvPersist::GetInfoList()
{
    if( !pChildList )
        pChildList = new SvInfoObjectMemberList();
    return pChildList;
}

/*************************************************************************
|*    SvPersist::SetModifiedFormat()
|*
|*    Beschreibung
*************************************************************************/
#ifdef _NO_MODIFY_ADVISE
void SvPersist::SetModifiedFormat( ULONG nFormat )
{
    if( !Owner() )
    { // sonst wird das durch die Baumstruktur erledigt
        if( aAdvSink.Is() )
        {
            DBG_ASSERT( !nFormat, "SvPersist::SetModifiedFormat: double set" );
            SvAdviseRef aAdv( this );
            aAdv->RemoveAdvise( &aAdvSink, ADVISE_DATA );
            aAdvSink.Clear();
        }
        if( nFormat )
        {
            SvAdviseRef aAdv( this );
            if( aAdv.Is() )
            { // Es kann ein Link gesetzt werden
                aAdvSink = new SvPersistAdviseSink( this );
                aAdv->AddDataAdvise( &aAdvSink, SvDataType( nFormat ),
                                     ADVISEMODE_NODATA );
                return;
            }
            DBG_ASSERT( aAdv.Is(), " cannot install data advise" );
        }
    }
}
#endif

/*************************************************************************
|*    SvPersist::Insert()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::Insert( SvInfoObject * pInfoObj )
{
    ASSERT_INIT()
    SvInfoObjectRef aRef( pInfoObj );

    DBG_ASSERT( pInfoObj->GetObjName().Len(),
                "Try to Insert Object without name" );
    BOOL bRet = TRUE;
    if( !GetInfoList() )
        return FALSE; // es konnte keine Liste erstellt werden

    SvPersist * pChild = pInfoObj->GetPersist();
    if( pChild && this == pChild->pParent )
        bRet = FALSE;

    if( pChild )
    {
        // als erstes, wegen Patch in IsModified
        if( pChild->Owner() && pChild->IsModified() )
            // modify-Zaehler fuer Child raufzaehlen
            CountModified( TRUE );

        if( pChild->pParent )
            // Aus altem Parent entfernen
            pChild->pParent->Remove( pChild );

        pChild->pParent   = this;        // kein zusaetzlicher RefCount
    }

    //pChild->SetModifiedFormat( SvEmbeddedObject::GetFormat() );
    pChildList->Append( pInfoObj );
    SetModified( TRUE );

    return TRUE;
}

/*************************************************************************
|*    SvPersist::Move()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::Move( SvInfoObject * pInfoObj, const String & rStorName, BOOL bCopyStorage )
{
    ASSERT_INIT()
    SvInfoObjectRef aRef( pInfoObj );
    DBG_ASSERT( pInfoObj->GetObjName().Len(), "Try to Move Object without name" );

    BOOL bRet = TRUE;
    if( !GetInfoList() )
        bRet = FALSE;

    SvPersist * pChild = pInfoObj->GetPersist();
    if( !pChild )
        bRet = FALSE;

    if( bRet && ( this != pChild->pParent || !GetStorage()->IsStorage( rStorName ) ) )
    {
        bRet = FALSE;
        String aRealName;
        SvPseudoObjectRef xPO( pChild );
        if ( !GetStorage()->IsOLEStorage() && xPO.Is() && ( xPO->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) )
        {
            (void)bCopyStorage;
            // save in binary format - there is no other format !
            DBG_ASSERT( !bCopyStorage, "Impossible to copy storage!" );
            aRealName = ::utl::TempFile().GetURL();
            SvStorageRef aNewStor( new SvStorage( FALSE, aRealName, STREAM_STD_READWRITE, 0 ) );
            if ( pChild->DoSaveAs( aNewStor ) )
            {
                bRet = pChild->DoSaveCompleted( aNewStor );
                if ( !bRet )
                    pChild->DoSaveCompleted();
            }
        }
        else
        {
            bRet = ImplCopy( pChild, rStorName, TRUE );
        }

        if( bRet )
        {
            pInfoObj->pImp->SetRealStorageName( aRealName );
            bRet = Insert( pInfoObj );
        }
        else if ( aRealName.Len() )
            ::utl::UCBContentHelper::Kill( aRealName );
    }

    return bRet;
}

/*************************************************************************
|*    SvPersist::CopyObject()
|*
|*    Beschreibung
*************************************************************************/
SvPersistRef SvPersist::CopyObject( const String& rObjName, const String& rNewName, SvPersist * pSrc )
{
    SvPersistRef    xNewObject;
    SvPersist       *pSrcPersist = ( pSrc ? pSrc : this );
    SvInfoObject    *pOld = pSrcPersist->Find( rObjName );
    if( pOld )
    {
        SvInfoObjectRef xNewInfo( pOld->CreateCopy() );

        if ( pOld->GetPersist() )
        {
            SvEmbeddedInfoObject* pI = PTR_CAST( SvEmbeddedInfoObject, pOld );
            SvEmbeddedObjectRef xEmbObj( pOld->GetPersist() );
            if ( pI && xEmbObj.Is() )
                pI->SetInfoVisArea( xEmbObj->GetVisArea() );
        }

        SvPersistRef        xOldObject( pSrcPersist->GetObject( rObjName ) );
        SvPseudoObjectRef   xPO( xOldObject );
        if ( !GetStorage()->IsOLEStorage() && xPO.Is() && ( xPO->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) )
        {
            // save in binary format - there is no other format !
            String aRealName = ::utl::TempFile().GetURL();
            SvStorageRef xNewStor( new SvStorage( FALSE, aRealName, STREAM_STD_READWRITE, 0 ) );
            if ( xOldObject->DoSaveAs( xNewStor ) )
            {
                xOldObject->DoSaveCompleted();
                xNewInfo->SetObjName( rNewName );
                xNewInfo->pImp->aRealStorageName = xNewStor->GetName();
                GetInfoList()->Append( xNewInfo );
                SetModified( TRUE );

                // create and load new object
                xNewObject = CreateObjectFromStorage( xNewInfo, xNewStor );
            }
            else
                ::utl::UCBContentHelper::Kill( aRealName );
        }
        else
        {
            Copy( rNewName, rNewName, pOld, pSrcPersist );
            xNewObject = GetObject( rNewName );
        }
    }

    return xNewObject;
}

/*************************************************************************
|*    SvPersist::Copy()
|*
|*    Beschreibung: Kopiert ein Objekt von der einen in die andere
|*                  Persist Instanz
*************************************************************************/
BOOL SvPersist::Copy( const String & rNewObjName, const String & rNewStorName,
                      SvInfoObject * pSrcI, SvPersist * pSrc )
{
    BOOL bRet = TRUE;
    if( !GetInfoList() )
        bRet = FALSE; // es konnte keine Liste erstellt werden

    SvInfoObjectRef xI( pSrcI->CreateCopy() );
    xI->SetObjName( rNewObjName );
    xI->SetStorageName( rNewStorName );
    xI->pImp->aRealStorageName.Erase();

    DBG_ASSERT( rNewStorName == rNewObjName || !rNewStorName.Len(), "Can't set storage name in this context!" );

    if( pSrcI->GetPersist() )
    {
        bRet = ImplCopy( pSrcI->GetPersist(), xI->GetStorageName(), FALSE );
    }
    else
    {
        // copy storage of new element into my storage
        bRet = pSrc->GetStorage()->CopyTo( pSrcI->GetStorageName(), GetStorage(), xI->GetStorageName() );
    }

    if( bRet )
    {
        pChildList->Append( xI );
        SetModified( TRUE );
    }

    return bRet;
}


BOOL SvPersist::ImplCopy( SvPersist* pSrc, const String& rStorageName, BOOL bMoving )
{
    BOOL bRet = FALSE;
    SvStorageRef aSrcEleStor = pSrc->GetStorage();
    SvStorage* pStor = GetStorage();

    long nObjVersion = aSrcEleStor->GetVersion();
    BOOL bIntern = SvFactory::IsIntern( aSrcEleStor->GetClassName(), &nObjVersion );
    if( nObjVersion >= SOFFICE_FILEFORMAT_60 )
    {
        ULONG nFormat = aSrcEleStor->GetFormat();
        switch( nFormat )
        {
        case SOT_FORMATSTR_ID_STARWRITER_8:
        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
        case SOT_FORMATSTR_ID_STARDRAW_8:
        case SOT_FORMATSTR_ID_STARIMPRESS_8:
        case SOT_FORMATSTR_ID_STARCALC_8:
        case SOT_FORMATSTR_ID_STARCHART_8:
        case SOT_FORMATSTR_ID_STARMATH_8:
            nObjVersion = SOFFICE_FILEFORMAT_8;
            break;
        default:
            break;
        }
    }

    SvPseudoObjectRef xPO( pSrc );
    if ( !pStor->IsOLEStorage() && xPO.Is() && ( xPO->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) )
    {
        // these object are saved into the document stream
        bRet = FALSE;
        DBG_ERROR("Can't copy special objects into XML format!");
    }
    else
    {
        SvStorageRef aNewStor;
        if( !bIntern || pStor->IsOLEStorage() )
            aNewStor = pStor->OpenOLEStorage( rStorageName, STREAM_STD_READWRITE | STREAM_TRUNC, 0 );
        else
            aNewStor = pStor->OpenUCBStorage( rStorageName, STREAM_STD_READWRITE | STREAM_TRUNC, 0 );

        if( aNewStor->GetError() == SVSTREAM_OK )
        {
            BOOL bSave = pSrc->IsModified();
            if ( !bSave )
            {
                SvStorageInfoList aList;
                aSrcEleStor->FillInfoList( &aList );
                bSave = !aList.Count();
                if ( !bSave )
                {
                    if( pStor->GetVersion() <= SOFFICE_FILEFORMAT_50 )
                    {
                        // If we store into the old binary format, objects
                        // that have a newer file format version than the
                        // destination storage have to be saved.
                        bSave = bIntern && nObjVersion > pStor->GetVersion();
                    }
                    else
                    {
                        // If we store into an XML format, all internal objects that
                        // aren't stored in the XML format or that are stored in
                        // a newer XML format have to be saved.
                        bSave =
                                ( nObjVersion < SOFFICE_FILEFORMAT_60 ||
                                    nObjVersion > pStor->GetVersion() );
                    }
                }
            }

            aNewStor->SetVersion( pStor->GetVersion() );

            if( bSave )
            {
                bRet = pSrc->DoSaveAs( aNewStor );
                if ( bRet && !bMoving )
                    pSrc->DoSaveCompleted();
            }
            else
            {
                SvStorageRef xEleStor = pSrc->GetStorage();
                pSrc->DoHandsOff();
                bRet = xEleStor->CopyTo( aNewStor );
                if ( !bRet || !bMoving )
                    pSrc->DoSaveCompleted( xEleStor );
            }

            if ( bRet && bMoving )
                pSrc->DoSaveCompleted( aNewStor );
        }
    }

    return bRet;
}

/*************************************************************************
|*    SvPersist::unload()
|*
|*    Beschreibung:
*************************************************************************/
BOOL SvPersist::Unload( SvInfoObject *pInfoObj)
{
    if( bOpSaveAs || bOpHandsOff || bOpSave )
        return FALSE;

    DBG_ASSERT(pInfoObj,"Kein InfoObj");
    SvPersistRef xChild = pInfoObj->GetPersist();
    DBG_ASSERT(xChild->pParent == this,"wrong parent");
    if( xChild.Is() )
    {
        if( xChild->Owner() )
        {
            DBG_ASSERT(!xChild->IsModified(),"unload modified object");
            if( xChild->IsModified() )
                // modified, do not unload
                return FALSE;
        }

        // update info object before unload
        SvEmbeddedInfoObject * pI = PTR_CAST(SvEmbeddedInfoObject, pInfoObj );
        if( pI )
        {
            pI->GetVisArea();
            pI->IsLink();
        }

        //if ( pInfoObj->pImp->GetRealStorageName().Len() )
            // can't unload at the moment
            //return TRUE;

        SvPersistRef x;
        pInfoObj->SetObj( x );

        // Ein Hack, da ueber den RefCount keine Logik implementiert werden daerf
        if ( (xChild->bIsObjectShell && xChild->GetRefCount() == 2)
          || (!xChild->bIsObjectShell && xChild->GetRefCount() == 1) )
        {
            xChild->DoClose();
            xChild->pParent = NULL;
            return TRUE;
        }
        else
            pInfoObj->SetObj( xChild );
    }

    return FALSE;
}

BOOL SvPersist::Unload( SvPersist * pChild )
{
    DBG_ASSERT( pChild, "SvPersist::Unload(): pChild == NULL" );
    if( pChildList )
    {
        SvInfoObjectRef pEle = pChildList->First();
        while( pEle.Is() )
        {
            if( pEle->GetPersist() == pChild )
                return Unload( pEle );
            pEle = pChildList->Next();
        }
    }
    return FALSE;
}

/*************************************************************************
|*    SvPersist::Remove()
|*
|*    Beschreibung:
*************************************************************************/
void SvPersist::Remove( SvInfoObject *pInfoObj)
{
    DBG_ASSERT(pInfoObj,"Kein InfoObj");
    SvPersist * pChild = pInfoObj->GetPersist();
    if( pChild )
    {
        if( pChild->Owner() && pChild->IsModified() )
            // modify-Zaehler fuer Child runterzaehlen
            CountModified( FALSE );
        if( pChild->pParent == this )
            pChild->pParent = NULL;  // kein zusaetzlicher RefCount
    }
    pChildList->Remove( pInfoObj );
    SetModified( TRUE );
}


void SvPersist::Remove( const String & rName )
{
    SvInfoObjectRef pInfo = Find( rName );
    if( pInfo.Is() )
        Remove(pInfo);
}

void SvPersist::Remove( SvPersist * pChild )
{
    DBG_ASSERT( pChild, "SvPersist::Remove(): pChild == NULL" );
    if( pChildList )
    {
        SvInfoObjectRef pEle = pChildList->First();
        while( pEle.Is() )
        {
            if( pEle->GetPersist() == pChild )
            {
                Remove(pEle);
                break;
            }
            pEle = pChildList->Next();
        }
    }
}

void SvPersist::dtorClear()
{
    if( pChildList )
    {
        SvInfoObjectMemberList * pList = pChildList;
        pChildList = NULL; //Schutz vor Remove bei Destruktor
        SvInfoObjectRef pEle = pList->Last();
        pList->Remove();
        while( pEle.Is() )
        {
            if( pEle->GetPersist() )
            {
                //pEle->GetPersist()->SetModifiedFormat( 0 );
                pEle->GetPersist()->pParent = NULL;  // kein zusaetzlicher RefCount
            }
            pEle = pList->Last();
            pList->Remove();
        }
        delete pList;
    }
}

SvInfoObject * SvPersist::Find( const String & rName ) const
{
    if( pChildList )
    { // Befindet sich das Objekt in der aktuellen Liste
        SvInfoObjectRef pEle = pChildList->First();
        while( pEle.Is() )
        {
            if( pEle->GetObjName() == rName )
                return pEle;
            pEle = pChildList->Next();
        }
    }
    return NULL;
}

SvInfoObject * SvPersist::Find( const SvPersist * pObj_ ) const
{
    if( pChildList )
    { // Befindet sich das Objekt in der aktuellen Liste
        SvInfoObject* pEle = pChildList->First();
        while( pEle )
        {
            if( pEle->GetPersist() == pObj_ )
                return pEle;
            pEle = pChildList->Next();
        }
    }
    return NULL;
}




/*************************************************************************
|*    SvPersist::HasObject()
|*    SvPersist::CanRunObject()
|*    SvPersist::GetObject()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::HasObject( const String & rObjName )
{
    if( Owner() )
    {
        // Befindet sich das Objekt in der aktuellen Liste
        if( Find( rObjName ) )
            return TRUE;
    }
    return FALSE;
}

SvPersistRef SvPersist::CreateObjectFromStorage( SvInfoObject* pEle, const SvStorageRef& xStor )
{
    // Es gibt einen Storage zum Child
    // entsprechende Sv-Klasse erzeugen
    SvFactory* pFact=NULL;
    SvPersistRef xPer = &pFact->CreateAndLoad( xStor );
    if( xPer.Is() )
    {
        // Parent einsetzten
        xPer->pParent = this;

        // ist ein persistentes Objekt
        // muss initialisiert sein
        pEle->SetObj( xPer );
    }

    SvEmbeddedInfoObject* pI = PTR_CAST( SvEmbeddedInfoObject, pEle );
    SvEmbeddedObjectRef xEmbObj( xPer );
    if ( pI && xEmbObj.Is() )
    {
        BOOL bOld = xEmbObj->IsEnableSetModified();
        xEmbObj->EnableSetModified(FALSE);
        xEmbObj->SetVisArea( pI->GetVisArea() );
        xEmbObj->EnableSetModified(bOld);
    }

    return xPer;
}

SvPersistRef SvPersist::GetObject( const String & rObjName )
{
    SvPersistRef xReturn;
    if( Owner() )
    {
        SvInfoObject * pEle = Find( rObjName );
        if( !pEle )
            return SvPersistRef();
        if( pEle->GetPersist() )
            return pEle->GetPersist();

        SvStorageRef xSt = GetObjectStorage( pEle );
        if( xSt.Is() && xSt->SotStorage::GetError() == SVSTREAM_OK )
        {
            xReturn = CreateObjectFromStorage( pEle, xSt );
        }
        else
            GetStorage()->ResetError();
    }

    return xReturn;
}

SvStorageRef SvPersist::GetObjectStorage( SvInfoObject* pEle )
{
    SvStorageRef xRet;
    SvPersist* pP = pEle->GetPersist();
    if( pP )
        xRet = pP->GetStorage();
    else
    {
        if ( pEle->pImp->GetRealStorageName().Len() )
            xRet = new SvStorage( pEle->pImp->GetRealStorageName() );
        else
            xRet = GetStorage()->OpenStorage( pEle->GetStorageName() );
    }

    return xRet;
}

/*************************************************************************
|*    SvPersist::SetModified()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::SetModified( BOOL bModifiedP )
{
    ASSERT_INIT()
    DBG_ASSERT( bModifiedP || IsEnableSetModified(),
                "SetModified( FALSE ), obwohl IsEnableSetModified() == FALSE" );

    if( !IsEnableSetModified() )
        return;

    if( bIsModified != bModifiedP )
    { // der Status hat sich geaendert
        // Hilfsflag setzen
        bIsModified = bModifiedP;
        // Hierarchisch ModifyCount setzen
        CountModified( bModifiedP );
    }

    aModifiedTime = Time();
}

/*************************************************************************
|*    SvPersist::CountModified()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::CountModified( BOOL bMod )
{
    DBG_ASSERT( bMod || nModifyCount != 0, "modifiy count underflow" );
    nModifyCount += bMod ? 1 : -1;
    if( pParent )
    {
        /* Den ModifyCount des Parent nur einmal rauf oder runter
           zaehlen. Bei den Insert und Move Methoden muss dieser
           Zaehler dann nur einmal erhoeht oder erniedrigt werden
        */
        if( (bMod && nModifyCount == 1)
          || (!bMod && nModifyCount == 0) )
            pParent->CountModified( bMod );
    }
    if ( ( nModifyCount == 1 && bMod ) ||
         ( nModifyCount == 0 ) )
        ModifyChanged();
}

/*************************************************************************
|*    SvPersist::SetModifiedChanged()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::EnableSetModified( BOOL bEnable )
{
    DBG_ASSERT( bEnable != bEnableSetModified,
                "EnableSetModified 2x mit dem gleichen Wert gerufen" );
    bEnableSetModified = bEnable;
}

/*************************************************************************
|*    SvPersist::ModifyChanged()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::ModifyChanged()
{
}

/*************************************************************************
|*    SvPersist::IsModified()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::IsModified()
{
    ASSERT_INIT()
    if( nModifyCount )
        // Wenn Count gesetzt, auf jeden Fall modifiziert
        return TRUE;
    if( Owner() )
    {
        if( pChildList )
        {
            SvInfoObject * pObj_ = pChildList->First();
            while( pObj_ )
            {
                if( pObj_->GetPersist() && pObj_->GetPersist()->IsModified() )
                    return TRUE;
                pObj_ = pChildList->Next();
            }
        }
    }
    return FALSE;
}

/*************************************************************************
|*    SvPersist::Init()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::InitMembers( SvStorage * pStor )
{
    bIsInit     = TRUE;
    if ( pStor )
        aStorage    = pStor;
    else
        bCreateTempStor = TRUE;
}

/*************************************************************************
|*    SvPersist::DoInitNew()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::DoInitNew( SvStorage * pStor )
{
    EnableSetModified( FALSE );
    BOOL bRet = InitNew( pStor );
    EnableSetModified( TRUE );
    return bRet;
}

/*************************************************************************
|*    SvPersist::DoLoad()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::DoLoad( SvStorage * pStor )
{
    EnableSetModified( FALSE );
    BOOL bRet = Load( pStor );
    EnableSetModified( TRUE );
    return bRet;
}

//=========================================================================
BOOL SvPersist::DoLoad
(
    const String & rFileName,   /* Name der Datei, aus der das Objekt
                                   geladen werden soll. */
    StreamMode nStreamMode,     /* Attribute f"ur um die Datei zu "offnen */
    StorageMode nStorageMode    /* Attribute f"ur um die Datei zu "offnen */
)
/*  [Beschreibung]

    Das Objekt soll aus der angegebenen Datei geladen werden.

    [Anmerkung]

    Aufgrund fehlerhafter Ole funktionalitaet, wird der Stream auch
    schreibend geoeffnet, wenn nur READ angegeben ist und er nicht
    schreibgeschuetzt ist.

    [Querverweise]

    <SvPseudoObject::DoSave>
*/
{
    SvStorageRef xStg;
    SvGlobalName aGN;
    // Es wird nur die IPersistStorage-Schnittstelle angeboten
    xStg = new SvStorage( rFileName, nStreamMode | STREAM_WRITE, nStorageMode );
    if( !xStg.Is() )
        xStg = new SvStorage( rFileName, nStreamMode, nStorageMode );
    aGN = xStg->GetClassName();
    if( !xStg.Is() && aGN == *GetSvFactory() )
    {
        xStg = new SvStorage( rFileName, nStreamMode | STREAM_WRITE, nStorageMode );
        if( !xStg.Is() )
            xStg = new SvStorage( rFileName, nStreamMode, nStorageMode );
    }
    if( xStg.Is() && SVSTREAM_OK != xStg->GetError() )
        return FALSE;

    SetFileName( rFileName );
    return DoLoad( xStg );
}

BOOL SvPersist::DoSave()
{
    EnableSetModified( FALSE );
    bSaveFailed = !Save();
    EnableSetModified( TRUE );
    return !bSaveFailed;
}

BOOL SvPersist::DoSaveAs( SvStorage * pStor )
{
    EnableSetModified( FALSE );
    BOOL bRet = SaveAs( pStor );
    EnableSetModified( TRUE );
    return bRet;
}

void SvPersist::DoHandsOff()
{
    HandsOff();
}

BOOL SvPersist::DoSaveCompleted( SvStorage * pStor )
{
    return SaveCompleted( pStor );
}


/*************************************************************************
|*    SvPersist::GetStorage() const
|*
|*    Beschreibung
*************************************************************************/
SvStorage * SvPersist::GetStorage() const
{
    ASSERT_INIT()
    DBG_ASSERT( !bOpHandsOff, "in hands off state, no accessible storage" );
    if( bCreateTempStor )
    {
        SvPersist *pThis = (SvPersist*) this;
        DBG_ASSERT( !aStorage.Is(), "bCreateTempStorage && aStorage.Is()" );
        pThis->aStorage = new SvStorage( FALSE, String() );
        pThis->bCreateTempStor = FALSE;
        SetupStorage( pThis->aStorage );
    }

    return aStorage;
}

/*************************************************************************
|*    SvPersist::InitNew()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::InitNew( SvStorage * pStor )
{
    InitMembers( pStor );
    BOOL bRet = FALSE;

    if( pStor )
        SetupStorage( pStor );

    if( Owner() )
        bRet = TRUE;
    return bRet;
}

/*************************************************************************
|*    SvPersist::Load()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::DoOwnerLoad( SvStorage * pStor )
{
    InitMembers( pStor );

    return DoLoadContent( pStor, TRUE );
}

/*************************************************************************
|*    SvPersist::Load()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::Load( SvStorage * pStor )
{
    dtorClear();
    InitMembers( pStor );

    SvGlobalName aActualClassName =
        SvFactory::GetAutoConvertTo( GetStorage()->GetClassName() );

    if( aActualClassName == *GetSvFactory() && SOFFICE_FILEFORMAT_60 > pStor->GetVersion() )
        return DoLoadContent( pStor, TRUE );

    return TRUE;
    //return DoLoadContent( pStor, FALSE );
}

/*************************************************************************
|*    SvPersist::Save()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::Save()
{
    ASSERT_INIT()

    SvGlobalName aNoName; // wegen MAC
    if( GetStorage()->GetClassName() == aNoName )
        // kein Typ im Storage gesetzt
        SetupStorage( GetStorage() );

    bOpSave = TRUE;
    if( !IsModified() )
        // keine Aendeungen
        return TRUE;

    BOOL bRet = TRUE;
//    GetStorage()->SetClassName( GetClassName() );

    SvStorage *pStor = GetStorage();
    if( SOFFICE_FILEFORMAT_60 > pStor->GetVersion() )
        bRet = DoSaveContent( GetStorage(), TRUE );
    return bRet;
}

/*************************************************************************
|*    SvPersist::SaveAs()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::SaveAs( SvStorage * pStor )
{
    ASSERT_INIT()

    bOpSaveAs = TRUE;

    BOOL bRet = TRUE;

    SvGlobalName aNoName; // wegen MAC
    if( pStor->GetClassName() == aNoName )
        // kein Typ im Storage gesetzt
        SetupStorage( pStor );

    if( SOFFICE_FILEFORMAT_60 > pStor->GetVersion() )
    {
        if( SOFFICE_FILEFORMAT_31 == pStor->GetVersion() && GetParent() )
        {
            // Es ist ein Objekt
            bRet = DoSaveContent( pStor,
                SvFactory::IsIntern31( pStor->GetClassName() ) );
        }
        else
            bRet = DoSaveContent( pStor, TRUE );
    }

    return bRet;
}

/*************************************************************************
|*    SvPersist::HandsOff()
|*
|*    Beschreibung
*************************************************************************/
void SvPersist::HandsOff()
{
    ASSERT_INIT()

    if ( bOpHandsOff )
        return;

    if( pChildList )
    {
        for( ULONG i = 0; i < pChildList->Count(); i++ )
        {
            SvInfoObject * pEle = pChildList->GetObject( i );
            if( pEle->GetPersist() && !pEle->IsDeleted() )
            {
                // deleted objects don't need to take their hands off!
                ULONG nVersion = GetStorage()->GetVersion();
                SvEmbeddedObjectRef xEmb( pEle->GetPersist() );
                if ( xEmb.Is() && nVersion >= SOFFICE_FILEFORMAT_60 && ( xEmb->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) )
                    continue;
                // Im HandsOff darf nicht mehr auf Persist zugegriffen werden
                // deswegen alten Namen sichern
//??                pEle->aStorName = pEle->GetStorageName();
                pEle->GetPersist()->DoHandsOff();
            }
            pEle = pChildList->Next();
        }
    }

    bOpHandsOff = TRUE;
    aStorage.Clear();
}

/*************************************************************************
|*    SvPersist::SaveCompleted()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::SaveCompleted( SvStorage * pStor )
{
    //DBG_ASSERT( bOpHandsOff | bOpSaveAs | bOpSave, aTest );
    ASSERT_INIT()

    if( !pStor && aStorage.Is() )
        // falls beim Save ein Fehler aufgetreten ist, muss er zurueckgesetzt
        // werden
        aStorage->ResetError();

    BOOL bRet = TRUE;

    if( pStor )
    {
        aStorage  = pStor;
        SvGlobalName aNoName; // wegen MAC
        if( pStor->GetClassName() == aNoName )
            // kein Typ im Storage gesetzt
            SetupStorage( pStor );
        bCreateTempStor=FALSE;
    }

    if( Owner() )
    {
        if( !bSaveFailed  )
        {
            if( bOpSave )
            {
                if ( IsModified() && GetParent() )
                    GetParent()->SetModified( TRUE );

                SetModified( FALSE );
                DBG_ASSERT( !nModifyCount, "IsModified() == TRUE after save" );
            }
            if( bOpSaveAs )
            {
                if( pStor )
                {   // nur wenn SaveAs und danach der Storage umgesetzt wird,
                    // sonst war es z.B. ein SaveAs fuers Clipboard
                    if ( IsModified() && GetParent() )
                        GetParent()->SetModified( TRUE );
                    SetModified( FALSE );
                    DBG_ASSERT( !IsModified(), "SvPersist::SaveCompleted: IsModified() == TRUE" );
                }
            }
        }
    }
    bOpSaveAs = bOpSave = bOpHandsOff = bSaveFailed = FALSE;
    return bRet;
}

/*************************************************************************
|*    SvPersist::LoadContent()
|*
|*    Beschreibung
*************************************************************************/
#define PERSIST_STREAM "persist elements"
BOOL SvPersist::DoLoadContent( SvStorage * pStor, BOOL bOwner_ )
{
    SvStorageStreamRef aContStm;
    if( bOwner_ )
        aContStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( PERSIST_STREAM ) ),
                                     STREAM_READ | STREAM_NOCREATE );
    else
    {

        aContStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SVEXT_PERSIST_STREAM ) ),
                                     STREAM_READ | STREAM_NOCREATE );
        if( aContStm->GetError() == SVSTREAM_FILE_NOT_FOUND )
            // Stream nicht vorhanden, Ole10Native probieren
            aContStm = pStor->OpenStream( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole10Native" ) ),
                                         STREAM_READ | STREAM_NOCREATE );
    }

    // nicht vorhandener Stream ist kein Fehler
    if( aContStm->GetError() == SVSTREAM_FILE_NOT_FOUND )
        return TRUE;

    aContStm->SetVersion( pStor->GetVersion() );
    if( aContStm->GetError() == SVSTREAM_OK )
    {
        aContStm->SetBufferSize( 8192 );
#if defined( SOLARIS ) && defined( C40 )
     // patch to work around Sparc-Compiler bug
     SvEmbeddedObjectRef xSvEmbeddedObjectRef = this;
     if( xSvEmbeddedObjectRef.Is() )
        xSvEmbeddedObjectRef->LoadContent( *aContStm, bOwner_ );
     else
#else
        LoadContent( *aContStm, bOwner_ );
#endif
        aContStm->SetBufferSize( 0 );
        return aContStm->GetError() == SVSTREAM_OK;
    }
    return FALSE;
}

#define CHILD_LIST_VER (BYTE)2
void SvPersist::LoadContent( SvStream & rStm, BOOL bOwner_ )
{
    if( bOwner_ )
    {
        BYTE nVers;
        rStm >> nVers;
        DBG_ASSERT( nVers == CHILD_LIST_VER, "version conflict" );

        if( nVers != CHILD_LIST_VER )
            rStm.SetError( SVSTREAM_WRONGVERSION );
        else
        {
            BOOL bList;
            rStm >> bList;
            if( bList )
            {
                SvPersistStream aPStm( SOAPP->aInfoClassMgr, &rStm );
                aPStm >> *GetInfoList();
            }
        }
    }
    // nichts tun bei Fremd-Format
}

/*************************************************************************
|*    SvPersist::SaveContent()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::DoSaveContent( SvStorage * pStor, BOOL bOwner_ )
{
    // MAC MPW mag's sonst nicht ...
    String aPersistStream( bOwner_ ? String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( PERSIST_STREAM ) )
                                    : String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( SVEXT_PERSIST_STREAM ) ) );
    SvStorageStreamRef aContStm = pStor->OpenStream( aPersistStream );
    if( aContStm.Is() )
    {
        // Version am Stream setzen
        aContStm->SetVersion( pStor->GetVersion() );
        aContStm->SetBufferSize( 8192 );
#if defined( SOLARIS ) && defined( C40 )
     // patch to work around Sparc-Compiler bug with virtual multiple inheritance
     SvEmbeddedObjectRef xSvEmbeddedObjectRef = this;
     if( xSvEmbeddedObjectRef.Is() )
         xSvEmbeddedObjectRef->SaveContent( *aContStm, bOwner_ );
     else
#else
        SaveContent( *aContStm, bOwner_ );
#endif
        aContStm->SetBufferSize( 0 );
        return aContStm->GetError() == SVSTREAM_OK;
    }
    return FALSE;
}

void SvPersist::SaveContent( SvStream & rStm, BOOL bOwner_ )
{
    if( bOwner_ )
    {
        rStm << (BYTE)CHILD_LIST_VER;
        const SvInfoObjectMemberList * pList = GetObjectList();
        if( pList && pList->Count() )
        {
            rStm << (BOOL)TRUE;
            SvPersistStream aPStm( SOAPP->aInfoClassMgr, &rStm );
            aPStm << *pList;
        }
        else
            rStm << (BOOL)FALSE;
    }
    // nichts tun bei Fremd-Format
}

/*************************************************************************
|*    SvPersist::SaveChilds()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::SaveChilds()
{
    BOOL bRet = TRUE;
    if( pChildList )
    {
        for( ULONG i = 0; i < pChildList->Count(); i++ )
        {
            SvInfoObject * pEle = pChildList->GetObject( i );
            if( pEle->GetPersist() && !pEle->IsDeleted() )
            {
                if ( pEle->pImp->GetRealStorageName().Len() )
                {
                    // element is currently stored in an external storage
                    // save it into my storage
                    bRet = SaveElement( GetStorage(), pEle );
                }
                else if( pEle->GetPersist()->DoSave() )
                {
                    // always commit child objects
                    if( !pEle->GetPersist()->GetStorage()->Commit() )
                        bRet = FALSE;
                }
                else
                    bRet = FALSE;
            }

            pEle = pChildList->Next();
        }
    }
    return bRet;
}

BOOL SvPersist::SaveElement( SvStorage* pStor, SvInfoObject* pEle )
{
    BOOL bRet = TRUE;
    BOOL bLoad = FALSE;

    // create OLE substorages as default
    // only internal objects stored into XML format should be UCB substorages
    SvStorageRef aSrcEleStor = GetObjectStorage( pEle );
    if ( pEle->GetClassName() == SvGlobalName() )
        pEle->SetClassName( aSrcEleStor->GetClassName() );

    DBG_ASSERT( aSrcEleStor.Is(), "no object storage" );
    long nObjVersion = aSrcEleStor->GetVersion();

    // element must be saved as OLEStorage if it currently has an OLE storage or
    // the target storage is an OLEStorage
#ifdef DBG_UTIL
    BOOL bOLESubStorage = pStor->IsOLEStorage() || aSrcEleStor->IsOLEStorage();
#endif
    BOOL bIntern = SvFactory::IsIntern( aSrcEleStor->GetClassName(), &nObjVersion );
    if( nObjVersion >= SOFFICE_FILEFORMAT_60 )
    {
        ULONG nFormat = aSrcEleStor->GetFormat();
        switch( nFormat )
        {
        case SOT_FORMATSTR_ID_STARWRITER_8:
        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
        case SOT_FORMATSTR_ID_STARDRAW_8:
        case SOT_FORMATSTR_ID_STARIMPRESS_8:
        case SOT_FORMATSTR_ID_STARCALC_8:
        case SOT_FORMATSTR_ID_STARCHART_8:
        case SOT_FORMATSTR_ID_STARMATH_8:
            nObjVersion = SOFFICE_FILEFORMAT_8;
            break;
        default:
            break;
        }
    }

    bLoad = (nObjVersion != pStor->GetVersion() );
/*
    if( pStor->GetVersion() <= SOFFICE_FILEFORMAT_50 )
    {
        // If we store into the old binary format, objects
        // that have a newer file format version than the
        // destination storage have to be saved.
        bLoad = bIntern && nObjVersion > pStor->GetVersion();
    }
    else
    {
        // If we store into an XML format, all internal objects that
        // aren't stored in the XML format or that are stored in
        // a newer XML format have to be saved.
        bLoad = bIntern &&
                ( nObjVersion < SOFFICE_FILEFORMAT_60 ||
                    nObjVersion > pStor->GetVersion() );
    }
*/
    // load object if neccessary
    if( bLoad && !pEle->GetPersist() )
        CreateObjectFromStorage( pEle, aSrcEleStor );

    SvPseudoObjectRef xPO( pEle->GetPersist() );
    if ( !pStor->IsOLEStorage() && xPO.Is() && ( xPO->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) )
        // these object are saved into the document stream
        return TRUE;

    SvStorageRef aEleStor;
    if( !bIntern || pStor->IsOLEStorage() )
        aEleStor = pStor->OpenOLEStorage( pEle->GetStorageName() );
    else
        aEleStor = pStor->OpenUCBStorage( pEle->GetStorageName() );

    if( !aEleStor.Is() )
        return FALSE;

    aEleStor->SetVersion( pStor->GetVersion() );

    BOOL bSave = ( pEle->GetPersist() != 0 );
    if ( bSave && !bLoad )
    {
        bSave = pEle->GetPersist()->IsModified();
        if ( !bSave )
        {
            SvStorageInfoList aList;
            aSrcEleStor->FillInfoList( &aList );
            bSave = !aList.Count();
        }
    }

    if( bSave )
    {
        DBG_ASSERT( bOLESubStorage || ( xPO->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) == 0, "Trying to save special object" );
        bRet = pEle->GetPersist()->DoSaveAs( aEleStor );
    }
    else
    {
        bRet = aSrcEleStor->CopyTo( aEleStor );
        if ( pEle->GetPersist() )
            pEle->GetPersist()->bOpSaveAs = TRUE;
    }

    if( bRet )
    {
        // always commit child objects
        bRet = aEleStor->Commit();
    }

    return bRet;
}

/*************************************************************************
|*    SvPersist::SaveAsChilds()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::SaveAsChilds( SvStorage * pStor )
{
    BOOL bRet = TRUE;
    if( pChildList )
    {
        for( ULONG i=0; bRet && i<pChildList->Count(); i++ )
        {
            SvInfoObject * pEle = pChildList->GetObject( i );
            if ( !pEle->IsDeleted() )
                bRet = SaveElement( pStor, pEle );
        }
    }

    // for the 6.0 documents save all alien folders that are registered
    // in manifest.xml with nonempty mediatype

    if( GetStorage() && pStor
        && aStorage->GetVersion() >= SOFFICE_FILEFORMAT_60
        && pStor->GetVersion() >= SOFFICE_FILEFORMAT_60 )
    {
        SvStorageInfoList aChildren;
        aStorage->FillInfoList( &aChildren );

        for( ULONG i=0; bRet && i < aChildren.Count(); i++ )
        {
            SvStorageInfo& rEle = aChildren.GetObject( i );
            SvInfoObjectRef rObj;
            if( pChildList )
            {
                rObj = pChildList->First();
                while( rObj.Is() )
                {
                    if( rObj->GetStorageName() == rEle.GetName() )
                        break;
                    rObj = pChildList->Next();
                }
            }

            if( !rObj.Is() && rEle.IsStorage() )
            {
                ::com::sun::star::uno::Any aAny;
                ::rtl::OUString aMediaType;
                ::rtl::OUString aPropNameStr = ::rtl::OUString::createFromAscii( "MediaType" );

                GetStorage()->GetProperty( rEle.GetName(), aPropNameStr, aAny );
                if( ( aAny >>= aMediaType ) && aMediaType.getLength()
                    && aMediaType.compareToAscii( "application/vnd.sun.star.oleobject" ) != COMPARE_EQUAL
                    && rEle.GetClassName() == SvGlobalName() )
                {
                    SvStorageRef aEleStor = pStor->OpenUCBStorage( rEle.GetName() );
                    SvStorageRef aSrcEleStor = GetStorage()->OpenUCBStorage( rEle.GetName() );
                    bRet = aSrcEleStor->CopyTo( aEleStor );
                    if( bRet )
                    {
                        aEleStor->SetProperty( aPropNameStr, aAny );
                        bRet = aEleStor->Commit();
                    }
                }
            }
        }
    }

    return bRet;
}

/*************************************************************************
|*    SvPersist::SaveCompletedChilds()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvPersist::SaveCompletedChilds( SvStorage * pStor )
{
    BOOL bRet = TRUE;
    if( pChildList && pChildList->Count() )
    {
        for( ULONG i = 0; i < pChildList->Count(); i++ )
        {
            SvInfoObject * pEle = pChildList->GetObject( i );
            if( pEle->GetPersist() && !pEle->IsDeleted() )
            {
                ULONG nVersion = pStor ? pStor->GetVersion() : GetStorage()->GetVersion();
                SvEmbeddedObjectRef xEmb( pEle->GetPersist() );
                if ( xEmb.Is() && nVersion >= SOFFICE_FILEFORMAT_60 && ( xEmb->GetMiscStatus() & SVOBJ_MISCSTATUS_SPECIALOBJECT ) )
                {
                    xEmb->SetModified( FALSE );
                    continue;
                }

                if( pStor )
                {
                    SvStorageRef aEleStor;
                    aEleStor = pStor->OpenStorage( pEle->GetStorageName() );
                    if( !aEleStor.Is() || !pEle->GetPersist()->DoSaveCompleted( aEleStor ) )
                        return FALSE;

                    // the object now is definitely part of the container
                    pEle->pImp->SetRealStorageName( String() );
                }
                else
                {
                    // set objects on their old storage again
                    if( !pEle->GetPersist()->DoSaveCompleted() )
                        return FALSE;
                }
            }

            pEle = pChildList->Next();
        }
    }

    return bRet;
}

/*************************************************************************
|*    SvPersist::CleanUp()
|*
|*    Beschreibung
|*
|*    Loescht Storages, die im SvInfoObject dafuer geflagt wurden
*************************************************************************/

void SvPersist::CleanUp( BOOL bRecurse)
{
    if( pChildList && pChildList->Count() )
    {
        for(ULONG i=0;i<pChildList->Count();)
        {
            SvInfoObjectRef pEle = pChildList->GetObject(i);
            if( bRecurse )
            {
                SvPersistRef xPer = pEle->GetPersist();
                if(!xPer.Is())
                {
                    SvStorageRef aEleStor;
                    aEleStor = GetStorage()->OpenStorage( pEle->GetStorageName() );
                    if( !aEleStor.Is() )
                        continue;

                    xPer=new SvPersist;
                    xPer->DoOwnerLoad( aEleStor);
                    pEle->SetObj(xPer);
                    xPer->CleanUp();
                }
            }

            if( pEle->IsDeleted() )
            {
                DBG_ASSERT( pEle->GetRefCount()==2, "Loeschen von referenziertem Storage" );
                String aStorName(pEle->GetStorageName());
                Remove(pEle);
                GetStorage()->Remove(aStorName);
            }
            else
                i++;
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
