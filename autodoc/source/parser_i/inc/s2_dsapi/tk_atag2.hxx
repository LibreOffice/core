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

#ifndef DSAPI_TK_ATAG2_HXX
#define DSAPI_TK_ATAG2_HXX

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


class Tok_AtTag : public Token
{
  public:
    // TYPE
    enum E_TokenId
    {
        e_none = 0,
        author =  1,
        see = 2,
        param = 3,
        e_return = 4,
        e_throw = 5,
        example = 6,
        deprecated = 7,
        suspicious = 8,
        missing = 9,
        incomplete = 10,
        version = 11,
        guarantees = 12,
        exception = 13,
        since = 14
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;

    // Spring and Fall
                        Tok_AtTag(
                            EV_TokenId			i_eTag )
                                                :	eTag(i_eTag) {}
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char*	Text() const;
    E_TokenId			Id() const				{ return eTag; }

  private:
    EV_TokenId			eTag;
};

}   // namespace dsapi
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
