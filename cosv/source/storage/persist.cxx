/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: persist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:06:50 $
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
#include <cosv/persist.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/streamstr.hxx>
#include <cosv/ploc.hxx>


#ifdef WNT
#include <io.h>

namespace csv
{
namespace ploc
{

bool
Persistent::Exists() const
{
    return access( StrPath(), 00) == 0;
}

} // namespace ploc
} // namespace csv


#elif defined(UNX)
#include <unistd.h>

/*
#ifndef __SUNPRO_CC
#include <unistd.h>
#else
#define F_OK    0   // Test for existence of File
extern int access(const char *, int);
#endif
*/

namespace csv
{
namespace ploc
{

bool
Persistent::Exists() const
{
    return access( StrPath(), F_OK ) == 0;
}


} // namespace ploc
} // namespace csv

#else
#error  For using csv::ploc there has to be defined: WNT or UNX.
#endif

namespace csv
{
namespace ploc
{

const char *
Persistent::StrPath() const
{
    if (sPath.empty() )
    {
#ifndef CSV_NO_MUTABLE
        StreamStr & rsPath = sPath;
#else
        StreamStr & rsPath = const_cast< StreamStr& >(sPath);
#endif
        rsPath.seekp(0);
        rsPath << MyPath();
        if (MyPath().IsDirectory())
            rsPath.pop_back(1);    // Remove closing delimiter.
    }
    return sPath.c_str();
}

} // namespace ploc
} // namespace csv




