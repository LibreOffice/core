/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strings.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:57:34 $
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

#ifndef _CONFIGMGR_STRINGS_HXX_
#define _CONFIGMGR_STRINGS_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


//.........................................................................
namespace configmgr
{
//.........................................................................

struct UStringDescription
{
    const sal_Char*     m_pZeroTerminatedName;
    sal_Int32           m_nLen;
    rtl_TextEncoding    m_encoding;

    UStringDescription(const sal_Char* _pName, sal_Int32 _nLen, rtl_TextEncoding _encoding)
        : m_pZeroTerminatedName( _pName )
        , m_nLen( _nLen )
        , m_encoding(_encoding )
        , m_aString (_pName, _nLen, _encoding)
        {
        }

    sal_Int32 getLength() const { return m_nLen; }
    operator ::rtl::OUString const&() const { return m_aString; }
    operator const sal_Char*() const { return m_pZeroTerminatedName; }

private:
    rtl::OUString m_aString;
    UStringDescription();
};

#define DECLARE_CONSTASCII_USTRING(name)    \
    extern ::configmgr::UStringDescription name

#define IMPLEMENT_CONSTASCII_USTRING(name, asciivalue)  \
    ::configmgr::UStringDescription name(RTL_CONSTASCII_USTRINGPARAM(asciivalue))

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _CONFIGMGR_STRINGS_HXX_

