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

#ifndef _SB_SBMOD_HXX
#define _SB_SBMOD_HXX

#include <com/sun/star/script/XInvocation.hpp>
#include <basic/sbdef.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxdef.hxx>
#include <rtl/ustring.hxx>
#include <vector>

#include <deque>

class SbMethod;
class SbProperty;
class SbiRuntime;
typedef std::deque< sal_uInt16 > SbiBreakpoints;
class SbiImage;
class SbProcedureProperty;
class SbIfaceMapperMethod;
class SbClassModuleObject;

class ModuleInitDependencyMap;
struct ClassModuleRunInitItem;
struct SbClassData;
class SbModuleImpl;

class SbModule : public SbxObject
{
    friend class    TestToolObj;    // allows module initialisation at runtime
    friend class    SbiCodeGen;
    friend class    SbMethod;
    friend class    SbiRuntime;
    friend class    StarBASIC;
    friend class    SbClassModuleObject;

    SbModuleImpl*   mpSbModuleImpl;     // Impl data
    std::vector< String > mModuleVariableNames;

    void            implClearIfVarDependsOnDeletedBasic( SbxVariable* pVar, StarBASIC* pDeletedBasic );

protected:
    com::sun::star::uno::Reference< com::sun::star::script::XInvocation > mxWrapper;
    ::rtl::OUString     aOUSource;
    String              aComment;
    SbiImage*           pImage;        // the Image
    SbiBreakpoints*     pBreaks;       // Breakpoints
    SbClassData*        pClassData;
    sal_Bool mbVBACompat;
    sal_Int32 mnType;
    SbxObjectRef pDocObject; // an impl object ( used by Document Modules )
    bool    bIsProxyModule;

    static void     implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem );
    void            StartDefinitions();
    SbMethod*       GetMethod( const String&, SbxDataType );
    SbProperty*     GetProperty( const String&, SbxDataType );
    SbProcedureProperty* GetProcedureProperty( const String&, SbxDataType );
    SbIfaceMapperMethod* GetIfaceMapperMethod( const String&, SbMethod* );
    void            EndDefinitions( sal_Bool=sal_False );
    sal_uInt16          Run( SbMethod* );
    void            RunInit();
    void            ClearPrivateVars();
    void            ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic );
    void            GlobalRunInit( sal_Bool bBasicStart );  // for all modules
    void            GlobalRunDeInit( void );
    const sal_uInt8*    FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16& ) const;
    const sal_uInt8*    FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16&,
                        sal_Bool bFollowJumps, const SbiImage* pImg=NULL ) const;
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
    virtual sal_Bool LoadCompleted();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    void handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual ~SbModule();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICMOD,2);
    TYPEINFO();
                    SbModule( const String&, sal_Bool bCompat = sal_False );
    virtual void    SetParent( SbxObject* );
    virtual void    Clear();

    virtual SbxVariable* Find( const String&, SbxClassType );

    virtual const String&   GetSource() const;
    const ::rtl::OUString&  GetSource32() const;
    const String&   GetComment() const            { return aComment; }
    virtual void    SetSource( const String& r );
    void            SetSource32( const ::rtl::OUString& r );
    void            SetComment( const String& r );

    virtual sal_Bool    Compile();
    sal_Bool            Disassemble( String& rText );
    virtual sal_Bool    IsCompiled() const;
    const SbxObject* FindType( String aTypeName ) const;

    virtual sal_Bool    IsBreakable( sal_uInt16 nLine ) const;
    virtual size_t  GetBPCount() const;
    virtual sal_uInt16  GetBP( size_t n ) const;
    virtual sal_Bool    IsBP( sal_uInt16 nLine ) const;
    virtual sal_Bool    SetBP( sal_uInt16 nLine );
    virtual sal_Bool    ClearBP( sal_uInt16 nLine );
    virtual void    ClearAllBP();

    // Lines of Subs
    virtual SbMethod*   GetFunctionForLine( sal_uInt16 );

    // Store only image, no source (needed for new password protection)
       sal_Bool StoreBinaryData( SvStream& );
       sal_Bool StoreBinaryData( SvStream&, sal_uInt16 nVer );
    sal_Bool LoadBinaryData( SvStream&, sal_uInt16 nVer );
    sal_Bool LoadBinaryData( SvStream& );
    sal_Bool ExceedsLegacyModuleSize();
    void fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = NULL ) const;
    sal_Bool IsVBACompat() const;
    void SetVBACompat( sal_Bool bCompat );
    sal_Int32 GetModuleType() { return mnType; }
    void SetModuleType( sal_Int32 nType ) { mnType = nType; }
    bool isProxyModule() { return bIsProxyModule; }
    void AddVarName( const String& aName );
    void RemoveVars();
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > GetUnoModule();
    bool createCOMWrapperForIface( ::com::sun::star::uno::Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject );
};

#ifndef __SB_SBMODULEREF_HXX
#define __SB_SBMODULEREF_HXX

SV_DECL_IMPL_REF(SbModule)

#endif

class SbClassModuleImpl;

// Object class for instances of class modules
class SbClassModuleObject : public SbModule
{
    SbClassModuleImpl* mpSbClassModuleImpl;

    SbModule*   mpClassModule;
    bool        mbInitializeEventDone;

public:
    TYPEINFO();
    SbClassModuleObject( SbModule* pClassModule );
    ~SbClassModuleObject();

    // Overridden to support NameAccess etc.
    virtual SbxVariable* Find( const String&, SbxClassType );

    virtual void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );

    SbModule* getClassModule( void )
        { return mpClassModule; }

    void triggerInitializeEvent( void );
    void triggerTerminateEvent( void );
};

#ifndef __SB_SBCLASSMODULEREF_HXX
#define __SB_SBCLASSMODULEREF_HXX

SV_DECL_IMPL_REF(SbClassModuleObject);

#endif

#endif
