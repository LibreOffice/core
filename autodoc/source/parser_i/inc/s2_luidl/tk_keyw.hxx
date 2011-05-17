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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
