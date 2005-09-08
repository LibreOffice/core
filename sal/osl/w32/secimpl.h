/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: secimpl.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:10:52 $
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

#ifndef _OSL_SECURITYIMPL_H_
#define _OSL_SECURITYIMPL_H_

#include <winnetwk.h>

#include <osl/security.h>

#define USER_BUFFER_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _oslSecurityImpl {
    HANDLE m_hProfile;
    HANDLE m_hToken;
    sal_Unicode   m_User[USER_BUFFER_SIZE];
/* ts: Erweiterung um Fileserver-login */
    NETRESOURCEW *m_pNetResource;
} oslSecurityImpl;

#ifdef __cplusplus
}
#endif

#endif

