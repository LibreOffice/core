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

#ifndef ADC_ADOC_TK_DOCW_HXX
#define ADC_ADOC_TK_DOCW_HXX

// USED SERVICES
    // BASE CLASSES
#include <adoc/adoc_tok.hxx>
    // COMPONENTS
    // PARAMETERS

namespace adoc {


class Tok_DocWord : public Token
{
  public:
    // Spring and Fall
                        Tok_DocWord(
                            const char *        i_sText )
                                                :   sText(i_sText) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    uintt               Length() const          { return sText.length(); }

  private:
    // DATA
    String              sText;
};

class Tok_Whitespace : public Token
{
  public:
    // Spring and Fall
                        Tok_Whitespace(
                            UINT8               i_nSize )
                                                :   nSize(i_nSize) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    UINT8               Size() const            { return nSize; }

  private:
    // DATA
    UINT8               nSize;
};

class Tok_LineStart : public Token
{
  public:
    // Spring and Fall
                        Tok_LineStart(
                            UINT8               i_nSize )
                                                :   nSize(i_nSize) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    UINT8               Size() const            { return nSize; }

  private:
    // DATA
    UINT8               nSize;
};

class Tok_Eol : public Token
{ public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};

class Tok_EoDocu : public Token
{ public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};


}   // namespace adoc

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
