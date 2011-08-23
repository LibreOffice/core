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

#ifndef ARY_CPP_TPLPARAM_HXX
#define ARY_CPP_TPLPARAM_HXX

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>




namespace ary
{
namespace cpp
{
    class UsedType;
    class Gate;

namespace ut
{


class TemplateParameter
{
  public:
    virtual             ~TemplateParameter() {}

    virtual intt		Compare(
                            const TemplateParameter &
                                                i_rOther ) const = 0;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const = 0;
};


class TplParameter_Type : public TemplateParameter
{
  public:
                        TplParameter_Type(
                            Tid					i_nType );
                        ~TplParameter_Type();

    virtual intt		Compare(
                            const TemplateParameter &
                                                i_rOther ) const;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
  private:
    Tid					nType;
};

}   // namespace ut
}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
