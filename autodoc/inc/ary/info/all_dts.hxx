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

#ifndef ARY_INFO_ALL_DTS_HXX
#define ARY_INFO_ALL_DTS_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{

class DocuDisplay;

class DocuToken
{
  public:
    virtual             ~DocuToken() {}

    void                StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    bool                IsWhite() const;

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const = 0;
    virtual bool        inq_IsWhite() const = 0;
};

class DT_Text : public DocuToken
{
  public:
                        DT_Text(
                            const char *        i_sText )
                                                :   sText( i_sText ) {}

    const String  &     Text() const            { return sText; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    String              sText;
};

class DT_MaybeLink : public DocuToken
{
  public:
                        DT_MaybeLink(
                            const char *        i_sText,
                            bool                i_bIsGlobal,
                            bool                i_bIsFunction  )
                                                :   sText( i_sText ),
                                                    bIsGlobal(i_bIsGlobal),
                                                    bIsFunction(i_bIsFunction) { }

    const String  &     Text() const            { return sText; }
    bool                IsAbsolute() const      { return bIsGlobal; }
    bool                IsFunction() const      { return bIsFunction; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    String              sText;
    bool                bIsGlobal;
    bool                bIsFunction;
};

class DT_Whitespace : public DocuToken
{
  public:
                        DT_Whitespace(
                            UINT8               i_nLength )
                                                :   nLength( i_nLength ) {}
    UINT8               Length() const          { return nLength; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    UINT8               nLength;
};


class DT_Eol : public DocuToken
{
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;
};

class DT_Xml : public DocuToken
{
  public:
                        DT_Xml(
                            const char *        i_sText )
                                                :   sText( i_sText ) {}

    const String  &     Text() const            { return sText; }

  private:
    virtual void        do_StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    virtual bool        inq_IsWhite() const;

    String              sText;
};


// IMPLEMENTATION

inline void
DocuToken::StoreAt( DocuDisplay & o_rDisplay ) const
    { do_StoreAt(o_rDisplay); }
inline bool
DocuToken::IsWhite() const
    { return inq_IsWhite(); }



}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
