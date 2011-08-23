/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    virtual bool		PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  protected:
    // LIFECYCLE
                        Cx_Base(
                            TkpContext *		io_pFollowUpContext );

    void				SetNewToken(
                            DYN ::TextToken *	let_dpToken );
    void				SetFollowUpContext(
                            TkpContext  *	    io_pContext );

    Distributor &       Dealer() const;

  private:
    // DATA
    Distributor *		pDealer;
    TkpContext *		pFollowUpContext;
    Dyn< ::TextToken >	pNewToken;
};




inline void
Cx_Base::SetNewToken( DYN ::TextToken *	let_dpToken )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
