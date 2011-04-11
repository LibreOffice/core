/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef _CONNECTIVITY_SQLSTATEMENTHELPER_HXX_
#define _CONNECTIVITY_SQLSTATEMENTHELPER_HXX_

#include "connectivity/dbtoolsdllapi.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
namespace rtl
{
    class OUStringBuffer;
}
//.........................................................................
namespace dbtools
{
    class OOO_DLLPUBLIC_DBTOOLS ISQLStatementHelper
    {
    public:
        virtual void addComment(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor,::rtl::OUStringBuffer& _rOut) = 0;
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_SQLSTATEMENTHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
