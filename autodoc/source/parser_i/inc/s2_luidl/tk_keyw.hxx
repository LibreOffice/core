/*************************************************************************
 *
 *  $RCSfile: tk_keyw.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:46:19 $
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

#ifndef ADC_UIDL_TK_KEYW_HXX
#define ADC_UIDL_TK_KEYW_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/uidl_tok.hxx>
    // COMPONENTS
#include <luxenum.hxx>
    // PARAMETERS


namespace csi
{
namespace uidl
{


class TokKeyword : public Token
{
};


class TokBuiltInType : public TokKeyword
{
  public:
    enum E_TokenId
    {
        e_none = 0,
        bty_any = 1,
        bty_boolean = 2,
        bty_byte = 3,
        bty_char = 4,
        bty_double = 5,
        bty_hyper = 6,
        bty_long = 7,
        bty_short = 8,
        bty_string = 9,
        bty_void = 10,
        bty_ellipse = 11
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

                        TokBuiltInType(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}

    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};


class TokTypeModifier : public TokKeyword
{
  public:
    enum E_TokenId
    {
        e_none = 0,
        tmod_unsigned = 1,
        tmod_sequence
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

                        TokTypeModifier(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class TokMetaType : public TokKeyword
{
  public:
    enum E_TokenId
    {
        e_none = 0,
        mt_attribute = 1,
        mt_constants,
        mt_enum,
        mt_exception,
        mt_ident,
        mt_interface,
        mt_module,
        mt_property,
        mt_service,
        mt_singleton,
        mt_struct,
        mt_typedef,
        mt_uik
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

                        TokMetaType(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}

    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
    E_TokenId           Id() const              { return eTag; }


  private:
    // DATA
    EV_TokenId          eTag;
};

class TokStereotype : public TokKeyword
{
  public:
    // TYPES
    enum E_TokenId
    {
        e_none = 0,
        ste_bound = 1,
        ste_const,
        ste_constrained,
        ste_maybeambiguous,
        ste_maybedefault,
        ste_maybevoid,
        ste_oneway,
        ste_optional,
        ste_readonly,
        ste_removable,
        ste_virtual,
        ste_transient,
        ste_published
    };

    typedef lux::Enum<E_TokenId> EV_TokenId;

                        TokStereotype(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class TokParameterHandling : public TokKeyword
{
  public:
    // TYPES
    enum E_TokenId
    {
        e_none = 0,
        ph_in,
        ph_out,
        ph_inout
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

                        TokParameterHandling(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class TokRaises : public TokKeyword
{
  public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};

class TokNeeds : public TokKeyword
{
  public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};

class TokObserves : public TokKeyword
{
  public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};


}   // namespace uidl
}   // namespace csi

#endif


