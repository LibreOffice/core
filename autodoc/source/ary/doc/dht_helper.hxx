/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_helper.hxx,v $
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

#ifndef ARY_DOC_DHT_HELPER_HXX
#define ARY_DOC_DHT_HELPER_HXX



namespace ary
{
namespace doc
{
namespace ht
{

class Component;
class Interpreter;


/** Joins two strings with a separator and creates a component
    from the result.

    If ->i_str2 is empty, only i_str1 is used without the separator.
*/
Component           Create_Component(
                        const Interpreter & i_intepreter,
                        const String &      i_str1,
                        char                i_separator,
                        const String &      i_str2 );

/** Joins two strings with a separator and creates a component
    from the result.

    If ->i_str2 is empty, only i_str1 is used without the separator.
*/
void                Resolve_ComponentData(
                        String &            o_1,
                        String &            o_2,
                        char                i_separator,
                        const String &      i_data );



}   // namespace ht
}   // namespace doc
}   // namespace ary

#endif
