/*************************************************************************
 *
 *  $RCSfile: analysishelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: gt $ $Date: 2001-04-06 13:59:16 $
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

#ifndef ANALYSISHELPER_HXX
#define ANALYSISHELPER_HXX


#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>
#include <com/sun/star/util/Date.hpp>

#include <math.h>
//#include <tools/list.hxx>
//#include <tools/solmath.hxx>

#include "analysisdefs.hxx"
//#include "analysis_solmath.hxx"


using namespace com::sun::star;
using namespace rtl;


class SortedIndividualInt32List;


#define PI          3.1415926535897932
#define PI_2        (PI/2.0)
//#define   EULER       2.7182818284590452
#define EOL         ( ( const sal_Char* ) 1 )
#define EOE         ( ( const sal_Char* ) 2 )


double              _Test( sal_Int32 nMode, double f1, double f2, double f3 );
inline sal_Bool     IsLeapYear( sal_uInt16 nYear );
sal_uInt16          DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear );
sal_uInt16          DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear, sal_Bool bLeapYear );
sal_Int32           DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear );
void                DaysToDate( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear );
sal_Int32           GetNullDate( const REF( beans::XPropertySet )& xOptions ) THROWDEF_RTE;
sal_Int32           GetDiffDate360(
                        sal_uInt16 nDay1, sal_uInt16 nMonth1, sal_uInt16 nYear1, sal_Bool bLeapYear1,
                        sal_uInt16 nDay2, sal_uInt16 nMonth2, sal_uInt16 nYear2,
                        sal_Bool bUSAMethod );
sal_Int32           GetDaysInYears( sal_uInt16 nYear1, sal_uInt16 nYear2 );
inline sal_Int16    GetDayOfWeek( sal_Int32 nDate );
double              Fak( sal_Int32 n );
double              GetGcd( double f1, double f2 );
double              GammaHelp( double& x, sal_Bool& bReflect );
double              Gamma( double f );
double              GammaN( double f, sal_uInt32 nIter );
double              Bessel( double fNum, sal_Int32 nOrder, sal_Bool bModfied ) THROWDEF_RTE;
double              BesselR( double fNum, double fOrder ) THROWDEF_RTE;
double              ConvertToDec( const STRING& rFromNum, sal_uInt16 nBaseFrom, sal_uInt16 nCharLim ) THROWDEF_RTE_IAE;
STRING              ConvertFromDec(
                        sal_Int64 nNum, sal_Int64 nMin, sal_Int64 nMax, sal_uInt16 nBase,
                        sal_Int32 nPlaces, sal_Int32 nMaxPlaces ) THROWDEF_RTE_IAE;
double              Erf( double fX );
sal_Bool            ParseDouble( const sal_Unicode*& rpDoubleAsString, double& rReturn );
STRING              GetString( double fNumber, sal_Bool bLeadingSign = sal_False, sal_uInt16 nMaxNumOfDigits = 15 );
inline double       Exp10( sal_Int16 nPower );      // 10 ^ nPower




class List
{
private:
    static const sal_uInt32 nStartSize;
    static const sal_uInt32 nIncrSize;

    void**                  pData;          // pointer array
    sal_uInt32              nSize;          // array size
    sal_uInt32              nNew;           // next index to be inserted at
    sal_uInt32              nAct;           // actual for iterations

    void                    _Grow( void );
    inline void             Grow( void );
protected:
public:
                            List( void );
    virtual                 ~List();

    inline const void*      GetObject( sal_uInt32 nIndex ) const;
    inline const void*      First( void );
    inline const void*      Next( void );

    inline void             Append( void* pNewElement );
    void                    Insert( void* pNewLement, sal_uInt32 nPlace );

    inline sal_uInt32       Count( void ) const;
};




class FuncData
{
public:
    const sal_Char*         pIntName;
    const sal_Char*         pGerman;
    const sal_Char*         pEnglish;
    const sal_Char*         pDescr;
private:
    const sal_Char**        ppParam;        // list with naming of the parameters
    sal_uInt32              nParam;         // len of ~
    const sal_Char**        ppParamDescr;   // list with description the parameters
    sal_uInt32              nParamDescr;    // len of ~

public:
                            FuncData( void );
    virtual                 ~FuncData();
    static FuncData*        CloneFromList( const sal_Char**& rppStringList );
                                        // returns the first ** to first string after actual readed

    const sal_Char*         GetParam( sal_uInt32 nInd ) const;
    const sal_Char*         GetParamDescr( sal_uInt32 nInd ) const;
};




class CStrList : private List
{
public:
    inline void             Append( const sal_Char* pNew );
    inline const sal_Char*  Get( sal_uInt32 nIndex ) const;
    List::Count;
};




class FuncDataList : private List
{
    OUString                aLastName;
    sal_uInt32              nLast;
public:
                            FuncDataList( const sal_Char** pFuncDatas );
    virtual                 ~FuncDataList();
    inline void             Append( FuncData* pNew );
    inline const FuncData*  Get( sal_uInt32 nIndex ) const;
    List::Count;

//  AnalysisFunc            GetFunc( const OUString& aProgrammaticName ) const;
    const FuncData*         Get(  const OUString& aProgrammaticName ) const;
};




class SortedIndividualInt32List : private List
{
    // sorted list were values are unique
private:
protected:
public:
                        SortedIndividualInt32List();
    virtual             ~SortedIndividualInt32List();

    void                Insert( sal_Int32 nNewVal );

    List::Count;

    inline sal_Int32    Get( sal_uInt32 nIndex ) const; // reterns a value in every situation,
                                                        //  even if nIndex is invalid,
                                                        //  so care about nIndex _before_ using Get()

    sal_Int32           CountCondition( sal_Int32 nMinVal, sal_Int32 nMaxVal ) const;
                                                        // count number of elements, which are in the range
                                                        //  of nMinVal ... nMaxVal (both included)
    sal_Bool            Find( sal_Int32 nVal ) const;

    void                InsertHolidayList(
                            const SEQSEQ( sal_Int32 )& aHDay,
                            sal_Int32 nNullDate, sal_Bool bInsertAlsoOnWeekends );
};




class DoubleList : protected List
{
protected:
    inline void             _Append( double fVal );
public:
    virtual                 ~DoubleList();


    inline const double*    Get( sal_uInt32 nIndex ) const;
    inline const double*    First( void );
    inline const double*    Next( void );

    List::Count;

    inline sal_Bool         Append( double fVal );
    sal_Bool                Append( const SEQSEQ( double )& aValList );
                            // return = FALSE if one or more values don't match IsValid()
                            //  but, even if an error occur, the list might be changed!
    virtual sal_Bool        IsProper( double fVal ) const;
    virtual sal_Bool        IsFaulty( double fVal ) const;
};




class ChkDoubleList1 : public DoubleList
{
public:
    virtual sal_Bool        IsProper( double fVal ) const;
    virtual sal_Bool        IsFaulty( double fVal ) const;
};




class Complex
{
    double                  r;
    double                  i;

public:
//  inline                  Complex( void );
    inline                  Complex( double fReal, double fImag = 0.0 );
    inline                  Complex( const Complex& rCopy );
                            Complex( const STRING& rComplexAsString ) THROWDEF_RTE_IAE;

    static sal_Bool         ParseString( const STRING& rComplexAsString, Complex& rReturn );
    STRING                  GetString( sal_Bool bUse_i = sal_True ) const;

    inline double           Real( void ) const;
    inline double           Imag( void ) const;

    double                  Arg( void ) const THROWDEF_RTE_IAE;
    inline double           Abs( void ) const;

    // following functions change the complex number itself to avoid unnecessary copy actions!
    void                    Power( double fPower ) THROWDEF_RTE_IAE;
    void                    Sin( void ) THROWDEF_RTE_IAE;
    void                    Cos( void ) THROWDEF_RTE_IAE;
    void                    Div( const Complex& rDivisor ) THROWDEF_RTE_IAE;
    void                    Exp( void );
    inline void             Conjugate( void );
    void                    Ln( void ) THROWDEF_RTE_IAE;
    void                    Log10( void ) THROWDEF_RTE_IAE;
    void                    Log2( void ) THROWDEF_RTE_IAE;
    inline void             Mult( double fFact );
    inline void             Mult( const Complex& rMult );
    inline void             Sub( const Complex& rMult );
    inline void             Add( const Complex& rAdd );
};




class ComplexList : protected List
{
public:
    virtual                 ~ComplexList();

    inline const Complex*   Get( sal_uInt32 nIndex ) const;
    inline const Complex*   First( void );
    inline const Complex*   Next( void );

    List::Count;

    inline void             Append( Complex* pNew );
    void                    Append( const SEQSEQ( STRING )& rComplexNumList ) THROWDEF_RTE_IAE;
};




enum ConvertDataClass
{
    CDC_Mass, CDC_Length, CDC_Time, CDC_Pressure, CDC_Force, CDC_Energy, CDC_Power, CDC_Magnetism,
    CDC_Temperature, CDC_Volume, CDC_Area, CDC_Speed
};


#define INV_MATCHLEV        1764                    // guess, what this is... :-)


class ConvertDataList;




class ConvertData
{
protected:
    friend ConvertDataList;
    double                  fConst;
    STRING                  aName;
    ConvertDataClass        eClass;
public:
                            ConvertData(
                                const sal_Char      pUnitName[],
                                double              fConvertConstant,
                                ConvertDataClass    eClass );

    sal_Int16               GetMatchingLevel( const STRING& rRef ) const;
                                    // 0.0 = no equality
                                    // 1.0 = matches exact
                                    // rest = matches without an assumed prefix of one character
                                    //  rest gives power for 10 represented by the prefix (e.g. 3 for k or -9 for n

    virtual double          Convert( double fVal, const ConvertData& rTo,
                                sal_Int16 nMatchLevelFrom, sal_Int16 nMatchLevelTo ) const THROWDEF_RTE_IAE;
                                    // converts fVal from this unit to rFrom unit
                                    // throws exception if not from same class
                                    // this implementation is for proportional cases only
    virtual double          ConvertToBase( double fVal, sal_Int16 nMatchLevel ) const;
    virtual double          ConvertFromBase( double fVal, sal_Int16 nMatchLevel ) const;

    inline ConvertDataClass Class( void ) const;
};




class ConvertDataLinear : public ConvertData
{
protected:
    double                  fOffs;
public:
    inline                  ConvertDataLinear(
                                const sal_Char      pUnitName[],
                                double              fConvertConstant,
                                double              fConvertOffset,
                                ConvertDataClass    eClass );

    virtual double          Convert( double fVal, const ConvertData& rTo,
                                sal_Int16 nMatchLevelFrom, sal_Int16 nMatchLevelTo ) const THROWDEF_RTE_IAE;
                                    // for cases where f(x) = a + bx applies (e.g. Temperatures)

    virtual double          ConvertToBase( double fVal, sal_Int16 nMatchLevel ) const;
    virtual double          ConvertFromBase( double fVal, sal_Int16 nMatchLevel ) const;
};




class ConvertDataList : protected List
{
private:
protected:
    inline ConvertData*     First( void );
    inline ConvertData*     Next( void );
public:
                            ConvertDataList( void );
    virtual                 ~ConvertDataList();

    double                  Convert( double fVal, const STRING& rFrom, const STRING& rTo ) THROWDEF_RTE_IAE;
};




inline sal_Bool IsLeapYear( sal_uInt16 n )
{
    return ( ( ( n % 4 ) == 0 ) && ( ( n % 100 ) != 0) || ( ( n % 400 ) == 0 ) );
}


inline sal_Int16 GetDayOfWeek( sal_Int32 n )
{   // monday = 0, ..., sunday = 6
    return ( n - 1 ) % 7;
}




inline void List::Grow( void )
{
    if( nNew >= nSize )
        _Grow();
}


inline const void* List::GetObject( sal_uInt32 n ) const
{
    if( n < nNew )
        return pData[ n ];
    else
        return NULL;
}


inline const void* List::First( void )
{
    nAct = 0;
    if( nNew )
        return pData[ 0 ];
    else
        return NULL;
}


inline const void* List::Next( void )
{
    nAct++;
    if( nAct < nNew )
        return pData[ nAct ];
    else
    {
        nAct--;
        return NULL;
    }
}


inline void List::Append( void* p )
{
    Grow();
    pData[ nNew ] = p;
    nNew++;
}


inline sal_uInt32 List::Count( void ) const
{
    return nNew;
}




inline void CStrList::Append( const sal_Char* p )
{
//  List::Insert( ( void* ) p, LIST_APPEND );
    List::Append( ( void* ) p );
}


inline const sal_Char* CStrList::Get( sal_uInt32 n ) const
{
    return ( const sal_Char* ) List::GetObject( n );
}




inline void FuncDataList::Append( FuncData* p )
{
//  List::Insert( p, LIST_APPEND );
    List::Append( p );
}


inline const FuncData* FuncDataList::Get( sal_uInt32 n ) const
{
    return ( const FuncData* ) List::GetObject( n );
}




inline sal_Int32 SortedIndividualInt32List::Get( sal_uInt32 n ) const
{
    return ( sal_Int32 ) List::GetObject( n );
}




inline void DoubleList::_Append( double f )
{
//  List::Insert( new double( f ), LIST_APPEND );
    List::Append( new double( f ) );
}


inline const double* DoubleList::Get( sal_uInt32 n ) const
{
    return ( const double* ) List::GetObject( n );
}


inline const double* DoubleList::First( void )
{
    return ( const double* ) List::First();
}


inline const double* DoubleList::Next( void )
{
    return ( const double* ) List::Next();
}


inline sal_Bool DoubleList::Append( double f )
{
    if( IsFaulty( f ) )
        return sal_False;
    if( IsProper( f ) )
        _Append( f );
    return sal_True;
}




/*inline Complex::Complex( void )
{
//#ifdef DEBUG
//  SetNAN( i );
//  r = i;
//#endif
}*/


inline Complex::Complex( double fReal, double fImag ) : r( fReal ), i( fImag )
{
}


inline Complex::Complex( const Complex& rCpy ) : r( rCpy.r ), i( rCpy.i )
{
}


inline double Complex::Real( void ) const
{
    return r;
}


inline double Complex::Imag( void ) const
{
    return i;
}


inline double Complex::Abs( void ) const
{
    return sqrt( r * r + i * i );
}


void Complex::Conjugate( void )
{
    i = -i;
}


inline void Complex::Mult( double f )
{
    i *= f;
    r *= f;
}


inline void Complex::Mult( const Complex& rM )
{
    double  r_ = r;
    double  i_ = i;

    r = r_ * rM.r - i_ * rM.i;
    i = r_ * rM.i + i_ * rM.r;
}


inline void Complex::Sub( const Complex& rC )
{
    r -= rC.r;
    i -= rC.i;
}


inline void Complex::Add( const Complex& rAdd )
{
    r += rAdd.r;
    i += rAdd.i;
}




inline const Complex* ComplexList::Get( sal_uInt32 n ) const
{
    return ( const Complex* ) List::GetObject( n );
}


inline const Complex* ComplexList::First( void )
{
    return ( const Complex* ) List::First();
}


inline const Complex* ComplexList::Next( void )
{
    return ( const Complex* ) List::Next();
}


inline void ComplexList::Append( Complex* p )
{
//  List::Insert( p, LIST_APPEND );
    List::Append( p );
}




inline ConvertDataClass ConvertData::Class( void ) const
{
    return eClass;
}




inline ConvertDataLinear::ConvertDataLinear( const sal_Char* p, double fC, double fO, ConvertDataClass e ) :
    ConvertData( p, fC, e ),
    fOffs( fO )
{
}




inline ConvertData* ConvertDataList::First( void )
{
    return ( ConvertData* ) List::First();
}


inline ConvertData* ConvertDataList::Next( void )
{
    return ( ConvertData* ) List::Next();
}


#endif

