/*************************************************************************
 *
 *  $RCSfile: symtbl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SYMTBL_HXX
#define _SYMTBL_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SBXDEF_HXX //autogen
#include <svtools/sbxdef.hxx>
#endif

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

    USHORT Define( const String& );     // Label definieren
    USHORT Reference( const String& );  // Label referenzieren
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
    USHORT     nChain;              // Backchain-Kette
    BOOL       bNew     : 1;        // TRUE: Dim As New...
    BOOL       bChained : 1;        // TRUE: Symbol ist in Code definiert
    BOOL       bByVal   : 1;        // TRUE: ByVal-Parameter
    BOOL       bOpt     : 1;        // TRUE: optionaler Parameter
    BOOL       bStatic  : 1;        // TRUE: STATIC-Variable
    BOOL       bAs      : 1;        // TRUE: Datentyp per AS XXX definiert
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
    USHORT     GetAddr() const  { return nChain;    }
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
    void       SetByVal()       { bByVal = TRUE;    }
    void       SetStatic()      { bStatic = TRUE;   }
    void       SetNew()         { bNew = TRUE;      }
    void       SetDefinedAs()   { bAs = TRUE;       }
    BOOL       IsOptional() const{ return bOpt;     }
    BOOL       IsByVal() const  { return bByVal;    }
    BOOL       IsStatic() const { return bStatic;   }
    BOOL       IsNew() const    { return bNew;      }
    BOOL       IsDefinedAs() const { return bAs;    }

    SbiSymPool& GetPool();
    USHORT     Define();        // Symbol in Code definieren
    USHORT     Reference();     // Symbol in Code referenzieren

private:
    SbiSymDef( const SbiSymDef& );

};

class SbiProcDef : public SbiSymDef {   // Prozedur-Definition (aus Basic):
    SbiSymPool aParams;             // Parameter
    SbiSymPool aLabels;             // lokale Sprungziele
    String aLibName;                // LIB "name"
    String aAlias;                  // ALIAS "name"
    USHORT nLine1, nLine2;          // Zeilenbereich
    BOOL   bCdecl  : 1;             // TRUE: CDECL angegeben
    BOOL   bPublic : 1;             // TRUE: proc ist PUBLIC
public:
    SbiProcDef( SbiParser*, const String& );        // Name
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
    void SetLine1( USHORT n )       { nLine1 = n;      }
    USHORT GetLine1() const         { return nLine1;   }
    void SetLine2( USHORT n )       { nLine2 = n;      }
    USHORT GetLine2() const         { return nLine2;   }

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

