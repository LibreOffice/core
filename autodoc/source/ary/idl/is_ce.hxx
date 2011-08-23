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

#ifndef ARY_IDL_IS_CE_HXX
#define ARY_IDL_IS_CE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{


/** The data base for all ->ary::idl::CodeEntity objects.
*/
class Ce_Storage : public ::ary::stg::Storage< ::ary::idl::CodeEntity >
{
  public:
                        Ce_Storage();
    virtual             ~Ce_Storage();

    static Ce_Storage & Instance_()               { csv_assert(pInstance_ != 0);
                                                    return *pInstance_; }
  private:
    // DATA
    static Ce_Storage * pInstance_;
};




namespace predefined
{

enum E_CodeEntity
{
    ce_GlobalNamespace = 1,
    ce_MAX
};

}   // namespace predefined




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
