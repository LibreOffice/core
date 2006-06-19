/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StdTypeDefs.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:59:28 $
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
#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#define CONNECTIVITY_STDTYPEDEFS_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <map>
#include <vector>

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

namespace rtl { class OUString; }

namespace connectivity
{
    typedef ::std::vector< ::rtl::OUString>         TStringVector;
    typedef ::std::vector< sal_Int32>               TIntVector;
    typedef ::std::map<sal_Int32,sal_Int32>         TInt2IntMap;
    typedef ::std::map< ::rtl::OUString,sal_Int32>  TString2IntMap;
    typedef ::std::map< sal_Int32,::rtl::OUString>  TInt2StringMap;
}

#endif // CONNECTIVITY_STDTYPEDEFS_HXX

