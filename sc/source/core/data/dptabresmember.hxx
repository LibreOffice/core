/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dptabresmember.hxx,v $
 * $Revision: 1.0 $
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

#ifndef DPTABRESMEMBER_HXX
#define DPTABRESMEMBER_HXX

class ScDPNormalResultMember: public ScDPResultMember
{
private:
    // Wang Xu Ming -- 2009-6-9
    // DataPilot Migration
    ScDPParentDimData       aParentDimData;
    // End Comments
    BOOL                    bHasElements:1;
    BOOL                    bForceSubTotal:1;
    BOOL                    bHasHiddenDetails:1;
    BOOL                    bInitialized:1;
    BOOL                    bAutoHidden:1;
    ScDPAggData             aColTotal;              // to store column totals

// Wang Xu Ming -- 2009-6-9
// DataPilot Migration
    USHORT                  nMemberStep;            // step to show details
// End Comments
public:
                            // Wang Xu Ming -- 2009-6-9
                            // DataPilot Migration
                            ScDPNormalResultMember(  const ScDPResultData* pData,  const ScDPParentDimData& rParentDimData,
                                BOOL bForceSub );   //! Ref
                            ScDPNormalResultMember(  const ScDPResultData* pData, BOOL bForceSub );
                            // End Comments
                            ~ScDPNormalResultMember();

                            // Wang Xu Ming -- 2009-6-9
                            // DataPilot Migration
                            // Add parameter: BOOL bInitChild
                            void                InitFrom( const ::std::vector<ScDPDimension*>& ppDim,
                                const ::std::vector<ScDPLevel*>& ppLev,
                                size_t nPos,
                                ScDPInitState& rInitState,
                                BOOL bInitChild = TRUE );
                            // End Comments
                            // Wang Xu Ming -- 2009-6-9
                            // DataPilot Migration
                            void     LateInitFrom(
                                LateInitParams& rParams,
                                const ::std::vector< SCROW >& pItemData,
                                size_t nPos,
                                ScDPInitState& rInitState);
                            // End Comments

    String              GetName() const;
    void                  FillItemData( ScDPItemData& rData ) const;
    BOOL                IsValid() const;
    BOOL                IsVisible() const;
    long                    GetSize(long nMeasure) const;
    BOOL                HasHiddenDetails() const;
    BOOL              IsSubTotalInTitle(long nMeasure) const;

//  BOOL                SubTotalEnabled() const;
    long                GetSubTotalCount( long* pUserSubStart = NULL ) const;

// Wang Xu Ming -- 2009-6-9
// DataPilot Migration
// Use item index
//  BOOL                IsNamedItem( const ScDPItemData& r ) const;
    BOOL                IsNamedItem( SCROW nIndex ) const;
// End Comments
    bool IsValidEntry( const ::std::vector< SCROW >& aMembers ) const;
    ENTRYSTATUS GetEntryStatus( const ::std::vector<SCROW>& aMembers ) const;

    void                SetHasElements()    { bHasElements = TRUE; }
    void                SetAutoHidden()     { bAutoHidden = TRUE; }

    void                ProcessData( const ::std::vector<SCROW>& aChildMembers,
                                     const ScDPResultDimension* pDataDim,
                                     const ::std::vector<SCROW>& aDataMembers,
                                     const ::std::vector<ScDPValueData>& aValues );

    void                FillMemberResults( com::sun::star::uno::Sequence<
                                                com::sun::star::sheet::MemberResult>* pSequences,
                                            long& rPos, long nMeasure, BOOL bRoot,
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

    void                ResetResults( BOOL bRoot );

    void                DumpState( const ScDPResultMember* pRefMember, ScDocument* pDoc, ScAddress& rPos ) const;

    // Wang Xu Ming -- 2009-6-9
    // DataPilot Migration
    const ScDPDimension*        GetParentDim() const            { return aParentDimData.mpParentDim; }      //! Ref
    const ScDPLevel*                GetParentLevel() const          { return aParentDimData.mpParentLevel; }    //! Ref
    const ScDPMember*           GetDPMember()const              { return aParentDimData.mpMemberDesc; }     //! Ref
    inline SCROW                    GetOrder() const                { return aParentDimData.mnOrder; }          //! Ref
    inline BOOL                     IsRoot() const                   { return GetParentLevel() == NULL; }
    // End Comments

    ScDPAggData*        GetColTotal( long nMeasure ) const;

    void                FillVisibilityData(ScDPResultVisibilityData& rData) const;
// Wang Xu Ming -- 2009-6-10
// DataPilot Migration
    SCROW GetDataId( ) const ;
// End Comments
};

class ScDPHideDetailsMember: public ScDPResultMember
{
private:
    ScDPNormalResultMember *  pOrigMember;
public:
    ScDPHideDetailsMember(  const ScDPResultData* pData,  const ScDPParentDimData& rParentDimData,
                                BOOL bForceSub );


};


#endif //DPTABRESMEMBER_HXX
