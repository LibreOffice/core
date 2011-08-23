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

#include "sb.hxx"
#include <tools/rcid.h>
#include <tools/config.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <sbx.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_RC_HXX //autogen
#include <tools/rc.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include "sbunoobj.hxx"
#include "sbjsmeth.hxx"
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "image.hxx"
/*?*/ //#include "disas.hxx"
/*?*/ //#include "runtime.hxx"
/*?*/ //#include "sbuno.hxx"
/*?*/ //#include "stdobj.hxx"
#include "filefmt.hxx"
/*?*/ //#include "sb.hrc"
/*?*/ //#include "basrid.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

namespace binfilter {

// #pragma SW_SEGMENT_CLASS( SBASIC, SBASIC_CODE )

TYPEINIT1(StarBASIC,SbxObject)

#define RTLNAME "@SBRTL"


//========================================================================
// Array zur Umrechnung SFX <-> VB-Fehlercodes anlegen

struct SFX_VB_ErrorItem
{
    USHORT  nErrorVB;
    SbError nErrorSFX;
};

const SFX_VB_ErrorItem __FAR_DATA SFX_VB_ErrorTab[] =
{
    { 1, SbERR_BASIC_EXCEPTION },  // #87844 Map exception to error code 1
    { 2, SbERR_SYNTAX },
    { 3, SbERR_NO_GOSUB },
    { 4, SbERR_REDO_FROM_START },
    { 5, SbERR_BAD_ARGUMENT },
    { 6, SbERR_MATH_OVERFLOW },
    { 7, SbERR_NO_MEMORY },
    { 8, SbERR_ALREADY_DIM },
    { 9, SbERR_OUT_OF_RANGE },
    { 10, SbERR_DUPLICATE_DEF },
    { 11, SbERR_ZERODIV },
    { 12, SbERR_VAR_UNDEFINED },
    { 13, SbERR_CONVERSION },
    { 14, SbERR_BAD_PARAMETER },
    { 18, SbERR_USER_ABORT },
    { 20, SbERR_BAD_RESUME },
    { 28, SbERR_STACK_OVERFLOW },
    { 35, SbERR_PROC_UNDEFINED },
    { 48, SbERR_BAD_DLL_LOAD },
    { 49, SbERR_BAD_DLL_CALL },
    { 51, SbERR_INTERNAL_ERROR },
    { 52, SbERR_BAD_CHANNEL },
    { 53, SbERR_FILE_NOT_FOUND },
    { 54, SbERR_BAD_FILE_MODE },
    { 55, SbERR_FILE_ALREADY_OPEN },
    { 57, SbERR_IO_ERROR },
    { 58, SbERR_FILE_EXISTS },
    { 59, SbERR_BAD_RECORD_LENGTH },
    { 61, SbERR_DISK_FULL },
    { 62, SbERR_READ_PAST_EOF },
    { 63, SbERR_BAD_RECORD_NUMBER },
    { 67, SbERR_TOO_MANY_FILES },
    { 68, SbERR_NO_DEVICE },
    { 70, SbERR_ACCESS_DENIED },
    { 71, SbERR_NOT_READY },
    { 73, SbERR_NOT_IMPLEMENTED },
    { 74, SbERR_DIFFERENT_DRIVE },
    { 75, SbERR_ACCESS_ERROR },
    { 76, SbERR_PATH_NOT_FOUND },
    { 91, SbERR_NO_OBJECT },
    { 93, SbERR_BAD_PATTERN },
    { 94, SBERR_IS_NULL },
    { 250, SbERR_DDE_ERROR },
    { 280, SbERR_DDE_WAITINGACK },
    { 281, SbERR_DDE_OUTOFCHANNELS },
    { 282, SbERR_DDE_NO_RESPONSE },
    { 283, SbERR_DDE_MULT_RESPONSES },
    { 284, SbERR_DDE_CHANNEL_LOCKED },
    { 285, SbERR_DDE_NOTPROCESSED },
    { 286, SbERR_DDE_TIMEOUT },
    { 287, SbERR_DDE_USER_INTERRUPT },
    { 288, SbERR_DDE_BUSY },
    { 289, SbERR_DDE_NO_DATA },
    { 290, SbERR_DDE_WRONG_DATA_FORMAT },
    { 291, SbERR_DDE_PARTNER_QUIT },
    { 292, SbERR_DDE_CONV_CLOSED },
    { 293, SbERR_DDE_NO_CHANNEL },
    { 294, SbERR_DDE_INVALID_LINK },
    { 295, SbERR_DDE_QUEUE_OVERFLOW },
    { 296, SbERR_DDE_LINK_ALREADY_EST },
    { 297, SbERR_DDE_LINK_INV_TOPIC },
    { 298, SbERR_DDE_DLL_NOT_FOUND },
    { 323, SbERR_CANNOT_LOAD },
    { 341, SbERR_BAD_INDEX },
    { 366, SbERR_NO_ACTIVE_OBJECT },
    { 380, SbERR_BAD_PROP_VALUE },
    { 382, SbERR_PROP_READONLY },
    { 394, SbERR_PROP_WRITEONLY },
    { 420, SbERR_INVALID_OBJECT },
    { 423, SbERR_NO_METHOD },
    { 424, SbERR_NEEDS_OBJECT },
    { 425, SbERR_INVALID_USAGE_OBJECT },
    { 430, SbERR_NO_OLE },
    { 438, SbERR_BAD_METHOD },
    { 440, SbERR_OLE_ERROR },
    { 445, SbERR_BAD_ACTION },
    { 446, SbERR_NO_NAMED_ARGS },
    { 447, SbERR_BAD_LOCALE },
    { 448, SbERR_NAMED_NOT_FOUND },
    { 449, SbERR_NOT_OPTIONAL },
    { 450, SbERR_WRONG_ARGS },
    { 451, SbERR_NOT_A_COLL },
    { 452, SbERR_BAD_ORDINAL },
    { 453, SbERR_DLLPROC_NOT_FOUND },
    { 460, SbERR_BAD_CLIPBD_FORMAT },
    { 951, SbERR_UNEXPECTED },
    { 952, SbERR_EXPECTED },
    { 953, SbERR_SYMBOL_EXPECTED },
    { 954, SbERR_VAR_EXPECTED },
    { 955, SbERR_LABEL_EXPECTED },
    { 956, SbERR_LVALUE_EXPECTED },
    { 957, SbERR_VAR_DEFINED },
    { 958, SbERR_PROC_DEFINED },
    { 959, SbERR_LABEL_DEFINED },
    { 960, SbERR_UNDEF_VAR },
    { 961, SbERR_UNDEF_ARRAY },
    { 962, SbERR_UNDEF_PROC },
    { 963, SbERR_UNDEF_LABEL },
    { 964, SbERR_UNDEF_TYPE },
    { 965, SbERR_BAD_EXIT },
    { 966, SbERR_BAD_BLOCK },
    { 967, SbERR_BAD_BRACKETS },
    { 968, SbERR_BAD_DECLARATION },
    { 969, SbERR_BAD_PARAMETERS },
    { 970, SbERR_BAD_CHAR_IN_NUMBER },
    { 971, SbERR_MUST_HAVE_DIMS },
    { 972, SbERR_NO_IF },
    { 973, SbERR_NOT_IN_SUBR },
    { 974, SbERR_NOT_IN_MAIN },
    { 975, SbERR_WRONG_DIMS },
    { 976, SbERR_BAD_OPTION },
    { 977, SbERR_CONSTANT_REDECLARED },
    { 978, SbERR_PROG_TOO_LARGE },
    { 979, SbERR_NO_STRINGS_ARRAYS },
    { 1000, SbERR_PROPERTY_NOT_FOUND },
    { 1001, SbERR_METHOD_NOT_FOUND },
    { 1002, SbERR_ARG_MISSING },
    { 1003, SbERR_BAD_NUMBER_OF_ARGS },
    { 1004, SbERR_METHOD_FAILED },
    { 1005, SbERR_SETPROP_FAILED },
    { 1006, SbERR_GETPROP_FAILED },
    { 0xFFFF, 0xFFFFFFFFL }		// End-Marke
};


////////////////////////////////////////////////////////////////////////////

// Die StarBASIC-Factory hat einen Hack. Wenn ein SbModule eingerichtet wird,
// wird der Pointer gespeichert und an nachfolgende SbProperties/SbMethods
// uebergeben. Dadurch wird die Modul-Relationship wiederhergestellt. Das
// klappt aber nur, wenn ein Modul geladen wird. Fuer getrennt geladene
// Properties kann es Probleme geben!

SbxBase* SbiFactory::Create( UINT16 nSbxId, UINT32 nCreator )
{
    if( nCreator ==  SBXCR_SBX )
    {
        String aEmpty;
        switch( nSbxId )
        {
            case SBXID_BASIC:
                return new StarBASIC( NULL );
            case SBXID_BASICMOD:
                return new SbModule( aEmpty );
            case SBXID_BASICPROP:
                return new SbProperty( aEmpty, SbxVARIANT, NULL );
            case SBXID_BASICMETHOD:
                return new SbMethod( aEmpty, SbxVARIANT, NULL );
            case SBXID_JSCRIPTMOD:
                return new SbJScriptModule( aEmpty );
            case SBXID_JSCRIPTMETH:
                return new SbJScriptMethod( aEmpty, SbxVARIANT, NULL );
        }
    }
    return NULL;
}

SbxObject* SbiFactory::CreateObject( const String& rClass )
{
    if( rClass.EqualsIgnoreCaseAscii( "StarBASIC" ) )
        return new StarBASIC( NULL );
    else
    if( rClass.EqualsIgnoreCaseAscii( "StarBASICModule" ) )
    {
        String aEmpty;
        return new SbModule( aEmpty );
    }
    else
    if( rClass.EqualsIgnoreCaseAscii( "Collection" ) )
    {
        String aCollectionName( RTL_CONSTASCII_USTRINGPARAM("Collection") );
        return new BasicCollection( aCollectionName );
    }
    else
        return NULL;
}


// Factory class to create OLE objects
class SbOLEFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

SbxBase* SbOLEFactory::Create( UINT16, UINT32 )
{
    // Not supported
    return NULL;
}

SbUnoObject* createOLEObject_Impl( const String& aType );	// sbunoobj.cxx

SbxObject* SbOLEFactory::CreateObject( const String& rClassName )
{
    SbxObject* pRet = createOLEObject_Impl( rClassName );
    return pRet;
}


// Factory class to create user defined objects (type command)
class SbTypeFactory : public SbxFactory
{
    SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj );

public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
    virtual SbxObject* CreateObject( const String& );
};

SbxBase* SbTypeFactory::Create( UINT16, UINT32 )
{
    // Not supported
    return NULL;
}

SbxObject* SbTypeFactory::cloneTypeObjectImpl( const SbxObject& rTypeObj )
{
    SbxObject* pRet = new SbxObject( rTypeObj );
    pRet->PutObject( pRet );

    // Copy the properties, not only the reference to them
    SbxArray* pProps = pRet->GetProperties();
    UINT32 nCount = pProps->Count32();
    for( UINT32 i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pVar = pProps->Get32( i );
        SbxProperty* pProp = PTR_CAST( SbxProperty, pVar );
        if( pProp )
        {
            SbxProperty* pNewProp = new SbxProperty( *pProp );
            pProps->PutDirect( pNewProp, i );
        }
    }
    return pRet;
}

SbxObject* SbTypeFactory::CreateObject( const String& rClassName )
{
    SbxObject* pRet = NULL;
    SbModule* pMod = pMOD;
    if( pMod )
    {
        const SbxObject* pObj = pMod->FindType( rClassName );
        if( pObj )
            pRet = cloneTypeObjectImpl( *pObj );
    }
    return pRet;
}

TYPEINIT1(SbClassModuleObject,SbModule)

SbClassModuleObject::SbClassModuleObject( SbModule* pClassModule )
    : SbModule( pClassModule->GetName() )
    , mpClassModule( pClassModule )
    , mbInitializeEventDone( false )
{
    aOUSource = pClassModule->aOUSource;
    aComment = pClassModule->aComment;
    pImage = pClassModule->pImage;
    
    SetClassName( pClassModule->GetName() );

    // Allow search only internally
    ResetFlag( SBX_GBLSEARCH );

    // Copy the methods from original class module
    SbxArray* pClassMethods = pClassModule->GetMethods();
    UINT32 nMethodCount = pClassMethods->Count32();
    UINT32 i;
    for( i = 0 ; i < nMethodCount ; i++ )
    {
        SbxVariable* pVar = pClassMethods->Get32( i );

        // Exclude SbIfaceMapperMethod to copy them in a second step
        SbIfaceMapperMethod* pIfaceMethod = PTR_CAST( SbIfaceMapperMethod, pVar );
        if( !pIfaceMethod )
        {
            SbMethod* pMethod = PTR_CAST(SbMethod, pVar );
            if( pMethod )
            {
                USHORT nFlags_ = pMethod->GetFlags();
                pMethod->SetFlag( SBX_NO_BROADCAST );
                SbMethod* pNewMethod = new SbMethod( *pMethod );
                pNewMethod->ResetFlag( SBX_NO_BROADCAST );
                pMethod->SetFlags( nFlags_ );
                pNewMethod->pMod = this;
                pNewMethod->SetParent( this );
                pMethods->PutDirect( pNewMethod, i );
                StartListening( pNewMethod->GetBroadcaster(), TRUE );
            }
        }
    }

    // Copy SbIfaceMapperMethod in a second step to ensure that 
    // the corresponding base methods have already been copied
    for( i = 0 ; i < nMethodCount ; i++ )
    {
        SbxVariable* pVar = pClassMethods->Get32( i );

        SbIfaceMapperMethod* pIfaceMethod = PTR_CAST( SbIfaceMapperMethod, pVar );
        if( pIfaceMethod )
        {
            SbMethod* pImplMethod = pIfaceMethod->getImplMethod();
            if( !pImplMethod )
            {
                DBG_ERROR( "No ImplMethod" );
                continue;
            }

            // Search for own copy of ImplMethod
            String aImplMethodName = pImplMethod->GetName();
            SbxVariable* p = pMethods->Find( aImplMethodName, SbxCLASS_METHOD );
            SbMethod* pImplMethodCopy = p ? PTR_CAST(SbMethod,p) : NULL;
            if( !pImplMethodCopy )
            {
                DBG_ERROR( "Found no ImplMethod copy" );
                continue;
            }
            SbIfaceMapperMethod* pNewIfaceMethod = 
                new SbIfaceMapperMethod( pIfaceMethod->GetName(), pImplMethodCopy );
            pMethods->PutDirect( pNewIfaceMethod, i );
        }
    }

    // Copy the properties from original class module
    SbxArray* pClassProps = pClassModule->GetProperties();
    UINT32 nPropertyCount = pClassProps->Count32();
    for( i = 0 ; i < nPropertyCount ; i++ )
    {
        SbxVariable* pVar = pClassProps->Get32( i );
        SbProcedureProperty* pProcedureProp = PTR_CAST( SbProcedureProperty, pVar );
        if( pProcedureProp )
        {
            USHORT nFlags_ = pProcedureProp->GetFlags();
            pProcedureProp->SetFlag( SBX_NO_BROADCAST );
            SbProcedureProperty* pNewProp = new SbProcedureProperty
                ( pProcedureProp->GetName(), pProcedureProp->GetType() );
                // ( pProcedureProp->GetName(), pProcedureProp->GetType(), this );
            pNewProp->ResetFlag( SBX_NO_BROADCAST );
            pProcedureProp->SetFlags( nFlags_ );
            pProps->PutDirect( pNewProp, i );
            StartListening( pNewProp->GetBroadcaster(), TRUE );
        }
        else
        {
            SbxProperty* pProp = PTR_CAST( SbxProperty, pVar );
            if( pProp )
            {
                USHORT nFlags_ = pProp->GetFlags();
                pProp->SetFlag( SBX_NO_BROADCAST );
                SbxProperty* pNewProp = new SbxProperty( *pProp );
                pNewProp->ResetFlag( SBX_NO_BROADCAST );
                pNewProp->SetParent( this );
                pProps->PutDirect( pNewProp, i );
                pProp->SetFlags( nFlags_ );
            }
        }
    }
}

SbClassModuleObject::~SbClassModuleObject()
{
    triggerTerminateEvent();

    // Must be deleted by base class dtor because this data 
    // is not owned by the SbClassModuleObject object
    pImage = NULL;
}

void SbClassModuleObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    bool bDone = false;

    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbProcedureProperty* pProcProperty = PTR_CAST( SbProcedureProperty, pVar );
        if( pProcProperty )
        {
            bDone = true;

            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                String aProcName;
                aProcName.AppendAscii( "Property Get " );
                aProcName += pProcProperty->GetName();

                SbxVariable* pMeth = Find( aProcName, SbxCLASS_METHOD );
                if( pMeth )
                {
                    SbxValues aVals;
                    aVals.eType = SbxVARIANT;
                    pMeth->Get( aVals );
                    pVar->Put( aVals );
                }
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                SbxVariable* pMeth = NULL;

                bool bSet = pProcProperty->isSet();
                if( bSet )
                {
                    pProcProperty->setSet( false );

                    String aProcName;
                    aProcName.AppendAscii( "Property Set " );
                    aProcName += pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxCLASS_METHOD );
                }
                if( !pMeth )	// Let
                {
                    String aProcName;
                    aProcName.AppendAscii( "Property Let " );
                    aProcName += pProcProperty->GetName();
                    pMeth = Find( aProcName, SbxCLASS_METHOD );
                }

                if( pMeth )
                {
                    // Setup parameters
                    SbxArrayRef xArray = new SbxArray;
                    xArray->Put( pMeth, 0 );	// Method as parameter 0
                    xArray->Put( pVar, 1 );
                    pMeth->SetParameters( xArray );

                    SbxValues aVals;
                    pMeth->Get( aVals );
                    pMeth->SetParameters( NULL );
                }
            }
        }
    }

    if( !bDone )
        SbModule::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
}

SbxVariable* SbClassModuleObject::Find( const XubString& rName, SbxClassType t )
{
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( pRes )
    {
        triggerInitializeEvent();

        SbIfaceMapperMethod* pIfaceMapperMethod = PTR_CAST(SbIfaceMapperMethod,pRes);
        if( pIfaceMapperMethod )
        {
            pRes = pIfaceMapperMethod->getImplMethod();
            pRes->SetFlag( SBX_EXTFOUND );
        }
    }
    return pRes;
}

void SbClassModuleObject::triggerInitializeEvent( void )
{
    static String aInitMethodName( RTL_CONSTASCII_USTRINGPARAM("Class_Initialize") );

    if( mbInitializeEventDone )
        return;

    mbInitializeEventDone = true;

    // Search method
    SbxVariable* pMeth = SbxObject::Find( aInitMethodName, SbxCLASS_METHOD );
    if( pMeth )
    {
        SbxValues aVals;
        pMeth->Get( aVals );
    }
}

void SbClassModuleObject::triggerTerminateEvent( void )
{
    static String aTermMethodName( RTL_CONSTASCII_USTRINGPARAM("Class_Terminate") );

    if( !mbInitializeEventDone || GetSbData()->bRunInit )
        return;

    // Search method
    SbxVariable* pMeth = SbxObject::Find( aTermMethodName, SbxCLASS_METHOD );
    if( pMeth )
    {
        SbxValues aVals;
        pMeth->Get( aVals );
    }
}


void SbClassData::clear( void )
{
    mxIfaces->Clear(); 
}

SbClassFactory::SbClassFactory( void )
{
    String aDummyName;
    xClassModules = new SbxObject( aDummyName );
}

SbClassFactory::~SbClassFactory()
{}

SbxBase* SbClassFactory::Create( UINT16, UINT32 )
{
    // Not supported
    return NULL;
}

SbxObject* SbClassFactory::CreateObject( const String& rClassName )
{
    SbxVariable* pVar = xClassModules->Find( rClassName, SbxCLASS_DONTCARE );
    SbxObject* pRet = NULL;
    if( pVar )
    {
        SbModule* pMod = (SbModule*)pVar;
        pRet = new SbClassModuleObject( pMod );
    }
    return pRet;
}

////////////////////////////////////////////////////////////////////////////

StarBASIC::StarBASIC( StarBASIC* p )
    : SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("StarBASIC") ) )
{
    SetParent( p );
    pLibInfo = NULL;
    bNoRtl = bBreak = FALSE;
    pModules = new SbxArray;

    if( !GetSbData()->nInst++ )
    {
        pSBFAC = new SbiFactory;
        AddFactory( pSBFAC );
        pUNOFAC = new SbUnoFactory;
        AddFactory( pUNOFAC );
        pTYPEFAC = new SbTypeFactory;
        AddFactory( pTYPEFAC );
        pCLASSFAC = new SbClassFactory;
        AddFactory( pCLASSFAC );
        pOLEFAC = new SbOLEFactory;
        AddFactory( pOLEFAC );
    }
//	pRtl = new SbiStdObject( String( RTL_CONSTASCII_USTRINGPARAM(RTLNAME) ), this );
    // Suche ueber StarBASIC ist immer global
    SetFlag( SBX_GBLSEARCH );
}

// #51727 SetModified ueberladen, damit der Modified-
// Zustand nicht an den Parent weitergegeben wird.
void StarBASIC::SetModified( BOOL b )
{
    SbxBase::SetModified( b );
}

//***

StarBASIC::~StarBASIC()
{
    if( !--GetSbData()->nInst )
    {
        RemoveFactory( pSBFAC );
        pSBFAC = NULL;
        RemoveFactory( pUNOFAC );
        pUNOFAC = NULL;
        RemoveFactory( pTYPEFAC );
        pTYPEFAC = NULL;
        RemoveFactory( pCLASSFAC );
        pCLASSFAC = NULL;
        RemoveFactory( pOLEFAC );
        pOLEFAC = NULL;
    }

    // #100326 Set Parent NULL in registered listeners
    if( xUnoListeners.Is() )
    {
        USHORT uCount = xUnoListeners->Count();
        for( USHORT i = 0 ; i < uCount ; i++ )
        {
            SbxVariable* pListenerObj = xUnoListeners->Get( i );
            pListenerObj->SetParent( NULL );
        }
        xUnoListeners = NULL;
    }
}

// operator new() wird hier versenkt, damit jeder eine Instanz
// anlegen kann, ohne neu zu bilden.

void* StarBASIC::operator new( size_t n )
{
    if( n < sizeof( StarBASIC ) )
    {
//		DBG_ASSERT( FALSE, "Warnung: inkompatibler BASIC-Stand!" );
        n = sizeof( StarBASIC );
    }
    return ::operator new( n );
}

void StarBASIC::operator delete( void* p )
{
    ::operator delete( p );
}

/**************************************************************************
*
*	Erzeugen/Verwalten von Modulen
*
**************************************************************************/

SbModule* StarBASIC::MakeModule32( const String& rName, const ::rtl::OUString& rSrc )
{
    SbModule* p = new SbModule( rName );
    p->SetSource32( rSrc );
    p->SetParent( this );
    pModules->Insert( p, pModules->Count() );
    SetModified( TRUE );
    return p;
}

void StarBASIC::Insert( SbxVariable* pVar )
{
    if( pVar->IsA( TYPE(SbModule) ) )
    {
        pModules->Insert( pVar, pModules->Count() );
        pVar->SetParent( this );
        StartListening( pVar->GetBroadcaster(), TRUE );
    }
    else
    {
        BOOL bWasModified = IsModified();
        SbxObject::Insert( pVar );
        if( !bWasModified && pVar->IsSet( SBX_DONTSTORE ) )
            SetModified( FALSE );
    }
}

void StarBASIC::Remove( SbxVariable* pVar )
{
    if( pVar->IsA( TYPE(SbModule) ) )
    {
        // #87540 Can be last reference!
        SbxVariableRef xVar = pVar;
        pModules->Remove( pVar );
        pVar->SetParent( 0 );
        EndListening( pVar->GetBroadcaster() );
    }
    else
        SbxObject::Remove( pVar );
}

void StarBASIC::Clear()
{
    while( pModules->Count() )
        pModules->Remove( pModules->Count() - 1 );
}

SbModule* StarBASIC::FindModule( const String& rName )
{
    for( USHORT i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = (SbModule*) pModules->Get( i );
        if( p->GetName().EqualsIgnoreCaseAscii( rName ) )
            return p;
    }
    return NULL;
}

// Init-Code aller Module ausfuehren (auch in inserteten Bibliotheken)
void StarBASIC::InitAllModules( StarBASIC* pBasicNotToInit )
{
    // Eigene Module initialisieren
/*?*/ // 	for ( USHORT nMod = 0; nMod < pModules->Count(); nMod++ )
/*?*/ // 	{
/*?*/ // 		SbModule* pModule = (SbModule*)pModules->Get( nMod );
/*?*/ // 		if(	!pModule->IsCompiled() )
/*?*/ // 			pModule->Compile();
/*?*/ // 		pModule->RunInit();
/*?*/ // 	}
    DBG_ERROR( "StarBASIC::InitAllModules: dead code!" );
    // Alle Objekte ueberpruefen, ob es sich um ein Basic handelt
    // Wenn ja, auch dort initialisieren
    for ( USHORT nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
        if( pBasic && pBasic != pBasicNotToInit )
            pBasic->InitAllModules();
    }
}

// #88329 Put modules back to not initialised state to
// force reinitialisation at next start
void StarBASIC::DeInitAllModules( void )
{
    // Eigene Module initialisieren
    for ( USHORT nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pModules->Get( nMod );
        if( pModule->pImage )
            pModule->pImage->bInit = false;
    }
    // Alle Objekte ueberpruefen, ob es sich um ein Basic handelt
    // Wenn ja, auch dort initialisieren
    for ( USHORT nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
        if( pBasic )
            pBasic->DeInitAllModules();
    }
}

// Diese Implementation sucht erst innerhalb der Runtime-Library, dann
// nach einem Element innerhalb eines Moduls. Dieses Element kann eine
// Public-Variable oder ein Entrypoint sein. Wenn nicht gefunden, wird,
// falls nach einer Methode gesucht wird und ein Modul mit dem angege-
// benen Namen gefunden wurde, der Entrypoint "Main" gesucht. Wenn das
// auch nicht klappt, laeuft die traditionelle Suche ueber Objekte an.

SbxVariable* StarBASIC::Find( const String& rName, SbxClassType t )
{
    static String aMainStr( RTL_CONSTASCII_USTRINGPARAM("Main") );

    SbxVariable* pRes = NULL;
    SbModule* pNamed = NULL;
    // "Extended" search in Runtime Lib
    // aber nur, wenn SbiRuntime nicht das Flag gesetzt hat
    if( !bNoRtl )
    {
        if( t == SbxCLASS_DONTCARE || t == SbxCLASS_OBJECT )
        {
            if( rName.EqualsIgnoreCaseAscii( RTLNAME ) )
                pRes = pRtl;
        }
/*?*/ //		if( !pRes )
/*?*/ //			pRes = ((SbiStdObject*) (SbxObject*) pRtl)->Find( rName, t );
        if( pRes )
            pRes->SetFlag( SBX_EXTFOUND );
    }
    // Module durchsuchen
    if( !pRes )
      for( USHORT i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = (SbModule*) pModules->Get( i );
        if( p->IsVisible() )
        {
            // Modul merken fuer Main()-Aufruf
            // oder stimmt etwa der Name ueberein?!?
            if( p->GetName().EqualsIgnoreCaseAscii( rName ) )
            {
                if( t == SbxCLASS_OBJECT || t == SbxCLASS_DONTCARE )
                {
                    pRes = p; break;
                }
                pNamed = p;
            }
            // Sonst testen, ob das Element vorhanden ist
            // GBLSEARCH-Flag rausnehmen (wg. Rekursion)
            USHORT nGblFlag = p->GetFlags() & SBX_GBLSEARCH;
            p->ResetFlag( SBX_GBLSEARCH );
            pRes = p->Find( rName, t );
            p->SetFlag( nGblFlag );
            if( pRes )
                break;
        }
    }
    if( !pRes && pNamed && ( t == SbxCLASS_METHOD || t == SbxCLASS_DONTCARE ) &&
        !pNamed->GetName().EqualsIgnoreCaseAscii( aMainStr ) )
            pRes = pNamed->Find( aMainStr, SbxCLASS_METHOD );
    if( !pRes )
        pRes = SbxObject::Find( rName, t );
    return pRes;
}

BOOL StarBASIC::Call( const String& rName, SbxArray* pParam )
{
    BOOL bRes = SbxObject::Call( rName, pParam );
    if( !bRes )
    {
        SbxError eErr = SbxBase::GetError();
        SbxBase::ResetError();
        if( eErr != SbxERR_OK )
            RTError( (SbError)eErr, 0, 0, 0 );
    }
    return bRes;
}

void StarBASIC::Stop()
{
/*?*/ //	SbiInstance* p = pINST;
/*?*/ //	while( p )
/*?*/ //	{
/*?*/ //		p->Stop();
/*?*/ //		p = p->pNext;
/*?*/ //	}
}

BOOL StarBASIC::IsRunning()
{
    return BOOL( pINST != NULL );
}

/**************************************************************************
*
*	Debugging und Fehlerbehandlung
*
**************************************************************************/

USHORT __EXPORT StarBASIC::BreakHdl()
{
    return (USHORT) ( aBreakHdl.IsSet()
        ? aBreakHdl.Call( this ) : SbDEBUG_CONTINUE );
}

// AB: 29.3.96
// Das Mapping zwischen alten und neuen Fehlercodes erfolgt, indem die Tabelle
// SFX_VB_ErrorTab[] durchsucht wird. Dies ist zwar nicht besonders performant,
// verbraucht aber viel weniger Speicher als entsprechende switch-Bloecke.
// Die Umrechnung von Fehlercodes muss nicht schnell sein, daher auch keine
// binaere Suche bei VB-Error -> SFX-Error.

// Neue Fehler-Codes auf alte, Sbx-Kompatible zurueckmappen
USHORT StarBASIC::GetVBErrorCode( SbError nError )
{
    USHORT nRet = 0;

    // Suchschleife
    const SFX_VB_ErrorItem* pErrItem;
    USHORT nIndex = 0;
    do
    {
        pErrItem = SFX_VB_ErrorTab + nIndex;
        if( pErrItem->nErrorSFX == nError )
        {
            nRet = pErrItem->nErrorVB;
            break;
        }
        nIndex++;
    }
    while( pErrItem->nErrorVB != 0xFFFF );		// bis End-Marke
    return nRet;
}

SbError	StarBASIC::GetSfxFromVBError( USHORT nError )
{
    SbError nRet = 0L;

    // Suchschleife
    const SFX_VB_ErrorItem* pErrItem;
    USHORT nIndex = 0;
    do
    {
        pErrItem = SFX_VB_ErrorTab + nIndex;
        if( pErrItem->nErrorVB == nError )
        {
            nRet = pErrItem->nErrorSFX;
            break;
        }
        else if( pErrItem->nErrorVB > nError )
            break;				// kann nicht mehr gefunden werden

        nIndex++;
    }
    while( pErrItem->nErrorVB != 0xFFFF );		// bis End-Marke
    return nRet;
}

// Error- / Break-Daten setzen
void StarBASIC::SetErrorData
( SbError nCode, USHORT nLine, USHORT nCol1, USHORT nCol2 )
{
    SbiGlobals& aGlobals = *GetSbData();
    aGlobals.nCode = nCode;
    aGlobals.nLine = nLine;
    aGlobals.nCol1 = nCol1;
    aGlobals.nCol2 = nCol2;
}

//----------------------------------------------------------------
// Hilfsklasse zum Zugriff auf String SubResourcen einer Resource.
// Quelle: sfx2\source\doc\docfile.cxx (TLX)
struct BasicStringList_Impl : private Resource
{
    ResId aResId;

    BasicStringList_Impl( ResId& rErrIdP,  USHORT nId)
        : Resource( rErrIdP ),aResId(nId,*rErrIdP.GetResMgr()){}
    ~BasicStringList_Impl() { FreeResource(); }

    String GetString(){ return String( aResId ); }
    BOOL IsErrorTextAvailable( void )
        { return IsAvailableRes(aResId.SetRT(RSC_STRING)); }
};
//----------------------------------------------------------------

// #60175 Flag, das bei Basic-Fehlern das Anziehen der SFX-Resourcen verhindert
static BOOL bStaticSuppressSfxResource = FALSE;

void StarBASIC::MakeErrorText( SbError nId, const String& /*aMsg*/ )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    if( bStaticSuppressSfxResource )
    {
        GetSbData()->aErrMsg = String( RTL_CONSTASCII_USTRINGPARAM("No resource: Error message not available") );
        return;
    }

    USHORT nOldID = GetVBErrorCode( nId );

    // Hilfsklasse instanzieren
/*?*/ //	BasicResId aId( RID_BASIC_START );
/*?*/ //	BasicStringList_Impl aMyStringList( aId, USHORT(nId & ERRCODE_RES_MASK) );

/*?*/ //	if( aMyStringList.IsErrorTextAvailable() )
/*?*/ ///*?*/ //	{
/*?*/ //		// Merge Message mit Zusatztext
/*?*/ //		String aMsg1 = aMyStringList.GetString();
/*?*/ //		// Argument-Platzhalter durch %s ersetzen
/*?*/ //		String aSrgStr( RTL_CONSTASCII_USTRINGPARAM("$(ARG1)") );
/*?*/ //		USHORT nResult = aMsg1.Search( aSrgStr );
/*?*/ //
/*?*/ //		if( nResult != STRING_NOTFOUND )
/*?*/ //		{
/*?*/ //			aMsg1.Erase( nResult, aSrgStr.Len() );
/*?*/ //			aMsg1.Insert( aMsg, nResult );
/*?*/ //		}
/*?*/ //		GetSbData()->aErrMsg = aMsg1;
/*?*/ //	}
/*?*/ //	else if( nOldID != 0 )
    if( nOldID != 0 )
    {
        String aStdMsg( RTL_CONSTASCII_USTRINGPARAM("error ") );
        aStdMsg += String::CreateFromInt32( nOldID);
        aStdMsg += String( RTL_CONSTASCII_USTRINGPARAM(": no error message available!") );
        GetSbData()->aErrMsg = aStdMsg;
    }
    else
        GetSbData()->aErrMsg = String::EmptyString();
}

BOOL StarBASIC::CError
    ( SbError code, const String& rMsg, USHORT l, USHORT c1, USHORT c2 )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    // Compiler-Fehler waehrend der Laufzeit -> Programm anhalten
    if( IsRunning() )
    {
        // #109018 Check if running Basic is affected
/*?*/ //		StarBASIC* pStartedBasic = pINST->GetBasic();
/*?*/ //		if( pStartedBasic != this )
/*?*/ //			return FALSE;

        Stop();
    }

    // Flag setzen, damit GlobalRunInit den Fehler mitbekommt
    GetSbData()->bGlobalInitErr = TRUE;

    // Fehlertext basteln
    MakeErrorText( code, rMsg );

    // Umsetzung des Codes fuer String-Transport in SFX-Error
    if( rMsg.Len() )
        code = (ULONG)*new StringErrorInfo( code, String(rMsg) );

    SetErrorData( code, l, c1, c2 );
    GetSbData()->bCompiler = TRUE;
    BOOL bRet;
    if( GetSbData()->aErrHdl.IsSet() )
        bRet = (BOOL) GetSbData()->aErrHdl.Call( this );
    else
        bRet = ErrorHdl();
    GetSbData()->bCompiler = FALSE;		// nur TRUE fuer Error-Handler
    return bRet;
}

BOOL StarBASIC::RTError
    ( SbError code, USHORT l, USHORT c1, USHORT c2 )
{
    return RTError( code, String(), l, c1, c2 );
}

BOOL StarBASIC::RTError( SbError code, const String& rMsg, USHORT l, USHORT c1, USHORT c2 )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    SbError c = code;
    if( (c & ERRCODE_CLASS_MASK) == ERRCODE_CLASS_COMPILER )
        c = 0;
    MakeErrorText( c, rMsg );

    // Umsetzung des Codes fuer String-Transport in SFX-Error
    if( rMsg.Len() )
        code = (ULONG)*new StringErrorInfo( code, String(rMsg) );

    SetErrorData( code, l, c1, c2 );
    if( GetSbData()->aErrHdl.IsSet() )
        return (BOOL) GetSbData()->aErrHdl.Call( this );
    else
        return ErrorHdl();
}

void StarBASIC::Error( SbError n )
{
    Error( n, String() );
}

void StarBASIC::Error( SbError /*n*/, const String& /*rMsg*/ )
{
/*?*/ //	if( pINST )
/*?*/ //		pINST->Error( n, rMsg );
}

void StarBASIC::FatalError( SbError /*n*/ )
{
/*?*/ //	if( pINST )
/*?*/ //		pINST->FatalError( n );
}

BOOL __EXPORT StarBASIC::ErrorHdl()
{
    return (BOOL) ( aErrorHdl.IsSet()
        ? aErrorHdl.Call( this ) : FALSE );
}

/**************************************************************************
*
*	Laden und Speichern
*
**************************************************************************/

BOOL StarBASIC::LoadData( SvStream& r, USHORT nVer )
{
    if( !SbxObject::LoadData( r, nVer ) )
        return FALSE;

    // #95459 Delete dialogs, otherwise endless recursion
    // in SbxVarable::GetType() if dialogs are accessed
    USHORT nObjCount = pObjs->Count();
    SbxVariable** ppDeleteTab = new SbxVariable*[ nObjCount ];
    USHORT nObj;

    for( nObj = 0 ; nObj < nObjCount ; nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST( StarBASIC, pVar );
        ppDeleteTab[nObj] = pBasic ? NULL : pVar;
    }
    for( nObj = 0 ; nObj < nObjCount ; nObj++ )
    {
        SbxVariable* pVar = ppDeleteTab[nObj];
        if( pVar )
            pObjs->Remove( pVar );
    }
    delete[] ppDeleteTab;

    UINT16 nMod;
    pModules->Clear();
    r >> nMod;
    for( USHORT i = 0; i < nMod; i++ )
    {
        SbModule* pMod = (SbModule*) SbxBase::Load( r );
        if( !pMod )
            return FALSE;
        else if( pMod->ISA(SbJScriptModule) )
        {
            // Ref zuweisen, damit pMod deleted wird
            SbModuleRef xRef = pMod;
        }
        else
        {
            pMod->SetParent( this );
            pModules->Put( pMod, i );
        }
    }
    // HACK fuer SFX-Mist!
    SbxVariable* p = Find( String( RTL_CONSTASCII_USTRINGPARAM("FALSE") ), SbxCLASS_PROPERTY );
    if( p )
        Remove( p );
    p = Find( String( RTL_CONSTASCII_USTRINGPARAM("TRUE") ), SbxCLASS_PROPERTY );
    if( p )
        Remove( p );
    // Ende des Hacks!
    // Suche ueber StarBASIC ist immer global
    DBG_ASSERT( IsSet( SBX_GBLSEARCH ), "Basic ohne GBLSEARCH geladen" );
    SetFlag( SBX_GBLSEARCH );
    return TRUE;
}

BOOL StarBASIC::StoreData( SvStream& r ) const
{
    if( !SbxObject::StoreData( r ) )
        return FALSE;
    r << (UINT16) pModules->Count();
    for( USHORT i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = (SbModule*) pModules->Get( i );
        if( !p->Store( r ) )
            return FALSE;
    }
    return TRUE;
}

//========================================================================
// #118116 Implementation Collection object

TYPEINIT1(BasicCollection,SbxObject)

static const char pCountStr[]	= "Count";
static const char pAddStr[]		= "Add";
static const char pItemStr[]	= "Item";
static const char pRemoveStr[]	= "Remove";
static USHORT nCountHash = 0, nAddHash, nItemHash, nRemoveHash;

BasicCollection::BasicCollection( const XubString& rClass )
             : SbxObject( rClass )
{
    if( !nCountHash )
    {
        nCountHash  = MakeHashCode( String::CreateFromAscii( pCountStr ) );
        nAddHash    = MakeHashCode( String::CreateFromAscii( pAddStr ) );
        nItemHash   = MakeHashCode( String::CreateFromAscii( pItemStr ) );
        nRemoveHash = MakeHashCode( String::CreateFromAscii( pRemoveStr ) );
    }
    Initialize();
}

BasicCollection::~BasicCollection()
{}

void BasicCollection::Clear()
{
    SbxObject::Clear();
    Initialize();
}

void BasicCollection::Initialize()
{
    xItemArray = new SbxArray();
    SetType( SbxOBJECT );
    SetFlag( SBX_FIXED );
    ResetFlag( SBX_WRITE );
    SbxVariable* p;
    p = Make( String::CreateFromAscii( pCountStr ), SbxCLASS_PROPERTY, SbxINTEGER );
    p->ResetFlag( SBX_WRITE );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( String::CreateFromAscii( pAddStr ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( String::CreateFromAscii( pItemStr ), SbxCLASS_METHOD, SbxVARIANT );
    p->SetFlag( SBX_DONTSTORE );
    p = Make( String::CreateFromAscii( pRemoveStr ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
}

SbxVariable* BasicCollection::Find( const XubString& rName, SbxClassType t )
{
    SbxVariable* pFind = SbxObject::Find( rName, t );
    return pFind;
}

void BasicCollection::SFX_NOTIFY( SfxBroadcaster& rCst, const TypeId& rId1,
                                const SfxHint& rHint, const TypeId& rId2 )
{
    const SbxHint* p = PTR_CAST(SbxHint,&rHint);
    if( p )
    {
        ULONG nId = p->GetId();
        BOOL bRead  = BOOL( nId == SBX_HINT_DATAWANTED );
        BOOL bWrite = BOOL( nId == SBX_HINT_DATACHANGED );
        SbxVariable* pVar = p->GetVar();
        SbxArray* pArg = pVar->GetParameters();
        if( bRead || bWrite )
        {
            XubString aVarName( pVar->GetName() );
            if( pVar->GetHashCode() == nCountHash
                  && aVarName.EqualsIgnoreCaseAscii( pCountStr ) )
                pVar->PutLong( xItemArray->Count32() );
            else if( pVar->GetHashCode() == nAddHash
                  && aVarName.EqualsIgnoreCaseAscii( pAddStr ) )
                CollAdd( pArg );
            else if( pVar->GetHashCode() == nItemHash
                  && aVarName.EqualsIgnoreCaseAscii( pItemStr ) )
                CollItem( pArg );
            else if( pVar->GetHashCode() == nRemoveHash
                  && aVarName.EqualsIgnoreCaseAscii( pRemoveStr ) )
                CollRemove( pArg );
            else
                SbxObject::SFX_NOTIFY( rCst, rId1, rHint, rId2 );
            return;
        }
    }
    SbxObject::SFX_NOTIFY( rCst, rId1, rHint, rId2 );
}

INT32 BasicCollection::implGetIndex( SbxVariable* pIndexVar )
{
    INT32 nIndex = -1;
    if( pIndexVar->GetType() == SbxSTRING )
        nIndex = implGetIndexForName( pIndexVar->GetString() );
    else
        nIndex = pIndexVar->GetLong() - 1;
    return nIndex;
}

INT32 BasicCollection::implGetIndexForName( const String& rName )
{
    INT32 nIndex = -1;
    INT32 nCount = xItemArray->Count32();
    INT32 nNameHash = MakeHashCode( rName );
    for( INT32 i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pVar = xItemArray->Get32( i );
        if( pVar->GetHashCode() == nNameHash &&
            pVar->GetName().EqualsIgnoreCaseAscii( rName ) )
        {
            nIndex = i;
            break;
        }
    }
    return nIndex;
}

void BasicCollection::CollAdd( SbxArray* pPar_ )
{
    USHORT nCount = pPar_->Count();
    if( nCount < 2 || nCount > 5 )
    {
        SetError( SbxERR_WRONG_ARGS );
        return;
    }

    SbxVariable* pItem = pPar_->Get(1);
    if( pItem )
    {
        int nNextIndex;
        if( nCount < 4 )
        {
            nNextIndex = xItemArray->Count();
        }
        else
        {
            SbxVariable* pBefore = pPar_->Get(3);
            if( nCount == 5 )
            {
                if( !pBefore->IsErr() )
                {
                    SetError( SbERR_BAD_ARGUMENT );
                    return;
                }
                SbxVariable* pAfter = pPar_->Get(4);
                INT32 nAfterIndex = implGetIndex( pAfter );
                if( nAfterIndex == -1 )
                {
                    SetError( SbERR_BAD_ARGUMENT );
                    return;
                }
                nNextIndex = nAfterIndex + 1;
            }
            else // if( nCount == 4 )
            {
                INT32 nBeforeIndex = implGetIndex( pBefore );
                if( nBeforeIndex == -1 )
                {
                    SetError( SbERR_BAD_ARGUMENT );
                    return;
                }
                nNextIndex = nBeforeIndex;
            }
        }

        SbxVariableRef pNewItem = new SbxVariable( *pItem );
        if( nCount >= 3 )
        {
            SbxVariable* pKey = pPar_->Get(2);
            if( !pKey->IsErr() )
            {
                if( pKey->GetType() != SbxSTRING )
                {
                    SetError( SbERR_BAD_ARGUMENT );
                    return;
                }
                String aKey = pKey->GetString();
                if( implGetIndexForName( aKey ) != -1 )
                {
                    SetError( SbERR_BAD_ARGUMENT );
                    return;
                }
                pNewItem->SetName( aKey );
            }
        }
        pNewItem->SetFlag( SBX_READWRITE );
        xItemArray->Insert32( pNewItem, nNextIndex );
    }
    else
    {
        SetError( SbERR_BAD_ARGUMENT );
        return;
    }
}

void BasicCollection::CollItem( SbxArray* pPar_ )
{
    if( pPar_->Count() != 2 )
    {
        SetError( SbxERR_WRONG_ARGS );
        return;
    }
    SbxVariable* pRes = NULL;
    SbxVariable* p = pPar_->Get( 1 );
    INT32 nIndex = implGetIndex( p );
    if( nIndex >= 0 && nIndex < (INT32)xItemArray->Count32() )
        pRes = xItemArray->Get32( nIndex );
    if( !pRes )
        SetError( SbxERR_BAD_INDEX );
    *(pPar_->Get(0)) = *pRes;
}

void BasicCollection::CollRemove( SbxArray* pPar_ )
{
    if( pPar_ == NULL || pPar_->Count() != 2 )
    {
        SetError( SbxERR_WRONG_ARGS );
        return;
    }

    SbxVariable* p = pPar_->Get( 1 );
    INT32 nIndex = implGetIndex( p );
    if( nIndex >= 0 && nIndex < (INT32)xItemArray->Count32() )
        xItemArray->Remove32( nIndex );
    else
        SetError( SbxERR_BAD_INDEX );
}

}
