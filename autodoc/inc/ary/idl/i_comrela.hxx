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

#ifndef ARY_IDL_I_COMRELA_HXX
#define ARY_IDL_I_COMRELA_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>

namespace ary
{
namespace doc
{
    class OldIdlDocu;
}
}




namespace ary
{
namespace idl
{


/** Contains data for an IDL code entity related to another one like a base of
    an interface or of a service or the supported interface of a service.
*/
class CommentedRelation
{
  public:
    // LIFECYCLE

                        CommentedRelation(
                            Type_id             i_nType,
                            doc::OldIdlDocu *   i_pInfo )
                                                :   nType(i_nType),
                                                    pInfo(i_pInfo)
                                                {}
    // INQUIRY
    Type_id             Type() const            { return nType; }
    doc::OldIdlDocu *   Info() const            { return pInfo; }

  private:
    // DATA
    Type_id             nType;
    doc::OldIdlDocu *   pInfo;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
