/*************************************************************************
 *
 *  $RCSfile: dptabres.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-07 17:43:47 $
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

#ifndef SC_DPTABRES_HXX
#define SC_DPTABRES_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_MEMBERRESULT_HPP_
#include <com/sun/star/sheet/MemberResult.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_DATARESULT_HPP_
#include <com/sun/star/sheet/DataResult.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"       // enum ScSubTotalFunc
#endif


class ScAddress;
class ScDocument;
class ScDPSource;
class ScDPDimension;
class ScDPLevel;
class ScDPMember;
class ScDPAggData;

struct ScDPValueData;

//
//  aggregated data
//! separate header file?
//

class ScDPAggData
{
private:
    double          fVal;
    double          fSquare;
    long            nCount;
    ScDPAggData*    pChild;

public:
            ScDPAggData() : fVal(0.0), fSquare(0.0), nCount(0), pChild(NULL) {}
            ~ScDPAggData() { delete pChild; }

    void    Update( const ScDPValueData& rNext, ScSubTotalFunc eFunc );
    double  GetResult( ScSubTotalFunc eFunc ) const;
    BOOL    HasError( ScSubTotalFunc eFunc ) const;
    BOOL    HasData() const                         { return ( nCount != 0 ); }

    const ScDPAggData*  GetExistingChild() const    { return pChild; }
    ScDPAggData*        GetChild();
};


// --------------------------------------------------------------------
//
//  results for a hierarchy dimension
//

#define SC_DP_RES_GROW  16

class ScDPResultDimension;
class ScDPDataDimension;
class ScDPDataMember;
struct ScDPItemData;


#define SC_DPMEASURE_ALL    -1
#define SC_DPMEASURE_ANY    -2

class ScDPResultData
{
private:
    ScDPSource*             pSource;                //! Ref
    //! keep things like measure lists here

    long                    nMeasCount;
    ScSubTotalFunc*         pMeasFuncs;
    String*                 pMeasNames;
    BOOL                    bDataAtCol;
    BOOL                    bDataAtRow;

public:
                        ScDPResultData( ScDPSource* pSrc );     //! Ref
                        ~ScDPResultData();

    void                SetMeasureData( long nCount, const ScSubTotalFunc* pFunctions,
                                        const String* pNames );
    void                SetDataLayoutOrientation( USHORT nOrient );

    long                GetMeasureCount() const     { return nMeasCount; }
    ScSubTotalFunc      GetMeasureFunction(long nMeasure) const;
    String              GetMeasureString(long nMeasure, BOOL bForce, ScSubTotalFunc eForceFunc) const;
    String              GetMeasureDimensionName(long nMeasure) const;

    BOOL                IsDataAtCol() const             { return bDataAtCol; }
    BOOL                IsDataAtRow() const             { return bDataAtRow; }

    long                GetColStartMeasure() const;
    long                GetRowStartMeasure() const;

    long                GetCountForMeasure( long nMeas ) const
                                { return ( nMeas == SC_DPMEASURE_ALL ) ? nMeasCount : 1; }
};


class ScDPResultMember
{
private:
    ScDPResultData*         pResultData;
    ScDPDimension*          pParentDim;             //! Ref
    ScDPLevel*              pParentLevel;           //! Ref
    ScDPMember*             pMemberDesc;            //! Ref
    ScDPResultDimension*    pChildDimension;
    ScDPDataMember*         pDataRoot;
    BOOL                    bHasElements;
    BOOL                    bForceSubTotal;
    BOOL                    bInitialized;

public:
                        ScDPResultMember( ScDPResultData* pData, ScDPDimension* pDim,
                                            ScDPLevel* pLev, ScDPMember* pDesc,
                                            BOOL bForceSub );   //! Ref
                        ~ScDPResultMember();

    void                InitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev );
    void                LateInitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPItemData* pItemData );

    String              GetName() const;
    BOOL                IsValid() const;
    BOOL                IsVisible() const;
    long                GetSize(long nMeasure) const;

//  BOOL                SubTotalEnabled() const;
    long                GetSubTotalCount() const;

    BOOL                IsNamedItem( const ScDPItemData& r ) const;

    void                SetHasElements()    { bHasElements = TRUE; }

    void                ProcessData( const ScDPItemData* pChildMembers,
                                        ScDPResultDimension* pDataDim,
                                        const ScDPItemData* pDataMembers,
                                        const ScDPValueData* pValues );

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

                        //! this will be removed!
    const ScDPResultDimension*  GetChildDimension() const   { return pChildDimension; }
    ScDPResultDimension*        GetChildDimension()         { return pChildDimension; }

    ScDPDimension*          GetParentDim()      { return pParentDim; }          //! Ref
    ScDPLevel*              GetParentLevel()    { return pParentLevel; }        //! Ref
};

class ScDPDataMember
{
private:
    ScDPResultData*         pResultData;
    ScDPResultMember*       pResultMember;          //! Ref?
    ScDPDataDimension*      pChildDimension;
    ScDPAggData             aAggregate;

    void                UpdateValues(const ScDPValueData* pValues);

public:
                        ScDPDataMember( ScDPResultData* pData, ScDPResultMember* pRes );
                        ~ScDPDataMember();

    void                InitFrom( ScDPResultDimension* pDim );

    String              GetName() const;
    BOOL                HasData(long nMeasure) const;

    BOOL                IsNamedItem( const ScDPItemData& r ) const;

    void                ProcessData( const ScDPItemData* pChildMembers, const ScDPValueData* pValues );

    BOOL                HasError(long nMeasure) const;
    double              GetAggregate(long nMeasure) const;

    void                FillDataRow( const ScDPResultMember* pRefMember,
                                    com::sun::star::uno::Sequence<com::sun::star::sheet::DataResult>& rSequence,
                                    long& rCol, long nMeasure, BOOL bIsSubTotalRow ) const;

                        //! this will be removed!
    const ScDPDataDimension*    GetChildDimension() const   { return pChildDimension; }
    ScDPDataDimension*          GetChildDimension()         { return pChildDimension; }
};

//! replace PtrArr with 32-bit array ????

typedef ScDPResultMember* ScDPResultMemberPtr;
SV_DECL_PTRARR_DEL(ScDPResultMembers, ScDPResultMemberPtr, SC_DP_RES_GROW, SC_DP_RES_GROW);

typedef ScDPDataMember* ScDPDataMemberPtr;
SV_DECL_PTRARR_DEL(ScDPDataMembers, ScDPDataMemberPtr, SC_DP_RES_GROW, SC_DP_RES_GROW);


//  result dimension contains only members

class ScDPResultDimension
{
private:
    ScDPResultData*         pResultData;
    ScDPResultMembers       aMembers;
    BOOL                    bIsDataLayout;      //! or ptr to IntDimension?

public:
                        ScDPResultDimension( ScDPResultData* pData );
                        ~ScDPResultDimension();

                        //  allocates new members
    void                InitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev );
    void                LateInitFrom( ScDPDimension** ppDim, ScDPLevel** ppLev, ScDPItemData* pItemData );

    long                GetSize(long nMeasure) const;

    BOOL                IsValidEntry( const ScDPItemData* pMembers ) const;

                        //  modifies existing members, allocates data dimensions
    void                ProcessData( const ScDPItemData* pMembers,
                                        ScDPResultDimension* pDataDim,
                                        const ScDPItemData* pDataMembers,
                                        const ScDPValueData* pValues ); //! Test

    void                FillMemberResults( com::sun::star::uno::Sequence<
                                                com::sun::star::sheet::MemberResult>* pSequences,
                                            long nStart, long nMeasure );

    void                FillDataResults( const ScDPResultMember* pRefMember,
                                    com::sun::star::uno::Sequence<
                                        com::sun::star::uno::Sequence<
                                            com::sun::star::sheet::DataResult> >& rSequence,
                                    long nRow, long nMeasure ) const;

                        //  for ScDPDataDimension::InitFrom
    long                GetMemberCount() const;
    ScDPResultMember*   GetMember(long n) const;

    BOOL                IsDataLayout() const    { return bIsDataLayout; }

    ScDPResultDimension* GetFirstChildDimension() const;
};

class ScDPDataDimension
{
private:
    ScDPResultData*     pResultData;
    ScDPDataMembers     aMembers;
    BOOL                bIsDataLayout;      //! or ptr to IntDimension?

public:
                        ScDPDataDimension( ScDPResultData* pData );
                        ~ScDPDataDimension();

    void                InitFrom( ScDPResultDimension* pDim );      // recursive
    void                ProcessData( const ScDPItemData* pDataMembers, const ScDPValueData* pValues );

    void                FillDataRow( const ScDPResultDimension* pRefDim,
                                    com::sun::star::uno::Sequence<com::sun::star::sheet::DataResult>& rSequence,
                                    long nCol, long nMeasure, BOOL bIsSubTotalRow ) const;
};

#endif

