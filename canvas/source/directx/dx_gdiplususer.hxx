/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_gdiplususer.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _DXCANVAS_GDIPLUSUSER_HXX
#define _DXCANVAS_GDIPLUSUSER_HXX

#include <sal/config.h>

#include <boost/shared_ptr.hpp>

/* Definition of GDIPlusUser class */

namespace dxcanvas
{
    class GDIPlusUser
    {
    public:
        typedef ::boost::shared_ptr< GDIPlusUser > GDIPlusUserSharedPtr;

        static GDIPlusUserSharedPtr createInstance();
        ~GDIPlusUser();

    private:
        GDIPlusUser(); // create us via factory method
    };

    typedef GDIPlusUser::GDIPlusUserSharedPtr GDIPlusUserSharedPtr;

}

#endif /* _DXCANVAS_GDIPLUSUSER_HXX */
