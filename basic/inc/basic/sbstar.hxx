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

#ifndef _SB_SBSTAR_HXX
#define _SB_SBSTAR_HXX

#include <basic/sbx.hxx>
#include <basic/sbxobj.hxx>
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#include <osl/mutex.hxx>

#include <basic/sbdef.hxx>
#include <basic/sberrors.hxx>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>

class SbModule;                     // completed module
class SbiInstance;                  // runtime instance
class SbiRuntime;                   // currently running procedure
class SbiImage;                     // compiled image
class BasicLibInfo;                 // info block for basic manager
class SbTextPortions;
class SbMethod;
class BasicManager;
class DocBasicItem;

class StarBASICImpl;

class StarBASIC : public SbxObject
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
    BOOL            bNoRtl;                 // if TRUE: do not search RTL
    BOOL            bBreak;                 // if TRUE: Break, otherwise Step
    BOOL            bDocBasic;
    BOOL            bVBAEnabled;
    BasicLibInfo*   pLibInfo;           // Info block for basic manager
    SbLanguageMode  eLanguageMode;      // LanguageMode of the basic object
    BOOL            bQuit;

    SbxObjectRef pVBAGlobals;
    SbxObject* getVBAGlobals( );

    void implClearDependingVarsOnDelete( StarBASIC* pDeletedBasic );

protected:
    BOOL            CError( SbError, const String&, xub_StrLen, xub_StrLen, xub_StrLen );
private:
    BOOL            RTError( SbError, xub_StrLen, xub_StrLen, xub_StrLen );
    BOOL            RTError( SbError, const String& rMsg, xub_StrLen, xub_StrLen, xub_StrLen );
    USHORT          BreakPoint( xub_StrLen nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
    USHORT          StepPoint( xub_StrLen nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;

protected:

    virtual BOOL    ErrorHdl();
    virtual USHORT  BreakHdl();
    virtual ~StarBASIC();

public:

    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASIC,1);
    TYPEINFO();

    StarBASIC( StarBASIC* pParent = NULL, BOOL bIsDocBasic = FALSE );

    // #51727 SetModified overridden so that the Modfied-State is
        // not delivered to Parent.
    virtual void SetModified( BOOL );

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
    BOOL            Compile( SbModule* );
    BOOL            Disassemble( SbModule*, String& rText );
    static void     Stop();
    static void     Error( SbError );
    static void     Error( SbError, const String& rMsg );
    static void     FatalError( SbError );
    static void     FatalError( SbError, const String& rMsg );
    static BOOL     IsRunning();
    static SbError  GetErrBasic();
    // #66536 make additional message accessible by RTL function Error
    static String   GetErrorMsg();
    static xub_StrLen GetErl();
    // Highlighting
    void            Highlight( const String& rSrc, SbTextPortions& rList );

    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual BOOL Call( const String&, SbxArray* = NULL );

    SbxArray*       GetModules() { return pModules; }
    SbxObject*      GetRtl()     { return pRtl;     }
    SbModule*       FindModule( const String& );
    // Run init code of all modules (including the inserted Doc-Basics)
    void            InitAllModules( StarBASIC* pBasicNotToInit = NULL );
    void            DeInitAllModules( void );
    void            ClearAllModuleVars( void );
    void            ActivateObject( const String*, BOOL );
    BOOL            LoadOldModules( SvStream& );

    // #43011 For TestTool; deletes global vars
    void            ClearGlobalVars( void );

    // Calls for error and break handler
    static USHORT   GetLine();
    static USHORT   GetCol1();
    static USHORT   GetCol2();
    static void     SetErrorData( SbError nCode, USHORT nLine,
                                  USHORT nCol1, USHORT nCol2 );

    // Specific to error handler
    static void     MakeErrorText( SbError, const String& aMsg );
    static const    String& GetErrorText();
    static SbError  GetErrorCode();
    static BOOL     IsCompilerError();
    static USHORT   GetVBErrorCode( SbError nError );
    static SbError  GetSfxFromVBError( USHORT nError );
    static void     SetGlobalLanguageMode( SbLanguageMode eLangMode );
    static SbLanguageMode GetGlobalLanguageMode();
    // Local settings
    void SetLanguageMode( SbLanguageMode eLangMode )
        { eLanguageMode = eLangMode; }
    SbLanguageMode GetLanguageMode();

    // Specific for break handler
    BOOL            IsBreak() const             { return bBreak; }

    static Link     GetGlobalErrorHdl();
    static void     SetGlobalErrorHdl( const Link& rNewHdl );
    Link            GetErrorHdl() const { return aErrorHdl; }
    void            SetErrorHdl( const Link& r ) { aErrorHdl = r; }

    static Link     GetGlobalBreakHdl();
    static void     SetGlobalBreakHdl( const Link& rNewHdl );
    Link            GetBreakHdl() const { return aBreakHdl; }
    void            SetBreakHdl( const Link& r ) { aBreakHdl = r; }

    SbxArrayRef     getUnoListeners( void );

    static SbxBase* FindSBXInCurrentScope( const String& rName );
    static SbxVariable* FindVarInCurrentScopy
                    ( const String& rName, USHORT& rStatus );
    static SbMethod* GetActiveMethod( USHORT nLevel = 0 );
    static SbModule* GetActiveModule();
    void SetVBAEnabled( BOOL bEnabled );
    BOOL isVBAEnabled();

    // #60175 TRUE: SFX-Resource is not displayed on basic errors
    static void StaticSuppressSfxResource( BOOL bSuppress );

    // #91147 TRUE: Reschedule is enabled (default>, FALSE: No reschedule
    static void StaticEnableReschedule( BOOL bReschedule );

    SbxObjectRef getRTL( void ) { return pRtl; }
    BOOL IsDocBasic() { return bDocBasic; }
    SbxVariable* VBAFind( const String& rName, SbxClassType t );
    bool GetUNOConstant( const sal_Char* _pAsciiName, ::com::sun::star::uno::Any& aOut );
    void QuitAndExitApplication();
    BOOL IsQuitApplication() { return bQuit; };

    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModelFromBasic( SbxObject* pBasic );
};

#ifndef __SB_SBSTARBASICREF_HXX
#define __SB_SBSTARBASICREF_HXX

SV_DECL_IMPL_REF(StarBASIC)

#endif

#endif

