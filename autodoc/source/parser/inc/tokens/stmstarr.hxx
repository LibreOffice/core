/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstarr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:39:53 $
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

#ifndef ADC_STMSTARR_HXX
#define ADC_STMSTARR_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstate.hxx>
    // COMPONENTS
    // PARAMETERS
#include <tokens/token.hxx>


class StmArrayStatus : public StmStatus
{
  public:
    typedef TextToken::F_CRTOK F_CRTOK;

    // LIFECYCLE
                        StmArrayStatus(
                            intt                i_nStatusSize,
                            const INT16 *       in_aArrayModel,
                            F_CRTOK             i_fTokenCreateFunction,
                            bool                in_bIsDefault );
                        ~StmArrayStatus();

    // INQUIRY
    StmStatus::Branch   NextBy(
                            intt                in_nFollowersIndex) const;
    F_CRTOK             TokenCreateFunction() const
                                                { return fTokenCreateFunction; }
    virtual bool        IsADefault() const;

    // ACCESS
    virtual StmArrayStatus *
                        AsArray();
    void                SetBranches(
                            intt                in_nStartBranchIx,
                            StmStatus::Branch * in_aBranchValues,
                            intt                in_nNrofValues );
    bool                SetBranch(
                            intt                in_nBranchIx,
                            StmStatus::Branch   in_nBranch );
    void                SetTokenCreateFunction(
                            F_CRTOK             i_fTokenCreateFunction );
  private:
    StmStatus::Branch * dpBranches;
    intt                nNrOfBranches;
    F_CRTOK             fTokenCreateFunction;
    bool                bIsADefault;
};


// IMPLEMENTATION

inline void
StmArrayStatus::SetTokenCreateFunction( F_CRTOK i_fTokenCreateFunction )
    { fTokenCreateFunction = i_fTokenCreateFunction; }



#endif


