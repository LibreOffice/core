/*************************************************************************
 *
 *  $RCSfile: formel.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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


#ifndef _FORMEL_HXX
#define _FORMEL_HXX


#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _TOKSTACK_HXX
#include "tokstack.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include <global.hxx>
#endif
#ifndef SC_COMPILER_HXX
#include <compiler.hxx>
#endif


// ----- forwards --------------------------------------------------------

class SvStream;
class ScTokenArray;
class ScFormulaCell;
struct SingleRefData;
struct ComplRefData;




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
    FT_SharedFormula
};


enum BoolError { BE_NULL, BE_DIV0, BE_VALUE, BE_REF, BE_NAME, BE_NUM,
    BE_NA, BE_TRUE, BE_FALSE, BE_UNKNOWN };




//--------------------------------------------------------- class ScRangeList -

class _ScRangeList : protected List
{
private:
protected:
public:
    virtual                 ~_ScRangeList();
    inline void             Append( const ScRange& rRange );
    inline void             Append( ScRange* pRange );
    inline void             Append( const SingleRefData& rSRD );
    inline void             Append( const ComplRefData& rCRD );

    List::Count;
    inline BOOL             HasRanges( void ) const;

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


inline BOOL _ScRangeList::HasRanges( void ) const
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


inline void _ScRangeList::Append( const SingleRefData& r )
{
    List::Insert( new ScRange( r.nCol, r.nRow, r.nTab ), LIST_APPEND );
}


inline void _ScRangeList::Append( const ComplRefData& r )
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
    BOOL                        bHasRanges;
    _ScRangeList**              ppTabLists;
    _ScRangeList*               pAct;
    UINT16                      nAct;
public:
                                _ScRangeListTabs( void );
    virtual                     ~_ScRangeListTabs();

    void                        Append( SingleRefData aSRD, const BOOL bLimit = TRUE );
    void                        Append( ComplRefData aCRD, const BOOL bLimit = TRUE );

    inline BOOL                 HasRanges( void ) const;

    const ScRange*              First( const UINT16 nTab = 0 );
    const ScRange*              Next( void );
//      const ScRange*              NextContinue( void );
    inline INT32                GetActTab( void ) const;    // < 0 -> keine aktuelle Tab
    inline const _ScRangeList*  GetActList( void ) const;
};


inline BOOL _ScRangeListTabs::HasRanges( void ) const
{
    return bHasRanges;
}


inline INT32 _ScRangeListTabs::GetActTab( void ) const
{
    if( pAct )
        return nAct;
    else
        return -1;
}


inline const _ScRangeList* _ScRangeListTabs::GetActList( void ) const
{
    return pAct;
}




class ConverterBase
{
protected:
    SvStream&           aIn;
    ConvErr             eStatus;
    TokenPool           aPool;          // User Token + Predefined Token
    TokenStack          aStack;
    INT32               nBytesLeft;
    sal_Char*           pBuffer;        // Universal-Puffer
    UINT16              nBufferSize;    // ...und seine Groesse
    ScAddress           aEingPos;
    // -------------------------------------------------------------------
    inline void         Ignore( const long nSeekRel );

    inline void         Read( sal_Char& nByte );

    inline void         Read( BYTE& nByte );

    inline void         Read( UINT16& nUINT16 );

    inline void         Read( INT16& nINT16 );

    inline void         Read( double& fDouble );

                        ConverterBase( SvStream& rStr, UINT16 nNewBuffer );
    virtual             ~ConverterBase();

public:
    void                Reset( INT32 nLen );

    void                Reset( INT32 nLen, ScAddress aEingPos );

    void                Reset( ScAddress aEingPos );

    inline UINT16       GetEingabeCol( void ) const;

    inline UINT16       GetEingabeRow( void ) const;

    inline UINT16       GetEingabeTab( void ) const;

    inline ScAddress    GetEingPos( void ) const;

    virtual ConvErr     Convert( const ScTokenArray*& rpErg, INT32& nRest,
                                    const FORMULA_TYPE eFT = FT_CellFormula ) = 0;
};


inline void ConverterBase::Ignore( const long nSeekRel )
{
    aIn.SeekRel( nSeekRel );
    nBytesLeft -= nSeekRel;
}


inline void ConverterBase::Read( sal_Char& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}


inline void ConverterBase::Read( BYTE& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}


inline void ConverterBase::Read( UINT16& nUINT16 )
{
    aIn >> nUINT16;
    nBytesLeft -= 2;
}


inline void ConverterBase::Read( INT16& nINT16 )
{
    aIn >> nINT16;
    nBytesLeft -= 2;
}


inline void ConverterBase::Read( double& fDouble )
{
    aIn >> fDouble;
    nBytesLeft -= 8;
}


inline UINT16 ConverterBase::GetEingabeCol( void ) const
{
    return aEingPos.Col();
}


inline UINT16 ConverterBase::GetEingabeRow( void ) const
{
    return aEingPos.Row();
}


inline UINT16 ConverterBase::GetEingabeTab( void ) const
{
    return aEingPos.Tab();
}


inline ScAddress ConverterBase::GetEingPos( void ) const
{
    return aEingPos;
}




#endif


