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

#ifndef ARY_REPOSY_HXX
#define ARY_REPOSY_HXX

// BASE CLASSES
#include <ary/ary.hxx>
// USED SERVICES
#include <cosv/ploc_dir.hxx>

namespace ary
{
namespace idl
{
    class InternalGate;
}
}   // namespace ary




namespace ary
{


/** Implements ::ary::Repository.

    @see Repository
*/

class RepositoryCenter : public ::ary::Repository
{
  public:
    //  LIFECYCLE
                        RepositoryCenter();
    virtual             ~RepositoryCenter();

    // INHERITED
        // Interface Repository:
    virtual const idl::Gate &   Gate_Idl() const;
    virtual const String &      Title() const;
    virtual idl::Gate &         Gate_Idl();
    virtual void                Set_Title(const String & i_sName );

  private:
    // DATA
    String              sDisplayedName;     /// Name to be displayed for human users.
    csv::ploc::Directory
                        aLocation;
    Dyn<idl::InternalGate>
                        pIdlPartition;
};




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
