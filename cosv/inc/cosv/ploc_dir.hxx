/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ploc_dir.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:56:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


