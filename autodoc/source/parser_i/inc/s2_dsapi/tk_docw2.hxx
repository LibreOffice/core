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

#ifndef DSAPI_TK_DOCW2_HXX
#define DSAPI_TK_DOCW2_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/dsapitok.hxx>
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace dsapi
{


class Tok_Word : public Token
{
  public:
    // Spring and Fall
                        Tok_Word(
                            const char *		i_sText )
                                                :	sText(i_sText) {}
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;

  private:
    // DATA
    String 				sText;
};

class Tok_Comma : public Token
{
  public:
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;
};

class Tok_DocuEnd : public Token
{
  public:
    // Spring and Fall
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;
};

class Tok_EOL : public Token
{
  public:
    // Spring and Fall
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;
};

class Tok_EOF : public Token
{
  public:
    // Spring and Fall
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;
};

class Tok_White : public Token
{
  public:
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;
};



}   // namespace dsapi
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
