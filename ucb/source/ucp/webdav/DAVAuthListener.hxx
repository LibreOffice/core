/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DAVAuthListener.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _DAVAUTHLISTENER_HXX_
#define _DAVAUTHLISTENER_HXX_

#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_UCB_XREFERENCE_HPP_
#include <com/sun/star/uno/XReference.hpp>
#endif
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace webdav_ucp
{

class DAVAuthListener : public salhelper::SimpleReferenceObject
{
    public:
        virtual int authenticate(
            const ::rtl::OUString & inRealm,
            const ::rtl::OUString & inHostName,
            ::rtl::OUString & inoutUserName,
            ::rtl::OUString & outPassWord,
            sal_Bool bAllowPersistentStoring,
            sal_Bool bCanUseSystemCredentials ) = 0;
};

} // namespace webdav_ucp

#endif // _DAVAUTHLISTENER_HXX_
