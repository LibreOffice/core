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

#ifndef CSV_PLOC_HXX
#define CSV_PLOC_HXX

// USED SERVICES
#include <cosv/string.hxx>
#include <cosv/plocroot.hxx>
#include <cosv/dirchain.hxx>
#include <cosv/tpl/dyn.hxx>
#include <cosv/csv_ostream.hxx>




namespace csv
{
    class bostream;

namespace ploc
{
    class Root;


/** Represents a path in the file system.

    The path can be relative or absolute and in Unix- or Windows-syntax.
*/
class Path
{
  public:

    // LIFECYCLE
    explicit            Path(
                            const char *        i_sPath = ".",                  /// Dirs have to be ended with a '\\ or '/'.
                            bool                i_bPathIsAlwaysDir = false,     /// This overrides a missing Delimiter at the end of the i_sPath, if true.
                            const char *        i_sDelimiter = Delimiter() );
                        Path(
                            const Path &        i_rPath );
                        ~Path();
    // OPERATORS
    Path &              operator=(
                            const Path &        i_rPath );
    // OPERATIONS
    void                Set(
                            const char *        i_sPath,
                            bool                i_bPathIsAlwaysDir = false,
                            const char *        i_sDelimiter = Delimiter() );
    void                SetFile(                // If there is already a file, that is exchanged.
                            const String &      i_sName );
    // INQUIRY
    const Root &        RootDir() const         { return *pRoot; }
    const DirectoryChain &
                        DirChain() const        { return aPath; }
    const String  &     File() const            { return sFile; }
    const char *        FileExtension() const;
    bool                IsValid() const;
    bool                IsDirectory() const     { return sFile.length() == 0; }
    bool                IsFile() const          { return sFile.length() > 0; }

    /// Directories have a delimiter at the end, files not.
    void                Get(
                            ostream &           o_rPath ) const;
    /// Directories have a delimiter at the end, files not.
    void                Get(
                            bostream &          o_rPath ) const;
    // ACCESS
    DirectoryChain &    DirChain()              { return aPath; }

  private:
    Dyn<Root>           pRoot;
    DirectoryChain      aPath;
    String              sFile;
};




}   // namespace ploc
}   // namespace csv



/// Directories produce a delimiter at the end, files not.
inline csv::ostream &
operator<<( csv::ostream &           o_rOut,
            const csv::ploc::Path &  i_rPath )
{
     i_rPath.Get(o_rOut);
    return o_rOut;
}

/// Directories produce a delimiter at the end, files not.
inline csv::bostream &
operator<<( csv::bostream &         o_rOut,
            const csv::ploc::Path & i_rPath )
{
     i_rPath.Get(o_rOut);
    return o_rOut;
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
