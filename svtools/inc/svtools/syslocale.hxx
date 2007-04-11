/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: syslocale.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:39:07 $
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

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#define INCLUDED_SVTOOLS_SYSLOCALE_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


class SvtSysLocale_Impl;
namespace osl { class Mutex; }
class LocaleDataWrapper;

/**
    SvtSysLocale provides a refcounted single instance of an application wide
    <type>LocaleDataWrapper</type> and <type>CharClass</type> which always
    follow the locale as it is currently configured by the user. You may use
    it anywhere to access the locale data elements like decimal separator and
    simple date formatting and so on. Contructing and destructing a
    SvtSysLocale is not expensive as long as there is at least one instance
    left.
 */
class SVT_DLLPUBLIC SvtSysLocale
{
    friend class SvtSysLocale_Impl;     // access to mutex

    static  SvtSysLocale_Impl*  pImpl;
    static  sal_Int32           nRefCount;

    SVT_DLLPRIVATE static  ::osl::Mutex&               GetMutex();

public:
                                        SvtSysLocale();
                                        ~SvtSysLocale();

            const LocaleDataWrapper&    GetLocaleData() const;
            const CharClass&            GetCharClass() const;

    /** It is safe to store the pointers locally and use them AS LONG AS THE
        INSTANCE OF SvtSysLocale LIVES!
        It is a faster access but be sure what you do!
     */
            const LocaleDataWrapper*    GetLocaleDataPtr() const;
            const CharClass*            GetCharClassPtr() const;

};

#endif  // INCLUDED_SVTOOLS_SYSLOCALE_HXX
