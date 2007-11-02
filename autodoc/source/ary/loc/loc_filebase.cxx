/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_filebase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:16:34 $
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
#include <ary/loc/loc_filebase.hxx>


// NOT FULLY DEFINED SERVICES



namespace ary
{
namespace loc
{


String
FileBase::Extension() const
{
    const char *
        extension = strrchr(sLocalName.c_str(), '.');
    if (extension != 0)
        return String(extension + 1);
    return String::Null_();
}


FileBase::FileBase( const String  &     i_localName,
                    Le_id               i_parentDirectory )
    :   sLocalName(i_localName),
        nParentDirectory(i_parentDirectory)
{
}

const String &
FileBase::inq_LocalName() const
{
    return sLocalName;
}

Le_id
FileBase::inq_ParentDirectory() const
{
    return nParentDirectory;
}



} // namespace loc
} // namespace ary
