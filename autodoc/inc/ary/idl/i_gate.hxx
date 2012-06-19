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

#ifndef ARY_IDL_I_GATE_HXX
#define ARY_IDL_I_GATE_HXX

// USED SERVICES

namespace autodoc
{
    class Options;
}
namespace ary
{
namespace idl
{
    class CePilot;
    class TypePilot;
}
}




namespace ary
{
namespace idl
{


/** Main entry to access the IDL parts of the repository.
*/
class Gate
{
  public:
    // LIFECYCLE
    virtual             ~Gate() {}

    // OPERATIONS
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath ) = 0;
//                            const ::autodoc::Options &
//                                                i_options ) = 0;
    // INQUIRY
    virtual const CePilot &
                        Ces() const = 0;
    virtual const TypePilot &
                        Types() const = 0;
    // ACCESS
    virtual CePilot &   Ces() = 0;
    virtual TypePilot & Types() = 0;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
