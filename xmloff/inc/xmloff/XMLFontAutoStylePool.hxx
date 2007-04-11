/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLFontAutoStylePool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:23:28 $
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

#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

class XMLFontAutoStylePool_Impl;
class XMLFontAutoStylePoolNames_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLFontAutoStylePool : public UniRefBase
{
    SvXMLExport& rExport;

    XMLFontAutoStylePool_Impl *pPool;
    XMLFontAutoStylePoolNames_Impl *pNames;
    sal_uInt32 nName;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLFontAutoStylePool( SvXMLExport& rExport );
    ~XMLFontAutoStylePool();

    ::rtl::OUString Add(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc );

    ::rtl::OUString Find(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc )const;
    ::rtl::OUString Find( const ::rtl::OUString& rInternalName ) const;

    void exportXML();
};

#endif  //  _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
