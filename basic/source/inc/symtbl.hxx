/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SYMTBL_HXX
#define _SYMTBL_HXX

#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include <basic/sbxdef.hxx>
#include <basic/sbdef.hxx>

class SbiSymDef;                    // Basisklasse
class SbiProcDef;                   // Prozedur
class SbiConstDef;                  // Konstante
class SbiSymPool;                   // Symbol-Pool
class SbiStringPool;                // gepoolte Strings

class SvStream;
class SbiParser;

enum SbiSymScope { SbLOCAL, SbPARAM, SbPUBLIC, SbGLOBAL, SbRTL };

///////////////////////////////////////////////////////////////////////////

// Der String-Pool nimmt String-Eintraege auf und sorgt dafuer,
// dass sie nicht doppelt vorkommen.

SV_DECL_PTRARR_DEL(SbiStrings,String*,5,5)

class SbiStringPool {               // String-Pool
    SbiStrings aData;               // Daten
    String     aEmpty;              // for convenience
    SbiParser* pParser;             // der Parser
public:
    SbiStringPool( SbiParser* );
   ~SbiStringPool();
    USHORT GetSize() const { return aData.Count(); }
    // AB 8.4.1999, Default wegen #64236 auf TRUE geaendert
    // Wenn der Bug sauber behoben ist, wieder auf FALSE aendern.
    short Add( const String&, BOOL=TRUE );
    short Add( double, SbxDataType );
    const String& Find( USHORT ) const;
    SbiParser* GetParser() { return pParser; }
};

///////////////////////////////////////////////////////////////////////////

SV_DECL_PTRARR_DEL(SbiSymbols,SbiSymDef*,5,5)

class SbiSymPool {                  // Symbol-Pool
    friend class SbiSymDef;
    friend class SbiProcDef;
protected:
    SbiStringPool& rStrings;        // verwendeter Stringpool
    SbiSymbols  aData;              // Daten
    SbiSymPool* pParent;            // uebergeordneter Symbol-Pool
    SbiParser*  pParser;            // der Parser
    SbiSymScope eScope;             // Scope des Pools
    USHORT     nProcId;             // aktuelles ProcId fuer STATIC-Variable
    USHORT     nCur;                // Iterator
public:
    SbiSymPool( SbiStringPool&, SbiSymScope );
   ~SbiSymPool();

    void Clear();

    void   SetParent( SbiSymPool* p )   { pParent = p;      }
    void   SetProcId( short n )         { nProcId = n;      }
    USHORT GetSize() const              { return aData.Count(); }
    SbiSymScope GetScope() const        { return eScope;    }
    void   SetScope( SbiSymScope s )    { eScope = s;       }
    SbiParser* GetParser()              { return pParser;   }

    SbiSymDef* AddSym( const String& ); // Symbol hinzufuegen
    SbiProcDef* AddProc( const String& );// Prozedur hinzufuegen
    void Add( SbiSymDef* );             // Symbol uebernehmen
    SbiSymDef* Find( const String& ) const;// Variablenname
    SbiSymDef* FindId( USHORT ) const;  // Variable per ID suchen
    SbiSymDef* Get( USHORT ) const;     // Variable per Position suchen
    SbiSymDef* First(), *Next();        // Iteratoren

    UINT32 Define( const String& );     // Label definieren
    UINT32 Reference( const String& );  // Label referenzieren
    void   CheckRefs();                 // offene Referenzen suchen
};

///////////////////////////////////////////////////////////////////////////

class SbiSymDef {                   // Allgemeiner Symboleintrag
    friend class SbiSymPool;
protected:
    String     aName;               // Name des Eintrags
    SbxDataType eType;              // Typ des Eintrags
    SbiSymPool* pIn;                // Parent-Pool
    SbiSymPool* pPool;              // Pool fuer Unterelemente
    short      nLen;                // Stringlaenge bei STRING*n
    short      nDims;               // Array-Dimensionen
    USHORT     nId;                 // Symbol-Nummer
    USHORT     nTypeId;             // String-ID des Datentyps (Dim X AS Dytentyp)
    USHORT     nProcId;             // aktuelles ProcId fuer STATIC-Variable
    USHORT     nPos;                // Positions-Nummer
    UINT32     nChain;              // Backchain-Kette
    BOOL       bNew     : 1;        // TRUE: Dim As New...
    BOOL       bChained : 1;        // TRUE: Symbol ist in Code definiert
    BOOL       bByVal   : 1;        // TRUE: ByVal-Parameter
    BOOL       bOpt     : 1;        // TRUE: optionaler Parameter
    BOOL       bStatic  : 1;        // TRUE: STATIC-Variable
    BOOL       bAs      : 1;        // TRUE: Datentyp per AS XXX definiert
    BOOL       bGlobal  : 1;        // TRUE: Global-Variable
    BOOL       bParamArray : 1;     // TRUE: ParamArray parameter
    BOOL       bWithEvents : 1;     // TRUE: Declared WithEvents
    USHORT     nDefaultId;          // Symbol number of default value
    short      nFixedStringLength;  // String length in: Dim foo As String*Length
public:
    SbiSymDef( const String& );
    virtual ~SbiSymDef();
    virtual SbiProcDef* GetProcDef();
    virtual SbiConstDef* GetConstDef();

    SbxDataType GetType() const { return eType;     }
    virtual void SetType( SbxDataType );
    const String& GetName();
    SbiSymScope GetScope() const;
    USHORT     GetProcId() const{ return nProcId;   }
    UINT32     GetAddr() const  { return nChain;    }
    USHORT     GetId() const    { return nId;       }
    USHORT     GetTypeId() const{ return nTypeId;   }
    void       SetTypeId( USHORT n ) { nTypeId = n; eType = SbxOBJECT; }
    USHORT     GetPos() const   { return nPos;      }
    void       SetLen( short n ){ nLen = n;         }
    short      GetLen() const   { return nLen;      }
    void       SetDims( short n ) { nDims = n;      }
    short      GetDims() const  { return nDims;     }
    BOOL       IsDefined() const{ return bChained;  }
    void       SetOptional()    { bOpt = TRUE;      }
    void       SetParamArray()  { bParamArray = TRUE;       }
    void       SetWithEvents()  { bWithEvents = TRUE;       }
    void       SetByVal( BOOL bByVal_ = TRUE )
                { bByVal = bByVal_; }
    void       SetStatic( BOOL bAsStatic = TRUE )       { bStatic = bAsStatic;  }
    void       SetNew()         { bNew = TRUE;      }
    void       SetDefinedAs()   { bAs = TRUE;       }
    void       SetGlobal(BOOL b){ bGlobal = b;  }
    void       SetDefaultId( USHORT n ) { nDefaultId = n; }
    USHORT     GetDefaultId( void ) { return nDefaultId; }
    BOOL       IsOptional() const{ return bOpt;     }
    BOOL       IsParamArray() const{ return bParamArray; }
    BOOL       IsWithEvents() const{ return bWithEvents; }
    BOOL       IsByVal() const  { return bByVal;    }
    BOOL       IsStatic() const { return bStatic;   }
    BOOL       IsNew() const    { return bNew;      }
    BOOL       IsDefinedAs() const { return bAs;    }
    BOOL       IsGlobal() const { return bGlobal;   }
    short      GetFixedStringLength( void ) const { return nFixedStringLength; }
    void       SetFixedStringLength( short n ) { nFixedStringLength = n; }

    SbiSymPool& GetPool();
    UINT32     Define();        // Symbol in Code definieren
    UINT32     Reference();     // Symbol in Code referenzieren

private:
    SbiSymDef( const SbiSymDef& );

};

class SbiProcDef : public SbiSymDef {   // Prozedur-Definition (aus Basic):
    SbiSymPool aParams;             // Parameter
    SbiSymPool aLabels;             // lokale Sprungziele
    String aLibName;                // LIB "name"
    String aAlias;                  // ALIAS "name"
    USHORT nLine1, nLine2;          // Zeilenbereich
    PropertyMode mePropMode;        // Marks if this is a property procedure and which
    String maPropName;              // Property name if property procedure (!= proc name)
    BOOL   bCdecl  : 1;             // TRUE: CDECL angegeben
    BOOL   bPublic : 1;             // TRUE: proc ist PUBLIC
    BOOL   mbProcDecl : 1;          // TRUE: instanciated by SbiParser::ProcDecl
public:
    SbiProcDef( SbiParser*, const String&, BOOL bProcDecl=false );
    virtual ~SbiProcDef();
    virtual SbiProcDef* GetProcDef();
    virtual void SetType( SbxDataType );
    SbiSymPool& GetParams()         { return aParams;  }
    SbiSymPool& GetLabels()         { return aLabels;  }
    SbiSymPool& GetLocals()         { return GetPool();}
    String& GetLib()                { return aLibName; }
    String& GetAlias()              { return aAlias;   }
    void SetPublic( BOOL b )        { bPublic = b;     }
    BOOL IsPublic() const           { return bPublic;  }
    void SetCdecl( BOOL b = TRUE)   { bCdecl = b;      }
    BOOL IsCdecl() const            { return bCdecl;   }
    BOOL IsUsedForProcDecl() const  { return mbProcDecl; }
    void SetLine1( USHORT n )       { nLine1 = n;      }
    USHORT GetLine1() const         { return nLine1;   }
    void SetLine2( USHORT n )       { nLine2 = n;      }
    USHORT GetLine2() const         { return nLine2;   }
    PropertyMode getPropertyMode()  { return mePropMode; }
    void setPropertyMode( PropertyMode ePropMode );
    const String& GetPropName()     { return maPropName; }

    // Match mit einer Forward-Deklaration. Die Parameternamen
    // werden abgeglichen und die Forward-Deklaration wird
    // durch this ersetzt
    void Match( SbiProcDef* pForward );

private:
    SbiProcDef( const SbiProcDef& );

};

class SbiConstDef : public SbiSymDef
{
    double nVal;
    String aVal;
public:
    SbiConstDef( const String& );
    virtual ~SbiConstDef();
    virtual SbiConstDef* GetConstDef();
    void Set( double, SbxDataType );
    void Set( const String& );
    double GetValue()           { return nVal; }
    const String& GetString()   { return aVal; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
