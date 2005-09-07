/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_base.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:17:22 $
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

#ifndef ADC_CPP_CX_BASE_HXX
#define ADC_CPP_CX_BASE_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
    // PARAMETERS


class TextToken;


namespace cpp
{

class Distributor;


class Cx_Base : public ::TkpContext
{
  public:
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  protected:
    // LIFECYCLE
                        Cx_Base(
                            TkpContext *        io_pFollowUpContext );

    void                SetNewToken(
                            DYN ::TextToken *   let_dpToken );
    void                SetFollowUpContext(
                            TkpContext  *       io_pContext );

    Distributor &       Dealer() const;

  private:
    // DATA
    Distributor *       pDealer;
    TkpContext *        pFollowUpContext;
    Dyn< ::TextToken >  pNewToken;
};




inline void
Cx_Base::SetNewToken( DYN ::TextToken * let_dpToken )
    { pNewToken = let_dpToken; }
inline void
Cx_Base::SetFollowUpContext( TkpContext * io_pContext )
    { pFollowUpContext = io_pContext; }
inline Distributor &
Cx_Base::Dealer() const
    { csv_assert(pDealer != 0);
      return *pDealer; }





} // namespace cpp

#endif

