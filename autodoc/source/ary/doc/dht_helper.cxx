/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_helper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:39:32 $
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
