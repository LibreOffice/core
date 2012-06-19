/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
