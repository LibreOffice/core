/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_helper.cxx,v $
 * $Revision: 1.3 $
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

#include <precomp.h>
#include "dht_helper.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_component.hxx>



namespace ary
{
namespace doc
{
namespace ht
{

Component
Create_Component( const Interpreter &  i_intepreter,
                  const String &       i_str1,
                  char                 i_separator,
                  const String &       i_str2 )
{
    if (i_str2.empty())
        return Component(i_intepreter, i_str1);

    StreamLock
        sl(700);
    sl() << i_str1 << i_separator << i_str2;
    return Component( i_intepreter, String(sl().c_str()) );
}

void
Resolve_ComponentData(  String &            o_1,
                        String &            o_2,
                        char                i_separator,
                        const String &      i_data )
{
    const char *
        p_begin = i_data.begin();
    const char *
        p_separator = strchr(p_begin, i_separator);
    if (p_separator != 0)
    {
        o_1 = String(p_begin, p_separator);
        o_2 = String(p_separator+1, i_data.end());
    }
    else
    {
        o_1 = i_data;
        o_2 = i_data;
    }
}



}   // namespace ht
}   // namespace doc
}   // namespace ary
