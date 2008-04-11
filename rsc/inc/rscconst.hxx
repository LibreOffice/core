/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscconst.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _RSCCONST_HXX
#define _RSCCONST_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

/******************* R s c C o n s t *************************************/
class RscConst : public RscTop
{
protected:
    struct VarEle {
        Atom    nId;    // Name der Konstante
        INT32   lValue; // Wert der Konstante
    };
    VarEle *    pVarArray;  // Zeiger auf das Feld mit Konstanten
    sal_uInt32      nEntries;   // Anzahle der Eintraege im Feld
public:
                    RscConst( Atom nId, sal_uInt32 nTypId );
                    ~RscConst();
    virtual RSCCLASS_TYPE   GetClassType() const;
    sal_uInt32          GetEntryCount() const { return nEntries; }
                    // Die erlaubten Werte werden gesetzt
    ERRTYPE         SetConstant( Atom nVarName, INT32 lValue );
    Atom            GetConstant( sal_uInt32 nPos );
    BOOL            GetConstValue( Atom nConstId, INT32 * pVal ) const;
    BOOL            GetValueConst( INT32 nValue, Atom  * pConstId ) const;
    sal_uInt32          GetConstPos( Atom nConstId );
    virtual void    WriteSyntax( FILE * fOutput, RscTypCont * pTC );
    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c E n u m ***************************************/
class RscEnum : public RscConst {
    struct RscEnumInst {
        sal_uInt32  nValue; // Position der Konstanten im Array
        BOOL    bDflt;  // Ist Default
    };
    sal_uInt32          nSize;
public:
                    RscEnum( Atom nId, sal_uInt32 nTypId );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
    sal_uInt32          Size(){ return nSize; }

    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscEnumInst*)rInst.pData)->bDflt = TRUE;
                    }
    BOOL            IsDefault( const RSCINST & rInst )
                    {
                        return( ((RscEnumInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              INT32 nValue );
    ERRTYPE         SetNumber( const RSCINST & rInst, INT32 nValue );
    ERRTYPE         GetConst( const RSCINST & rInst, Atom * );
    ERRTYPE         GetNumber( const RSCINST & rInst, INT32 * nValue );
    void            WriteSrc( const RSCINST &rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
};

class RscNameTable;

sal_uInt32 GetLangId( const ByteString& alang);

class RscLangEnum : public RscEnum
{
    long mnLangId;
public:
    RscLangEnum();

    void Init( RscNameTable& rNames );

    Atom AddLanguage( const char* pLang, RscNameTable& rNames );
};

#endif // _RSCCONST_HXX
