/*************************************************************************
 *
 *  $RCSfile: ScriptData.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dfoster $ $Date: 2002-10-30 16:12:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SCRIPTING_STORAGE_SCRIPTDATA_HXX_
#define _SCRIPTING_STORAGE_SCRIPTDATA_HXX_

#include <vector>
#include <hash_map>

#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>



namespace scripting_impl
{

typedef ::std::pair< ::rtl::OUString, ::rtl::OUString > str_pair;
typedef ::std::vector< str_pair > props_vec;
typedef ::std::hash_map< ::rtl::OUString, props_vec, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > strpairvec_map;
typedef ::std::hash_map< ::rtl::OUString, ::std::pair< ::rtl::OUString,
    ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > strpair_map;
typedef ::std::hash_map< ::rtl::OUString, ::std::pair< props_vec, strpairvec_map >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > filesets_map;

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

