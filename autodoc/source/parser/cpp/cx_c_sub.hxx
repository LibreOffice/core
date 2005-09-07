/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_c_sub.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:19:16 $
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

#ifndef ADC_CPP_CX_C_SUB_HXX
#define ADC_CPP_CX_C_SUB_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace cpp {


class Context_Comment : public Cx_Base
{
  public:
                        Context_Comment(
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(&i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    void                SetMode_IsMultiLine(
                            bool                i_bTrue )
                                                { bCurrentModeIsMultiline = i_bTrue; }
  private:
    bool                bCurrentModeIsMultiline;
};

class Context_ConstString : public Cx_Base
{
  public:
                        Context_ConstString(
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(&i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};

class Context_ConstChar : public Cx_Base
{
  public:
                        Context_ConstChar(
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(&i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};

class Context_ConstNumeric : public Cx_Base
{
  public:
                        Context_ConstNumeric(
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(&i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};

class Context_UnblockMacro : public Cx_Base
{
  public:
                        Context_UnblockMacro(
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(&i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};



}   // namespace cpp


#endif

