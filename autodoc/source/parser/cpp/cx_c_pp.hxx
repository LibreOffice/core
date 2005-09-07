/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_c_pp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:17:58 $
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

#ifndef ADC_CPP_CX_C_PP_HXX
#define ADC_CPP_CX_C_PP_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace cpp
{

class Context_Preprocessor : public Cx_Base
{
  public:
                        Context_Preprocessor(
                            TkpContext &        i_rFollowUpContext );
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  private:
    // Locals
    void                ReadDefault(
                            CharacterSource &   io_rText );
    void                ReadDefine(
                            CharacterSource &   io_rText );

    // DATA
    TkpContext *        pContext_Parent;
    Dyn<Cx_Base>        pContext_PP_MacroParams;
    Dyn<Cx_Base>        pContext_PP_Definition;
};

class Context_PP_MacroParams : public Cx_Base
{
  public:
                        Context_PP_MacroParams(
                            Cx_Base &           i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
  private:
    // DATA
    Cx_Base *           pContext_PP_Definition;
};

class Context_PP_Definition : public Cx_Base
{
  public:
                        Context_PP_Definition(
                            TkpContext &        i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );

  private:
    // DATA
    TkpContext *        pContext_Parent;
};


}   // namespace cpp

#endif

