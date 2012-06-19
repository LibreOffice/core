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
