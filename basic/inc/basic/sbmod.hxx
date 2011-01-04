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
typedef std::deque< USHORT > SbiBreakpoints;
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
    BOOL mbVBACompat;
    INT32 mnType;
    SbxObjectRef pDocObject; // an impl object ( used by Document Modules )
    bool    bIsProxyModule;

    static void     implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem );
    void            StartDefinitions();
    SbMethod*       GetMethod( const String&, SbxDataType );
    SbProperty*     GetProperty( const String&, SbxDataType );
    SbProcedureProperty* GetProcedureProperty( const String&, SbxDataType );
    SbIfaceMapperMethod* GetIfaceMapperMethod( const String&, SbMethod* );
    void            EndDefinitions( BOOL=FALSE );
    USHORT          Run( SbMethod* );
    void            RunInit();
    void            ClearPrivateVars();
    void            ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic );
    void            GlobalRunInit( BOOL bBasicStart );  // for all modules
    void            GlobalRunDeInit( void );
    const BYTE*     FindNextStmnt( const BYTE*, USHORT&, USHORT& ) const;
    const BYTE*     FindNextStmnt( const BYTE*, USHORT&, USHORT&,
                        BOOL bFollowJumps, const SbiImage* pImg=NULL ) const;
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
    virtual BOOL LoadCompleted();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
    void handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual ~SbModule();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICMOD,2);
    TYPEINFO();
                    SbModule( const String&, BOOL bCompat = FALSE );
    virtual void    SetParent( SbxObject* );
    virtual void    Clear();

    virtual SbxVariable* Find( const String&, SbxClassType );

    virtual const String&   GetSource() const;
    const ::rtl::OUString&  GetSource32() const;
    const String&   GetComment() const            { return aComment; }
    virtual void    SetSource( const String& r );
    void            SetSource32( const ::rtl::OUString& r );
    void            SetComment( const String& r );

    virtual BOOL    Compile();
    BOOL            Disassemble( String& rText );
    virtual BOOL    IsCompiled() const;
    const SbxObject* FindType( String aTypeName ) const;

    virtual BOOL    IsBreakable( USHORT nLine ) const;
    virtual size_t  GetBPCount() const;
    virtual USHORT  GetBP( size_t n ) const;
    virtual BOOL    IsBP( USHORT nLine ) const;
    virtual BOOL    SetBP( USHORT nLine );
    virtual BOOL    ClearBP( USHORT nLine );
    virtual void    ClearAllBP();

    // Lines of Subs
    virtual SbMethod*   GetFunctionForLine( USHORT );

    // Store only image, no source (needed for new password protection)
       BOOL StoreBinaryData( SvStream& );
       BOOL StoreBinaryData( SvStream&, USHORT nVer );
    BOOL LoadBinaryData( SvStream&, USHORT nVer );
    BOOL LoadBinaryData( SvStream& );
    BOOL ExceedsLegacyModuleSize();
    void fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = NULL ) const;
    BOOL IsVBACompat() const;
    void SetVBACompat( BOOL bCompat );
    INT32 GetModuleType() { return mnType; }
    void SetModuleType( INT32 nType ) { mnType = nType; }
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
