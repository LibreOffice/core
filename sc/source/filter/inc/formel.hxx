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



#ifndef SC_FORMEL_HXX
#define SC_FORMEL_HXX

#include <tools/solar.h>
#include <tools/list.hxx>
#include <tools/string.hxx>
#include "tokstack.hxx"
#include "root.hxx"
#include <global.hxx>
#include <compiler.hxx>


// ----- forwards --------------------------------------------------------

class XclImpStream;
class ScTokenArray;
class ScFormulaCell;
struct ScSingleRefData;
struct ScComplexRefData;




//------------------------------------------------------------------------

enum ConvErr
{
    ConvOK = 0,
    ConvErrNi,      // nicht implemntierter/unbekannter Opcode aufgetreten
    ConvErrNoMem,   // Fehler beim Speicheranfordern
    ConvErrExternal,// Add-Ins aus Excel werden nicht umgesetzt
    ConvErrCount    // Nicht alle Bytes der Formel 'erwischt'
};


enum FORMULA_TYPE
{
    FT_CellFormula,
    FT_RangeName,
    FT_SharedFormula,
    FT_Conditional
};




//--------------------------------------------------------- class ScRangeList -

class _ScRangeList : protected List
{
private:
protected:
public:
    virtual                 ~_ScRangeList();
    inline void             Append( const ScRange& rRange );
    inline void             Append( ScRange* pRange );
    inline void             Append( const ScSingleRefData& rSRD );
    inline void             Append( const ScComplexRefData& rCRD );

    using                   List::Count;
    inline sal_Bool             HasRanges( void ) const;

    inline const ScRange*   First( void );
    inline const ScRange*   Next( void );
};


inline void _ScRangeList::Append( const ScRange& r )
{
    List::Insert( new ScRange( r ), LIST_APPEND );
}


inline void _ScRangeList::Append( ScRange* p )
{
    List::Insert( p, LIST_APPEND );
}


inline sal_Bool _ScRangeList::HasRanges( void ) const
{
    return Count() > 0;
}


inline const ScRange* _ScRangeList::First( void )
{
    return ( const ScRange* ) List::First();
}


inline const ScRange* _ScRangeList::Next( void )
{
    return ( const ScRange* ) List::Next();
}


inline void _ScRangeList::Append( const ScSingleRefData& r )
{
    List::Insert( new ScRange( r.nCol, r.nRow, r.nTab ), LIST_APPEND );
}


inline void _ScRangeList::Append( const ScComplexRefData& r )
{
    List::Insert(   new ScRange(    r.Ref1.nCol, r.Ref1.nRow, r.Ref1.nTab,
                                    r.Ref2.nCol, r.Ref2.nRow, r.Ref2.nTab ),
                    LIST_APPEND );
}




//----------------------------------------------------- class ScRangeListTabs -

class _ScRangeListTabs
{
private:
protected:
    sal_Bool                        bHasRanges;
    _ScRangeList**              ppTabLists;
    _ScRangeList*               pAct;
    sal_uInt16                      nAct;
public:
                                _ScRangeListTabs( void );
    virtual                     ~_ScRangeListTabs();

    void                        Append( ScSingleRefData aSRD, const sal_Bool bLimit = sal_True );
    void                        Append( ScComplexRefData aCRD, const sal_Bool bLimit = sal_True );

    inline sal_Bool                 HasRanges( void ) const;

    const ScRange*              First( const sal_uInt16 nTab = 0 );
    const ScRange*              Next( void );
//      const ScRange*              NextContinue( void );
    inline const _ScRangeList*  GetActList( void ) const;
};


inline sal_Bool _ScRangeListTabs::HasRanges( void ) const
{
    return bHasRanges;
}


inline const _ScRangeList* _ScRangeListTabs::GetActList( void ) const
{
    return pAct;
}




class ConverterBase
{
protected:
    TokenPool           aPool;          // User Token + Predefined Token
    TokenStack          aStack;
    ScAddress           aEingPos;
    ConvErr             eStatus;
    sal_Char*           pBuffer;        // Universal-Puffer
    sal_uInt16              nBufferSize;    // ...und seine Groesse

                        ConverterBase( sal_uInt16 nNewBuffer );
    virtual             ~ConverterBase();

    void                Reset();

public:
    inline SCCOL        GetEingabeCol( void ) const { return aEingPos.Col(); }
    inline SCROW        GetEingabeRow( void ) const { return aEingPos.Row(); }
    inline SCTAB        GetEingabeTab( void ) const { return aEingPos.Tab(); }
    inline ScAddress    GetEingPos( void ) const    { return aEingPos; }
};



class ExcelConverterBase : public ConverterBase
{
protected:
                        ExcelConverterBase( sal_uInt16 nNewBuffer );
    virtual             ~ExcelConverterBase();

public:
    void                Reset();
    void                Reset( const ScAddress& rEingPos );

    virtual ConvErr     Convert( const ScTokenArray*& rpErg, XclImpStream& rStrm, sal_Size nFormulaLen,
                                 bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula ) = 0;
    virtual ConvErr     Convert( _ScRangeListTabs&, XclImpStream& rStrm, sal_Size nFormulaLen,
                                    const FORMULA_TYPE eFT = FT_CellFormula ) = 0;
};



class LotusConverterBase : public ConverterBase
{
protected:
    SvStream&           aIn;
    sal_Int32               nBytesLeft;

    inline void         Ignore( const long nSeekRel );
    inline void         Read( sal_Char& nByte );
    inline void         Read( sal_uInt8& nByte );
    inline void         Read( sal_uInt16& nUINT16 );
    inline void         Read( sal_Int16& nINT16 );
    inline void         Read( double& fDouble );
        inline void                     Read( sal_uInt32& nUINT32 );

                        LotusConverterBase( SvStream& rStr, sal_uInt16 nNewBuffer );
    virtual             ~LotusConverterBase();

public:
//UNUSED2008-05  void                Reset( sal_Int32 nLen );
//UNUSED2008-05  void                Reset( sal_Int32 nLen, const ScAddress& rEingPos );
    void                Reset( const ScAddress& rEingPos );

    virtual ConvErr     Convert( const ScTokenArray*& rpErg, sal_Int32& nRest,
                                    const FORMULA_TYPE eFT = FT_CellFormula ) = 0;

protected:
    using               ConverterBase::Reset;
};


inline void LotusConverterBase::Ignore( const long nSeekRel )
{
    aIn.SeekRel( nSeekRel );
    nBytesLeft -= nSeekRel;
}

inline void LotusConverterBase::Read( sal_Char& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}

inline void LotusConverterBase::Read( sal_uInt8& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}

inline void LotusConverterBase::Read( sal_uInt16& nUINT16 )
{
    aIn >> nUINT16;
    nBytesLeft -= 2;
}

inline void LotusConverterBase::Read( sal_Int16& nINT16 )
{
    aIn >> nINT16;
    nBytesLeft -= 2;
}

inline void LotusConverterBase::Read( double& fDouble )
{
    aIn >> fDouble;
    nBytesLeft -= 8;
}

inline void LotusConverterBase::Read( sal_uInt32& nUINT32 )
{
    aIn >> nUINT32;
    nBytesLeft -= 4;
}

#endif


