/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_gdiplususer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:55:23 $
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

#ifndef _DXCANVAS_GDIPLUSUSER_HXX
#define _DXCANVAS_GDIPLUSUSER_HXX

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

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
