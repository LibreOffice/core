/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbstar.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 09:58:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SB_SBSTAR_HXX
#define _SB_SBSTAR_HXX

#ifndef _SBXCLASS_HXX //autogen
#include <basic/sbx.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <basic/sbdef.hxx>
#include <basic/sberrors.hxx>

class SbModule;                     // fertiges Modul
class SbiInstance;                  // Laufzeit-Instanz
class SbiRuntime;                   // aktuell laufende Prozedur
class SbiImage;                     // compiliertes Image
class BasicLibInfo;                 // Infoblock fuer Basic-Manager
class SbiBreakpoints;
class SbTextPortions;
class SbMethod;
class BasicManager;

class StarBASICImpl;

class StarBASIC : public SbxObject
{
    friend class SbiScanner;
    friend class SbiExpression;         // Zugriff auf RTL
    friend class SbiInstance;
    friend class SbiRuntime;

    StarBASICImpl*  mpStarBASICImpl;

    SbxArrayRef     pModules;           // Liste aller Module
    SbxObjectRef    pRtl;               // Runtime Library
    SbxArrayRef     xUnoListeners;      // Listener handled by CreateUnoListener
                                        // Handler-Support:
    Link            aErrorHdl;          // Fehlerbehandlung
    Link            aBreakHdl;          // Breakpoint-Handler
    BOOL            bNoRtl;             // TRUE: RTL nicht durchsuchen
    BOOL            bBreak;             // TRUE: Break, sonst Step
    BOOL            bDocBasic;
    BasicLibInfo*   pLibInfo;           // Infoblock fuer Basic-Manager
    SbLanguageMode  eLanguageMode;      // LanguageMode des Basic-Objekts
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

    // #51727 SetModified ueberladen, damit der Modified-
    // Zustand nicht an den Parent weitergegeben wird.
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
    BOOL            Compile( SbModule* );
    BOOL            Disassemble( SbModule*, String& rText );
    static void     Stop();
    static void     Error( SbError );
    static void     Error( SbError, const String& rMsg );
    static void     FatalError( SbError );
    static BOOL     IsRunning();
    static SbError  GetErrBasic();
    // #66536 Zusatz-Message fuer RTL-Funktion Error zugreifbar machen
    static String   GetErrorMsg();
    static xub_StrLen GetErl();
    // Highlighting
    void            Highlight( const String& rSrc, SbTextPortions& rList );

    virtual SbxVariable* Find( const String&, SbxClassType );
    virtual BOOL Call( const String&, SbxArray* = NULL );

    SbxArray*       GetModules() { return pModules; }
    SbxObject*      GetRtl()     { return pRtl;     }
    SbModule*       FindModule( const String& );
    // Init-Code aller Module ausfuehren (auch in inserteten Doc-Basics)
    void            InitAllModules( StarBASIC* pBasicNotToInit = NULL );
    void            DeInitAllModules( void );
    void            ClearAllModuleVars( void );
    void            ActivateObject( const String*, BOOL );
    BOOL            LoadOldModules( SvStream& );

    // #43011 Fuer das TestTool, um globale Variablen loeschen zu koennen
    void            ClearGlobalVars( void );

    // Abfragen fuer den Error-Handler und den Break-Handler:
    static USHORT   GetLine();
    static USHORT   GetCol1();
    static USHORT   GetCol2();
    static void     SetErrorData( SbError nCode, USHORT nLine,
                                  USHORT nCol1, USHORT nCol2 );

    // Spezifisch fuer den Error-Handler:
    static void     MakeErrorText( SbError, const String& aMsg );
    static const    String& GetErrorText();
    static SbError  GetErrorCode();
    static BOOL     IsCompilerError();
    static USHORT   GetVBErrorCode( SbError nError );
    static SbError  GetSfxFromVBError( USHORT nError );
    static void     SetGlobalLanguageMode( SbLanguageMode eLangMode );
    static SbLanguageMode GetGlobalLanguageMode();
    // Lokale Einstellung
    void SetLanguageMode( SbLanguageMode eLangMode )
        { eLanguageMode = eLangMode; }
    SbLanguageMode GetLanguageMode();

    // Spezifisch fuer den Break-Handler:
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

    // #60175 TRUE: SFX-Resource wird bei Basic-Fehlern nicht angezogen
    static void StaticSuppressSfxResource( BOOL bSuppress );

    // #91147 TRUE: Reschedule is enabled (default>, FALSE: No reschedule
    static void StaticEnableReschedule( BOOL bReschedule );

    SbxObjectRef getRTL( void ) { return pRtl; }
    BOOL IsDocBasic() { return bDocBasic; }
};

#ifndef __SB_SBSTARBASICREF_HXX
#define __SB_SBSTARBASICREF_HXX

SV_DECL_IMPL_REF(StarBASIC)

#endif

#endif

