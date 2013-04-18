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

#ifndef CSV_PLOCDIR_HXX
#define CSV_PLOCDIR_HXX


// USED SERVICES
    // BASE CLASSES
#include <cosv/persist.hxx>
    // COMPONENTS
#include <cosv/ploc.hxx>
    // PARAMETERS

namespace csv
{
namespace ploc
{

class DirectoryChain;

enum E_Recursivity
{
    flat,
    recursive
};

class Directory : public Persistent
{
  public:
    // LIFECYCLE
                        Directory();
                        Directory(
                            const Path &        i_rLocation );
                        Directory(
                            const char *        i_rLocation );
                        Directory(
                            const Directory &   i_rDir );
    virtual             ~Directory();

    // OPERATORS
    Directory &         operator+=(
                            const String &      i_sName );
    Directory &         operator+=(
                            const DirectoryChain &
                                                i_sDirChain );
    Directory &         operator-=(
                            uintt               i_nLevels );

    // OPERATIONS
    bool                PhysicalCreate(
                            bool                i_bCreateParentsIfNecessary = true ) const;

    // INQUIRY
    void                GetContainedDirectories(
                            StringVector &      o_rResult ) const;
    /** @param i_sFilter
        Currently only filters of the form "*.ending" or "*.*"
        (the default) are processed correctly under UNIX. Under WNT this
        restriction does not apply.
    */
    void                GetContainedFiles(
                            StringVector &      o_rResult,
                            const char *        i_sFilter = "*.*",
                            E_Recursivity       i_eRecursivity = flat ) const;
  private:
    // Interface Peristent:
    virtual const Path &
                        inq_MyPath() const;

    // Locals:
    /** @return
        true, if parent(!) directory exists or could be created.
        false, if this is a root directory.
    */
    bool                Check_Parent() const;
    bool                PhysicalCreate_Dir(
                            const char *        i_sStr ) const;
    // DATA
    Path                aPath;
};



}   // namespace ploc
}   // namespace csv



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
