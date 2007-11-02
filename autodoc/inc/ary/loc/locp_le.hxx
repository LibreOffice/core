/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: locp_le.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:19:41 $
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

#ifndef ARY_LOCP_LE_HXX
#define ARY_LOCP_LE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/ploc.hxx>
#include <ary/loc/loc_types4loc.hxx>


namespace ary
{
namespace loc
{
    class Root;
    class Directory;
    class File;
}
}


namespace ary
{
namespace loc
{



/** Provides access to files and directories stored in the
    repository.
*/
class LocationPilot
{
  public:
    virtual             ~LocationPilot() {}

    virtual Root &      CheckIn_Root(
                            const csv::ploc::Path &
                                                i_rPath ) = 0;
    virtual File &      CheckIn_File(
                            const String  &     i_name,
                            const csv::ploc::DirectoryChain &
                                                i_subPath,
                            Le_id               i_root ) = 0;

    virtual Root &      Find_Root(
                            Le_id               i_id ) const = 0;
    virtual Directory & Find_Directory(
                            Le_id               i_id ) const = 0;
    virtual File &      Find_File(
                            Le_id               i_id ) const = 0;
};




}   // namespace loc
}   // namespace ary
#endif
