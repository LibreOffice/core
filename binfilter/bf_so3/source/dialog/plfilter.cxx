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

#include <set>
#include <map>
#include <unotools/processfactory.hxx>

#include <tools/debug.hxx>
#include <vcl/stdtext.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/plugin/PluginDescription.hpp>
#include <com/sun/star/plugin/XPluginManager.hpp>

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::plugin;

namespace binfilter {

struct ltstr
{
    bool operator()( const String& s1, const String& s2 ) const
    {
        return ( s1.CompareTo( s2 ) == COMPARE_LESS );
    }
};

typedef set< String, ltstr > StrSet;
typedef map< String, StrSet, ltstr > FilterMap;


}

