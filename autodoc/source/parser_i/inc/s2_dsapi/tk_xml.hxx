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

#ifndef DSAPI_TK_XML_HXX
#define DSAPI_TK_XML_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/dsapitok.hxx>
    // COMPONENTS
    // PARAMETERS
#include <luxenum.hxx>


namespace csi
{
namespace dsapi
{


class Tok_XmlTag : public Token
{
  public:
};

class Tok_XmlConst : public Tok_XmlTag
{
  public:
    // TYPE
    enum  E_TokenId
    {
        e_none = 0,
        e_true = 1,
        e_false = 2,
        e_null = 3,
        e_void = 4
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

    // Spring and Fall
                        Tok_XmlConst(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class Tok_XmlLink_Tag : public Tok_XmlTag
{
  public:
    // TYPE
    enum E_TokenId
    {
        e_none = 0,
        e_const = 1,
        member = 2,
        type = 3
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;
};

class Tok_XmlLink_BeginTag : public Tok_XmlLink_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlLink_BeginTag(
                            EV_TokenId          i_eTag,
                            const String  &     i_sScope,
                            const String  &     i_sDim )
                                                :   eTag(i_eTag),
                                                    sScope(i_sScope),
                                                    sDim(i_sDim) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }
    const String  &     Scope() const           { return sScope; }
    const String  &     Dim() const             { return sDim; }

  private:
    // DATA
    EV_TokenId          eTag;
    String              sScope;
    String              sDim;
};

class Tok_XmlLink_EndTag : public Tok_XmlLink_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlLink_EndTag(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};

class Tok_XmlFormat_Tag : public Tok_XmlTag
{
  public:
    // TYPE
    enum E_TokenId
    {
        e_none = 0,
        code = 1,
        listing = 2,
        atom = 3
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;
};

class Tok_XmlFormat_BeginTag : public Tok_XmlFormat_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlFormat_BeginTag(
                            EV_TokenId          i_eTag,
                            const String  &     i_sDim )
                                                :   eTag(i_eTag),
                                                    sDim(i_sDim) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }
    const String  &     Dim() const             { return sDim; }

  private:
    // DATA
    EV_TokenId          eTag;
    String              sDim;
};

class Tok_XmlFormat_EndTag : public Tok_XmlFormat_Tag
{
  public:
    // Spring and Fall
                        Tok_XmlFormat_EndTag(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    E_TokenId           Id() const              { return eTag; }

  private:
    // DATA
    EV_TokenId          eTag;
};


}   // namespace dsapi
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
