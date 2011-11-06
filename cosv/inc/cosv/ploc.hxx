/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
