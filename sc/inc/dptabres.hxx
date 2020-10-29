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

#ifndef INCLUDED_SC_INC_DPTABRES_HXX
#define INCLUDED_SC_INC_DPTABRES_HXX

#include "global.hxx"
#include "dpfilteredcache.hxx"
#include "calcmacros.hxx"
#include "dpitemdata.hxx"
#include "subtotal.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>

namespace com::sun::star::sheet { struct DataResult; }
namespace com::sun::star::sheet { struct MemberResult; }
namespace com::sun::star::uno { template <typename > class Sequence; }

class ScDPSource;
class ScDPDimension;
class ScDPLevel;
class ScDPMember;
class ScDPResultMember;
class ScDPResultVisibilityData;

struct ScDPValue;
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
        tools::Long mnSrcIndex;
        SCROW mnNameIndex;

        Member(tools::Long nSrcIndex, SCROW nNameIndex);
    };

    void AddMember(tools::Long nSourceIndex, SCROW nMember);
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
    tools::Long nColSubTotalFunc;
    tools::Long nRowSubTotalFunc;

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
    typedef std::vector<sal_Int32> IndexArray; /// array of sal_Int32 terminated by -1.

    ScDPRunningTotalState( ScDPResultMember* pColRoot, ScDPResultMember* pRowRoot );

    ScDPResultMember*   GetColResRoot() const   { return pColResRoot; }
    ScDPResultMember*   GetRowResRoot() const   { return pRowResRoot; }

    const IndexArray& GetColVisible() const { return maColVisible;}
    const IndexArray& GetColSorted() const { return maColSorted;}
    const IndexArray& GetRowVisible() const { return maRowVisible;}
    const IndexArray& GetRowSorted() const { return maRowSorted;}

    void    AddColIndex( sal_Int32 nVisible, tools::Long nSorted );
    void    AddRowIndex( sal_Int32 nVisible, tools::Long nSorted );
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
    tools::Long    nBasePos;       // simple count, without sort order applied
    tools::Long    nDirection;

    ScDPRelativePos( tools::Long nBase, tools::Long nDir );
};

//  aggregated data
//! separate header file?

//  Possible values for the nCount member:
//  (greater than 0 counts the collected values)
const sal_Int64 SC_DPAGG_EMPTY        =  0;  // empty during data collection
const sal_Int64 SC_DPAGG_DATA_ERROR   = -1;  // error during data collection
const sal_Int64 SC_DPAGG_RESULT_EMPTY = -2;  // empty result calculated
const sal_Int64 SC_DPAGG_RESULT_VALID = -3;  // valid result calculated
const sal_Int64 SC_DPAGG_RESULT_ERROR = -4;  // error in calculated result

class ScDPAggData
{
private:
    WelfordRunner   maWelford;
    double          fVal;
    double          fAux;
    sal_Int64       nCount;
    std::unique_ptr<ScDPAggData> pChild;
    std::vector<double> mSortedValues;

public:
            ScDPAggData() : fVal(0.0), fAux(0.0), nCount(SC_DPAGG_EMPTY) {}

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

    const ScDPAggData*  GetExistingChild() const    { return pChild.get(); }
    ScDPAggData*        GetChild();

#if DUMP_PIVOT_TABLE
    void Dump(int nIndent) const;
#endif
};

//  Row and grand total state, passed down (column total is at result member)

class ScDPRowTotals
{
    ScDPAggData aRowTotal;
    ScDPAggData aGrandTotal;
    bool bIsInColRoot;

public:
            ScDPRowTotals();
            ~ScDPRowTotals();

    ScDPAggData*    GetRowTotal( tools::Long nMeasure );
    ScDPAggData*    GetGrandTotal( tools::Long nMeasure );

    bool IsInColRoot() const { return bIsInColRoot; }
    void SetInColRoot(bool bSet) { bIsInColRoot = bSet; }
};

//  results for a hierarchy dimension

class ScDPResultDimension;
class ScDPDataDimension;
class ScDPDataMember;

#define SC_DPMEASURE_ALL    -1
#define SC_DPMEASURE_ANY    -2

struct ScDPParentDimData
{
    const SCROW                     mnOrder;                //! Ref
    const ScDPDimension*            mpParentDim;            //! Ref
    const ScDPLevel*                mpParentLevel;          //! Ref
    const ScDPMember*               mpMemberDesc;           //! Ref

    ScDPParentDimData();
    ScDPParentDimData(SCROW nIndex, const ScDPDimension* pDim, const ScDPLevel* pLev, const ScDPMember* pMember);
};

class ResultMembers final
{
    std::unordered_map<SCROW, ScDPParentDimData> maMemberHash;
    bool mbHasHideDetailsMember;
public:
    const ScDPParentDimData* FindMember( SCROW nIndex ) const;
    void InsertMember( ScDPParentDimData const & rNew );
    bool IsHasHideDetailsMembers() const { return mbHasHideDetailsMember; }
    void SetHasHideDetailsMembers( bool b ) { mbHasHideDetailsMember = b; }
    ResultMembers();
    ~ResultMembers();
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
        bool bRow);
    ~LateInitParams();

    void SetInitChild( bool b ) { mbInitChild = b; }
    void SetInitAllChildren( bool b ) { mbAllChildren = b; }

    ScDPDimension* GetDim( size_t nPos ) const { return mppDim[nPos];}
    ScDPLevel*         GetLevel( size_t nPos ) const { return mppLev[nPos];}

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
    std::vector<css::sheet::DataPilotFieldReference> maMeasureRefs;
    std::vector<css::sheet::DataPilotFieldOrientation> maMeasureRefOrients;
    std::vector<OUString> maMeasureNames;

    bool                    bLateInit:1;
    bool                    bDataAtCol:1;
    bool                    bDataAtRow:1;

    //! add "displayed values" settings
    mutable std::vector<std::unique_ptr<ResultMembers>> maDimMembers;
public:
    ScDPResultData( ScDPSource& rSrc );
    ~ScDPResultData();

    void SetMeasureData(
        std::vector<ScSubTotalFunc>& rFunctions,
        std::vector<css::sheet::DataPilotFieldReference>& rRefs,
        std::vector<css::sheet::DataPilotFieldOrientation>& rRefOrient,
        std::vector<OUString>& rNames );

    void                SetDataLayoutOrientation( css::sheet::DataPilotFieldOrientation nOrient );
    void                SetLateInit( bool bSet );

    tools::Long                GetMeasureCount() const { return maMeasureFuncs.size(); }
    ScSubTotalFunc      GetMeasureFunction(tools::Long nMeasure) const;
    OUString            GetMeasureString(tools::Long nMeasure, bool bForce, ScSubTotalFunc eForceFunc, bool& rbTotalResult) const;
    OUString            GetMeasureDimensionName(tools::Long nMeasure) const;
    const css::sheet::DataPilotFieldReference& GetMeasureRefVal(tools::Long nMeasure) const;
    css::sheet::DataPilotFieldOrientation      GetMeasureRefOrient(tools::Long nMeasure) const;

    bool                IsLateInit() const              { return bLateInit; }

    tools::Long                GetColStartMeasure() const;
    tools::Long                GetRowStartMeasure() const;

    tools::Long                GetCountForMeasure( tools::Long nMeas ) const { return (nMeas == SC_DPMEASURE_ALL) ? maMeasureFuncs.size() : 1; }

    bool                IsBaseForGroup( tools::Long nDim ) const;              // any group
    tools::Long                GetGroupBase( tools::Long nGroupDim ) const;
    bool                IsNumOrDateGroup( tools::Long nDim ) const;
    bool                IsInGroup( SCROW nGroupDataId, tools::Long nGroupIndex,
                                   const ScDPItemData& rBaseData, tools::Long nBaseIndex ) const;
    bool                HasCommonElement( SCROW nFirstDataId, tools::Long nFirstIndex,
                                          const ScDPItemData& rSecondData, tools::Long nSecondIndex ) const;

    ResultMembers&      GetDimResultMembers(tools::Long nDim, const ScDPDimension* pDim, ScDPLevel* pLevel) const;

    const ScDPSource& GetSource() const { return mrSource;}
};

class ScDPResultMember
{
private:
    const ScDPResultData*   pResultData;
    ScDPParentDimData        aParentDimData;
    std::unique_ptr<ScDPResultDimension>  pChildDimension;
    std::unique_ptr<ScDPDataMember>       pDataRoot;
    bool                    bHasElements:1;
    bool                    bForceSubTotal:1;
    bool                    bHasHiddenDetails:1;
    bool                    bInitialized:1;
    bool                    bAutoHidden:1;
    ScDPAggData             aColTotal;              // to store column totals

    sal_uInt16                  nMemberStep;            // step to show details
public:
    ScDPResultMember(
        const ScDPResultData* pData,  const ScDPParentDimData& rParentDimData );  //! Ref
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
    OUString GetDisplayName( bool bLocaleIndependent ) const;

    ScDPItemData FillItemData() const;
    bool IsValid() const;
    bool IsVisible() const;
    tools::Long                GetSize(tools::Long nMeasure) const;
    // bHasHiddenDetails is set only if the "show details" flag is off,
    // and there was a child dimension to skip
    bool HasHiddenDetails() const { return bHasHiddenDetails; }
    bool IsSubTotalInTitle(tools::Long nMeasure) const;

    tools::Long                GetSubTotalCount( tools::Long* pUserSubStart = nullptr ) const;

    bool IsNamedItem( SCROW nIndex ) const;
    bool IsValidEntry( const ::std::vector< SCROW >& aMembers ) const;

    void SetHasElements() { bHasElements = true; }
    void SetAutoHidden() { bAutoHidden = true; }

    void                ProcessData( const ::std::vector<SCROW>& aChildMembers,
                                        const ScDPResultDimension* pDataDim,
                                        const ::std::vector<SCROW>& aDataMembers,
                                        const ::std::vector<ScDPValue>& aValues );
    void FillMemberResults(
        css::uno::Sequence< css::sheet::MemberResult>* pSequences,
        tools::Long& rPos, tools::Long nMeasure, bool bRoot, const OUString* pMemberName, const OUString* pMemberCaption );

    void FillDataResults(
        const ScDPResultMember* pRefMember,
        ScDPResultFilterContext& rFilterCxt,
        css::uno::Sequence< css::uno::Sequence<  css::sheet::DataResult> >& rSequence,
        tools::Long nMeasure) const;

    void                UpdateDataResults( const ScDPResultMember* pRefMember, tools::Long nMeasure ) const;
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, tools::Long nMeasure,
                                                ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const;

    void                SortMembers( ScDPResultMember* pRefMember );
    void                DoAutoShow( ScDPResultMember* pRefMember );

    void ResetResults();

#if DUMP_PIVOT_TABLE
    void DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;

    void Dump(int nIndent) const;
#endif

                        //! this will be removed!
    const ScDPResultDimension*  GetChildDimension() const   { return pChildDimension.get(); }
    ScDPResultDimension*        GetChildDimension()         { return pChildDimension.get(); }

    ScDPDataMember*         GetDataRoot() const             { return pDataRoot.get(); }

    const ScDPDimension*  GetParentDim() const               { return aParentDimData.mpParentDim; }     //! Ref
    const ScDPLevel*         GetParentLevel() const         { return aParentDimData.mpParentLevel; }   //! Ref
    const ScDPMember*     GetDPMember()const              { return aParentDimData.mpMemberDesc; }    //! Ref
    SCROW GetOrder() const { return aParentDimData.mnOrder; }         //! Ref
    bool IsRoot() const { return GetParentLevel() == nullptr; }
    SCROW                       GetDataId( ) const ;
    ScDPAggData*        GetColTotal( tools::Long nMeasure ) const;

    void                FillVisibilityData(ScDPResultVisibilityData& rData) const;
};

class ScDPDataMember
{
private:
    const ScDPResultData*       pResultData;
    const ScDPResultMember*     pResultMember;          //! Ref?
    std::unique_ptr<ScDPDataDimension>
                                pChildDimension;
    ScDPAggData                 aAggregate;

    void                UpdateValues( const ::std::vector<ScDPValue>& aValues, const ScDPSubTotalState& rSubState );

public:
                        ScDPDataMember( const ScDPResultData* pData, const ScDPResultMember* pRes );
                        ~ScDPDataMember();

    void                InitFrom( const ScDPResultDimension* pDim );

    OUString GetName() const;
    bool IsVisible() const;
    bool HasData( tools::Long nMeasure, const ScDPSubTotalState& rSubState ) const;

    bool IsNamedItem( SCROW nRow ) const;
    bool HasHiddenDetails() const;

    void                ProcessData( const ::std::vector< SCROW >& aChildMembers, const ::std::vector<ScDPValue>& aValues,
                                       const ScDPSubTotalState& rSubState );
    bool HasError( tools::Long nMeasure, const ScDPSubTotalState& rSubState ) const;
    double              GetAggregate( tools::Long nMeasure, const ScDPSubTotalState& rSubState ) const;
    const ScDPAggData*  GetConstAggData( tools::Long nMeasure, const ScDPSubTotalState& rSubState ) const;
    ScDPAggData*        GetAggData( tools::Long nMeasure, const ScDPSubTotalState& rSubState );

    void FillDataRow(
        const ScDPResultMember* pRefMember,
        ScDPResultFilterContext& rFilterCxt,
        css::uno::Sequence<css::sheet::DataResult>& rSequence,
        tools::Long nMeasure, bool bIsSubTotalRow,
        const ScDPSubTotalState& rSubState) const;

    void UpdateDataRow( const ScDPResultMember* pRefMember, tools::Long nMeasure, bool bIsSubTotalRow,
                        const ScDPSubTotalState& rSubState );
    void UpdateRunningTotals( const ScDPResultMember* pRefMember, tools::Long nMeasure, bool bIsSubTotalRow,
                              const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                              ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent );

    void                SortMembers( ScDPResultMember* pRefMember );
    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults();

#if DUMP_PIVOT_TABLE
    void DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;
    void Dump(int nIndent) const;
#endif

                        //! this will be removed!
    const ScDPDataDimension*    GetChildDimension() const   { return pChildDimension.get(); }
    ScDPDataDimension*          GetChildDimension()         { return pChildDimension.get(); }
};

//  result dimension contains only members

class ScDPResultDimension
{
public:
    typedef std::vector<std::unique_ptr<ScDPResultMember>> MemberArray;
    typedef std::map<SCROW, ScDPResultMember*> MemberHash;
private:
    const ScDPResultData*   pResultData;
    MemberArray             maMemberArray;
    MemberHash              maMemberHash;
    OUString                aDimensionName;     //! or ptr to IntDimension?
    tools::Long                    nSortMeasure;
    ScMemberSortOrder       aMemberOrder;       // used when sorted by measure
    bool                    bIsDataLayout:1;      //! or ptr to IntDimension?
    bool                    bSortByData:1;
    bool                    bSortAscending:1;
    bool                    bAutoShow:1;
    bool                    bAutoTopItems:1;
    bool                    bInitialized:1;
    tools::Long                    nAutoMeasure;
    tools::Long                    nAutoCount;

    ScDPResultMember*        FindMember(   SCROW  iData ) const;
    ScDPResultMember*        AddMember( const ScDPParentDimData& aData );
    ScDPResultMember*        InsertMember( const ScDPParentDimData* pMemberData );
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

    tools::Long                GetSize(tools::Long nMeasure) const;

    bool                IsValidEntry( const ::std::vector<SCROW>& aMembers ) const;

    //  modifies existing members, allocates data dimensions
    void                ProcessData( const ::std::vector<SCROW>& aMembers,
                                     const ScDPResultDimension* pDataDim,
                                     const ::std::vector<SCROW>& aDataMembers,
                                     const ::std::vector<ScDPValue>& aValues ) const;   //! Test
    void                FillMemberResults( css::uno::Sequence<
                                                css::sheet::MemberResult>* pSequences,
                                            tools::Long nStart, tools::Long nMeasure );

    void FillDataResults(
        const ScDPResultMember* pRefMember,
        ScDPResultFilterContext& rFilterCxt,
        css::uno::Sequence<
            css::uno::Sequence<
                css::sheet::DataResult> >& rSequence,
        tools::Long nMeasure) const;

    void                UpdateDataResults( const ScDPResultMember* pRefMember, tools::Long nMeasure ) const;
    void                UpdateRunningTotals( const ScDPResultMember* pRefMember, tools::Long nMeasure,
                                            ScDPRunningTotalState& rRunning, ScDPRowTotals& rTotals ) const;

    void                SortMembers( ScDPResultMember* pRefMember );
    tools::Long                GetSortedIndex( tools::Long nUnsorted ) const;

    void                DoAutoShow( ScDPResultMember* pRefMember );

    void                ResetResults();

    //  called for the reference dimension
    ScDPDataMember* GetRowReferenceMember(
        const ScDPRelativePos* pMemberPos, const OUString* pName,
        const sal_Int32* pRowIndexes, const sal_Int32* pColIndexes ) const;

    // uses row root member from ScDPRunningTotalState
    static ScDPDataMember* GetColReferenceMember(
        const ScDPRelativePos* pMemberPos, const OUString* pName,
        sal_Int32 nRefDimPos, const ScDPRunningTotalState& rRunning );

#if DUMP_PIVOT_TABLE
    void DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;
    void Dump(int nIndent) const;
#endif

                        //  for ScDPDataDimension::InitFrom
    tools::Long                GetMemberCount() const;
    const ScDPResultMember* GetMember(tools::Long n) const;
    ScDPResultMember*       GetMember(tools::Long n);

    const ScMemberSortOrder& GetMemberOrder() const     { return aMemberOrder; }
    ScMemberSortOrder&  GetMemberOrder()                { return aMemberOrder; }

    bool IsDataLayout() const { return bIsDataLayout; }
    const OUString& GetName() const { return aDimensionName; }

    bool IsSortByData() const { return bSortByData; }
    bool IsSortAscending() const { return bSortAscending; }
    tools::Long                GetSortMeasure() const  { return nSortMeasure; }

    bool IsAutoShow() const { return bAutoShow; }
    bool IsAutoTopItems() const { return bAutoTopItems; }
    tools::Long                GetAutoMeasure() const  { return nAutoMeasure; }
    tools::Long                GetAutoCount() const    { return nAutoCount; }

    ScDPResultDimension* GetFirstChildDimension() const;

    void                FillVisibilityData(ScDPResultVisibilityData& rData) const;
};

class ScDPDataDimension
{
private:
    const ScDPResultData*       pResultData;
    const ScDPResultDimension* pResultDimension;  // column
    std::vector<std::unique_ptr<ScDPDataMember>> maMembers;
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
        css::uno::Sequence<css::sheet::DataResult>& rSequence,
        tools::Long nMeasure, bool bIsSubTotalRow, const ScDPSubTotalState& rSubState) const;

    void                UpdateDataRow( const ScDPResultDimension* pRefDim, tools::Long nMeasure, bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState ) const;
    void                UpdateRunningTotals( const ScDPResultDimension* pRefDim, tools::Long nMeasure, bool bIsSubTotalRow,
                                    const ScDPSubTotalState& rSubState, ScDPRunningTotalState& rRunning,
                                    ScDPRowTotals& rTotals, const ScDPResultMember& rRowParent ) const;

    void                SortMembers( ScDPResultDimension* pRefDim );
    tools::Long                GetSortedIndex( tools::Long nUnsorted ) const;

    void                DoAutoShow( ScDPResultDimension* pRefDim );

    void                ResetResults();

#if DUMP_PIVOT_TABLE
    void DumpState( const ScDPResultDimension* pRefDim, ScDocument* pDoc, ScAddress& rPos ) const;
    void Dump(int nIndent) const;
#endif

    tools::Long                GetMemberCount() const;
    const ScDPDataMember*     GetMember(tools::Long n) const;
    ScDPDataMember*     GetMember(tools::Long n);
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
    typedef std::unordered_set<ScDPItemData, MemberHash> VisibleMemberType;
    typedef std::unordered_map<OUString, VisibleMemberType> DimMemberType;
    DimMemberType maDimensions;

    ScDPSource* mpSource;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
