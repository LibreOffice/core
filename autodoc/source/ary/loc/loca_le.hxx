/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loca_le.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:17:32 $
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

#ifndef ARY_LOC_LOCA_LE_HXX
#define ARY_LOC_LOCA_LE_HXX

// BASE CLASSES
#include <ary/loc/locp_le.hxx>

namespace ary
{
namespace loc
{
    class Le_Storage;
}
}




namespace ary
{
namespace loc
{


/** Provides access to files and directories stored in the
    repository.
*/
class LocationAdmin : public LocationPilot
{
  public:
                        LocationAdmin();
    virtual              ~LocationAdmin();

    // INHERITED
        // Interface LocationPilot:
    virtual Root &      CheckIn_Root(
                            const csv::ploc::Path &
                                                i_rPath );
    virtual File &      CheckIn_File(
                            const String  &     i_name,
                            const csv::ploc::DirectoryChain &
                                                i_subPath,
                            Le_id               i_root );

    virtual Root &      Find_Root(
                            Le_id               i_id ) const;
    virtual Directory & Find_Directory(
                            Le_id               i_id ) const;
    virtual File &      Find_File(
                            Le_id               i_id ) const;
  private:
    // Locals
    Le_Storage &        Storage() const;
    Directory &         CheckIn_Directory(
                            Directory &         io_parent,
                            const String &      i_name );
    Directory &         CheckIn_Directories(
                            Directory &         io_root,
                            StringVector::const_iterator
                                                i_beginSubPath,
                            StringVector::const_iterator
                                                i_endSubPath );
    // DATA
    Dyn<Le_Storage>     pStorage;
};




}   // namespace loc
}   // namespace ary
#endif
