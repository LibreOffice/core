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

#ifndef INCLUDED_BASIC_SBSTAR_HXX
#define INCLUDED_BASIC_SBSTAR_HXX

#include <basic/sbx.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbmod.hxx>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <tools/link.hxx>

#include <basic/sbdef.hxx>
#include <basic/sberrors.hxx>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <basic/basicdllapi.h>

class SbiInstance;                  // runtime instance
class SbiRuntime;                   // currently running procedure
class SbiImage;                     // compiled image
class BasicLibInfo;                 // info block for basic manager
class SbMethod;
class BasicManager;
class DocBasicItem;

class BASIC_DLLPUBLIC StarBASIC : public SbxObject
{
    friend class SbiScanner;
    friend class SbiExpression; // Access to RTL
    friend class SbiInstance;
    friend class SbiRuntime;
    friend class DocBasicItem;

    SbModules       pModules;               // List of all modules
    SbxObjectRef    pRtl;               // Runtime Library
    SbxArrayRef     xUnoListeners;          // Listener handled by CreateUnoListener

   // Handler-Support:
    Link<StarBASIC*,bool>       aErrorHdl;              // Error handler
    Link<StarBASIC*,sal_uInt16> aBreakHdl;              // Breakpoint handler
    bool            bNoRtl;                 // if true: do not search RTL
    bool            bBreak;                 // if true: Break, otherwise Step
    bool            bDocBasic;
    bool            bVBAEnabled;
    BasicLibInfo*   pLibInfo;           // Info block for basic manager
    bool            bQuit;

    SbxObjectRef pVBAGlobals;
    BASIC_DLLPRIVATE SbxObject* getVBAGlobals( );

    BASIC_DLLPRIVATE void implClearDependingVarsOnDelete( StarBASIC* pDeletedBasic );

protected:
    bool                                CError( SbError, const OUString&, sal_Int32, sal_Int32, sal_Int32 );
private:
    BASIC_DLLPRIVATE void               RTError( SbError, sal_Int32, sal_Int32, sal_Int32 );
    BASIC_DLLPRIVATE bool               RTError( SbError, const OUString& rMsg, sal_Int32, sal_Int32, sal_Int32 );
    BASIC_DLLPRIVATE sal_uInt16         BreakPoint( sal_Int32 nLine, sal_Int32 nCol1, sal_Int32 nCol2 );
    BASIC_DLLPRIVATE sal_uInt16         StepPoint( sal_Int32 nLine, sal_Int32 nCol1, sal_Int32 nCol2 );
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;

protected:
    bool        ErrorHdl();
    sal_uInt16  BreakHdl();
    virtual ~StarBASIC();

public:

    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASIC,1);

    StarBASIC( StarBASIC* pParent = nullptr, bool bIsDocBasic = false );

    // #51727 SetModified overridden so that the Modfied-State is
        // not delivered to Parent.
    virtual void SetModified( bool ) override;

    virtual void    Insert( SbxVariable* ) override;
    using SbxObject::Remove;
    virtual void    Remove( SbxVariable* ) override;
    virtual void    Clear() override;

    // Compiler-Interface
    SbModule*       MakeModule( const OUString& rName, const OUString& rSrc );
    SbModule*       MakeModule( const OUString& rName, const css::script::ModuleInfo& mInfo, const OUString& rSrc );
    static void     Stop();
    static void     Error( SbError );
    static void     Error( SbError, const OUString& rMsg );
    static void     FatalError( SbError );
    static void     FatalError( SbError, const OUString& rMsg );
    static bool     IsRunning();
    static SbError  GetErrBasic();
    // #66536 make additional message accessible by RTL function Error
    static OUString GetErrorMsg();
    static sal_Int32 GetErl();

    virtual SbxVariable* Find( const OUString&, SbxClassType ) override;
    virtual bool Call( const OUString&, SbxArray* = nullptr ) override;

    SbModules&      GetModules() { return pModules; }
    SbxObject*      GetRtl()     { return pRtl;     }
    SbModule*       FindModule( const OUString& );
    // Run init code of all modules (including the inserted Doc-Basics)
    void            InitAllModules( StarBASIC* pBasicNotToInit = nullptr );
    void            DeInitAllModules();
    void            ClearAllModuleVars();

    // Calls for error and break handler
    static sal_uInt16 GetLine();
    static sal_uInt16 GetCol1();
    static sal_uInt16 GetCol2();
    static void     SetErrorData( SbError nCode, sal_uInt16 nLine,
                                  sal_uInt16 nCol1, sal_uInt16 nCol2 );

    // Specific to error handler
    static void     MakeErrorText( SbError, const OUString& aMsg );
    static const    OUString& GetErrorText();
    static SbError  GetErrorCode();
    static bool     IsCompilerError();
    static sal_uInt16 GetVBErrorCode( SbError nError );
    static SbError  GetSfxFromVBError( sal_uInt16 nError );
    bool            IsBreak() const             { return bBreak; }

    static Link<StarBASIC*,bool> GetGlobalErrorHdl();
    static void     SetGlobalErrorHdl( const Link<StarBASIC*,bool>& rNewHdl );

    static void     SetGlobalBreakHdl( const Link<StarBASIC*,sal_uInt16>& rNewHdl );

    SbxArrayRef const & getUnoListeners();

    static SbxBase* FindSBXInCurrentScope( const OUString& rName );
    static SbMethod* GetActiveMethod( sal_uInt16 nLevel = 0 );
    static SbModule* GetActiveModule();
    void SetVBAEnabled( bool bEnabled );
    bool isVBAEnabled();

    const SbxObjectRef& getRTL() { return pRtl; }
    bool IsDocBasic() { return bDocBasic; }
    SbxVariable* VBAFind( const OUString& rName, SbxClassType t );
    bool GetUNOConstant( const OUString& rName, css::uno::Any& aOut );
    void QuitAndExitApplication();
    bool IsQuitApplication() { return bQuit; };

    static css::uno::Reference< css::frame::XModel >
        GetModelFromBasic( SbxObject* pBasic );

    static void DetachAllDocBasicItems();
};

typedef tools::SvRef<StarBASIC> StarBASICRef;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
