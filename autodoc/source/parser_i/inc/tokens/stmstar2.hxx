/*************************************************************************
 *
 *  $RCSfile: stmstar2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:36 $
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


