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

#ifndef _SYMTBL_HXX
#define _SYMTBL_HXX

#include <vector>

class SbiConstDef;
class SbiParser;
class SbiProcDef;
class SbiStringPool;
class SbiSymDef;                    // base class

enum SbiSymScope { SbLOCAL, SbPARAM, SbPUBLIC, SbGLOBAL, SbRTL };

// The string-pool collects string entries and
// makes sure that they don't exist twice.

class SbiStringPool {
    const rtl::OUString aEmpty;
    std::vector<rtl::OUString> aData;
    SbiParser* pParser;
public:
    SbiStringPool( SbiParser* );
   ~SbiStringPool();
    sal_uInt32 GetSize() const { return aData.size(); }
    // From 8.4.1999: default changed to sal_True because of #64236 -
    // change it back to sal_False when the bug is cleanly removed.
    short Add( const rtl::OUString&, sal_Bool=sal_True );
    short Add( double, SbxDataType );
    const rtl::OUString& Find( sal_uInt32 ) const;
    SbiParser* GetParser() { return pParser; }
};


class SbiSymbols : public std::vector<SbiSymDef*>
{
public:
    ~SbiSymbols();
};

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

    void   SetParent( SbiSymPool* p )   { pParent = p;      }
    void   SetProcId( short n )         { nProcId = n;      }
    sal_uInt16 GetSize() const              { return aData.size(); }
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
    bool       bNew     : 1;        // true: Dim As New...
    bool       bChained : 1;        // true: symbol is defined in code
    bool       bByVal   : 1;        // true: ByVal-parameter
    bool       bOpt     : 1;        // true: optional parameter
    bool       bStatic  : 1;        // true: STATIC variable
    bool       bAs      : 1;        // true: data type defined per AS XXX
    bool       bGlobal  : 1;        // true: global variable
    bool       bParamArray : 1;     // true: ParamArray parameter
    bool       bWithEvents : 1;     // true: Declared WithEvents
    bool       bWithBrackets : 1;   // true: Followed by ()
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
    bool       IsDefined() const{ return bChained;  }
    void       SetOptional()    { bOpt = true;      }
    void       SetParamArray()  { bParamArray = true;       }
    void       SetWithEvents()  { bWithEvents = true;       }
    void       SetWithBrackets(){ bWithBrackets = true;     }
    void       SetByVal( bool bByVal_ = true )
                { bByVal = bByVal_; }
    void       SetStatic( bool bAsStatic = true )      { bStatic = bAsStatic;  }
    void       SetNew()         { bNew = sal_True;      }
    void       SetDefinedAs()   { bAs = sal_True;       }
    void       SetGlobal(bool b){ bGlobal = b;  }
    void       SetDefaultId( sal_uInt16 n ) { nDefaultId = n; }
    sal_uInt16     GetDefaultId( void ) { return nDefaultId; }
    bool       IsOptional() const{ return bOpt;     }
    bool       IsParamArray() const{ return bParamArray; }
    bool       IsWithEvents() const{ return bWithEvents; }
    bool       IsWithBrackets() const{ return bWithBrackets; }
    bool       IsByVal() const  { return bByVal;    }
    bool       IsStatic() const { return bStatic;   }
    bool       IsNew() const    { return bNew;      }
    bool       IsDefinedAs() const { return bAs;    }
    bool       IsGlobal() const { return bGlobal;   }
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
    bool   bPublic : 1;             // true: proc is PUBLIC
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
    void SetPublic( bool b )        { bPublic = b;     }
    bool IsPublic() const           { return bPublic;  }
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
