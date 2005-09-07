/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstfi2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:08:58 $
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

#ifndef ADC_STMSTFI2_HXX
#define ADC_STMSTFI2_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstat2.hxx>
    // COMPONENTS
    // PARAMETERS


class TkpContext;
class StateMachineContext;

/**
**/
class StmBoundsStatu2 : public StmStatu2
{
  public:
    // LIFECYCLE
                        StmBoundsStatu2(
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
    virtual StmBoundsStatu2 *
                        AsBounds();

  private:
    StateMachineContext *
                        pOwner;
    TkpContext *        pFollowUpContext;
    uintt               nStatusFunctionNr;
    bool                bIsDefault;
};

inline TkpContext *
StmBoundsStatu2::FollowUpContext()
    { return pFollowUpContext; }
inline uintt
StmBoundsStatu2::StatusFunctionNr() const
    { return nStatusFunctionNr; }


#endif


