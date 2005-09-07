/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstfin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:40:23 $
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

#ifndef ADC_STMSTFIN_HXX
#define ADC_STMSTFIN_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstate.hxx>
    // COMPONENTS
    // PARAMETERS


class TkpContext;
class StateMachineContext;

/**
**/
class StmBoundsStatus : public StmStatus
{
  public:
    // LIFECYCLE
                        StmBoundsStatus(
                            StateMachineContext &
                                                o_rOwner,
                            TkpContext &        i_rFollowUpContext,
                            uintt               i_nStatusFunctionNr,
                            bool                i_bIsDefault  );
    // INQUIRY
    TkpContext *        FollowUpContext();
    uintt               StatusFunctionNr() const;
    virtual bool        IsADefault() const;

    // ACCESS
    virtual StmBoundsStatus *
                        AsBounds();

  private:
    StateMachineContext *
                        pOwner;
    TkpContext *        pFollowUpContext;
    uintt               nStatusFunctionNr;
    bool                bIsDefault;
};

inline TkpContext *
StmBoundsStatus::FollowUpContext()
    { return pFollowUpContext; }
inline uintt
StmBoundsStatus::StatusFunctionNr() const
    { return nStatusFunctionNr; }


#endif


