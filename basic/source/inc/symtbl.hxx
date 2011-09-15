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

class SbiSymDef;                    // base class
class SbiProcDef;
class SbiConstDef;
class SbiSymPool;
class SbiStringPool;

class SvStream;
class SbiParser;

enum SbiSymScope { SbLOCAL, SbPARAM, SbPUBLIC, SbGLOBAL, SbRTL };


// The string-pool collects string entries and
// makes sure that they don't exist twice.

SV_DECL_PTRARR_DEL(SbiStrings,String*,5,5)

class SbiStringPool {
    SbiStrings aData;
    String     aEmpty;              // for convenience
    SbiParser* pParser;
public:
    SbiStringPool( SbiParser* );
   ~SbiStringPool();
    sal_uInt16 GetSize() const { return aData.Count(); }
    // From 8.4.1999: default changed to sal_True because of #64236 -
    // change it back to sal_False when the bug is cleanly removed.
    short Add( const String&, sal_Bool=sal_True );
    short Add( double, SbxDataType );
    const String& Find( sal_uInt16 ) const;
    SbiParser* GetParser() { return pParser; }
};


SV_DECL_PTRARR_DEL(SbiSymbols,SbiSymDef*,5,5)

class SbiSymPool {
    friend class SbiSymDef;
    friend class SbiProcDef;
protected:
    SbiStringPool& rStrings;
    SbiSymbols  aData;
    SbiSymPool* pParent;
    SbiParser*  pParser;
    SbiSymScope eScope;
    sal_uInt16     nProcId;             // for STATIC-variable
    sal_uInt16     nCur;                // iterator
public:
    SbiSymPool( SbiStringPool&, SbiSymScope );
   ~SbiSymPool();

    void Clear();

    void   SetParent( SbiSymPool* p )   { pParent = p;      }
    void   SetProcId( short n )         { nProcId = n;      }
    sal_uInt16 GetSize() const              { return aData.Count(); }
    SbiSymScope GetScope() const        { return eScope;    }
    void   SetScope( SbiSymScope s )    { eScope = s;       }
    SbiParser* GetParser()              { return pParser;   }

    SbiSymDef* AddSym( const String& );
    SbiProcDef* AddProc( const String& );
    void Add( SbiSymDef* );
    SbiSymDef* Find( const String& ) const; // variable name
    SbiSymDef* FindId( sal_uInt16 ) const;
    SbiSymDef* Get( sal_uInt16 ) const;     // find variable per position
    SbiSymDef* First(), *Next();            // iterators

    sal_uInt32 Define( const String& );
    sal_uInt32 Reference( const String& );
    void   CheckRefs();
};


class SbiSymDef {                   // general symbol entry
    friend class SbiSymPool;
protected:
    String     aName;
    SbxDataType eType;
    SbiSymPool* pIn;                // parent pool
    SbiSymPool* pPool;              // pool for sub-elements
    short      nLen;                // string length for STRING*n
    short      nDims;
    sal_uInt16     nId;
    sal_uInt16     nTypeId;             // Dim X AS data type
    sal_uInt16     nProcId;
    sal_uInt16     nPos;
    sal_uInt32     nChain;
    sal_Bool       bNew     : 1;        // sal_True: Dim As New...
    sal_Bool       bChained : 1;        // sal_True: symbol is defined in code
    sal_Bool       bByVal   : 1;        // sal_True: ByVal-parameter
    sal_Bool       bOpt     : 1;        // sal_True: optional parameter
    sal_Bool       bStatic  : 1;        // sal_True: STATIC variable
    sal_Bool       bAs      : 1;        // sal_True: data type defined per AS XXX
    sal_Bool       bGlobal  : 1;        // sal_True: global variable
    sal_Bool       bParamArray : 1;     // sal_True: ParamArray parameter
    sal_Bool       bWithEvents : 1;     // sal_True: Declared WithEvents
    sal_Bool       bWithBrackets : 1;   // sal_True: Followed by ()
    sal_uInt16     nDefaultId;          // Symbol number of default value
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
    sal_uInt16     GetProcId() const{ return nProcId;   }
    sal_uInt32     GetAddr() const  { return nChain;    }
    sal_uInt16     GetId() const    { return nId;       }
    sal_uInt16     GetTypeId() const{ return nTypeId;   }
    void       SetTypeId( sal_uInt16 n ) { nTypeId = n; eType = SbxOBJECT; }
    sal_uInt16     GetPos() const   { return nPos;      }
    void       SetLen( short n ){ nLen = n;         }
    short      GetLen() const   { return nLen;      }
    void       SetDims( short n ) { nDims = n;      }
    short      GetDims() const  { return nDims;     }
    sal_Bool       IsDefined() const{ return bChained;  }
    void       SetOptional()    { bOpt = sal_True;      }
    void       SetParamArray()  { bParamArray = sal_True;       }
    void       SetWithEvents()  { bWithEvents = sal_True;       }
    void       SetWithBrackets(){ bWithBrackets = sal_True;     }
    void       SetByVal( sal_Bool bByVal_ = sal_True )
                { bByVal = bByVal_; }
    void       SetStatic( sal_Bool bAsStatic = sal_True )       { bStatic = bAsStatic;  }
    void       SetNew()         { bNew = sal_True;      }
    void       SetDefinedAs()   { bAs = sal_True;       }
    void       SetGlobal(sal_Bool b){ bGlobal = b;  }
    void       SetDefaultId( sal_uInt16 n ) { nDefaultId = n; }
    sal_uInt16     GetDefaultId( void ) { return nDefaultId; }
    sal_Bool       IsOptional() const{ return bOpt;     }
    sal_Bool       IsParamArray() const{ return bParamArray; }
    sal_Bool       IsWithEvents() const{ return bWithEvents; }
    sal_Bool       IsWithBrackets() const{ return bWithBrackets; }
    sal_Bool       IsByVal() const  { return bByVal;    }
    sal_Bool       IsStatic() const { return bStatic;   }
    sal_Bool       IsNew() const    { return bNew;      }
    sal_Bool       IsDefinedAs() const { return bAs;    }
    sal_Bool       IsGlobal() const { return bGlobal;   }
    short      GetFixedStringLength( void ) const { return nFixedStringLength; }
    void       SetFixedStringLength( short n ) { nFixedStringLength = n; }

    SbiSymPool& GetPool();
    sal_uInt32     Define();        // define symbol in code
    sal_uInt32     Reference();     // reference symbol in code

private:
    SbiSymDef( const SbiSymDef& );

};

class SbiProcDef : public SbiSymDef {   // procedure definition (from basic):
    SbiSymPool aParams;
    SbiSymPool aLabels;             // local jump targets
    String aLibName;
    String aAlias;
    sal_uInt16 nLine1, nLine2;      // line area
    PropertyMode mePropMode;        // Marks if this is a property procedure and which
    String maPropName;              // Property name if property procedure (!= proc name)
    sal_Bool   bCdecl  : 1;             // sal_True: CDECL given
    sal_Bool   bPublic : 1;             // sal_True: proc is PUBLIC
    sal_Bool   mbProcDecl : 1;          // sal_True: instanciated by SbiParser::ProcDecl
public:
    SbiProcDef( SbiParser*, const String&, sal_Bool bProcDecl=false );
    virtual ~SbiProcDef();
    virtual SbiProcDef* GetProcDef();
    virtual void SetType( SbxDataType );
    SbiSymPool& GetParams()         { return aParams;  }
    SbiSymPool& GetLabels()         { return aLabels;  }
    SbiSymPool& GetLocals()         { return GetPool();}
    String& GetLib()                { return aLibName; }
    String& GetAlias()              { return aAlias;   }
    void SetPublic( sal_Bool b )        { bPublic = b;     }
    sal_Bool IsPublic() const           { return bPublic;  }
    void SetCdecl( sal_Bool b = sal_True)   { bCdecl = b;      }
    sal_Bool IsCdecl() const            { return bCdecl;   }
    sal_Bool IsUsedForProcDecl() const  { return mbProcDecl; }
    void SetLine1( sal_uInt16 n )       { nLine1 = n;      }
    sal_uInt16 GetLine1() const         { return nLine1;   }
    void SetLine2( sal_uInt16 n )       { nLine2 = n;      }
    sal_uInt16 GetLine2() const         { return nLine2;   }
    PropertyMode getPropertyMode()  { return mePropMode; }
    void setPropertyMode( PropertyMode ePropMode );
    const String& GetPropName()     { return maPropName; }

    // Match with a forward-declaration. The parameter names are
    // compared and the forward declaration is replaced by this
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
