/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sb.hxx"
#include <tools/rcid.h>
#include <tools/stream.hxx>
#include <tools/errinf.hxx>
#include <tools/solarmutex.hxx>
#include <basic/sbx.hxx>
#include <tools/rc.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include "image.hxx"
#include "sbunoobj.hxx"
#include "sbjsmeth.hxx"
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "runtime.hxx"
#include <basic/sbuno.hxx>
#include "sbobjmod.hxx"
#include "stdobj.hxx"
#include "filefmt.hxx"
#include "sb.hrc"
#include <basrid.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include "errobject.hxx"
#include <memory>
#include <unordered_map>

#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <svtools/miscopt.hxx>
using namespace ::com::sun::star::script;


#define RTLNAME "@SBRTL"
//  i#i68894#
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::lang::XMultiServiceFactory;



class DocBasicItem : public ::cppu::WeakImplHelper< util::XCloseListener >
{
public:
    explicit DocBasicItem( StarBASIC& rDocBasic );
    virtual ~DocBasicItem();

    inline const SbxObjectRef& getClassModules() const { return mxClassModules; }
    inline bool isDocClosed() const { return mbDocClosed; }

    void clearDependingVarsOnDelete( StarBASIC& rDeletedBasic );

    void startListening();
    void stopListening();

    void setDisposed( bool bDisposed )
    {
        mbDisposed = bDisposed;
    }

    virtual void SAL_CALL queryClosing( const lang::EventObject& rSource, sal_Bool bGetsOwnership ) throw (util::CloseVetoException, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyClosing( const lang::EventObject& rSource ) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const lang::EventObject& rSource ) throw (uno::RuntimeException, std::exception) override;

private:
    StarBASIC&      mrDocBasic;
    SbxObjectRef    mxClassModules;
    bool            mbDocClosed;
    bool            mbDisposed;
};



DocBasicItem::DocBasicItem( StarBASIC& rDocBasic ) :
    mrDocBasic( rDocBasic ),
    mxClassModules( new SbxObject( OUString() ) ),
    mbDocClosed( false ),
    mbDisposed( false )
{
}

DocBasicItem::~DocBasicItem()
{
    // tdf#90969 HACK: don't use SolarMutexGuard - there is a horrible global
    // map GaDocBasicItems holding instances, and these get deleted from exit
    // handlers, when the SolarMutex is already dead
    tools::SolarMutex::Acquire();

    try
    {
        stopListening();
        mxClassModules.Clear(); // release with SolarMutex locked
    }
    catch (...)
    {
        assert(false);
    }

    tools::SolarMutex::Release();
}

void DocBasicItem::clearDependingVarsOnDelete( StarBASIC& rDeletedBasic )
{
    mrDocBasic.implClearDependingVarsOnDelete( &rDeletedBasic );
}

void DocBasicItem::startListening()
{
    Any aThisComp;
    mrDocBasic.GetUNOConstant( "ThisComponent", aThisComp );
    Reference< util::XCloseBroadcaster > xCloseBC( aThisComp, UNO_QUERY );
    mbDisposed = !xCloseBC.is();
    if( xCloseBC.is() )
    {
        try { xCloseBC->addCloseListener( this ); } catch(const uno::Exception& ) {}
    }
}

void DocBasicItem::stopListening()
{
    if( mbDisposed ) return;
    mbDisposed = true;
    Any aThisComp;
    if (!mrDocBasic.GetUNOConstant("ThisComponent", aThisComp))
        return;

    Reference< util::XCloseBroadcaster > xCloseBC( aThisComp, UNO_QUERY );
    if( xCloseBC.is() )
    {
        try { xCloseBC->removeCloseListener( this ); } catch(const uno::Exception& ) {}
    }
}

void SAL_CALL DocBasicItem::queryClosing( const lang::EventObject& /*rSource*/, sal_Bool /*bGetsOwnership*/ ) throw (util::CloseVetoException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL DocBasicItem::notifyClosing( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException, std::exception)
{
    stopListening();
    mbDocClosed = true;
}

void SAL_CALL DocBasicItem::disposing( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException, std::exception)
{
    stopListening();
}



namespace {

typedef ::rtl::Reference< DocBasicItem > DocBasicItemRef;
typedef std::unordered_map< const StarBASIC *, DocBasicItemRef > DocBasicItemMap;

class GaDocBasicItems : public rtl::Static<DocBasicItemMap,GaDocBasicItems> {};

const DocBasicItem* lclFindDocBasicItem( const StarBASIC* pDocBasic )
{
    DocBasicItemMap::iterator it = GaDocBasicItems::get().find( pDocBasic );
    DocBasicItemMap::iterator end = GaDocBasicItems::get().end();
    return (it != end) ? it->second.get() : nullptr;
}

void lclInsertDocBasicItem( StarBASIC& rDocBasic )
{
    DocBasicItemRef& rxDocBasicItem = GaDocBasicItems::get()[ &rDocBasic ];
    rxDocBasicItem.set( new DocBasicItem( rDocBasic ) );
    rxDocBasicItem->startListening();
}

void lclRemoveDocBasicItem( StarBASIC& rDocBasic )
{
    DocBasicItemMap::iterator it = GaDocBasicItems::get().find( &rDocBasic );
    if( it != GaDocBasicItems::get().end() )
    {
        it->second->stopListening();
        GaDocBasicItems::get().erase( it );
    }
    DocBasicItemMap::iterator it_end = GaDocBasicItems::get().end();
    for( it = GaDocBasicItems::get().begin(); it != it_end; ++it )
    {
        it->second->clearDependingVarsOnDelete( rDocBasic );
    }
}

StarBASIC* lclGetDocBasicForModule( SbModule* pModule )
{
    StarBASIC* pRetBasic = nullptr;
    SbxObject* pCurParent = pModule;
    while( pCurParent->GetParent() != nullptr )
    {
        pCurParent = pCurParent->GetParent();
        StarBASIC* pDocBasic = dynamic_cast<StarBASIC*>( pCurParent  );
        if( pDocBasic != nullptr && pDocBasic->IsDocBasic() )
        {
            pRetBasic = pDocBasic;
            break;
        }
    }
    return pRetBasic;
}

} // namespace



SbxObject* StarBASIC::getVBAGlobals( )
{
    if ( !pVBAGlobals )
    {
        Any aThisDoc;
        if ( GetUNOConstant("ThisComponent", aThisDoc) )
        {
            Reference< XMultiServiceFactory > xDocFac( aThisDoc, UNO_QUERY );
            if ( xDocFac.is() )
            {
                try
                {
                    xDocFac->createInstance("ooo.vba.VBAGlobals");
                }
                catch(const Exception& )
                {
                    // Ignore
                }
            }
        }
        const OUString aVBAHook("VBAGlobals");
        pVBAGlobals = static_cast<SbUnoObject*>(Find( aVBAHook , SbxCLASS_DONTCARE ));
    }
    return pVBAGlobals;
}

//  i#i68894#
SbxVariable* StarBASIC::VBAFind( const OUString& rName, SbxClassType t )
{
    if( rName == "ThisComponent" )
    {
        return nullptr;
    }
    // rename to init globals
    if ( getVBAGlobals( ) )
    {
        return pVBAGlobals->Find( rName, t );
    }
    return nullptr;
}

// Create array for conversion SFX <-> VB error code
struct SFX_VB_ErrorItem
{
    sal_uInt16  nErrorVB;
    SbError nErrorSFX;
};

const SFX_VB_ErrorItem SFX_VB_ErrorTab[] =
{
    { 1, ERRCODE_BASIC_EXCEPTION },  // #87844 Map exception to error code 1
    { 2, ERRCODE_BASIC_SYNTAX },
    { 3, ERRCODE_BASIC_NO_GOSUB },
    { 4, ERRCODE_BASIC_REDO_FROM_START },
    { 5, ERRCODE_BASIC_BAD_ARGUMENT },
    { 6, ERRCODE_BASIC_MATH_OVERFLOW },
    { 7, ERRCODE_BASIC_NO_MEMORY },
    { 8, ERRCODE_BASIC_ALREADY_DIM },
    { 9, ERRCODE_BASIC_OUT_OF_RANGE },
    { 10, ERRCODE_BASIC_DUPLICATE_DEF },
    { 11, ERRCODE_BASIC_ZERODIV },
    { 12, ERRCODE_BASIC_VAR_UNDEFINED },
    { 13, ERRCODE_BASIC_CONVERSION },
    { 14, ERRCODE_BASIC_BAD_PARAMETER },
    { 18, ERRCODE_BASIC_USER_ABORT },
    { 20, ERRCODE_BASIC_BAD_RESUME },
    { 28, ERRCODE_BASIC_STACK_OVERFLOW },
    { 35, ERRCODE_BASIC_PROC_UNDEFINED },
    { 48, ERRCODE_BASIC_BAD_DLL_LOAD },
    { 49, ERRCODE_BASIC_BAD_DLL_CALL },
    { 51, ERRCODE_BASIC_INTERNAL_ERROR },
    { 52, ERRCODE_BASIC_BAD_CHANNEL },
    { 53, ERRCODE_BASIC_FILE_NOT_FOUND },
    { 54, ERRCODE_BASIC_BAD_FILE_MODE },
    { 55, ERRCODE_BASIC_FILE_ALREADY_OPEN },
    { 57, ERRCODE_BASIC_IO_ERROR },
    { 58, ERRCODE_BASIC_FILE_EXISTS },
    { 59, ERRCODE_BASIC_BAD_RECORD_LENGTH },
    { 61, ERRCODE_BASIC_DISK_FULL },
    { 62, ERRCODE_BASIC_READ_PAST_EOF },
    { 63, ERRCODE_BASIC_BAD_RECORD_NUMBER },
    { 67, ERRCODE_BASIC_TOO_MANY_FILES },
    { 68, ERRCODE_BASIC_NO_DEVICE },
    { 70, ERRCODE_BASIC_ACCESS_DENIED },
    { 71, ERRCODE_BASIC_NOT_READY },
    { 73, ERRCODE_BASIC_NOT_IMPLEMENTED },
    { 74, ERRCODE_BASIC_DIFFERENT_DRIVE },
    { 75, ERRCODE_BASIC_ACCESS_ERROR },
    { 76, ERRCODE_BASIC_PATH_NOT_FOUND },
    { 91, ERRCODE_BASIC_NO_OBJECT },
    { 93, ERRCODE_BASIC_BAD_PATTERN },
    { 94, ERRCODE_BASIC_IS_NULL },
    { 250, ERRCODE_BASIC_DDE_ERROR },
    { 280, ERRCODE_BASIC_DDE_WAITINGACK },
    { 281, ERRCODE_BASIC_DDE_OUTOFCHANNELS },
    { 282, ERRCODE_BASIC_DDE_NO_RESPONSE },
    { 283, ERRCODE_BASIC_DDE_MULT_RESPONSES },
    { 284, ERRCODE_BASIC_DDE_CHANNEL_LOCKED },
    { 285, ERRCODE_BASIC_DDE_NOTPROCESSED },
    { 286, ERRCODE_BASIC_DDE_TIMEOUT },
    { 287, ERRCODE_BASIC_DDE_USER_INTERRUPT },
    { 288, ERRCODE_BASIC_DDE_BUSY },
    { 289, ERRCODE_BASIC_DDE_NO_DATA },
    { 290, ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT },
    { 291, ERRCODE_BASIC_DDE_PARTNER_QUIT },
    { 292, ERRCODE_BASIC_DDE_CONV_CLOSED },
    { 293, ERRCODE_BASIC_DDE_NO_CHANNEL },
    { 294, ERRCODE_BASIC_DDE_INVALID_LINK },
    { 295, ERRCODE_BASIC_DDE_QUEUE_OVERFLOW },
    { 296, ERRCODE_BASIC_DDE_LINK_ALREADY_EST },
    { 297, ERRCODE_BASIC_DDE_LINK_INV_TOPIC },
    { 298, ERRCODE_BASIC_DDE_DLL_NOT_FOUND },
    { 323, ERRCODE_BASIC_CANNOT_LOAD },
    { 341, ERRCODE_BASIC_BAD_INDEX },
    { 366, ERRCODE_BASIC_NO_ACTIVE_OBJECT },
    { 380, ERRCODE_BASIC_BAD_PROP_VALUE },
    { 382, ERRCODE_BASIC_PROP_READONLY },
    { 394, ERRCODE_BASIC_PROP_WRITEONLY },
    { 420, ERRCODE_BASIC_INVALID_OBJECT },
    { 423, ERRCODE_BASIC_NO_METHOD },
    { 424, ERRCODE_BASIC_NEEDS_OBJECT },
    { 425, ERRCODE_BASIC_INVALID_USAGE_OBJECT },
    { 430, ERRCODE_BASIC_NO_OLE },
    { 438, ERRCODE_BASIC_BAD_METHOD },
    { 440, ERRCODE_BASIC_OLE_ERROR },
    { 445, ERRCODE_BASIC_BAD_ACTION },
    { 446, ERRCODE_BASIC_NO_NAMED_ARGS },
    { 447, ERRCODE_BASIC_BAD_LOCALE },
    { 448, ERRCODE_BASIC_NAMED_NOT_FOUND },
    { 449, ERRCODE_BASIC_NOT_OPTIONAL },
    { 450, ERRCODE_BASIC_WRONG_ARGS },
    { 451, ERRCODE_BASIC_NOT_A_COLL },
    { 452, ERRCODE_BASIC_BAD_ORDINAL },
    { 453, ERRCODE_BASIC_DLLPROC_NOT_FOUND },
    { 460, ERRCODE_BASIC_BAD_CLIPBD_FORMAT },
    { 951, ERRCODE_BASIC_UNEXPECTED },
    { 952, ERRCODE_BASIC_EXPECTED },
    { 953, ERRCODE_BASIC_SYMBOL_EXPECTED },
    { 954, ERRCODE_BASIC_VAR_EXPECTED },
    { 955, ERRCODE_BASIC_LABEL_EXPECTED },
    { 956, ERRCODE_BASIC_LVALUE_EXPECTED },
    { 957, ERRCODE_BASIC_VAR_DEFINED },
    { 958, ERRCODE_BASIC_PROC_DEFINED },
    { 959, ERRCODE_BASIC_LABEL_DEFINED },
    { 960, ERRCODE_BASIC_UNDEF_VAR },
    { 961, ERRCODE_BASIC_UNDEF_ARRAY },
    { 962, ERRCODE_BASIC_UNDEF_PROC },
    { 963, ERRCODE_BASIC_UNDEF_LABEL },
    { 964, ERRCODE_BASIC_UNDEF_TYPE },
    { 965, ERRCODE_BASIC_BAD_EXIT },
    { 966, ERRCODE_BASIC_BAD_BLOCK },
    { 967, ERRCODE_BASIC_BAD_BRACKETS },
    { 968, ERRCODE_BASIC_BAD_DECLARATION },
    { 969, ERRCODE_BASIC_BAD_PARAMETERS },
    { 970, ERRCODE_BASIC_BAD_CHAR_IN_NUMBER },
    { 971, ERRCODE_BASIC_MUST_HAVE_DIMS },
    { 972, ERRCODE_BASIC_NO_IF },
    { 973, ERRCODE_BASIC_NOT_IN_SUBR },
    { 974, ERRCODE_BASIC_NOT_IN_MAIN },
    { 975, ERRCODE_BASIC_WRONG_DIMS },
    { 976, ERRCODE_BASIC_BAD_OPTION },
    { 977, ERRCODE_BASIC_CONSTANT_REDECLARED },
    { 978, ERRCODE_BASIC_PROG_TOO_LARGE },
    { 979, ERRCODE_BASIC_NO_STRINGS_ARRAYS },
    { 1000, ERRCODE_BASIC_PROPERTY_NOT_FOUND },
    { 1001, ERRCODE_BASIC_METHOD_NOT_FOUND },
    { 1002, ERRCODE_BASIC_ARG_MISSING },
    { 1003, ERRCODE_BASIC_BAD_NUMBER_OF_ARGS },
    { 1004, ERRCODE_BASIC_METHOD_FAILED },
    { 1005, ERRCODE_BASIC_SETPROP_FAILED },
    { 1006, ERRCODE_BASIC_GETPROP_FAILED },
    { 1007, ERRCODE_BASIC_COMPAT },
    { 0xFFFF, 0xFFFFFFFFL }     // End mark
};

// The StarBASIC factory is a hack. When a SbModule is created, its pointer
// is saved and given to the following SbProperties/SbMethods. This restores
// the Modul-relationshop. But it works only when a modul is loaded.
// Can cause troubles with separately loaded properties!

SbxBase* SbiFactory::Create( sal_uInt16 nSbxId, sal_uInt32 nCreator )
{
    if( nCreator ==  SBXCR_SBX )
    {
        switch( nSbxId )
        {
        case SBXID_BASIC:
            return new StarBASIC( nullptr );
        case SBXID_BASICMOD:
            return new SbModule( "" );
        case SBXID_BASICPROP:
            return new SbProperty( "", SbxVARIANT, nullptr );
        case SBXID_BASICMETHOD:
            return new SbMethod( "", SbxVARIANT, nullptr );
        case SBXID_JSCRIPTMOD:
            return new SbJScriptModule( "" );
        case SBXID_JSCRIPTMETH:
            return new SbJScriptMethod( "", SbxVARIANT, nullptr );
        }
    }
    return nullptr;
}

SbxObject* SbiFactory::CreateObject( const OUString& rClass )
{
    if( rClass.equalsIgnoreAsciiCase( "StarBASIC" ) )
    {
        return new StarBASIC( nullptr );
    }
    else if( rClass.equalsIgnoreAsciiCase( "StarBASICModule" ) )
    {
        return new SbModule( OUString() );
    }
    else if( rClass.equalsIgnoreAsciiCase( "Collection" ) )
    {
        return new BasicCollection( OUString("Collection"));
    }
    else if( rClass.equalsIgnoreAsciiCase( "FileSystemObject" ) )
    {
        try
        {
            Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory(), UNO_SET_THROW );
            OUString aServiceName("ooo.vba.FileSystemObject");
            Reference< XInterface > xInterface( xFactory->createInstance( aServiceName ), UNO_SET_THROW );
            return new SbUnoObject( aServiceName, uno::makeAny( xInterface ) );
        }
        catch(const Exception& )
        {
        }
    }
    return nullptr;
}


// Factory class to create OLE objects
class SbOLEFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

SbxBase* SbOLEFactory::Create( sal_uInt16, sal_uInt32 )
{
    // Not supported
    return nullptr;
}

SbxObject* SbOLEFactory::CreateObject( const OUString& rClassName )
{
    SbxObject* pRet = createOLEObject_Impl( rClassName );
    return pRet;
}



// SbFormFactory, show user forms by: dim as new <user form name>

class SbFormFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

SbxBase* SbFormFactory::Create( sal_uInt16, sal_uInt32 )
{
    // Not supported
    return nullptr;
}

SbxObject* SbFormFactory::CreateObject( const OUString& rClassName )
{
    if( SbModule* pMod = GetSbData()->pMod )
    {
        if( SbxVariable* pVar = pMod->Find( rClassName, SbxCLASS_OBJECT ) )
        {
            if( SbUserFormModule* pFormModule = dynamic_cast<SbUserFormModule*>( pVar->GetObject() )  )
            {
                bool bInitState = pFormModule->getInitState();
                if( bInitState )
                {
                    // Not the first instantiate, reset
                    bool bTriggerTerminateEvent = false;
                    pFormModule->ResetApiObj( bTriggerTerminateEvent );
                    pFormModule->setInitState( false );
                }
                else
                {
                    pFormModule->Load();
                }
                return pFormModule->CreateInstance();
            }
        }
    }
    return nullptr;
}



// SbTypeFactory

SbxObject* cloneTypeObjectImpl( const SbxObject& rTypeObj )
{
    SbxObject* pRet = new SbxObject( rTypeObj );
    pRet->PutObject( pRet );

    // Copy the properties, not only the reference to them
    SbxArray* pProps = pRet->GetProperties();
    sal_uInt32 nCount = pProps->Count32();
    for( sal_uInt32 i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pVar = pProps->Get32( i );
        SbxProperty* pProp = dynamic_cast<SbxProperty*>( pVar  );
        if( pProp )
        {
            SbxProperty* pNewProp = new SbxProperty( *pProp );
            SbxDataType eVarType = pVar->GetType();
            if( eVarType & SbxARRAY )
            {
                SbxBase* pParObj = pVar->GetObject();
                SbxDimArray* pSource = dynamic_cast<SbxDimArray*>( pParObj );
                SbxDimArray* pDest = new SbxDimArray( pVar->GetType() );

                pDest->setHasFixedSize( pSource && pSource->hasFixedSize() );
                if ( pSource && pSource->GetDims() && pSource->hasFixedSize() )
                {
                    sal_Int32 lb = 0;
                    sal_Int32 ub = 0;
                    for ( sal_Int32 j = 1 ; j <= pSource->GetDims(); ++j )
                    {
                        pSource->GetDim32( (sal_Int32)j, lb, ub );
                        pDest->AddDim32( lb, ub );
                    }
                }
                else
                {
                    pDest->unoAddDim( 0, -1 ); // variant array
                }
                SbxFlagBits nSavFlags = pVar->GetFlags();
                pNewProp->ResetFlag( SbxFlagBits::Fixed );
                // need to reset the FIXED flag
                // when calling PutObject ( because the type will not match Object )
                pNewProp->PutObject( pDest );
                pNewProp->SetFlags( nSavFlags );
            }
            if( eVarType == SbxOBJECT )
            {
                SbxBase* pObjBase = pVar->GetObject();
                SbxObject* pSrcObj = dynamic_cast<SbxObject*>( pObjBase );
                SbxObject* pDestObj = nullptr;
                if( pSrcObj != nullptr )
                    pDestObj = cloneTypeObjectImpl( *pSrcObj );
                pNewProp->PutObject( pDestObj );
            }
            pProps->PutDirect( pNewProp, i );
        }
    }
    return pRet;
}

// Factory class to create user defined objects (type command)
class SbTypeFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX ) override;
    virtual SbxObject* CreateObject( const OUString& ) override;
};

SbxBase* SbTypeFactory::Create( sal_uInt16, sal_uInt32 )
{
    // Not supported
    return nullptr;
}

SbxObject* SbTypeFactory::CreateObject( const OUString& rClassName )
{
    SbxObject* pRet = nullptr;
    SbModule* pMod = GetSbData()->pMod;
    if( pMod )
    {
        const SbxObject* pObj = pMod->FindType( rClassName );
        if( pObj )
        {
            pRet = cloneTypeObjectImpl( *pObj );
        }
    }
    return pRet;
}

SbxObject* createUserTypeImpl( const OUString& rClassName )
{
    SbxObject* pRetObj = GetSbData()->pTypeFac->CreateObject( rClassName );
    return pRetObj;
}



SbClassModuleObject::SbClassModuleObject( SbModule* pClassModule )
    : SbModule( pClassModule->GetName() )
    , mpClassModule( pClassModule )
    , mbInitializeEventDone( false )
{
    aOUSource = pClassModule->aOUSource;
    aComment = pClassModule->aComment;
    pImage = pClassModule->pImage;
    pBreaks = pClassModule->pBreaks;

    SetClassName( pClassModule->GetName() );

    // Allow search only internally
    ResetFlag( SbxFlagBits::GlobalSearch );

    // Copy the methods from original class module
    SbxArray* pClassMethods = pClassModule->GetMethods();
    sal_uInt32 nMethodCount = pClassMethods->Count32();
    sal_uInt32 i;
    for( i = 0 ; i < nMethodCount ; i++ )
    {
        SbxVariable* pVar = pClassMethods->Get32( i );

        // Exclude SbIfaceMapperMethod to copy them in a second step
        SbIfaceMapperMethod* pIfaceMethod = dynamic_cast<SbIfaceMapperMethod*>( pVar  );
        if( !pIfaceMethod )
        {
            SbMethod* pMethod = dynamic_cast<SbMethod*>( pVar  );
            if( pMethod )
            {
                SbxFlagBits nFlags_ = pMethod->GetFlags();
                pMethod->SetFlag( SbxFlagBits::NoBroadcast );
                SbMethod* pNewMethod = new SbMethod( *pMethod );
                pNewMethod->ResetFlag( SbxFlagBits::NoBroadcast );
                pMethod->SetFlags( nFlags_ );
                pNewMethod->pMod = this;
                pNewMethod->SetParent( this );
                pMethods->PutDirect( pNewMethod, i );
                StartListening( pNewMethod->GetBroadcaster(), true );
            }
        }
    }

    // Copy SbIfaceMapperMethod in a second step to ensure that
    // the corresponding base methods have already been copied
    for( i = 0 ; i < nMethodCount ; i++ )
    {
        SbxVariable* pVar = pClassMethods->Get32( i );

        SbIfaceMapperMethod* pIfaceMethod = dynamic_cast<SbIfaceMapperMethod*>( pVar  );
        if( pIfaceMethod )
        {
            SbMethod* pImplMethod = pIfaceMethod->getImplMethod();
            if( !pImplMethod )
            {
                OSL_FAIL( "No ImplMethod" );
                continue;
            }

            // Search for own copy of ImplMethod
            SbxVariable* p = pMethods->Find( pImplMethod->GetName(), SbxCLASS_METHOD );
            SbMethod* pImplMethodCopy = p ? dynamic_cast<SbMethod*>( p ) : nullptr;
            if( !pImplMethodCopy )
            {
                OSL_FAIL( "Found no ImplMethod copy" );
                continue;
            }
            SbIfaceMapperMethod* pNewIfaceMethod =
                new SbIfaceMapperMethod( pIfaceMethod->GetName(), pImplMethodCopy );
            pMethods->PutDirect( pNewIfaceMethod, i );
        }
    }

    // Copy the properties from original class module
    SbxArray* pClassProps = pClassModule->GetProperties();
    sal_uInt32 nPropertyCount = pClassProps->Count32();
    for( i = 0 ; i < nPropertyCount ; i++ )
    {
        SbxVariable* pVar = pClassProps->Get32( i );
        SbProcedureProperty* pProcedureProp = dynamic_cast<SbProcedureProperty*>( pVar  );
        if( pProcedureProp )
        {
            SbxFlagBits nFlags_ = pProcedureProp->GetFlags();
            pProcedureProp->SetFlag( SbxFlagBits::NoBroadcast );
            SbProcedureProperty* pNewProp = new SbProcedureProperty
                ( pProcedureProp->GetName(), pProcedureProp->GetType() );
            pNewProp->SetFlags( nFlags_ ); // Copy flags
            pNewProp->ResetFlag( SbxFlagBits::NoBroadcast ); // except the Broadcast if it was set
            pProcedureProp->SetFlags( nFlags_ );
            pProps->PutDirect( pNewProp, i );
            StartListening( pNewProp->GetBroadcaster(), true );
        }
        else
        {
            SbxProperty* pProp = dynamic_cast<SbxProperty*>( pVar  );
            if( pProp )
            {
                SbxFlagBits nFlags_ = pProp->GetFlags();
                pProp->SetFlag( SbxFlagBits::NoBroadcast );
                SbxProperty* pNewProp = new SbxProperty( *pProp );

                // Special handling for modules instances and collections, they need
                // to be instantiated, otherwise all refer to the same base object
                SbxDataType eVarType = pProp->GetType();
                if( eVarType == SbxOBJECT )
                {
                    SbxBase* pObjBase = pProp->GetObject();
                    SbxObject* pObj = dynamic_cast<SbxObject*>( pObjBase );
                    if( pObj != nullptr )
                    {
                        OUString aObjClass = pObj->GetClassName();

                        SbClassModuleObject* pClassModuleObj = dynamic_cast<SbClassModuleObject*>( pObjBase );
                        if( pClassModuleObj != nullptr )
                        {
                            SbModule* pLclClassModule = pClassModuleObj->getClassModule();
                            SbClassModuleObject* pNewObj = new SbClassModuleObject( pLclClassModule );
                            pNewObj->SetName( pProp->GetName() );
                            pNewObj->SetParent( pLclClassModule->pParent );
                            pNewProp->PutObject( pNewObj );
                        }
                        else if( aObjClass.equalsIgnoreAsciiCase( "Collection" ) )
                        {
                            OUString aCollectionName("Collection");
                            BasicCollection* pNewCollection = new BasicCollection( aCollectionName );
                            pNewCollection->SetName( pProp->GetName() );
                            pNewCollection->SetParent( pClassModule->pParent );
                            pNewProp->PutObject( pNewCollection );
                        }
                    }
                }

                pNewProp->ResetFlag( SbxFlagBits::NoBroadcast );
                pNewProp->SetParent( this );
                pProps->PutDirect( pNewProp, i );
                pProp->SetFlags( nFlags_ );
            }
        }
    }
    SetModuleType( ModuleType::CLASS );
    mbVBACompat = pClassModule->mbVBACompat;
}

SbClassModuleObject::~SbClassModuleObject()
{
    // do not trigger termination event when document is already closed
    if( StarBASIC::IsRunning() )
        if( StarBASIC* pDocBasic = lclGetDocBasicForModule( this ) )
            if( const DocBasicItem* pDocBasicItem = lclFindDocBasicItem( pDocBasic ) )
                if( !pDocBasicItem->isDocClosed() )
                    triggerTerminateEvent();

    // Must be deleted by base class dtor because this data
    // is not owned by the SbClassModuleObject object
    pImage = nullptr;
    pBreaks = nullptr;
}

void SbClassModuleObject::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    handleProcedureProperties( rBC, rHint );
}

SbxVariable* SbClassModuleObject::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( pRes )
    {
        triggerInitializeEvent();

        SbIfaceMapperMethod* pIfaceMapperMethod = dynamic_cast<SbIfaceMapperMethod*>( pRes );
        if( pIfaceMapperMethod )
        {
            pRes = pIfaceMapperMethod->getImplMethod();
            pRes->SetFlag( SbxFlagBits::ExtFound );
        }
    }
    return pRes;
}

void SbClassModuleObject::triggerInitializeEvent()
{
    if( mbInitializeEventDone )
    {
        return;
    }

    mbInitializeEventDone = true;

    // Search method
    SbxVariable* pMeth = SbxObject::Find("Class_Initialize", SbxCLASS_METHOD);
    if( pMeth )
    {
        SbxValues aVals;
        pMeth->Get( aVals );
    }
}

void SbClassModuleObject::triggerTerminateEvent()
{
    if( !mbInitializeEventDone || GetSbData()->bRunInit )
    {
        return;
    }
    // Search method
    SbxVariable* pMeth = SbxObject::Find("Class_Terminate", SbxCLASS_METHOD );
    if( pMeth )
    {
        SbxValues aVals;
        pMeth->Get( aVals );
    }
}


SbClassData::SbClassData()
{
    mxIfaces = new SbxArray();
}

void SbClassData::clear()
{
    mxIfaces->Clear();
    maRequiredTypes.clear();
}

SbClassFactory::SbClassFactory()
{
    OUString aDummyName;
    xClassModules = new SbxObject( aDummyName );
}

SbClassFactory::~SbClassFactory()
{}

void SbClassFactory::AddClassModule( SbModule* pClassModule )
{
    SbxObjectRef xToUseClassModules = xClassModules;

    if( StarBASIC* pDocBasic = lclGetDocBasicForModule( pClassModule ) )
        if( const DocBasicItem* pDocBasicItem = lclFindDocBasicItem( pDocBasic ) )
            xToUseClassModules = pDocBasicItem->getClassModules();

    SbxObject* pParent = pClassModule->GetParent();
    xToUseClassModules->Insert( pClassModule );
    pClassModule->SetParent( pParent );
}

void SbClassFactory::RemoveClassModule( SbModule* pClassModule )
{
    xClassModules->Remove( pClassModule );
}

SbxBase* SbClassFactory::Create( sal_uInt16, sal_uInt32 )
{
    // Not supported
    return nullptr;
}

SbxObject* SbClassFactory::CreateObject( const OUString& rClassName )
{
    SbxObjectRef xToUseClassModules = xClassModules;

    if( SbModule* pMod = GetSbData()->pMod )
    {
        if( StarBASIC* pDocBasic = lclGetDocBasicForModule( pMod ) )
        {
            if( const DocBasicItem* pDocBasicItem = lclFindDocBasicItem( pDocBasic ) )
            {
                xToUseClassModules = pDocBasicItem->getClassModules();
            }
        }
    }
    SbxVariable* pVar = xToUseClassModules->Find( rClassName, SbxCLASS_OBJECT );
    SbxObject* pRet = nullptr;
    if( pVar )
    {
        SbModule* pVarMod = static_cast<SbModule*>(pVar);
        pRet = new SbClassModuleObject( pVarMod );
    }
    return pRet;
}

SbModule* SbClassFactory::FindClass( const OUString& rClassName )
{
    SbxVariable* pVar = xClassModules->Find( rClassName, SbxCLASS_DONTCARE );
    SbModule* pMod = pVar ? static_cast<SbModule*>(pVar) : nullptr;
    return pMod;
}

StarBASIC::StarBASIC( StarBASIC* p, bool bIsDocBasic  )
    : SbxObject( OUString("StarBASIC") ), bDocBasic( bIsDocBasic )
{
    SetParent( p );
    pLibInfo = nullptr;
    bNoRtl = bBreak = false;
    bVBAEnabled = false;
    pModules = new SbxArray;

    if( !GetSbData()->nInst++ )
    {
        GetSbData()->pSbFac = new SbiFactory;
        AddFactory( GetSbData()->pSbFac );
        GetSbData()->pTypeFac = new SbTypeFactory;
        AddFactory( GetSbData()->pTypeFac );
        GetSbData()->pClassFac = new SbClassFactory;
        AddFactory( GetSbData()->pClassFac );
        GetSbData()->pOLEFac = new SbOLEFactory;
        AddFactory( GetSbData()->pOLEFac );
        GetSbData()->pFormFac = new SbFormFactory;
        AddFactory( GetSbData()->pFormFac );
        GetSbData()->pUnoFac = new SbUnoFactory;
        AddFactory( GetSbData()->pUnoFac );
    }
    pRtl = new SbiStdObject(OUString(RTLNAME), this );
    // Search via StarBasic is always global
    SetFlag( SbxFlagBits::GlobalSearch );
    pVBAGlobals = nullptr;
    bQuit = false;

    if( bDocBasic )
    {
        lclInsertDocBasicItem( *this );
    }
}

// #51727 Override SetModified so that the modified state
// is not given to the parent
void StarBASIC::SetModified( bool b )
{
    SbxBase::SetModified( b );
}

StarBASIC::~StarBASIC()
{
    // Needs to be first action as it can trigger events
    disposeComVariablesForBasic( this );

    if( !--GetSbData()->nInst )
    {
        RemoveFactory( GetSbData()->pSbFac );
        delete GetSbData()->pSbFac; GetSbData()->pSbFac = nullptr;
        RemoveFactory( GetSbData()->pUnoFac );
        delete GetSbData()->pUnoFac; GetSbData()->pUnoFac = nullptr;
        RemoveFactory( GetSbData()->pTypeFac );
        delete GetSbData()->pTypeFac; GetSbData()->pTypeFac = nullptr;
        RemoveFactory( GetSbData()->pClassFac );
        delete GetSbData()->pClassFac; GetSbData()->pClassFac = nullptr;
        RemoveFactory( GetSbData()->pOLEFac );
        delete GetSbData()->pOLEFac; GetSbData()->pOLEFac = nullptr;
        RemoveFactory( GetSbData()->pFormFac );
        delete GetSbData()->pFormFac; GetSbData()->pFormFac = nullptr;

        if( SbiGlobals::pGlobals )
        {
            delete SbiGlobals::pGlobals;
            SbiGlobals::pGlobals = nullptr;
        }
    }
    else if( bDocBasic )
    {
        SbxError eOld = SbxBase::GetError();

        lclRemoveDocBasicItem( *this );

        SbxBase::ResetError();
        if( eOld != ERRCODE_SBX_OK )
        {
            SbxBase::SetError( eOld );
        }
    }

    // #100326 Set Parent NULL in registered listeners
    if( xUnoListeners.Is() )
    {
        sal_uInt16 uCount = xUnoListeners->Count();
        for( sal_uInt16 i = 0 ; i < uCount ; i++ )
        {
            SbxVariable* pListenerObj = xUnoListeners->Get( i );
            pListenerObj->SetParent( nullptr );
        }
        xUnoListeners = nullptr;
    }

    clearUnoMethodsForBasic( this );
}

void StarBASIC::implClearDependingVarsOnDelete( StarBASIC* pDeletedBasic )
{
    if( this != pDeletedBasic )
    {
        for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
        {
            SbModule* p = static_cast<SbModule*>(pModules->Get( i ));
            p->ClearVarsDependingOnDeletedBasic( pDeletedBasic );
        }
    }

    for( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = dynamic_cast<StarBASIC*>( pVar );
        if( pBasic && pBasic != pDeletedBasic )
        {
            pBasic->implClearDependingVarsOnDelete( pDeletedBasic );
        }
    }
}


/**************************************************************************
*
*    Creation/Management of modules
*
**************************************************************************/

SbModule* StarBASIC::MakeModule( const OUString& rName, const OUString& rSrc )
{
    return MakeModule32( rName, rSrc );
}

SbModule* StarBASIC::MakeModule32( const OUString& rName, const OUString& rSrc )
{
    ModuleInfo mInfo;
    mInfo.ModuleType = ModuleType::NORMAL;
    return MakeModule32(  rName, mInfo, rSrc );
}
SbModule* StarBASIC::MakeModule32( const OUString& rName, const ModuleInfo& mInfo, const OUString& rSrc )
{

    SAL_INFO(
        "basic",
        "create module " << rName  << " type mInfo " << mInfo.ModuleType);
    SbModule* p = nullptr;
    switch ( mInfo.ModuleType )
    {
    case ModuleType::DOCUMENT:
        // In theory we should be able to create Object modules
        // in ordinary basic ( in vba mode thought these are create
        // by the application/basic and not by the user )
        p = new SbObjModule( rName, mInfo, isVBAEnabled() );
        break;
    case ModuleType::CLASS:
        p = new SbModule( rName, isVBAEnabled() );
        p->SetModuleType( ModuleType::CLASS );
        break;
    case ModuleType::FORM:
        p = new SbUserFormModule( rName, mInfo, isVBAEnabled() );
        break;
    default:
        p = new SbModule( rName, isVBAEnabled() );
        break;
    }
    p->SetSource32( rSrc );
    p->SetParent( this );
    pModules->Insert( p, pModules->Count() );
    SetModified( true );
    return p;
}

void StarBASIC::Insert( SbxVariable* pVar )
{
    if( dynamic_cast<const SbModule*>(pVar) != nullptr)
    {
        pModules->Insert( pVar, pModules->Count() );
        pVar->SetParent( this );
        StartListening( pVar->GetBroadcaster(), true );
    }
    else
    {
        bool bWasModified = IsModified();
        SbxObject::Insert( pVar );
        if( !bWasModified && pVar->IsSet( SbxFlagBits::DontStore ) )
        {
            SetModified( false );
        }
    }
}

void StarBASIC::Remove( SbxVariable* pVar )
{
    if( dynamic_cast<const SbModule*>(pVar) != nullptr)
    {
        // #87540 Can be last reference!
        SbxVariableRef xVar = pVar;
        pModules->Remove( pVar );
        pVar->SetParent( nullptr );
        EndListening( pVar->GetBroadcaster() );
    }
    else
    {
        SbxObject::Remove( pVar );
    }
}

bool StarBASIC::Compile( SbModule* pMod )
{
    return pMod && pMod->Compile();
}

void StarBASIC::Clear()
{
    while( pModules->Count() )
    {
        pModules->Remove( pModules->Count() - 1 );
    }
}

SbModule* StarBASIC::FindModule( const OUString& rName )
{
    for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = static_cast<SbModule*>( pModules->Get( i ) );
        if( p->GetName().equalsIgnoreAsciiCase( rName ) )
        {
            return p;
        }
    }
    return nullptr;
}


struct ClassModuleRunInitItem
{
    SbModule*       m_pModule;
    bool            m_bProcessing;
    bool            m_bRunInitDone;

    ClassModuleRunInitItem()
        : m_pModule( nullptr )
        , m_bProcessing( false )
        , m_bRunInitDone( false )
    {}
    explicit ClassModuleRunInitItem( SbModule* pModule )
        : m_pModule( pModule )
        , m_bProcessing( false )
        , m_bRunInitDone( false )
    {}
};

// Derive from unordered_map type instead of typedef
// to allow forward declaration in sbmod.hxx
class ModuleInitDependencyMap : public
    std::unordered_map< OUString, ClassModuleRunInitItem,
                          OUStringHash, ::std::equal_to< OUString > >
{};

void SbModule::implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem )
{
    rItem.m_bProcessing = true;

    SbModule* pModule = rItem.m_pModule;
    if( pModule->pClassData != nullptr )
    {
        StringVector& rReqTypes = pModule->pClassData->maRequiredTypes;
        if( rReqTypes.size() > 0 )
        {
            for( const auto& rStr : rReqTypes )
            {
                // Is required type a class module?
                ModuleInitDependencyMap::iterator itFind = rMap.find( rStr );
                if( itFind != rMap.end() )
                {
                    ClassModuleRunInitItem& rParentItem = itFind->second;
                    if( rParentItem.m_bProcessing )
                    {
                        // TODO: raise error?
                        OSL_FAIL( "Cyclic module dependency detected" );
                        continue;
                    }

                    if( !rParentItem.m_bRunInitDone )
                    {
                        implProcessModuleRunInit( rMap, rParentItem );
                    }
                }
            }
        }
    }

    pModule->RunInit();
    rItem.m_bRunInitDone = true;
    rItem.m_bProcessing = false;
}

// Run Init-Code of all modules (including inserted libraries)
void StarBASIC::InitAllModules( StarBASIC* pBasicNotToInit )
{
    SolarMutexGuard guard;

    // Init own modules
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = static_cast<SbModule*>( pModules->Get( nMod ) );
        if( !pModule->IsCompiled() )
        {
            pModule->Compile();
        }
    }
    // compile modules first then RunInit ( otherwise there is
    // can be order dependency, e.g. classmodule A has a member
    // of type classmodule B and classmodule B hasn't been compiled yet )

    // Consider required types to init in right order. Class modules
    // that are required by other modules have to be initialized first.
    ModuleInitDependencyMap aMIDMap;
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = static_cast<SbModule*>(pModules->Get( nMod ));
        OUString aModuleName = pModule->GetName();
        if( pModule->isProxyModule() )
        {
            aMIDMap[aModuleName] = ClassModuleRunInitItem( pModule );
        }
    }

    ModuleInitDependencyMap::iterator it;
    for( it = aMIDMap.begin() ; it != aMIDMap.end(); ++it )
    {
        ClassModuleRunInitItem& rItem = it->second;
        SbModule::implProcessModuleRunInit( aMIDMap, rItem );
    }

    // Call RunInit on standard modules
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = static_cast<SbModule*>(pModules->Get( nMod ));
        if( !pModule->isProxyModule() )
        {
            pModule->RunInit();
        }
    }

    // Check all objects if they are BASIC,
    // if yes initialize
    for ( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = dynamic_cast<StarBASIC*>( pVar );
        if( pBasic && pBasic != pBasicNotToInit )
        {
            pBasic->InitAllModules();
        }
    }
}

// #88329 Put modules back to not initialised state to
// force reinitialisation at next start
void StarBASIC::DeInitAllModules()
{
    // Deinit own modules
    for ( sal_uInt16 nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = static_cast<SbModule*>(pModules->Get( nMod ));
        if( pModule->pImage && !pModule->isProxyModule() && nullptr == dynamic_cast<const SbObjModule*>( pModule) )
        {
            pModule->pImage->bInit = false;
        }
    }

    for ( sal_uInt16 nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = dynamic_cast<StarBASIC*>( pVar );
        if( pBasic )
        {
            pBasic->DeInitAllModules();
        }
    }
}

// This implementation at first searches within the runtime library,
// then it looks for an element within one module. This module can be
// a public var or an entrypoint. If it is not found and we look for a
// method and a module with the given name is found the search continues
// for entrypoint "Main".
// If this fails again a conventional search over objects is performend.
SbxVariable* StarBASIC::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pRes = nullptr;
    SbModule* pNamed = nullptr;
    // "Extended" search in Runtime Lib
    // but only if SbiRuntime has not set the flag
    if( !bNoRtl )
    {
        if( t == SbxCLASS_DONTCARE || t == SbxCLASS_OBJECT )
        {
            if( rName.equalsIgnoreAsciiCase( RTLNAME ) )
            {
                pRes = pRtl;
            }
        }
        if( !pRes )
        {
            pRes = static_cast<SbiStdObject*>(static_cast<SbxObject*>(pRtl))->Find( rName, t );
        }
        if( pRes )
        {
            pRes->SetFlag( SbxFlagBits::ExtFound );
        }
    }
    // Search module
    if( !pRes )
    {
        for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
        {
            SbModule* p = static_cast<SbModule*>( pModules->Get( i ) );
            if( p->IsVisible() )
            {
                // Remember modul fpr Main() call
                // or is the name equal?!?
                if( p->GetName().equalsIgnoreAsciiCase( rName ) )
                {
                    if( t == SbxCLASS_OBJECT || t == SbxCLASS_DONTCARE )
                    {
                        pRes = p; break;
                    }
                    pNamed = p;
                }
                // Only variables qualified by the Module Name e.g. Sheet1.foo
                // should work for Document && Class type Modules
                sal_Int32 nType = p->GetModuleType();
                if ( nType == ModuleType::DOCUMENT || nType == ModuleType::FORM )
                {
                    continue;
                }
                // otherwise check if the element is available
                // unset GBLSEARCH-Flag (due to Rekursion)
                SbxFlagBits nGblFlag = p->GetFlags() & SbxFlagBits::GlobalSearch;
                p->ResetFlag( SbxFlagBits::GlobalSearch );
                pRes = p->Find( rName, t );
                p->SetFlag( nGblFlag );
                if( pRes )
                {
                    break;
                }
            }
        }
    }
    OUString aMainStr("Main");
    if( !pRes && pNamed && ( t == SbxCLASS_METHOD || t == SbxCLASS_DONTCARE ) &&
        !pNamed->GetName().equalsIgnoreAsciiCase( aMainStr ) )
    {
        pRes = pNamed->Find( aMainStr, SbxCLASS_METHOD );
    }
    if( !pRes )
    {
        pRes = SbxObject::Find( rName, t );
    }
    return pRes;
}

bool StarBASIC::Call( const OUString& rName, SbxArray* pParam )
{
    bool bRes = SbxObject::Call( rName, pParam );
    if( !bRes )
    {
        SbxError eErr = SbxBase::GetError();
        SbxBase::ResetError();
        if( eErr != ERRCODE_SBX_OK )
        {
            RTError( (SbError)eErr, 0, 0, 0 );
        }
    }
    return bRes;
}

// Find method via name (e.g. query via BASIC IDE)
SbxBase* StarBASIC::FindSBXInCurrentScope( const OUString& rName )
{
    if( !GetSbData()->pInst )
    {
        return nullptr;
    }
    if( !GetSbData()->pInst->pRun )
    {
        return nullptr;
    }
    return GetSbData()->pInst->pRun->FindElementExtern( rName );
}

void StarBASIC::QuitAndExitApplication()
{
    Stop();
    bQuit = true;
}

void StarBASIC::Stop()
{
    SbiInstance* p = GetSbData()->pInst;
    while( p )
    {
        p->Stop();
        p = p->pNext;
    }
}

bool StarBASIC::IsRunning()
{
    return GetSbData()->pInst != nullptr;
}

/**************************************************************************
*
*    Debugging and error handling
*
**************************************************************************/

SbMethod* StarBASIC::GetActiveMethod( sal_uInt16 nLevel )
{
    if( GetSbData()->pInst )
    {
        return GetSbData()->pInst->GetCaller( nLevel );
    }
    else
    {
        return nullptr;
    }
}

SbModule* StarBASIC::GetActiveModule()
{
    if( GetSbData()->pInst && !IsCompilerError() )
    {
        return GetSbData()->pInst->GetActiveModule();
    }
    else
    {
        return GetSbData()->pCompMod;
    }
}

sal_uInt16 StarBASIC::BreakPoint( sal_Int32 l, sal_Int32 c1, sal_Int32 c2 )
{
    SetErrorData( 0, l, c1, c2 );
    bBreak = true;
    if( GetSbData()->aBreakHdl.IsSet() )
    {
        return GetSbData()->aBreakHdl.Call( this );
    }
    else
    {
        return BreakHdl();
    }
}

sal_uInt16 StarBASIC::StepPoint( sal_Int32 l, sal_Int32 c1, sal_Int32 c2 )
{
    SetErrorData( 0, l, c1, c2 );
    bBreak = false;
    if( GetSbData()->aBreakHdl.IsSet() )
    {
        return GetSbData()->aBreakHdl.Call( this );
    }
    else
    {
        return BreakHdl();
    }
}

sal_uInt16 StarBASIC::BreakHdl()
{
    return aBreakHdl.IsSet() ? aBreakHdl.Call( this ) : SbDEBUG_CONTINUE;
}

// Calls for error handler and break handler
sal_uInt16 StarBASIC::GetLine()     { return GetSbData()->nLine; }
sal_uInt16 StarBASIC::GetCol1()     { return GetSbData()->nCol1; }
sal_uInt16 StarBASIC::GetCol2()     { return GetSbData()->nCol2; }

// Specific to error handler
SbError StarBASIC::GetErrorCode()       { return GetSbData()->nCode; }
const OUString& StarBASIC::GetErrorText() { return GetSbData()->aErrMsg; }
bool StarBASIC::IsCompilerError()       { return GetSbData()->bCompiler; }

// From 1996-03-29:
// The mapping between the old and the new error codes take place by searching
// through the table SFX_VB_ErrorTab[]. This is indeed not with good performance,
// but it consumes much less memory than corresponding switch blocs.
// Because the conversion of error codes has not to be fast. there is no
// binary search by VB Error -> Error SFX.

// Map back new error codes to old, Sbx-compatible
sal_uInt16 StarBASIC::GetVBErrorCode( SbError nError )
{
    sal_uInt16 nRet = 0;

    if( SbiRuntime::isVBAEnabled() )
    {
        switch( nError )
        {
        case ERRCODE_BASIC_ARRAY_FIX:
            return 10;
        case ERRCODE_BASIC_STRING_OVERFLOW:
            return 14;
        case ERRCODE_BASIC_EXPR_TOO_COMPLEX:
            return 16;
        case ERRCODE_BASIC_OPER_NOT_PERFORM:
            return 17;
        case ERRCODE_BASIC_TOO_MANY_DLL:
            return 47;
        case ERRCODE_BASIC_LOOP_NOT_INIT:
            return 92;
        default:
            nRet = 0;
        }
    }

    // search loop
    const SFX_VB_ErrorItem* pErrItem;
    sal_uInt16 nIndex = 0;
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
    while( pErrItem->nErrorVB != 0xFFFF );      // up to end mark
    return nRet;
}

SbError StarBASIC::GetSfxFromVBError( sal_uInt16 nError )
{
    SbError nRet = 0L;

    if( SbiRuntime::isVBAEnabled() )
    {
        switch( nError )
        {
        case 1:
        case 2:
        case 4:
        case 8:
        case 12:
        case 73:
            return 0L;
        case 10:
            return ERRCODE_BASIC_ARRAY_FIX;
        case 14:
            return ERRCODE_BASIC_STRING_OVERFLOW;
        case 16:
            return ERRCODE_BASIC_EXPR_TOO_COMPLEX;
        case 17:
            return ERRCODE_BASIC_OPER_NOT_PERFORM;
        case 47:
            return ERRCODE_BASIC_TOO_MANY_DLL;
        case 92:
            return ERRCODE_BASIC_LOOP_NOT_INIT;
        default:
            nRet = 0L;
        }
    }
    const SFX_VB_ErrorItem* pErrItem;
    sal_uInt16 nIndex = 0;
    do
    {
        pErrItem = SFX_VB_ErrorTab + nIndex;
        if( pErrItem->nErrorVB == nError )
        {
            nRet = pErrItem->nErrorSFX;
            break;
        }
        else if( pErrItem->nErrorVB > nError )
        {
            break;              // couldn't found anymore
        }
        nIndex++;
    }
    while( pErrItem->nErrorVB != 0xFFFF );      // up to end mark
    return nRet;
}

// set Error- / Break-data
void StarBASIC::SetErrorData( SbError nCode, sal_uInt16 nLine,
                              sal_uInt16 nCol1, sal_uInt16 nCol2 )
{
    SbiGlobals& aGlobals = *GetSbData();
    aGlobals.nCode = nCode;
    aGlobals.nLine = nLine;
    aGlobals.nCol1 = nCol1;
    aGlobals.nCol2 = nCol2;
}


// help class for access to string SubResource of a Resource.
// Source: sfx2\source\doc\docfile.cxx (TLX)
struct BasicStringList_Impl : private Resource
{
    ResId aResId;

    BasicStringList_Impl( ResId& rErrIdP,  sal_uInt16 nId)
        : Resource( rErrIdP ),aResId(nId, *rErrIdP.GetResMgr() ){}
    ~BasicStringList_Impl() { FreeResource(); }

    OUString GetString(){ return aResId.toString(); }
    bool IsErrorTextAvailable()
        { return IsAvailableRes(aResId.SetRT(RSC_STRING)); }
};


void StarBASIC::MakeErrorText( SbError nId, const OUString& aMsg )
{
    SolarMutexGuard aSolarGuard;
    sal_uInt16 nOldID = GetVBErrorCode( nId );

    // instantiate the help class
    BasResId aId( RID_BASIC_START );
    BasicStringList_Impl aMyStringList( aId, sal_uInt16(nId & ERRCODE_RES_MASK) );

    if( aMyStringList.IsErrorTextAvailable() )
    {
        // merge message with additional text
        OUStringBuffer aMsg1(aMyStringList.GetString());
        // replace argument placeholder with %s
        OUString aSrgStr( "$(ARG1)" );
        sal_Int32 nResult = aMyStringList.GetString().indexOf( aSrgStr );

        if( nResult >= 0 )
        {
            aMsg1.remove(nResult, aSrgStr.getLength());
            aMsg1.insert(nResult, aMsg);
        }
        GetSbData()->aErrMsg = aMsg1.makeStringAndClear();
    }
    else if( nOldID != 0 )
    {
        OUString aStdMsg = "Fehler " + OUString::number(nOldID) +
                           ": Kein Fehlertext verfuegbar!";
        GetSbData()->aErrMsg = aStdMsg;
    }
    else
    {
        GetSbData()->aErrMsg.clear();
    }
}

bool StarBASIC::CError( SbError code, const OUString& rMsg,
                            sal_Int32 l, sal_Int32 c1, sal_Int32 c2 )
{
    SolarMutexGuard aSolarGuard;

    // compiler error during runtime -> stop programm
    if( IsRunning() )
    {
        // #109018 Check if running Basic is affected
        StarBASIC* pStartedBasic = GetSbData()->pInst->GetBasic();
        if( pStartedBasic != this )
        {
            return false;
        }
        Stop();
    }

    // set flag, so that GlobalRunInit notice the error
    GetSbData()->bGlobalInitErr = true;

    // tinker the error message
    MakeErrorText( code, rMsg );

    // Implementation of the code for the string transport to SFX-Error
    if( !rMsg.isEmpty() )
    {
        code = (sal_uIntPtr)*new StringErrorInfo( code, rMsg );
    }
    SetErrorData( code, l, c1, c2 );
    GetSbData()->bCompiler = true;
    bool bRet;
    if( GetSbData()->aErrHdl.IsSet() )
    {
        bRet = GetSbData()->aErrHdl.Call( this );
    }
    else
    {
        bRet = ErrorHdl();
    }
    GetSbData()->bCompiler = false;     // only true for error handler
    return bRet;
}

bool StarBASIC::RTError( SbError code, sal_Int32 l, sal_Int32 c1, sal_Int32 c2 )
{
    return RTError( code, OUString(), l, c1, c2 );
}

bool StarBASIC::RTError( SbError code, const OUString& rMsg, sal_Int32 l, sal_Int32 c1, sal_Int32 c2 )
{
    SolarMutexGuard aSolarGuard;

    SbError c = code;
    if( (c & ERRCODE_CLASS_MASK) == ERRCODE_CLASS_COMPILER )
    {
        c = 0;
    }
    MakeErrorText( c, rMsg );

    // Implementation of the code for the string transport to SFX-Error
    if( !rMsg.isEmpty() )
    {
        // very confusing, even though MakeErrorText sets up the error text
        // seems that this is not used ( if rMsg already has content )
        // In the case of VBA MakeErrorText also formats the error to be a little more
        // like vba ( adds an error number etc )
        if ( SbiRuntime::isVBAEnabled() && ( code == ERRCODE_BASIC_COMPAT ) )
        {
            OUString aTmp = "\'" + OUString::number(SbxErrObject::getUnoErrObject()->getNumber()) +
                            "\'\n" + OUString(!GetSbData()->aErrMsg.isEmpty() ? GetSbData()->aErrMsg : rMsg);
            code = (sal_uIntPtr)*new StringErrorInfo( code, aTmp );
        }
        else
        {
            code = (sal_uIntPtr)*new StringErrorInfo( code, rMsg );
        }
    }

    SetErrorData( code, l, c1, c2 );
    if( GetSbData()->aErrHdl.IsSet() )
    {
        return GetSbData()->aErrHdl.Call( this );
    }
    else
    {
        return ErrorHdl();
    }
}

void StarBASIC::Error( SbError n )
{
    Error( n, OUString() );
}

void StarBASIC::Error( SbError n, const OUString& rMsg )
{
    if( GetSbData()->pInst )
    {
        GetSbData()->pInst->Error( n, rMsg );
    }
}

void StarBASIC::FatalError( SbError n )
{
    if( GetSbData()->pInst )
    {
        GetSbData()->pInst->FatalError( n );
    }
}

void StarBASIC::FatalError( SbError _errCode, const OUString& _details )
{
    if( GetSbData()->pInst )
    {
        GetSbData()->pInst->FatalError( _errCode, _details );
    }
}

SbError StarBASIC::GetErrBasic()
{
    if( GetSbData()->pInst )
    {
        return GetSbData()->pInst->GetErr();
    }
    else
    {
        return 0;
    }
}

// make the additional message for the RTL function error accessible
OUString StarBASIC::GetErrorMsg()
{
    if( GetSbData()->pInst )
    {
        return GetSbData()->pInst->GetErrorMsg();
    }
    else
    {
        return OUString();
    }
}

sal_Int32 StarBASIC::GetErl()
{
    if( GetSbData()->pInst )
    {
        return GetSbData()->pInst->GetErl();
    }
    else
    {
        return 0;
    }
}

bool StarBASIC::ErrorHdl()
{
    return aErrorHdl.Call( this );
}

Link<StarBASIC*,bool> StarBASIC::GetGlobalErrorHdl()
{
    return GetSbData()->aErrHdl;
}

void StarBASIC::SetGlobalErrorHdl( const Link<StarBASIC*,bool>& rLink )
{
    GetSbData()->aErrHdl = rLink;
}

void StarBASIC::SetGlobalBreakHdl( const Link<StarBASIC*,sal_uInt16>& rLink )
{
    GetSbData()->aBreakHdl = rLink;
}

SbxArrayRef StarBASIC::getUnoListeners()
{
    if( !xUnoListeners.Is() )
    {
        xUnoListeners = new SbxArray();
    }
    return xUnoListeners;
}


/**************************************************************************
*
*   load and save
*
**************************************************************************/

bool StarBASIC::LoadData( SvStream& r, sal_uInt16 nVer )
{
    if( !SbxObject::LoadData( r, nVer ) )
    {
        return false;
    }
    // #95459 Delete dialogs, otherwise endless recursion
    // in SbxVarable::GetType() if dialogs are accessed
    sal_uInt16 nObjCount = pObjs->Count();
    std::unique_ptr<SbxVariable*[]> ppDeleteTab(new SbxVariable*[ nObjCount ]);
    sal_uInt16 nObj;

    for( nObj = 0 ; nObj < nObjCount ; nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = dynamic_cast<StarBASIC*>( pVar  );
        ppDeleteTab[nObj] = pBasic ? nullptr : pVar;
    }
    for( nObj = 0 ; nObj < nObjCount ; nObj++ )
    {
        SbxVariable* pVar = ppDeleteTab[nObj];
        if( pVar )
        {
            pObjs->Remove( pVar );
        }
    }
    ppDeleteTab.reset();

    sal_uInt16 nMod(0);
    pModules->Clear();
    r.ReadUInt16( nMod );
    const size_t nMinSbxSize(14);
    const size_t nMaxPossibleEntries = r.remainingSize() / nMinSbxSize;
    if (nMod > nMaxPossibleEntries)
    {
        nMod = nMaxPossibleEntries;
        SAL_WARN("basic", "Parsing error: " << nMaxPossibleEntries <<
                 " max possible entries, but " << nMod << " claimed, truncating");
    }
    for (sal_uInt16 i = 0; i < nMod; ++i)
    {
        SbxBase* pBase = SbxBase::Load( r );
        SbModule* pMod = dynamic_cast<SbModule*>(pBase);
        if( !pMod )
        {
            return false;
        }
        else if( nullptr != dynamic_cast<const SbJScriptModule*>( pMod) )
        {
            // assign Ref, so that pMod will be deleted
            SbModuleRef xRef = pMod;
        }
        else
        {
            pMod->SetParent( this );
            pModules->Put( pMod, i );
        }
    }
    // HACK for SFX-Bullshit!
    SbxVariable* p = Find( "FALSE", SbxCLASS_PROPERTY );
    if( p )
    {
        Remove( p );
    }
    p = Find( "TRUE", SbxCLASS_PROPERTY );
    if( p )
    {
        Remove( p );
    }
    // End of the hacks!
    // Search via StarBASIC is at all times global
    DBG_ASSERT( IsSet( SbxFlagBits::GlobalSearch ), "Basic loaded without GBLSEARCH" );
    SetFlag( SbxFlagBits::GlobalSearch );
    return true;
}

bool StarBASIC::StoreData( SvStream& r ) const
{
    if( !SbxObject::StoreData( r ) )
    {
        return false;
    }
    r.WriteUInt16( pModules->Count() );
    for( sal_uInt16 i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = static_cast<SbModule*>( pModules->Get( i ) );
        if( !p->Store( r ) )
        {
            return false;
        }
    }
    return true;
}

bool StarBASIC::GetUNOConstant( const sal_Char* _pAsciiName, css::uno::Any& aOut )
{
    bool bRes = false;
    OUString sVarName( OUString::createFromAscii( _pAsciiName ) );
    SbUnoObject* pGlobs = dynamic_cast<SbUnoObject*>( Find( sVarName, SbxCLASS_DONTCARE ) );
    if ( pGlobs )
    {
        aOut = pGlobs->getUnoAny();
        bRes = true;
    }
    return bRes;
}

Reference< frame::XModel > StarBASIC::GetModelFromBasic( SbxObject* pBasic )
{
    OSL_PRECOND( pBasic != nullptr, "getModelFromBasic: illegal call!" );
    if ( !pBasic )
    {
        return nullptr;
    }
    // look for the ThisComponent variable, first in the parent (which
    // might be the document's Basic), then in the parent's parent (which might be
    // the application Basic)
    const OUString sThisComponent( "ThisComponent");
    SbxVariable* pThisComponent = nullptr;

    SbxObject* pLookup = pBasic->GetParent();
    while ( pLookup && !pThisComponent )
    {
        pThisComponent = pLookup->Find( sThisComponent, SbxCLASS_OBJECT );
        pLookup = pLookup->GetParent();
    }
    if ( !pThisComponent )
    {
        OSL_TRACE("Failed to get ThisComponent");
            // the application Basic, at the latest, should have this variable
        return nullptr;
    }

    Any aThisComponentAny( sbxToUnoValue( pThisComponent ) );
    Reference< frame::XModel > xModel( aThisComponentAny, UNO_QUERY );
    if ( !xModel.is() )
    {
        // it's no XModel. Okay, ThisComponent nowadays is allowed to be a controller.
        Reference< frame::XController > xController( aThisComponentAny, UNO_QUERY );
        if ( xController.is() )
        {
            xModel = xController->getModel();
        }
    }
    if ( !xModel.is() )
    {
        return nullptr;
    }
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE("Have model ThisComponent points to url %s",
              OUStringToOString( xModel->getURL(),
                                 RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

    return xModel;
}

void StarBASIC::DetachAllDocBasicItems()
{
    DocBasicItemMap& rItems = GaDocBasicItems::get();
    DocBasicItemMap::iterator it = rItems.begin(), itEnd = rItems.end();
    for (; it != itEnd; ++it)
    {
        DocBasicItemRef xItem = it->second;
        xItem->setDisposed(true);
    }
}

// #118116 Implementation Collection object


static const char pCountStr[]   = "Count";
static const char pAddStr[]     = "Add";
static const char pItemStr[]    = "Item";
static const char pRemoveStr[]  = "Remove";
static sal_uInt16 nCountHash = 0, nAddHash, nItemHash, nRemoveHash;

SbxInfoRef BasicCollection::xAddInfo = nullptr;
SbxInfoRef BasicCollection::xItemInfo = nullptr;

BasicCollection::BasicCollection( const OUString& rClass )
             : SbxObject( rClass )
{
    if( !nCountHash )
    {
        nCountHash  = MakeHashCode( pCountStr );
        nAddHash    = MakeHashCode( pAddStr );
        nItemHash   = MakeHashCode( pItemStr );
        nRemoveHash = MakeHashCode( pRemoveStr );
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
    SetFlag( SbxFlagBits::Fixed );
    ResetFlag( SbxFlagBits::Write );
    SbxVariable* p;
    p = Make( pCountStr, SbxCLASS_PROPERTY, SbxINTEGER );
    p->ResetFlag( SbxFlagBits::Write );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pAddStr, SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pItemStr, SbxCLASS_METHOD, SbxVARIANT );
    p->SetFlag( SbxFlagBits::DontStore );
    p = Make( pRemoveStr, SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    if ( !xAddInfo.Is() )
    {
        xAddInfo = new SbxInfo;
        xAddInfo->AddParam(  "Item", SbxVARIANT );
        xAddInfo->AddParam(  "Key", SbxVARIANT, SbxFlagBits::Read | SbxFlagBits::Optional );
        xAddInfo->AddParam(  "Before", SbxVARIANT, SbxFlagBits::Read | SbxFlagBits::Optional );
        xAddInfo->AddParam(  "After", SbxVARIANT, SbxFlagBits::Read | SbxFlagBits::Optional );
    }
    if ( !xItemInfo.Is() )
    {
        xItemInfo = new SbxInfo;
        xItemInfo->AddParam(  "Index", SbxVARIANT, SbxFlagBits::Read | SbxFlagBits::Optional);
    }
}

SbxVariable* BasicCollection::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pFind = SbxObject::Find( rName, t );
    return pFind;
}

void BasicCollection::Notify( SfxBroadcaster& rCst, const SfxHint& rHint )
{
    const SbxHint* p = dynamic_cast<const SbxHint*>(&rHint);
    if( p )
    {
        const sal_uInt32 nId = p->GetId();
        bool bRead  = nId == SBX_HINT_DATAWANTED;
        bool bWrite = nId == SBX_HINT_DATACHANGED;
        bool bRequestInfo = nId == SBX_HINT_INFOWANTED;
        SbxVariable* pVar = p->GetVar();
        SbxArray* pArg = pVar->GetParameters();
        OUString aVarName( pVar->GetName() );
        if( bRead || bWrite )
        {
            if( pVar->GetHashCode() == nCountHash
                  && aVarName.equalsIgnoreAsciiCase( pCountStr ) )
            {
                pVar->PutLong( xItemArray->Count32() );
            }
            else if( pVar->GetHashCode() == nAddHash
                  && aVarName.equalsIgnoreAsciiCase( pAddStr ) )
            {
                CollAdd( pArg );
            }
            else if( pVar->GetHashCode() == nItemHash
                  && aVarName.equalsIgnoreAsciiCase( pItemStr ) )
            {
                CollItem( pArg );
            }
            else if( pVar->GetHashCode() == nRemoveHash
                  && aVarName.equalsIgnoreAsciiCase( pRemoveStr ) )
            {
                CollRemove( pArg );
            }
            else
            {
                SbxObject::Notify( rCst, rHint );
            }
            return;
        }
        else if ( bRequestInfo )
        {
            if( pVar->GetHashCode() == nAddHash
                  && aVarName.equalsIgnoreAsciiCase( pAddStr ) )
            {
                pVar->SetInfo( xAddInfo );
            }
            else if( pVar->GetHashCode() == nItemHash
                  && aVarName.equalsIgnoreAsciiCase( pItemStr ) )
            {
                pVar->SetInfo( xItemInfo );
            }
        }
    }
    SbxObject::Notify( rCst, rHint );
}

sal_Int32 BasicCollection::implGetIndex( SbxVariable* pIndexVar )
{
    sal_Int32 nIndex = -1;
    if( pIndexVar->GetType() == SbxSTRING )
    {
        nIndex = implGetIndexForName( pIndexVar->GetOUString() );
    }
    else
    {
        nIndex = pIndexVar->GetLong() - 1;
    }
    return nIndex;
}

sal_Int32 BasicCollection::implGetIndexForName( const OUString& rName )
{
    sal_Int32 nIndex = -1;
    sal_Int32 nCount = xItemArray->Count32();
    sal_Int32 nNameHash = MakeHashCode( rName );
    for( sal_Int32 i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pVar = xItemArray->Get32( i );
        if( pVar->GetHashCode() == nNameHash &&
            pVar->GetName().equalsIgnoreAsciiCase( rName ) )
        {
            nIndex = i;
            break;
        }
    }
    return nIndex;
}

void BasicCollection::CollAdd( SbxArray* pPar_ )
{
    sal_uInt16 nCount = pPar_->Count();
    if( nCount < 2 || nCount > 5 )
    {
        SetError( ERRCODE_SBX_WRONG_ARGS );
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
                if( !( pBefore->IsErr() || ( pBefore->GetType() == SbxEMPTY ) ) )
                {
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                    return;
                }
                SbxVariable* pAfter = pPar_->Get(4);
                sal_Int32 nAfterIndex = implGetIndex( pAfter );
                if( nAfterIndex == -1 )
                {
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                    return;
                }
                nNextIndex = nAfterIndex + 1;
            }
            else // if( nCount == 4 )
            {
                sal_Int32 nBeforeIndex = implGetIndex( pBefore );
                if( nBeforeIndex == -1 )
                {
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                    return;
                }
                nNextIndex = nBeforeIndex;
            }
        }

        SbxVariableRef pNewItem = new SbxVariable( *pItem );
        if( nCount >= 3 )
        {
            SbxVariable* pKey = pPar_->Get(2);
            if( !( pKey->IsErr() || ( pKey->GetType() == SbxEMPTY ) ) )
            {
                if( pKey->GetType() != SbxSTRING )
                {
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                    return;
                }
                OUString aKey = pKey->GetOUString();
                if( implGetIndexForName( aKey ) != -1 )
                {
                    SetError( ERRCODE_BASIC_BAD_ARGUMENT );
                    return;
                }
                pNewItem->SetName( aKey );
            }
        }
        pNewItem->SetFlag( SbxFlagBits::ReadWrite );
        xItemArray->Insert32( pNewItem, nNextIndex );
    }
    else
    {
        SetError( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
}

void BasicCollection::CollItem( SbxArray* pPar_ )
{
    if( pPar_->Count() != 2 )
    {
        SetError( ERRCODE_SBX_WRONG_ARGS );
        return;
    }
    SbxVariable* pRes = nullptr;
    SbxVariable* p = pPar_->Get( 1 );
    sal_Int32 nIndex = implGetIndex( p );
    if( nIndex >= 0 && nIndex < (sal_Int32)xItemArray->Count32() )
    {
        pRes = xItemArray->Get32( nIndex );
    }
    if( !pRes )
    {
        SetError( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        *(pPar_->Get(0)) = *pRes;
    }
}

void BasicCollection::CollRemove( SbxArray* pPar_ )
{
    if( pPar_ == nullptr || pPar_->Count() != 2 )
    {
        SetError( ERRCODE_SBX_WRONG_ARGS );
        return;
    }

    SbxVariable* p = pPar_->Get( 1 );
    sal_Int32 nIndex = implGetIndex( p );
    if( nIndex >= 0 && nIndex < (sal_Int32)xItemArray->Count32() )
    {
        xItemArray->Remove32( nIndex );

        // Correct for stack if necessary
        SbiInstance* pInst = GetSbData()->pInst;
        SbiRuntime* pRT = pInst ? pInst->pRun : nullptr;
        if( pRT )
        {
            SbiForStack* pStack = pRT->FindForStackItemForCollection( this );
            if( pStack != nullptr )
            {
                if( pStack->nCurCollectionIndex >= nIndex )
                {
                    --pStack->nCurCollectionIndex;
                }
            }
        }
    }
    else
    {
        SetError( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
