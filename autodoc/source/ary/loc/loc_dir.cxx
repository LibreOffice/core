/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_dir.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:16:13 $
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


#include <precomp.h>
#include <ary/loc/loc_dir.hxx>


// NOT FULLY DEFINED SERVICES


namespace ary
{
namespace loc
{


void
Directory::Add_ChildDir( const udmstri &     i_sName,
                         Lid                 i_nId )
{
    aChildDirs[i_sName] = i_nId;
}

void
Directory::Add_File( const udmstri &     i_sName,
                     Lid                 i_nId )
{
    aFiles[i_sName] = i_nId;
}

Directory::Directory( Lid i_nId )
    :   // aChildDirs,
        // aFiles,
        nId( i_nId )
{
}

SubDirectory::SubDirectory( Lid                 i_nId,
                            const udmstri &     i_sName,
                            Lid                 i_nParentDirectory )
    :   Directory(i_nId),
        sName(i_sName),
        nParentDirectory(i_nParentDirectory)
{
}



} // namespace loc
} // namespace ary

