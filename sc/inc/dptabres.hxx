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

#ifndef SC_DPTABRES_HXX
#define SC_DPTABRES_HXX

#include "global.hxx"
#include "dpfilteredcache.hxx"
#include "calcmacros.hxx"

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

struct ScDPValue;
class ScDPItemData;
struct ScDPResultFilterContext;

/**
 * Member names that are being processed for InitFrom/LateInitFrom (needed
 * for initialization of grouped items).
 */
class ScDPInitState
{
public:
    struct Member
    {
        long mnSrcIndex;
        SCROW mnNameIndex;

        Member(long nSrcIndex, SCROW nNameIndex);
    };

    void AddMember(long nSourceIndex, SCROW nMember);
    void RemoveMember();

    const std::vector<Member>& GetMembers() const { return maMembers; }

private:
    std::vector<Member> maMembers;
};

typedef ::std::vector<sal_Int32> ScMemberSortOrder;

/**
 * Select subtotal information, passed down the dimensions.
 */
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

/**
 * indexes when calculating running totals
 *
 * Col/RowVisible: simple counts from 0 - without sort order applied
 * - visible index (only used for running total / relative index)
 *
 * Col/RowSorted: with sort order applied - member index (used otherwise -
 * so other members' children can be accessed).
 */
class ScDPRunningTotalState
{
public:
    typedef std::vector<long> IndexArray; /// array of long integers terminated by -1.

    ScDPRunningTotalState( ScDPResultMember* pColRoot, ScDPResultMember* pRowRoot );

    ScDPResultMember*   GetColResRoot() const   { return pColResRoot; }
    ScDPResultMember*   GetRowResRoot() const   { return pRowResRoot; }

    const IndexArray& GetColVisible() const;
    const IndexArray& GetColSorted() const;
    const IndexArray& GetRowVisible() const;
    const IndexArray& GetRowSorted() const;

    void    AddColIndex( long nVisible, long nSorted );
    void    AddRowIndex( long nVisible, long nSorted );
    void    RemoveColIndex();
    void    RemoveRowIndex();

private:
    ScDPResultMember*   pColResRoot;
    ScDPResultMember*   pRowResRoot;

    mutable IndexArray maColVisible;
    mutable IndexArray maColSorted;
    mutable IndexArray maRowVisible;
    mutable IndexArray maRowSorted;
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

    void    Update( const ScDPValue& rNext, ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState );
    void    Calculate( ScSubTotalFunc eFunc, const ScDPSubTotalState& rSubState );
    bool IsCalculated() const;

    double  GetResult() const;
    bool HasError() const;
    bool HasData() const;

    void    SetResult( double fNew );
    void SetEmpty( bool bSet );
    void    SetError();

    double  GetAuxiliary() const;
    void    SetAuxiliary( double fNew );

    void    Reset();        // also deletes children

    const ScDPAggData*  GetExistingChild() const    { return pChild; }
    ScDPAggData*        GetChild();

#if DEBUG_PIVOT_TABLE
    void Dump(int nIndent) const;
#endif
};

//
//  Row and grand total state, passed down (column total is at result member)
//

class ScDPRowTotals
{
    ScDPAggData aRowTotal;
    ScDPAggData aGrandTotal;
    bool bIsInColRoot;

public:
            ScDPRowTotals();
            ~ScDPRowTotals();

    ScDPAggData*    GetRowTotal( long nMeasure );
    ScDPAggData*    GetGrandTotal( long nMeasure );

    bool IsInColRoot() const { return bIsInColRoot; }
    void SetInColRoot(bool bSet) { bIsInColRoot = bSet; }
};

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

struct ScDPParentDimData
{
    const SCROW                     mnOrder;                //! Ref
    const ScDPDimension*            mpParentDim;            //! Ref
    const ScDPLevel*                mpParentLevel;          //! Ref
    const ScDPMember*               mpMemberDesc;           //! Ref

    ScDPParentDimData();
    ScDPParentDimData(SCROW nIndex, const ScDPDimension* pDim, const ScDPLevel* pLev, const ScDPMember* pMember);
};

typedef std::vector <ScDPParentDimData *>                 DimMemberArray;
typedef boost::unordered_map < SCROW, ScDPParentDimData *, MemberHashIndexFunc>  DimMemberHash;

class ResultMembers
{
    DimMemberHash      maMemberHash;
    bool mbHasHideDetailsMember;
public:
    ScDPParentDimData* FindMember( SCROW nIndex ) const;
    void                             InsertMember(  ScDPParentDimData* pNew );
    bool IsHasHideDetailsMembers() const { return mbHasHideDetailsMember; }
    void SetHasHideDetailsMembers( bool b ) { mbHasHideDetailsMember = b; }
    ResultMembers();
    virtual ~ResultMembers();
};

class LateInitParams
{
private:
    const ::std::vector<ScDPDimension*>& mppDim;
    const ::std::vector<ScDPLevel*>& mppLev;

    bool mbRow:1;
    bool mbInitChild:1;
    bool mbAllChildren:1;
public:
    LateInitParams( const ::std::vector<ScDPDimension*>& ppDim, const ::std::vector<ScDPLevel*>& ppLev,
        bool bRow, bool bInitChild = true, bool bAllChildren = false);
    ~LateInitParams();

    void SetInitChild( bool b ) { mbInitChild = b; }
    void SetInitAllChildren( bool b ) { mbAllChildren = b; }

    inline ScDPDimension* GetDim( size_t nPos ) const { return mppDim[nPos];}
    inline ScDPLevel*         GetLevel( size_t nPos ) const { return mppLev[nPos];}

    bool GetInitChild() const {return mbInitChild; }
    bool GetInitAllChild() const { return mbAllChildren; }
    bool IsRow() const { return mbRow; }
    bool IsEnd( size_t nPos ) const ;
};

/**
 * The term 'measure' here roughly equals "data dimension" ?
 */
class ScDPResultData
{
    ScDPSource& mrSource;
    //! keep things like measure lists here

    std::vector<ScSubTotalFunc> maMeasureFuncs;
    std::vector<com::sun::star::sheet::DataPilotFieldReference> maMeasureRefs;
    std::vector<sal_uInt16> maMeasureRefOrients;
    std::vector<OUString> maMeasureNames;

    bool                    bLateInit:1;
    bool                    bDataAtCol:1;
    bool                    bDataAtRow:1;

    //! add "displayed values" settings
    mutable std::vector<ResultMembers*> maDimMembers;
public:
    ScDPResultData( ScDPSource& rSrc );
    ~ScDPResultData();

    void SetMeasureData(
        std::vector<ScSubTotalFunc>& rFunctions,
        std::vector<com::sun::star::sheet::DataPilotFieldReference>& rRefs,
        std::vector<sal_uInt16>& rRefOrient, std::vector<OUString>& rNames );

    void                SetDataLayoutOrientation( sal_uInt16 nOrient );
    void                SetLateInit( bool bSet );

    long GetMeasureCount() const { return maMeasureFuncs.size(); }
    ScSubTotalFunc      GetMeasureFunction(long nMeasure) const;
    OUString       GetMeasureString(long nMeasure, bool bForce, ScSubTotalFunc eForceFunc, bool& rbTotalResult) const;
    OUString       GetMeasureDimensionName(long nMeasure) const;
    const ::com::sun::star::sheet::DataPilotFieldReference& GetMeasureRefVal(long nMeasure) const;
    sal_uInt16              GetMeasureRefOrient(long nMeasure) const;

    bool                IsDataAtCol() const             { return bDataAtCol; }
    bool                IsDataAtRow() const             { return bDataAtRow; }
    bool                IsLateInit() const              { return bLateInit; }

    long                GetColStartMeasure() const;
    long                GetRowStartMeasure() const;

    long GetCountForMeasure( long nMeas ) const { return (nMeas == SC_DPMEASURE_ALL) ? maMeasureFuncs.size() : 1; }

    bool                IsBaseForGroup( long nDim ) const;              // any group
    long                GetGroupBase( long nGroupDim ) const;
    bool                IsNumOrDateGroup( long nDim ) const;
    bool                IsInGroup( SCROW nGroupDataId, long nGroupIndex,
                                   const ScDPItemData& rBaseData, long nBaseIndex ) const;
    bool                HasCommonElement( SCROW nFirstDataId, long nFirstIndex,
                                          const ScDPItemData& rSecondData, long nSecondIndex ) const;

    ResultMembers* GetDimResultMembers(long nDim, ScDPDimension* pDim, ScDPLevel* pLevel) const;

    const ScDPSource& GetSource() const;
};

class ScDPResultMember
{
private:
    const ScDPResultData*   pResultData;
    ScDPParentDimData        aParentDimData;
    ScDPResultDimension*    pChildDimension;
    ScDPDataMember*         pDataRoot;
    bool                    bHasElements:1;
    bool                    bForceSubTotal:1;
    bool                    bHasHiddenDetails:1;
    bool                    bInitialized:1;
    bool                    bAutoHidden:1;
    ScDPAggData             aColTotal;              // to store column totals

    sal_uInt16                  nMemberStep;            // step to show details
public:
    ScDPResultMember(
        const ScDPResultData* pData,  const ScDPParentDimData& rParentDimData, bool bForceSub );  //! Ref
    ScDPResultMember(  const ScDPResultData* pData, bool bForceSub );
    ~ScDPResultMember();

    void                InitFrom( const ::std::vector<ScDPDimension*>& ppDim,
                                        const ::std::vector<ScDPLevel*>& ppLev,
                                        size_t nPos,
                                        ScDPInitState& rInitState,
                                  bool bInitChild = true );
    void               LateInitFrom(
                                        LateInitParams& rParams,
                                        const ::std::vector< SCROW >& pItemData,
                                        size_t nPos,
                                        ScDPInitState& rInitState);
    void CheckShowEmpty( bool bShow = false );
    OUString GetName() const;
    OUString GetDisplayName() const;

    void                FillItemData( ScDPItemData& rData ) const;
    bool IsValid() const;
    bool IsVisible() const;
    long                GetSize(long nMeasure) const;
    bool HasHiddenDetails() const;
    bool IsSubTotalInTitle(long nMeasure) const;

    long                GetSubTotalCount( long* pUserSubStart = NULL ) const;

    bool IsNamedItem( SCROW nIndex ) const;
    bool IsValidEntry( const ::std::vector< SCROW >& aMembers ) const;

    void SetHasElements() { bHasElements = true; }
    void SetAutoHidden() { bAutoHidden = true; }

    void                ProcessData( const ::std::vector<SCROW>& aChildMembers,
                                        const ScDPResultDimension* pDataDim,
                                        const ::std::vector<SCROW>& aDataMembers,
                                        const ::std::vector<ScDPValue>& aValues );
    void FillMemberResults(
        com::sun::star::uno::Sequence<
            com::sun::star::sheet::MemberResult>* pSequences,
        long& rPos, long nMeasure, bool bRoot, const OUString* pMemberName, const OUString* pMemberCaption );

    void FillDataResults(
        const ScDPResultMember* pRefMember, ScDPResultFilterContext& rFilterCxt,
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Sequence<
                com::sun::star::sheet::DataResult> >& rSequence,
        long nMeasure) const;

    void                UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const;
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                                ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const;

    void                SortMembers( ScDPResultMember* pRefMember );
    void                DoAutoShow( ScDPResultMember* pRefMember );

    void ResetResults();

#if DEBUG_PIVOT_TABLE
    void DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;

    void Dump(int nIndent) const;
#endif

                        //! this will be removed!
    const ScDPResultDimension*  GetChildDimension() const   { return pChildDimension; }
    ScDPResultDimension*        GetChildDimension()         { return pChildDimension; }

    ScDPDataMember*         GetDataRoot() const             { return pDataRoot; }

    const ScDPDimension*  GetParentDim() const               { return aParentDimData.mpParentDim; }     //! Ref
    const ScDPLevel*         GetParentLevel() const         { return aParentDimData.mpParentLevel; }   //! Ref
    const ScDPMember*     GetDPMember()const              { return aParentDimData.mpMemberDesc; }    //! Ref
    SCROW GetOrder() const { return aParentDimData.mnOrder; }         //! Ref
    bool IsRoot() const { return GetParentLevel() == NULL; }
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

    void                UpdateValues( const ::std::vector<ScDPValue>& aValues, const ScDPSubTotalState& rSubState );

public:
                        ScDPDataMember( const ScDPResultData* pData, const ScDPResultMember* pRes );
                        ~ScDPDataMember();

    void                InitFrom( const ScDPResultDimension* pDim );

    OUString GetName() const;
    bool IsVisible() const;
    bool HasData( long nMeasure, const ScDPSubTotalState& rSubState ) const;

    bool IsNamedItem( SCROW nRow ) const;
    bool HasHiddenDetails() const;

    void                ProcessData( const ::std::vector< SCROW >& aChildMembers, const ::std::vector<ScDPValue>& aValues,
                                       const ScDPSubTotalState& rSubState );
    bool HasError( long nMeasure, const ScDPSubTotalState& rSubState ) const;
    double              GetAggregate( long nMeasure, const ScDPSubTotalState& rSubState ) const;
    const ScDPAggData*  GetConstAggData( long nMeasure, const ScDPSubTotalState& rSubState ) const;
    ScDPAggData*        GetAggData( long nMeasure, const ScDPSubTotalState& rSubState );

    void FillDataRow(
        const ScDPResultMember* pRefMember,
        ScDPResultFilterContext& rFilterCxt,
        com::sun::star::uno::Sequence<com::sun::star::sheet::DataResult>& rSequence,
        long nMeasure, bool bIsSubTotalRow,
        const ScDPSubTotalState& rSubState) const;

    void UpdateDataRow( const ScDPResultMember* pRefMember, long nMeasure, bool bIsSubTotalRow,
                        const ScDPSubTotalState& rSubState );
    void UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure, bool bIsSubTotalRow,
                              const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                              ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent );

    void                SortMembers( ScDPResultMember* pRefMember );
    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults();

#if DEBUG_PIVOT_TABLE
    void DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;
    void Dump(int nIndent) const;
#endif

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
    OUString           aDimensionName;     //! or ptr to IntDimension?
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
    void InitFrom(
        const ::std::vector<ScDPDimension*>& ppDim, const ::std::vector<ScDPLevel*>& ppLev,
        size_t nPos, ScDPInitState& rInitState,  bool bInitChild = true );
    void                LateInitFrom(  LateInitParams& rParams,
                                        const ::std::vector< SCROW >& pItemData,
                                        size_t nPos,
                                        ScDPInitState& rInitState );
    void CheckShowEmpty( bool bShow = false );

    long                GetSize(long nMeasure) const;

    bool                IsValidEntry( const ::std::vector<SCROW>& aMembers ) const;

    //  modifies existing members, allocates data dimensions
    void                ProcessData( const ::std::vector<SCROW>& aMembers,
                                     const ScDPResultDimension* pDataDim,
                                     const ::std::vector<SCROW>& aDataMembers,
                                     const ::std::vector<ScDPValue>& aValues ) const;   //! Test
    void                FillMemberResults( com::sun::star::uno::Sequence<
                                                com::sun::star::sheet::MemberResult>* pSequences,
                                            long nStart, long nMeasure );

    void FillDataResults(
        const ScDPResultMember* pRefMember,
        ScDPResultFilterContext& rFilterCxt,
        com::sun::star::uno::Sequence<
            com::sun::star::uno::Sequence<
                com::sun::star::sheet::DataResult> >& rSequence,
        long nMeasure) const;

    void                UpdateDataResults( const ScDPResultMember* pRefMember, long nMeasure ) const;
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, long nMeasure,
                                            ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const;

    void                SortMembers( ScDPResultMember* pRefMember );
    long                GetSortedIndex( long nUnsorted ) const;

    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults();

    //  called for the reference dimension
    ScDPDataMember* GetRowReferenceMember(
        const ScDPRelativePos* pMemberPos, const OUString* pName,
        const long* pRowIndexes, const long* pColIndexes ) const;

    // uses row root member from ScDPRunningTotalState
    static ScDPDataMember* GetColReferenceMember(
        const ScDPRelativePos* pMemberPos, const OUString* pName,
        long nRefDimPos, const ScDPRunningTotalState& rRunning );

#if DEBUG_PIVOT_TABLE
    void DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;
    void Dump(int nIndent) const;
#endif

                        //  for ScDPDataDimension::InitFrom
    long                GetMemberCount() const;
    const ScDPResultMember* GetMember(long n) const;
    ScDPResultMember*       GetMember(long n);

    const ScMemberSortOrder& GetMemberOrder() const     { return aMemberOrder; }
    ScMemberSortOrder&  GetMemberOrder()                { return aMemberOrder; }

    bool IsDataLayout() const { return bIsDataLayout; }
    const OUString& GetName() const { return aDimensionName; }

    bool IsSortByData() const { return bSortByData; }
    bool IsSortAscending() const { return bSortAscending; }
    long                GetSortMeasure() const  { return nSortMeasure; }

    bool IsAutoShow() const { return bAutoShow; }
    bool IsAutoTopItems() const { return bAutoTopItems; }
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
    bool bIsDataLayout;      //! or ptr to IntDimension?

public:
                        ScDPDataDimension( const ScDPResultData* pData );
                        ~ScDPDataDimension();

    void                InitFrom( const ScDPResultDimension* pDim );        // recursive
    void                ProcessData( const ::std::vector< SCROW >& aDataMembers, const ::std::vector<ScDPValue>& aValues,
                                       const ScDPSubTotalState& rSubState );
    void FillDataRow(
        const ScDPResultDimension* pRefDim,
        ScDPResultFilterContext& rFilterCxt,
        com::sun::star::uno::Sequence<com::sun::star::sheet::DataResult>& rSequence,
        long nMeasure, bool bIsSubTotalRow, const ScDPSubTotalState& rSubState) const;

    void                UpdateDataRow( const ScDPResultDimension* pRefDim, long nMeasure, bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const;
    void                UpdateRunningTotals( const ScDPResultDimension* pRefDim, long nMeasure, bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent ) const;

    void                SortMembers( ScDPResultDimension* pRefDim );
    long                GetSortedIndex( long nUnsorted ) const;

    void                DoAutoShow( ScDPResultDimension* pRefDim );

    void                ResetResults();

#if DEBUG_PIVOT_TABLE
    void DumpState( const ScDPResultDimension* pRefDim, ScDocument* pDoc, ScAddress& rPos ) const;
    void Dump(int nIndent) const;
#endif

    long                GetMemberCount() const;
    const ScDPDataMember*     GetMember(long n) const;
    ScDPDataMember*     GetMember(long n);
};

/**
 * This class collects visible members of each dimension and uses that
 * information to create filtering criteria (e.g. for drill-down data).
 */
class ScDPResultVisibilityData
{
public:
    ScDPResultVisibilityData( ScDPSource* pSource);
    ~ScDPResultVisibilityData();

    void addVisibleMember(const OUString& rDimName, const ScDPItemData& rMemberItem);
    void fillFieldFilters(::std::vector<ScDPFilteredCache::Criterion>& rFilters) const;

private:
    struct MemberHash
    {
        size_t operator()(const ScDPItemData& r) const;
    };
    typedef ::boost::unordered_set<ScDPItemData, MemberHash> VisibleMemberType;
    typedef ::boost::unordered_map<OUString, VisibleMemberType, ScStringHashCode> DimMemberType;
    DimMemberType maDimensions;

    ScDPSource* mpSource;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
