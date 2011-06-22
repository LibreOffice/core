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
                            const String &      i_rLocation );
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
