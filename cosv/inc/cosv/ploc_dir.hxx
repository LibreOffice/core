/*************************************************************************
 *
 *  $RCSfile: ploc_dir.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:25:39 $
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
                            bool                i_bCreateParentsIfNecessary = true );

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
    bool                Check_Parent();
    bool                PhysicalCreate_Dir(
                            const char *        i_sStr ) const;
    // DATA
    Path                aPath;
};



}   // namespace ploc
}   // namespace csv



#endif


