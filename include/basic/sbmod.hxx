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

#include <config_options.h>
#include <basic/sbdef.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxdef.hxx>
#include <basic/sbx.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <deque>
#include <basic/basicdllapi.h>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::script { class XInvocation; }

class SbMethod;
class SbProperty;
typedef std::deque< sal_uInt16 > SbiBreakpoints;
class SbiImage;
class SbClassModuleObject;
class CodeCompleteDataCache;
class SbUnoObject;

class ModuleInitDependencyMap;
struct ClassModuleRunInitItem;
struct SbClassData;

class BASIC_DLLPUBLIC SbModule : public SbxObject
{
    friend class    SbiCodeGen;
    friend class    SbMethod;
    friend class    SbiRuntime;
    friend class    StarBASIC;
    friend class    SbClassModuleObject;

    std::vector< OUString > mModuleVariableNames;

    BASIC_DLLPRIVATE static void implClearIfVarDependsOnDeletedBasic(SbxVariable& rVar, StarBASIC* pDeletedBasic);

    SbModule(const SbModule&) = delete;
    SbModule& operator=(const SbModule&) = delete;
protected:
    css::uno::Reference< css::script::XInvocation > mxWrapper;
    OUString            aOUSource;
    OUString            aComment;
    std::unique_ptr<SbiImage> pImage;        // the Image
    SbiBreakpoints*     pBreaks;       // Breakpoints
    std::unique_ptr<SbClassData> pClassData;
    bool mbVBASupport; // Option VBASupport
    bool mbCompat; // Option Compatible
    sal_Int32 mnType;
    tools::SvRef<SbUnoObject> pDocObject; // an impl object ( used by Document Modules )
    bool    bIsProxyModule;

    SAL_DLLPRIVATE static void implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem );
    SAL_DLLPRIVATE void StartDefinitions();
    SAL_DLLPRIVATE SbMethod* GetMethod( const OUString&, SbxDataType );
    SAL_DLLPRIVATE SbProperty* GetProperty( const OUString&, SbxDataType );
    SAL_DLLPRIVATE void GetProcedureProperty( const OUString&, SbxDataType );
    SAL_DLLPRIVATE void GetIfaceMapperMethod( const OUString&, SbMethod* );
    SAL_DLLPRIVATE void EndDefinitions( bool=false );
    SAL_DLLPRIVATE void Run( SbMethod* );
    SAL_DLLPRIVATE void RunInit();
    SAL_DLLPRIVATE void ClearPrivateVars();
    SAL_DLLPRIVATE void ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic );
    SAL_DLLPRIVATE void GlobalRunInit( bool bBasicStart );  // for all modules
    SAL_DLLPRIVATE void GlobalRunDeInit();
    SAL_DLLPRIVATE const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16& ) const;
    SAL_DLLPRIVATE const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16&,
                                    bool bFollowJumps, const SbiImage* pImg=nullptr ) const;
    SAL_DLLPRIVATE virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    SAL_DLLPRIVATE virtual std::pair<bool, sal_uInt32> StoreData( SvStream& ) const override;
    SAL_DLLPRIVATE virtual bool LoadCompleted() override;
    SAL_DLLPRIVATE virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    SAL_DLLPRIVATE void handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual ~SbModule() override;
    bool IsOptionCompatible() const override { return mbCompat; }

public:
    SBX_DECL_PERSIST_NODATA(SBXID_BASICMOD,2);
                    SbModule( const OUString&, bool bVBASupport = false );
    SAL_DLLPRIVATE virtual void SetParent( SbxObject* ) override;
    SAL_DLLPRIVATE virtual void Clear() override;

    SAL_DLLPRIVATE virtual SbxVariable* Find( const OUString&, SbxClassType ) override;

    const OUString&  GetSource32() const { return aOUSource;}
    void             SetSource32( const OUString& r );

    bool Compile();
    bool IsCompiled() const;
    SAL_DLLPRIVATE const SbxObject* FindType( const OUString& aTypeName ) const;

    SAL_DLLPRIVATE bool IsBreakable( sal_uInt16 nLine ) const;
    SAL_DLLPRIVATE bool IsBP( sal_uInt16 nLine ) const;
    bool SetBP( sal_uInt16 nLine );
    bool ClearBP( sal_uInt16 nLine );
    void ClearAllBP();

    // Store only image, no source (needed for new password protection)
    SAL_DLLPRIVATE void StoreBinaryData( SvStream& );
    SAL_DLLPRIVATE void LoadBinaryData( SvStream& );
    SAL_DLLPRIVATE bool ExceedsImgVersion12ModuleSize();
    SAL_DLLPRIVATE void fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = nullptr ) const;
    SAL_DLLPRIVATE bool HasExeCode();
    bool     IsVBASupport() const { return mbVBASupport; }
    SAL_DLLPRIVATE void SetVBASupport( bool bSupport );
    sal_Int32 GetModuleType() const { return mnType; }
    void     SetModuleType( sal_Int32 nType ) { mnType = nType; }
    bool     isProxyModule() const { return bIsProxyModule; }
    SAL_DLLPRIVATE void AddVarName( const OUString& aName );
    SAL_DLLPRIVATE void RemoveVars();
    css::uno::Reference< css::script::XInvocation > const & GetUnoModule();
    bool     createCOMWrapperForIface( css::uno::Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject );
    void     GetCodeCompleteDataFromParse(CodeCompleteDataCache& aCache);
    const SbxArrayRef& GetMethods() const { return pMethods;}
    SbMethod*       FindMethod( const OUString&, SbxClassType );
    static OUString GetKeywordCase( std::u16string_view sKeyword );
};

typedef tools::SvRef<SbModule> SbModuleRef;
typedef std::vector<SbModuleRef> SbModules;

// Object class for instances of class modules
class UNLESS_MERGELIBS(BASIC_DLLPUBLIC) SbClassModuleObject final : public SbModule
{
    SbModule*   mpClassModule;
    bool        mbInitializeEventDone;

public:
    SbClassModuleObject( SbModule* pClassModule );
    virtual ~SbClassModuleObject() override;

    // Overridden to support NameAccess etc.
    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;

    virtual void Notify( SfxBroadcaster&, const SfxHint& rHint ) override;

    SbModule* getClassModule()
        { return mpClassModule; }

    void triggerInitializeEvent();
    void triggerTerminateEvent();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
