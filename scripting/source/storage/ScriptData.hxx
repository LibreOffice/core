/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SCRIPTING_STORAGE_SCRIPTDATA_HXX_
#define _SCRIPTING_STORAGE_SCRIPTDATA_HXX_

#include <vector>
#include <boost/unordered_map.hpp>

#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>



namespace scripting_impl
{

typedef ::std::pair< ::rtl::OUString, ::rtl::OUString > str_pair;
typedef ::std::vector< str_pair > props_vec;
typedef ::boost::unordered_map< ::rtl::OUString, props_vec, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > strpairvec_map;
typedef ::boost::unordered_map< ::rtl::OUString, ::std::pair< ::rtl::OUString,
    ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > strpair_map;
typedef ::boost::unordered_map< ::rtl::OUString, ::std::pair< props_vec, strpairvec_map >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > filesets_map;

struct ScriptData
{

    inline ScriptData::ScriptData() SAL_THROW( () )
       : parcelURI()
       , language()
       , locales()
       , functionname()
       , logicalname()
       , languagedepprops()
       , filesets()
   {
   }

   inline ScriptData::ScriptData( const ::rtl::OUString __parcelURI,
       const ::rtl::OUString& __language,
       const strpair_map& __locales,
       const ::rtl::OUString& __functionname,
       const ::rtl::OUString& __logicalname,
       const props_vec& __languagedepprops,
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
#endif // _SCRIPTING_STORAGE_ScriptData_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
