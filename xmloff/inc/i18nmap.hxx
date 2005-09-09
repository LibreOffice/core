/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18nmap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:05:44 $
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

#ifndef _XMLOFF_I18NMAP_HXX
#define _XMLOFF_I18NMAP_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SOLAR_H //autogen wg. USHORT
#include <tools/solar.h>
#endif


namespace rtl
{
    class OUString;
}

class SvI18NMap_Impl;
class SvI18NMapEntry_Impl;

class XMLOFF_DLLPUBLIC SvI18NMap
{
    SvI18NMap_Impl      *pImpl;

    SAL_DLLPRIVATE SvI18NMapEntry_Impl *_Find( USHORT nKind,
                                const ::rtl::OUString& rName ) const;

public:

    SvI18NMap();
    ~SvI18NMap();

    // Add a name mapping
    void Add( USHORT nKind, const ::rtl::OUString& rName,
              const ::rtl::OUString& rNewName );

    // Return a mapped name. If the name could not be found, return the
    // original name.
    const ::rtl::OUString& Get( USHORT nKind,
                                const ::rtl::OUString& rName ) const;
};


#endif  //  _XMLOFF_I18NMAP_HXX

