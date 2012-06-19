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

#ifndef ADC_DISPLAY_HFI_PROPERTY_HXX
#define ADC_DISPLAY_HFI_PROPERTY_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>

class HF_SubTitleTable;

class HF_IdlDataMember : public HtmlFactory_Idl
{
  public:
    void                Produce_byData(
                            const client &      ce ) const;
  protected:
                        HF_IdlDataMember(
                            Environment &       io_rEnv,
                            HF_SubTitleTable &  o_table );
    virtual             ~HF_IdlDataMember() {}

  private:
    virtual void        write_Title(
                            const client &      i_ce ) const;

    virtual void        write_Declaration(
                            const client &      i_ce ) const = 0;

    virtual void        write_Description(
                            const client &      i_ce ) const;

    void                enter_ContentCell() const;
    void                leave_ContentCell() const;
};



class HF_IdlProperty : public HF_IdlDataMember
{
  public:
                        HF_IdlProperty(
                            Environment &       io_rEnv,
                            HF_SubTitleTable &  o_table )
                            :   HF_IdlDataMember(io_rEnv, o_table) {}
    virtual             ~HF_IdlProperty();
  private:
    virtual void        write_Declaration(
                            const client &      i_ce ) const;
};

class HF_IdlAttribute : public HF_IdlDataMember
{
  public:
                        HF_IdlAttribute(
                            Environment &       io_rEnv,
                            HF_SubTitleTable &  o_table )
                            :   HF_IdlDataMember(io_rEnv, o_table) {}
    virtual             ~HF_IdlAttribute();

  private:
    virtual void        write_Declaration(
                            const client &      i_ce ) const;
};


class HF_IdlEnumValue : public HF_IdlDataMember
{
  public:
                        HF_IdlEnumValue(
                            Environment &       io_rEnv,
                            HF_SubTitleTable &  o_table )
                            :   HF_IdlDataMember(io_rEnv, o_table) {}
    virtual             ~HF_IdlEnumValue();

  private:
    virtual void        write_Declaration(
                            const client &      i_ce ) const;
};

class HF_IdlConstant : public HF_IdlDataMember
{
  public:
                        HF_IdlConstant(
                            Environment &       io_rEnv,
                            HF_SubTitleTable &  o_table )
                            :   HF_IdlDataMember(io_rEnv, o_table) {}
    virtual             ~HF_IdlConstant();

  private:
    virtual void        write_Declaration(
                            const client &      i_ce ) const;
};


class HF_IdlStructElement : public HF_IdlDataMember
{
  public:
                        HF_IdlStructElement(
                            Environment &       io_rEnv,
                            HF_SubTitleTable &  o_table )
                            :   HF_IdlDataMember(io_rEnv, o_table) {}
    virtual             ~HF_IdlStructElement();

  private:
    virtual void        write_Declaration(
                            const client &      i_ce ) const;
};

class HF_IdlCommentedRelationElement : public HF_IdlDataMember
{
  public:
                        HF_IdlCommentedRelationElement(
                            Environment &                       io_rEnv,
                            HF_SubTitleTable &                  o_table,
                            const ary::idl::CommentedRelation&  i_relation )
                            :   HF_IdlDataMember(io_rEnv, o_table)
                            ,   m_relation( i_relation )
                        {
                        }
    virtual             ~HF_IdlCommentedRelationElement();

    typedef ::ary::idl::CommentedRelation       comref;

    static void         produce_LinkDoc(
                            Environment &   io_env,
                            const client &  i_ce,
                            Xml::Element &  io_context,
                            const comref &  i_commentedRef,
                            const E_DocType i_docType );

  private:
    virtual void        write_Title(
                            const client &      i_ce ) const;
    virtual void        write_Declaration(
                            const client &      i_ce ) const;
    virtual void        write_Description(
                            const client &      i_ce ) const;
  private:
    static void         produce_Summary( Environment &  io_env,
                                         Xml::Element & io_context,
                                         const comref & i_commentedRef,
                                         const client & i_rScopeGivingCe );

    static String       get_LocalLinkName( Environment &  io_env,
                                           const comref & i_commentedRef );

  private:
    const ary::idl::CommentedRelation&  m_relation;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
