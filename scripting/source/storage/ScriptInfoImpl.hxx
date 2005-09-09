/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptInfoImpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:36:39 $
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

