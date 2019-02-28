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

#include <basic/sbdef.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxdef.hxx>
#include <basic/sbx.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <deque>
#include <memory>
#include <basic/basicdllapi.h>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::script { class XInvocation; }

class SbMethod;
class SbProperty;
typedef std::deque< sal_uInt16 > SbiBreakpoints;
class SbiImage;
class SbClassModuleObject;
class CodeCompleteDataCache;


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

    BASIC_DLLPRIVATE static void implClearIfVarDependsOnDeletedBasic( SbxVariable* pVar, StarBASIC* pDeletedBasic );

    SbModule(const SbModule&) = delete;
    SbModule& operator=(const SbModule&) = delete;
protected:
    css::uno::Reference< css::script::XInvocation > mxWrapper;
    OUString            aOUSource;
    OUString            aComment;
    std::unique_ptr<SbiImage>        pImage;        // the Image
    std::unique_ptr<SbiBreakpoints>  pBreaks;       // Breakpoints
    std::unique_ptr<SbClassData>     pClassData;
    bool mbVBACompat;
    sal_Int32 mnType;
    SbxObjectRef pDocObject; // an impl object ( used by Document Modules )
    bool    bIsProxyModule;

    static void     implProcessModuleRunInit( ModuleInitDependencyMap& rMap, ClassModuleRunInitItem& rItem );
    void            StartDefinitions();
    SbMethod*       GetMethod( const OUString&, SbxDataType );
    SbProperty*     GetProperty( const OUString&, SbxDataType );
    void            GetProcedureProperty( const OUString&, SbxDataType );
    void            GetIfaceMapperMethod( const OUString&, SbMethod* );
    void            EndDefinitions( bool=false );
    void            Run( SbMethod* );
    void            RunInit();
    void            ClearPrivateVars();
    void            ClearVarsDependingOnDeletedBasic( StarBASIC* pDeletedBasic );
    void            GlobalRunInit( bool bBasicStart );  // for all modules
    void            GlobalRunDeInit();
    const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16& ) const;
    const sal_uInt8* FindNextStmnt( const sal_uInt8*, sal_uInt16&, sal_uInt16&,
                                    bool bFollowJumps, const SbiImage* pImg=nullptr ) const;
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
    virtual bool LoadCompleted() override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    void handleProcedureProperties( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual ~SbModule() override;
public:
    SBX_DECL_PERSIST_NODATA(SBXID_BASICMOD,2);
                    SbModule( const OUString&, bool bCompat = false );
    virtual void    SetParent( SbxObject* ) override;
    virtual void    Clear() override;

    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;

    const OUString&  GetSource() const;
    const OUString&  GetSource32() const { return aOUSource;}
    void             SetSource32( const OUString& r );

    bool Compile();
    bool IsCompiled() const;
    const SbxObject* FindType( const OUString& aTypeName ) const;

    bool IsBreakable( sal_uInt16 nLine ) const;
    bool IsBP( sal_uInt16 nLine ) const;
    bool SetBP( sal_uInt16 nLine );
    bool ClearBP( sal_uInt16 nLine );
    void ClearAllBP();

    // Store only image, no source (needed for new password protection)
    void     StoreBinaryData( SvStream& );
    void     LoadBinaryData( SvStream& );
    bool     ExceedsLegacyModuleSize();
    void     fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg = nullptr ) const;
    bool     HasExeCode();
    bool     IsVBACompat() const { return mbVBACompat;}
    void     SetVBACompat( bool bCompat );
    sal_Int32 GetModuleType() { return mnType; }
    void     SetModuleType( sal_Int32 nType ) { mnType = nType; }
    bool     isProxyModule() { return bIsProxyModule; }
    void     AddVarName( const OUString& aName );
    void     RemoveVars();
    css::uno::Reference< css::script::XInvocation > const & GetUnoModule();
    bool     createCOMWrapperForIface( css::uno::Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject );
    void     GetCodeCompleteDataFromParse(CodeCompleteDataCache& aCache);
    const SbxArrayRef& GetMethods() { return pMethods;}
    SbMethod*       FindMethod( const OUString&, SbxClassType );
    static OUString GetKeywordCase( const OUString& sKeyword );
};

typedef tools::SvRef<SbModule> SbModuleRef;
typedef std::vector<SbModuleRef> SbModules;

// Object class for instances of class modules
class BASIC_DLLPUBLIC SbClassModuleObject : public SbModule
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
