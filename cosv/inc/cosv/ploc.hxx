/*************************************************************************
 *
 *  $RCSfile: ploc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 12:18:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CSV_PLOC_HXX
#define CSV_PLOC_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
#include <cosv/plocroot.hxx>
#include <cosv/dirchain.hxx>
#include <cosv/template/dyn.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>


namespace csv
{
    class bostream;


namespace ploc
{
    class Root;

class Path
{
  public:

    // LIFECYCLE
                        Path(
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
    void                SetFile(               // If there is already a file, that is exchanged.
                            const String &      i_sName );
    // INQUIRY
    const Root &        RootDir() const         { return *pRoot; }
    const DirectoryChain &
                        DirChain() const        { return aPath; }
    const String  &     File() const            { return sFile; }
    const char *        FileEnding() const;
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



