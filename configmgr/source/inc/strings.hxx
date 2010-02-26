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

#ifndef _CONFIGMGR_STRINGS_HXX_
#define _CONFIGMGR_STRINGS_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>


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

