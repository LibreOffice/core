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

#ifndef INCLUDED_BASIC_SBMOD_HXX
#define INCLUDED_BASIC_SBMOD_HXX

#include <com/sun/star/script/XInvocation.hpp>
#include <basic/sbdef.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxdef.hxx>
#include <basic/sbx.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <deque>
#include <boost/utility.hpp>
#include <basic/basicdllapi.h>
#include <basic/codecompletecache.hxx>

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
    void            EndDefinitions( bool=false );
    sal_uInt16      Run( SbMethod* );
    void            RunInit();
    void            ClearPrivateVars();
    void            ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic );
    void            GlobalRunInit( bool bBasicStart );  // for all modules
    void            GlobalRunDeInit( void );
    const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16& ) const;
    const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16&,
                                    bool bFollowJumps, const SbiImage* pImg=NULL ) const;
    virtual bool LoadData( SvStream&, sal_uInt16 ) SAL_OVERRIDE;
    virtual bool StoreData( SvStream& ) const SAL_OVERRIDE;
    virtual bool LoadCompleted() SAL_OVERRIDE;
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType ) SAL_OVERRIDE;
    void handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual ~SbModule();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICMOD,2);
    TYPEINFO_OVERRIDE();
                    SbModule( const OUString&, bool bCompat = false );
    virtual void    SetParent( SbxObject* ) SAL_OVERRIDE;
    virtual void    Clear() SAL_OVERRIDE;

    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;

    virtual const OUString& GetSource() const;
    const OUString&  GetSource32() const { return aOUSource;}
    const OUString&  GetComment() const { return aComment; }
    virtual void     SetSource( const OUString& r );
    void             SetSource32( const OUString& r );

    virtual bool Compile();
    virtual bool IsCompiled() const;
    const SbxObject* FindType( const OUString& aTypeName ) const;

    virtual bool IsBreakable( sal_uInt16 nLine ) const;
    virtual bool IsBP( sal_uInt16 nLine ) const;
    virtual bool SetBP( sal_uInt16 nLine );
    virtual bool ClearBP( sal_uInt16 nLine );
    virtual void     ClearAllBP();

    // Store only image, no source (needed for new password protection)
    bool StoreBinaryData( SvStream& );
    bool StoreBinaryData( SvStream&, sal_uInt16 nVer );
    bool LoadBinaryData( SvStream&, sal_uInt16 nVer );
    bool LoadBinaryData( SvStream& );
    bool ExceedsLegacyModuleSize();
    void     fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = NULL ) const;
    bool     HasExeCode();
    bool     IsVBACompat() const { return mbVBACompat;}
    void     SetVBACompat( bool bCompat );
    sal_Int32 GetModuleType() { return mnType; }
    void     SetModuleType( sal_Int32 nType ) { mnType = nType; }
    bool     isProxyModule() { return bIsProxyModule; }
    void     AddVarName( const OUString& aName );
    void     RemoveVars();
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > GetUnoModule();
    bool createCOMWrapperForIface( ::com::sun::star::uno::Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject );
    void GetCodeCompleteDataFromParse(CodeCompleteDataCache& aCache);
    SbxArrayRef GetMethods() { return pMethods;}
    OUString GetKeywordCase( const OUString& sKeyword ) const;
};

typedef tools::SvRef<SbModule> SbModuleRef;

// Object class for instances of class modules
class BASIC_DLLPUBLIC SbClassModuleObject : public SbModule
{
    SbModule*   mpClassModule;
    bool        mbInitializeEventDone;

public:
    TYPEINFO_OVERRIDE();
    SbClassModuleObject( SbModule* pClassModule );
    virtual ~SbClassModuleObject();

    // Overridden to support NameAccess etc.
    virtual SbxVariable* Find( const OUString&, SbxClassType ) SAL_OVERRIDE;

    virtual void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& ) SAL_OVERRIDE;

    SbModule* getClassModule( void )
        { return mpClassModule; }

    void triggerInitializeEvent( void );
    void triggerTerminateEvent( void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
