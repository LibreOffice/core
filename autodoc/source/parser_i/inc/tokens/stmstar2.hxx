/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstar2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:08:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADC_STMSTAR2_HXX
#define ADC_STMSTAR2_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstat2.hxx>
    // COMPONENTS
    // PARAMETERS
    // SERVICES


class StmArrayStatu2 : public StmStatu2
{
  public:
    // LIFECYCLE
                        StmArrayStatu2(
                            intt            i_nStatusSize,
                            const INT16 *   in_aArrayModel,
                            uintt           i_nTokenId,
                            bool            in_bIsDefault );
                        ~StmArrayStatu2();

    // INQUIRY
    StmStatu2::Branch   NextBy(
                            intt            in_nFollowersIndex) const;
    UINT16              TokenId() const     { return nTokenId; }
    virtual bool        IsADefault() const;

    // ACCESS
    virtual StmArrayStatu2 *
                        AsArray();
    void                SetBranches(
                            intt            in_nStartBranchIx,
                            StmStatu2::Branch *
                                            in_aBranchValues,
                            intt            in_nNrofValues );
    bool                SetBranch(
                            intt            in_nBranchIx,
                            StmStatu2::Branch
                                            in_nBranch );
    void                SetTokenId(
                            UINT16          in_nTokenId );
  private:
    StmStatu2::Branch * dpBranches;
    intt                nNrOfBranches;
    UINT16              nTokenId;
    bool                bIsADefault;
};


// IMPLEMENTATION

inline void
StmArrayStatu2::SetTokenId( UINT16 in_nTokenId )
    { nTokenId = in_nTokenId; }



#endif


