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

#ifndef _SB_SBMOD_HXX
#define _SB_SBMOD_HXX

#include <com/sun/star/script/XInvocation.hpp>
#include <basic/sbdef.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxdef.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <deque>
#include <boost/utility.hpp>
#include "basicdllapi.h"

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

struct CodeCompleteData
{
    OUString sVarName;
    OUString sVarParent;
    OUString sVarType;

    inline bool operator==( const CodeCompleteData& aOther )
    {
        return (sVarName == aOther.sVarName && sVarParent == aOther.sVarParent);
    }

    inline bool IsGlobal() const
    {
        return ( sVarParent == OUString("") );
    }
};

class BASIC_DLLPUBLIC SbModule : public SbxObject, private ::boost::noncopyable
{
    friend class    SbiCodeGen;
    friend class    SbMethod;
    friend class    SbiRuntime;
    friend class    StarBASIC;
    friend class    SbClassModuleObject;

    std::vector< OUString > mModuleVariableNames;

    BASIC_DLLPRIVATE void implClearIfVarDependsOnDeletedBasic( SbxVariable* pVar, StarBASIC* pDeletedBasic );

protected:
    com::sun::star::uno::Reference< com::sun::star::script::XInvocation > mxWrapper;
    OUString            aOUSource;
    OUString            aComment;
    SbiImage*           pImage;        // the Image
    SbiBreakpoints*     pBreaks;       // Breakpoints
    SbClassData*        pClassData;
    bool mbVBACompat;
    sal_Int32 mnType;
    SbxObjectRef pDocObject; // an impl object ( used by Document Modules )
    bool    bIsProxyModule;

    static void     implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem );
    void            StartDefinitions();
    SbMethod*       GetMethod( const OUString&, SbxDataType );
    SbProperty*     GetProperty( const OUString&, SbxDataType );
    SbProcedureProperty* GetProcedureProperty( const OUString&, SbxDataType );
    SbIfaceMapperMethod* GetIfaceMapperMethod( const OUString&, SbMethod* );
    void            EndDefinitions( sal_Bool=sal_False );
    sal_uInt16      Run( SbMethod* );
    void            RunInit();
    void            ClearPrivateVars();
    void            ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic );
    void            GlobalRunInit( bool bBasicStart );  // for all modules
    void            GlobalRunDeInit( void );
    const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16& ) const;
    const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16&,
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
                    SbModule( const OUString&, sal_Bool bCompat = sal_False );
    virtual void    SetParent( SbxObject* );
    virtual void    Clear();

    virtual SbxVariable* Find( const OUString&, SbxClassType );

    virtual const OUString& GetSource() const;
    const OUString&  GetSource32() const;
    const OUString&  GetComment() const { return aComment; }
    virtual void     SetSource( const OUString& r );
    void             SetSource32( const OUString& r );

    virtual sal_Bool Compile();
    virtual sal_Bool IsCompiled() const;
    const SbxObject* FindType( OUString aTypeName ) const;

    virtual sal_Bool IsBreakable( sal_uInt16 nLine ) const;
    virtual sal_Bool IsBP( sal_uInt16 nLine ) const;
    virtual sal_Bool SetBP( sal_uInt16 nLine );
    virtual sal_Bool ClearBP( sal_uInt16 nLine );
    virtual void     ClearAllBP();

    // Store only image, no source (needed for new password protection)
    sal_Bool StoreBinaryData( SvStream& );
    sal_Bool StoreBinaryData( SvStream&, sal_uInt16 nVer );
    sal_Bool LoadBinaryData( SvStream&, sal_uInt16 nVer );
    sal_Bool LoadBinaryData( SvStream& );
    sal_Bool ExceedsLegacyModuleSize();
    void     fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = NULL ) const;
    bool     HasExeCode();
    bool     IsVBACompat() const;
    void     SetVBACompat( bool bCompat );
    sal_Int32 GetModuleType() { return mnType; }
    void     SetModuleType( sal_Int32 nType ) { mnType = nType; }
    bool     isProxyModule() { return bIsProxyModule; }
    void     AddVarName( const OUString& aName );
    void     RemoveVars();
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > GetUnoModule();
    bool createCOMWrapperForIface( ::com::sun::star::uno::Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject );
    std::vector< CodeCompleteData > GetCodeCompleteDataFromParse();
    SbxArrayRef GetMethods();
};

SV_DECL_IMPL_REF(SbModule)

// Object class for instances of class modules
class BASIC_DLLPUBLIC SbClassModuleObject : public SbModule
{
    SbModule*   mpClassModule;
    bool        mbInitializeEventDone;

public:
    TYPEINFO();
    SbClassModuleObject( SbModule* pClassModule );
    ~SbClassModuleObject();

    // Overridden to support NameAccess etc.
    virtual SbxVariable* Find( const OUString&, SbxClassType );

    virtual void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );

    SbModule* getClassModule( void )
        { return mpClassModule; }

    void triggerInitializeEvent( void );
    void triggerTerminateEvent( void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
