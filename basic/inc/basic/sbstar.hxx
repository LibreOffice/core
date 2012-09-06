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

#ifndef _SB_SBSTAR_HXX
#define _SB_SBSTAR_HXX

#include <basic/sbx.hxx>
#include <basic/sbxobj.hxx>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <tools/link.hxx>

#include <basic/sbdef.hxx>
#include <basic/sberrors.hxx>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "basicdllapi.h"

class SbModule;                     // completed module
class SbiInstance;                  // runtime instance
class SbiRuntime;                   // currently running procedure
class SbiImage;                     // compiled image
class BasicLibInfo;                 // info block for basic manager
class SbMethod;
class BasicManager;
class DocBasicItem;

class StarBASICImpl;

class BASIC_DLLPUBLIC StarBASIC : public SbxObject
{
    friend class SbiScanner;
    friend class SbiExpression; // Access to RTL
    friend class SbiInstance;
    friend class SbiRuntime;
    friend class DocBasicItem;

    StarBASICImpl*  mpStarBASICImpl;

    SbxArrayRef     pModules;               // List of all modules
    SbxObjectRef    pRtl;               // Runtime Library
    SbxArrayRef     xUnoListeners;          // Listener handled by CreateUnoListener

   // Handler-Support:
    Link            aErrorHdl;              // Error handler
    Link            aBreakHdl;              // Breakpoint handler
    sal_Bool            bNoRtl;                 // if sal_True: do not search RTL
    sal_Bool            bBreak;                 // if sal_True: Break, otherwise Step
    bool            bDocBasic;
    sal_Bool            bVBAEnabled;
    BasicLibInfo*   pLibInfo;           // Info block for basic manager
    sal_Bool            bQuit;

    SbxObjectRef pVBAGlobals;
    BASIC_DLLPRIVATE SbxObject* getVBAGlobals( );

    BASIC_DLLPRIVATE void implClearDependingVarsOnDelete( StarBASIC* pDeletedBasic );

protected:
    sal_Bool            CError( SbError, const ::rtl::OUString&, xub_StrLen, xub_StrLen, xub_StrLen );
private:
    BASIC_DLLPRIVATE sal_Bool           RTError( SbError, xub_StrLen, xub_StrLen, xub_StrLen );
    BASIC_DLLPRIVATE sal_Bool           RTError( SbError, const String& rMsg, xub_StrLen, xub_StrLen, xub_StrLen );
    BASIC_DLLPRIVATE sal_uInt16             BreakPoint( xub_StrLen nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
    BASIC_DLLPRIVATE sal_uInt16             StepPoint( xub_StrLen nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;

protected:

    virtual sal_Bool    ErrorHdl();
    virtual sal_uInt16  BreakHdl();
    virtual ~StarBASIC();

public:

    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASIC,1);
    TYPEINFO();

    StarBASIC( StarBASIC* pParent = NULL, bool bIsDocBasic = false );

    // #51727 SetModified overridden so that the Modfied-State is
        // not delivered to Parent.
    virtual void SetModified( sal_Bool );

    void* operator  new( size_t );
    void operator   delete( void* );

    virtual void    Insert( SbxVariable* );
    using SbxObject::Remove;
    virtual void    Remove( SbxVariable* );
    virtual void    Clear();

    BasicLibInfo*   GetLibInfo()                    { return pLibInfo;  }
    void            SetLibInfo( BasicLibInfo* p )   { pLibInfo = p;     }

    // Compiler-Interface
    SbModule*       MakeModule( const String& rName, const String& rSrc );
    SbModule*       MakeModule32( const String& rName, const ::rtl::OUString& rSrc );
    SbModule*       MakeModule32( const String& rName, const com::sun::star::script::ModuleInfo& mInfo, const ::rtl::OUString& rSrc );
    sal_Bool            Compile( SbModule* );
    static void     Stop();
    static void     Error( SbError );
    static void     Error( SbError, const String& rMsg );
    static void     FatalError( SbError );
    static void     FatalError( SbError, const String& rMsg );
    static sal_Bool     IsRunning();
    static SbError  GetErrBasic();
    // #66536 make additional message accessible by RTL function Error
    static String   GetErrorMsg();
    static xub_StrLen GetErl();

    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );
    virtual sal_Bool Call( const String&, SbxArray* = NULL );

    SbxArray*       GetModules() { return pModules; }
    SbxObject*      GetRtl()     { return pRtl;     }
    SbModule*       FindModule( const String& );
    // Run init code of all modules (including the inserted Doc-Basics)
    void            InitAllModules( StarBASIC* pBasicNotToInit = NULL );
    void            DeInitAllModules( void );
    void            ClearAllModuleVars( void );

    // Calls for error and break handler
    static sal_uInt16   GetLine();
    static sal_uInt16   GetCol1();
    static sal_uInt16   GetCol2();
    static void     SetErrorData( SbError nCode, sal_uInt16 nLine,
                                  sal_uInt16 nCol1, sal_uInt16 nCol2 );

    // Specific to error handler
    static void     MakeErrorText( SbError, const ::rtl::OUString& aMsg );
    static const    String& GetErrorText();
    static SbError  GetErrorCode();
    static bool     IsCompilerError();
    static sal_uInt16   GetVBErrorCode( SbError nError );
    static SbError  GetSfxFromVBError( sal_uInt16 nError );
    sal_Bool            IsBreak() const             { return bBreak; }

    static Link     GetGlobalErrorHdl();
    static void     SetGlobalErrorHdl( const Link& rNewHdl );
    Link            GetErrorHdl() const { return aErrorHdl; }
    void            SetErrorHdl( const Link& r ) { aErrorHdl = r; }

    static void     SetGlobalBreakHdl( const Link& rNewHdl );
    Link            GetBreakHdl() const { return aBreakHdl; }
    void            SetBreakHdl( const Link& r ) { aBreakHdl = r; }

    SbxArrayRef     getUnoListeners( void );

    static SbxBase* FindSBXInCurrentScope( const String& rName );
    static SbMethod* GetActiveMethod( sal_uInt16 nLevel = 0 );
    static SbModule* GetActiveModule();
    void SetVBAEnabled( sal_Bool bEnabled );
    sal_Bool isVBAEnabled();

    SbxObjectRef getRTL( void ) { return pRtl; }
    bool IsDocBasic() { return bDocBasic; }
    SbxVariable* VBAFind( const rtl::OUString& rName, SbxClassType t );
    bool GetUNOConstant( const sal_Char* _pAsciiName, ::com::sun::star::uno::Any& aOut );
    void QuitAndExitApplication();
    sal_Bool IsQuitApplication() { return bQuit; };

    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModelFromBasic( SbxObject* pBasic );
};

#ifndef __SB_SBSTARBASICREF_HXX
#define __SB_SBSTARBASICREF_HXX

SV_DECL_IMPL_REF(StarBASIC)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
