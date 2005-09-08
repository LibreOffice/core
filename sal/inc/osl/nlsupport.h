/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nlsupport.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:29:40 $
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


#ifndef _OSL_NLSUPPORT_H_
#define _OSL_NLSUPPORT_H_

#ifndef _RTL_LOCALE_H_
#   include <rtl/locale.h>
#endif

#ifndef _RTL_TEXTENC_H
#   include <rtl/textenc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
    Determines the text encoding used by the underlying platform for the
    specified locale.

    @param pLocale
    the locale to return the text encoding for. If this parameter is NULL,
    the default locale of the current process is used.

    @returns the rtl_TextEncoding that matches the platform specific encoding
    description or RTL_TEXTENCODING_DONTKNOW if no mapping is available.
*/

rtl_TextEncoding SAL_CALL osl_getTextEncodingFromLocale( rtl_Locale * pLocale );


#ifdef __cplusplus
}
#endif

#endif  /* _OSL_NLSUPPORT_H_ */


