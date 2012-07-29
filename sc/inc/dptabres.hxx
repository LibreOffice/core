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

#ifndef SC_DPTABRES_HXX
#define SC_DPTABRES_HXX

#include "global.hxx"
#include "dpcachetable.hxx"

#include <tools/string.hxx>
#include <com/sun/star/sheet/MemberResult.hpp>
#include <com/sun/star/sheet/DataResult.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <vector>
#include <memory>
#include <map>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
} } } }


class ScAddress;
class ScDocument;
class ScDPSource;
class ScDPDimension;
class ScDPLevel;
class ScDPMember;
class ScDPAggData;
class ScDPResultMember;
class ScDPResultVisibilityData;

struct ScDPValueData;
class ScDPItemData;
//
//  Member names that are being processed for InitFrom/LateInitFrom
//  (needed for initialization of grouped items)
//

class ScDPInitState
{
    long*           pIndex;     // array
    SCROW*     pData; // array
    long            nCount;

public:
            ScDPInitState();
            ~ScDPInitState();

    void    AddMember( long nSourceIndex,SCROW nMember);
    void    RemoveMember();

    long                GetCount() const    { return nCount; }
    const long*         GetSource() const   { return pIndex; }
    const SCROW* GetNameIds() const    { return pData; }
};

typedef ::std::vector<sal_Int32> ScMemberSortOrder;

//
//  selected subtotal information, passed down the dimensions
//

struct ScDPSubTotalState
{
    ScSubTotalFunc eColForce;
    ScSubTotalFunc eRowForce;
    long nColSubTotalFunc;
    long nRowSubTotalFunc;

    ScDPSubTotalState() :
        eColForce( SUBTOTAL_FUNC_NONE ),
        eRowForce( SUBTOTAL_FUNC_NONE ),
        nColSubTotalFunc( -1 ),
        nRowSubTotalFunc( -1 )
    {}
};

//
//  indexes when calculating running totals
//  Col/RowVisible: simple counts from 0 - without sort order applied - visible index
//                  (only used for running total / relative index)
//  Col/RowIndexes: with sort order applied - member index
//                  (used otherwise - so other members' children can be accessed)
//

class ScDPRunningTotalState
{
    ScDPResultMember*   pColResRoot;
    ScDPResultMember*   pRowResRoot;
    long*               pColVisible;
    long*               pColIndexes;
    long*               pRowVisible;
    long*               pRowIndexes;
    long                nColIndexPos;
    long                nRowIndexPos;

public:
            ScDPRunningTotalState( ScDPResultMember* pColRoot, ScDPResultMember* pRowRoot );
            ~ScDPRunningTotalState();

    ScDPResultMember*   GetColResRoot() const   { return pColResRoot; }
    ScDPResultMember*   GetRowResRoot() const   { return pRowResRoot; }

    const long*         GetColVisible() const   { return pColVisible; }
    const long*         GetColIndexes() const   { return pColIndexes; }
    const long*         GetRowVisible() const   { return pRowVisible; }
    const long*         GetRowIndexes() const   { return pRowIndexes; }

    void    AddColIndex( long nVisible, long nSorted );
    void    AddRowIndex( long nVisible, long nSorted );
    void    RemoveColIndex();
    void    RemoveRowIndex();
};

struct ScDPRelativePos
{
    long    nBasePos;       // simple count, without sort order applied
    long    nDirection;

    ScDPRelativePos( long nBase, long nDir );
};

//
//  aggregated data
//! separate header file?
//

//  Possible values for the nCount member:
//  (greater than 0 counts the collected values)
const long SC_DPAGG_EMPTY        =  0;  // empty during data collection
const long SC_DPAGG_DATA_ERROR   = -1;  // error during data collection
const long SC_DPAGG_RESULT_EMPTY = -2;  // empty result calculated
const long SC_DPAGG_RESULT_VALID = -3;  // valid result calculated
const long SC_DPAGG_RESULT_ERROR = -4;  // error in calculated result

class ScDPAggData
{
private:
    double          fVal;
    double          fAux;
    long            nCount;
    ScDPAggData*    pChild;

public:
            ScDPAggData() : fVal(0.0), fAux(0.0), nCount(SC_DPAGG_EMPTY), pChild(NULL) {}
            ~ScDPAggData() { delete pChild; }

    void    Update( const ScDPValueData& rNext, ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState );
    void    Calculate( ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState );
    sal_Bool    IsCalculated() const;

    double  GetResult() const;
    sal_Bool    HasError() const;
    sal_Bool    HasData() const;

    void    SetResult( double fNew );
    void    SetEmpty( sal_Bool bSet );
    void    SetError();

    double  GetAuxiliary() const;
    void    SetAuxiliary( double fNew );

    void    Reset();        // also deletes children

    const ScDPAggData*  GetExistingChild() const    { return pChild; }
    ScDPAggData*        GetChild();
};

//
//  Row and grand total state, passed down (column total is at result member)
//

class ScDPRowTotals
{
    ScDPAggData aRowTotal;
    ScDPAggData aGrandTotal;
    sal_Bool        bIsInColRoot;

public:
            ScDPRowTotals();
            ~ScDPRowTotals();

    ScDPAggData*    GetRowTotal( long nMeasure );
    ScDPAggData*    GetGrandTotal( long nMeasure );

    sal_Bool            IsInColRoot() const     { return bIsInColRoot; }
    void            SetInColRoot(sal_Bool bSet) { bIsInColRoot = bSet; }
};

// --------------------------------------------------------------------
//
//  results for a hierarchy dimension
//

class ScDPResultDimension;
class ScDPDataDimension;
class ScDPDataMember;

#define SC_DPMEASURE_ALL    -1
#define SC_DPMEASURE_ANY    -2

struct MemberHashIndexFunc : public std::unary_function< const SCROW &, size_t >
{
    size_t operator() (const SCROW &rDataIndex) const { return rDataIndex; }
};

class ScDPParentDimData
{
public:
    const SCROW                     mnOrder;                        //! Ref
    const ScDPDimension*            mpParentDim;            //! Ref
    const ScDPLevel*                mpParentLevel;          //! Ref
    const ScDPMember*               mpMemberDesc;           //! Ref

    ScDPParentDimData():mnOrder(-1), mpParentDim( NULL), mpParentLevel( NULL ), mpMemberDesc( NULL ){}
    ScDPParentDimData( const SCROW nIndex, ScDPDimension* pDim, const ScDPLevel* pLev, const ScDPMember* pMember ): mnOrder( nIndex ), mpParentDim( pDim), mpParentLevel( pLev ), mpMemberDesc( pMember ){}
};

typedef std::vector <ScDPParentDimData *>                 DimMemberArray;
typedef boost::unordered_map < SCROW, ScDPParentDimData *, MemberHashIndexFunc>  DimMemberHash;

class ResultMembers
{
    DimMemberHash      maMemberHash;
    sal_Bool                mbHasHideDetailsMember;
public:
    ScDPParentDimData* FindMember( const SCROW& nIndex ) const;
    void                             InsertMember(  ScDPParentDimData* pNew );
    sal_Bool            IsHasHideDetailsMembers() const { return mbHasHideDetailsMember; }
    void            SetHasHideDetailsMembers( sal_Bool b ) { mbHasHideDetailsMember=b; }
    ResultMembers();
    virtual ~ResultMembers();
};

class LateInitParams
{
private:
    const ::std::vector<ScDPDimension*>& mppDim;
    const ::std::vector<ScDPLevel*>& mppLev;

    sal_Bool      mbRow;
    sal_Bool      mbInitChild;
    sal_Bool      mbAllChildren;
public:
    LateInitParams( const ::std::vector<ScDPDimension*>& ppDim, const ::std::vector<ScDPLevel*>& ppLev,
        sal_Bool bRow, sal_Bool bInitChild = sal_True , sal_Bool bAllChildren = false);
    ~LateInitParams();

    void  SetInitChild( sal_Bool b ) { mbInitChild = b; }
    void  SetInitAllChildren( sal_Bool b ) { mbAllChildren = b; }

    inline ScDPDimension* GetDim( size_t nPos ) const { return mppDim[nPos];}
    inline ScDPLevel*         GetLevel( size_t nPos ) const { return mppLev[nPos];}

    inline sal_Bool  GetInitChild() const {return mbInitChild; }
    inline sal_Bool  GetInitAllChild() const { return mbAllChildren; }
    inline sal_Bool  IsRow() const { return mbRow; }
    sal_Bool  IsEnd( size_t nPos ) const ;
};

class ScDPResultData
{
private:
    ScDPSource*             pSource;                //! Ref
    //! keep things like measure lists here

    long                    nMeasCount;
    ScSubTotalFunc*         pMeasFuncs;
    ::com::sun::star::sheet::DataPilotFieldReference* pMeasRefs;
    sal_uInt16*                 pMeasRefOrient;
    std::vector<rtl::OUString> maMeasureNames;
    bool                    bLateInit:1;
    bool                    bDataAtCol:1;
    bool                    bDataAtRow:1;

    //! add "displayed values" settings
    mutable std::vector< ResultMembers* > mpDimMembers;
public:
                        ScDPResultData( ScDPSource* pSrc );     //! Ref
                        ~ScDPResultData();

    void                SetMeasureData( long nCount, const ScSubTotalFunc* pFunctions,
                                        const ::com::sun::star::sheet::DataPilotFieldReference* pRefs,
                                        const sal_uInt16* pRefOrient, std::vector<rtl::OUString>& rNames );
    void                SetDataLayoutOrientation( sal_uInt16 nOrient );
    void                SetLateInit( bool bSet );

    long                GetMeasureCount() const     { return nMeasCount; }
    ScSubTotalFunc      GetMeasureFunction(long nMeasure) const;
    rtl::OUString       GetMeasureString(long nMeasure, bool bForce, ScSubTotalFunc eForceFunc, bool& rbTotalResult) const;
    rtl::OUString       GetMeasureDimensionName(long nMeasure) const;
    const ::com::sun::star::sheet::DataPilotFieldReference& GetMeasureRefVal(long nMeasure) const;
    sal_uInt16              GetMeasureRefOrient(long nMeasure) const;

    bool                IsDataAtCol() const             { return bDataAtCol; }
    bool                IsDataAtRow() const             { return bDataAtRow; }
    bool                IsLateInit() const              { return bLateInit; }

    long                GetColStartMeasure() const;
    long                GetRowStartMeasure() const;

    long                GetCountForMeasure( long nMeas ) const
                                { return ( nMeas == SC_DPMEASURE_ALL ) ? nMeasCount : 1; }

    bool                IsBaseForGroup( long nDim ) const;              // any group
    long                GetGroupBase( long nGroupDim ) const;
    bool                IsNumOrDateGroup( long nDim ) const;
    bool                IsInGroup( SCROW nGroupDataId, long nGroupIndex,
                                   const ScDPItemData& rBaseData, long nBaseIndex ) const;
    bool                HasCommonElement( SCROW nFirstDataId, long nFirstIndex,
                                          const ScDPItemData& rSecondData, long nSecondIndex ) const;

    ResultMembers* GetDimResultMembers( long nDim , ScDPDimension* pDim , ScDPLevel*   pLevel) const ;

    const ScDPSource*   GetSource() const;
};


class ScDPResultMember
{
private:
    const ScDPResultData*   pResultData;
    ScDPParentDimData        aParentDimData;
    ScDPResultDimension*    pChildDimension;
    ScDPDataMember*         pDataRoot;
    sal_Bool                    bHasElements;
    sal_Bool                    bForceSubTotal;
    sal_Bool                    bHasHiddenDetails;
    sal_Bool                    bInitialized;
    sal_Bool                    bAutoHidden;
    ScDPAggData             aColTotal;              // to store column totals

    sal_uInt16                  nMemberStep;            // step to show details
public:
    ScDPResultMember(  const ScDPResultData* pData,  const ScDPParentDimData& rParentDimData,
        sal_Bool bForceSub );  //! Ref
    ScDPResultMember(  const ScDPResultData* pData, sal_Bool bForceSub );
    ~ScDPResultMember();

    void                InitFrom( const ::std::vector<ScDPDimension*>& ppDim,
                                        const ::std::vector<ScDPLevel*>& ppLev,
                                        size_t nPos,
                                        ScDPInitState& rInitState,
                                        sal_Bool bInitChild = sal_True );
    void               LateInitFrom(
                                        LateInitParams& rParams,
                                        const ::std::vector< SCROW >& pItemData,
                                        size_t nPos,
                                        ScDPInitState& rInitState);
    void              CheckShowEmpty( sal_Bool bShow = false );
    String              GetName() const;
    void                FillItemData( ScDPItemData& rData ) const;
    sal_Bool                IsValid() const;
    sal_Bool                IsVisible() const;
    long                GetSize(long nMeasure) const;
    sal_Bool                HasHiddenDetails() const;
    sal_Bool                IsSubTotalInTitle(long nMeasure) const;

    long                GetSubTotalCount( long* pUserSubStart = NULL ) const;

    sal_Bool              IsNamedItem( SCROW nIndex ) const;
    bool IsValidEntry( const ::std::vector< SCROW >& aMembers ) const;

    void                SetHasElements()    { bHasElements = sal_True; }
    void                SetAutoHidden()     { bAutoHidden = sal_True; }

    void                ProcessData( const ::std::vector<SCROW>& aChildMembers,
                                        const ScDPResultDimension* pDataDim,
                                        const ::std::vector<SCROW>& aDataMembers,
                                        const ::std::vector<ScDPValueData>& aValues );
    void                FillMemberResults( com::sun::star::uno::Sequence<
                                                com::sun::star::sheet::MemberResult>* pSequences,
                                            long& rPos, long nMeasure, sal_Bool bRoot,
                                            const String* pMemberName,
                                            const String* pMemberCaption );

    void                FillDataResults( const ScDPResultMember* pRefMember,
                                    com::sun::star::uno::Sequence<
                                        com::sun::star::uno::Sequence<
                                            com::sun::star::sheet::DataResult> >& rSequence,
                                    long& rRow, long nMeasure ) const;

    void                UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const;
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                                ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const;

    void                SortMembers( ScDPResultMember* pRefMember );
    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults( sal_Bool bRoot );

    void                DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;

                        //! this will be removed!
    const ScDPResultDimension*  GetChildDimension() const   { return pChildDimension; }
    ScDPResultDimension*        GetChildDimension()         { return pChildDimension; }

    ScDPDataMember*         GetDataRoot() const             { return pDataRoot; }

    const ScDPDimension*  GetParentDim() const               { return aParentDimData.mpParentDim; }     //! Ref
    const ScDPLevel*         GetParentLevel() const         { return aParentDimData.mpParentLevel; }   //! Ref
    const ScDPMember*     GetDPMember()const              { return aParentDimData.mpMemberDesc; }    //! Ref
    inline SCROW               GetOrder() const                           { return aParentDimData.mnOrder; }         //! Ref
    inline sal_Bool                  IsRoot() const                       { return GetParentLevel() == NULL; }
    SCROW                       GetDataId( ) const ;
    ScDPAggData*        GetColTotal( long nMeasure ) const;

    void                FillVisibilityData(ScDPResultVisibilityData& rData) const;
};

class ScDPDataMember
{
private:
    const ScDPResultData*       pResultData;
    const ScDPResultMember*     pResultMember;          //! Ref?
    ScDPDataDimension*      pChildDimension;
    ScDPAggData             aAggregate;

    void                UpdateValues( const ::std::vector<ScDPValueData>& aValues, const ScDPSubTotalState& rSubState );

public:
                        ScDPDataMember( const ScDPResultData* pData, const ScDPResultMember* pRes );
                        ~ScDPDataMember();

    void                InitFrom( const ScDPResultDimension* pDim );

    String              GetName() const;
    sal_Bool                IsVisible() const;
    sal_Bool                HasData( long nMeasure, const ScDPSubTotalState& rSubState ) const;

    sal_Bool              IsNamedItem(   SCROW r ) const;
    sal_Bool                HasHiddenDetails() const;

    void                ProcessData( const ::std::vector< SCROW >& aChildMembers, const ::std::vector<ScDPValueData>& aValues,
                                       const ScDPSubTotalState& rSubState );
    sal_Bool                HasError( long nMeasure, const ScDPSubTotalState& rSubState ) const;
    double              GetAggregate( long nMeasure, const ScDPSubTotalState& rSubState ) const;
    const ScDPAggData*  GetConstAggData( long nMeasure, const ScDPSubTotalState& rSubState ) const;
    ScDPAggData*        GetAggData( long nMeasure, const ScDPSubTotalState& rSubState );

    void                FillDataRow( const ScDPResultMember* pRefMember,
                                    com::sun::star::uno::Sequence<com::sun::star::sheet::DataResult>& rSequence,
                                    long& rCol, long nMeasure, sal_Bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const;

    void                UpdateDataRow( const ScDPResultMember* pRefMember, long nMeasure, sal_Bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState );
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure, sal_Bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent );

    void                SortMembers( ScDPResultMember* pRefMember );
    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults();

    void                DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;

                        //! this will be removed!
    const ScDPDataDimension*    GetChildDimension() const   { return pChildDimension; }
    ScDPDataDimension*          GetChildDimension()         { return pChildDimension; }
};

typedef std::vector<ScDPDataMember*> ScDPDataMembers;


//  result dimension contains only members

class ScDPResultDimension
{
public:
    typedef std::vector<ScDPResultMember*>     MemberArray;
    typedef std::map<SCROW, ScDPResultMember*> MemberHash;
private:
    const ScDPResultData*   pResultData;
    MemberArray             maMemberArray;
    MemberHash              maMemberHash;
    rtl::OUString           aDimensionName;     //! or ptr to IntDimension?
    long                    nSortMeasure;
    ScMemberSortOrder       aMemberOrder;       // used when sorted by measure
    bool                    bIsDataLayout:1;      //! or ptr to IntDimension?
    bool                    bSortByData:1;
    bool                    bSortAscending:1;
    bool                    bAutoShow:1;
    bool                    bAutoTopItems:1;
    bool                    bInitialized:1;
    long                    nAutoMeasure;
    long                    nAutoCount;

    ScDPResultMember*        FindMember(   SCROW  iData ) const;
    ScDPResultMember*        AddMember( const ScDPParentDimData& aData );
    ScDPResultMember*        InsertMember( ScDPParentDimData* pMemberData );
    void                                  InitWithMembers( LateInitParams& rParams,
                                                            const ::std::vector< SCROW >& pItemData,
                                                            size_t  nPos,
                                                            ScDPInitState& rInitState  );
public:
       ScDPResultDimension( const ScDPResultData* pData );
    ~ScDPResultDimension();

                        //  allocates new members
    void                InitFrom( const ::std::vector<ScDPDimension*>& ppDim,
                                        const ::std::vector<ScDPLevel*>& ppLev,
                                        size_t nPos,
                                        ScDPInitState& rInitState ,  sal_Bool bInitChild = sal_True );
    void                LateInitFrom(  LateInitParams& rParams,
                                        const ::std::vector< SCROW >& pItemData,
                                        size_t nPos,
                                        ScDPInitState& rInitState );
    void               CheckShowEmpty( sal_Bool bShow = false );

    long                GetSize(long nMeasure) const;

    bool                IsValidEntry( const ::std::vector<SCROW>& aMembers ) const;

    //  modifies existing members, allocates data dimensions
    void                ProcessData( const ::std::vector<SCROW>& aMembers,
                                     const ScDPResultDimension* pDataDim,
                                     const ::std::vector<SCROW>& aDataMembers,
                                     const ::std::vector<ScDPValueData>& aValues ) const;   //! Test
    void                FillMemberResults( com::sun::star::uno::Sequence<
                                                com::sun::star::sheet::MemberResult>* pSequences,
                                            long nStart, long nMeasure );

    void                FillDataResults( const ScDPResultMember* pRefMember,
                                    com::sun::star::uno::Sequence<
                                        com::sun::star::uno::Sequence<
                                            com::sun::star::sheet::DataResult> >& rSequence,
                                    long nRow, long nMeasure ) const;

    void                UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const;
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                            ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const;

    void                SortMembers( ScDPResultMember* pRefMember );
    long                GetSortedIndex( long nUnsorted ) const;

    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults();

                        //  called for the reference dimension
    ScDPDataMember*     GetRowReferenceMember( const ScDPRelativePos* pMemberPos, const String* pName,
                                    const long* pRowIndexes, const long* pColIndexes ) const;

                        //  uses row root member from ScDPRunningTotalState
    static ScDPDataMember* GetColReferenceMember( const ScDPRelativePos* pMemberPos, const String* pName,
                                    long nRefDimPos, const ScDPRunningTotalState& rRunning );

    void                DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;

                        //  for ScDPDataDimension::InitFrom
    long                GetMemberCount() const;
    const ScDPResultMember* GetMember(long n) const;
    ScDPResultMember*       GetMember(long n);

    const ScMemberSortOrder& GetMemberOrder() const     { return aMemberOrder; }
    ScMemberSortOrder&  GetMemberOrder()                { return aMemberOrder; }

    sal_Bool                IsDataLayout() const    { return bIsDataLayout; }
    String              GetName() const         { return aDimensionName; }

    sal_Bool                IsSortByData() const    { return bSortByData; }
    sal_Bool                IsSortAscending() const { return bSortAscending; }
    long                GetSortMeasure() const  { return nSortMeasure; }

    sal_Bool                IsAutoShow() const      { return bAutoShow; }
    sal_Bool                IsAutoTopItems() const  { return bAutoTopItems; }
    long                GetAutoMeasure() const  { return nAutoMeasure; }
    long                GetAutoCount() const    { return nAutoCount; }

    ScDPResultDimension* GetFirstChildDimension() const;

    void                FillVisibilityData(ScDPResultVisibilityData& rData) const;
};

class ScDPDataDimension
{
private:
    const ScDPResultData*       pResultData;
    const ScDPResultDimension* pResultDimension;  // column
    ScDPDataMembers     maMembers;
    sal_Bool                bIsDataLayout;      //! or ptr to IntDimension?

public:
                        ScDPDataDimension( const ScDPResultData* pData );
                        ~ScDPDataDimension();

    void                InitFrom( const ScDPResultDimension* pDim );        // recursive
    void                ProcessData( const ::std::vector< SCROW >& aDataMembers, const ::std::vector<ScDPValueData>& aValues,
                                       const ScDPSubTotalState& rSubState );
    void                FillDataRow( const ScDPResultDimension* pRefDim,
                                    com::sun::star::uno::Sequence<com::sun::star::sheet::DataResult>& rSequence,
                                    long nCol, long nMeasure, sal_Bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const;

    void                UpdateDataRow( const ScDPResultDimension* pRefDim, long nMeasure, sal_Bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const;
    void                UpdateRunningTotals( const ScDPResultDimension* pRefDim, long nMeasure, sal_Bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent ) const;

    void                SortMembers( ScDPResultDimension* pRefDim );
    long                GetSortedIndex( long nUnsorted ) const;

    void                DoAutoShow( ScDPResultDimension* pRefDim );

    void                ResetResults();

    void                DumpState( const ScDPResultDimension* pRefDim, ScDocument* pDoc, ScAddress& rPos ) const;

    long                GetMemberCount() const;
    const ScDPDataMember*     GetMember(long n) const;
    ScDPDataMember*     GetMember(long n);
};

// ----------------------------------------------------------------------------

/**
 * This class collects visible members of each dimension and uses that
 * information to create filtering criteria (e.g. for drill-down data).
 */
class ScDPResultVisibilityData
{
public:
    ScDPResultVisibilityData( ScDPSource* pSource);
    ~ScDPResultVisibilityData();

    void addVisibleMember(const String& rDimName, const ScDPItemData& rMemberItem);
    void fillFieldFilters(::std::vector<ScDPCacheTable::Criterion>& rFilters) const;

private:
    struct MemberHash
    {
        size_t operator()(const ScDPItemData& r) const;
    };
    typedef ::boost::unordered_set<ScDPItemData, MemberHash> VisibleMemberType;
    typedef ::boost::unordered_map<String, VisibleMemberType, ScStringHashCode> DimMemberType;
    DimMemberType maDimensions;

    ScDPSource* mpSource;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
