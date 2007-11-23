/***************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: altstrfunc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:51:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_lingucomponent.hxx"

#ifndef _ALT_STRFUNC_HXX_
#include "altstrfunc.hxx"
#endif

#include <sal/types.h>

std::string upperCase(const std::string &s) {
    std::string upper(s);
    for(size_t i = 0; i < s.length(); i++)
        upper[i] = sal::static_int_cast< char >( toupper(upper[i]) );
    return upper;
}


int start(const std::string &s1, const std::string &s2){
    size_t i;
    int ret = 0;

    size_t min = s1.length();
    if (min > s2.length())
        min = s2.length();

    for(i = 0; i < min && s2[i] && s1[i] && !ret; i++){
        ret = toupper(s1[i]) - toupper(s2[i]);
        if(s1[i] == '.' || s2[i] == '.'){ret = 0;}//. is a neutral character
    }
    return ret;
}

