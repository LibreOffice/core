/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
        sal_Int32   lValue; // Wert der Konstante
    };
    VarEle *    pVarArray;  // Zeiger auf das Feld mit Konstanten
    sal_uInt32      nEntries;   // Anzahle der Eintraege im Feld
public:
                    RscConst( Atom nId, sal_uInt32 nTypId );
                    ~RscConst();
    virtual RSCCLASS_TYPE   GetClassType() const;
    sal_uInt32          GetEntryCount() const { return nEntries; }
                    // Die erlaubten Werte werden gesetzt
    ERRTYPE         SetConstant( Atom nVarName, sal_Int32 lValue );
    Atom            GetConstant( sal_uInt32 nPos );
    sal_Bool            GetConstValue( Atom nConstId, sal_Int32 * pVal ) const;
    sal_Bool            GetValueConst( sal_Int32 nValue, Atom  * pConstId ) const;
    sal_uInt32          GetConstPos( Atom nConstId );
    virtual void    WriteSyntax( FILE * fOutput, RscTypCont * pTC );
    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c E n u m ***************************************/
class RscEnum : public RscConst {
    struct RscEnumInst {
        sal_uInt32  nValue; // Position der Konstanten im Array
        sal_Bool    bDflt;  // Ist Default
    };
    sal_uInt32          nSize;
public:
                    RscEnum( Atom nId, sal_uInt32 nTypId );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, sal_Bool );
    sal_uInt32          Size(){ return nSize; }

    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscEnumInst*)rInst.pData)->bDflt = sal_True;
                    }
    sal_Bool            IsDefault( const RSCINST & rInst )
                    {
                        return( ((RscEnumInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue );
    ERRTYPE         SetNumber( const RSCINST & rInst, sal_Int32 nValue );
    ERRTYPE         GetConst( const RSCINST & rInst, Atom * );
    ERRTYPE         GetNumber( const RSCINST & rInst, sal_Int32 * nValue );
    void            WriteSrc( const RSCINST &rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
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
