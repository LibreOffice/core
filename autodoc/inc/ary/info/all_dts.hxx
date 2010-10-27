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
