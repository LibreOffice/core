/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptInfoImpl.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SCRIPTING_STORAGE_SCRIPTINFOIMPL_HXX_
#define _SCRIPTING_STORAGE_SCRIPTINFOIMPL_HXX_

#include <vector>
#include <map>

#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>


typedef ::std::pair< ::rtl::OUString, ::rtl::OUString > str_pair;
typedef ::std::map< ::rtl::OUString, str_pair,
    ::std::equal_to< ::rtl::OUString > > strpair_map;
typedef ::std::vector< str_pair > props_vec;
typedef ::std::map< ::rtl::OUString, ::std::pair< props_vec, strpair_map >,
    ::std::equal_to< ::rtl::OUString > > filesets_map;

namespace scripting_impl
{

struct ScriptInfoImpl
{

    inline ScriptInfoImpl::ScriptInfoImpl() SAL_THROW( () )
       : parcelURI()
       , language()
       , locales()
       , functionname()
       , logicalname()
       , languagedepprops()
       , filesets()
       {
       }

   inline ScriptInfoImpl::ScriptInfoImpl( const ::rtl::OUString __parcelURI,
       const ::rtl::OUString& __language,
       const strpair_map& __locales,
       const ::rtl::OUString& __functionname,
       const ::rtl::OUString& __logicalname,
       const langdepprops_vec& __languagedepprops,
       const filesets_map& __filesets ) SAL_THROW( () )
       : parcelURI( __parcelURI )
       , language( __language )
       , locales( __locales )
       , functionname( __functionname )
       , logicalname( __logicalname )
       , languagedepprops( __languagedepprops )
       , filesets( __filesets )
   {
   }

   ::rtl::OUString parcelURI;
   ::rtl::OUString language;
   strpair_map locales;
   ::rtl::OUString functionname;
   ::rtl::OUString logicalname;
   props_vec languagedepprops;
   filesets_map filesets;

};


} // namespace scripting_impl

#endif // _SCRIPTING_STORAGE_SCRIPTINFOIMPL_HXX_

