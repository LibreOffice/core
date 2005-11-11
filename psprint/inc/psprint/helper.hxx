/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 11:44:22 $
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

#ifndef _PSPRINT_HELPER_HXX_
#define _PSPRINT_HELPER_HXX_

#include <list>
#include <hash_map>
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


// forwards
namespace osl { class File; }

namespace psp {
typedef int fontID;

const rtl::OUString& getPrinterPath();
void getPrinterPathList( std::list< rtl::OUString >& rPathList, const char* pSubDir );
const rtl::OUString& getFontPath();

bool convertPfbToPfa( osl::File& rInFile, osl::File& rOutFile );

// normalized path (equivalent to realpath)
void normPath( rtl::OString& rPath );

// splits rOrgPath into dirname and basename
// rOrgPath will be subject to normPath
void splitPath( rtl::OString& rOrgPath, rtl::OString& rDir, rtl::OString& rBase );

enum whichOfficePath { NetPath, UserPath, ConfigPath };
const rtl::OUString& getOfficePath( enum whichOfficePath ePath );
} // namespace

#endif // _PSPRINT_HELPER_HXX_
