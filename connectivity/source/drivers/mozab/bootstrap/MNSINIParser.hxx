/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSINIParser.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:47:00 $
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
#ifndef __MNSINIPARSER_HXX__
#define __MNSINIPARSER_HXX__

#include <rtl/ustring.hxx>
#include <com/sun/star/io/IOException.hpp>
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
using namespace rtl;

#include <map>
#include <list>

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

struct ini_NameValue
{
    rtl::OUString sName;
    rtl::OUString sValue;

    inline ini_NameValue() SAL_THROW( () )
        {}
    inline ini_NameValue(
        OUString const & name, OUString const & value ) SAL_THROW( () )
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<
    ini_NameValue
> NameValueList;

struct ini_Section
{
    rtl::OUString sName;
    NameValueList lList;
};
typedef std::map<rtl::OUString,
                ini_Section
                >IniSectionMap;


class IniParser
{
    IniSectionMap mAllSection;
public:
    IniSectionMap * getAllSection(){return &mAllSection;};
    ini_Section *  getSection(OUString const & secName);
    IniParser(OUString const & rIniName) throw(com::sun::star::io::IOException );
#if OSL_DEBUG_LEVEL > 0
    void Dump();
#endif

};

#endif

