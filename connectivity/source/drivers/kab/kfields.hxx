/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kfields.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 10:25:04 $
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

#ifndef _CONNECTIVITY_KAB_FIELDS_HXX_
#define _CONNECTIVITY_KAB_FIELDS_HXX_

#ifndef INCLUDED_VCL_KDE_HEADERS_H
#include <vcl/kde_headers.h>
#endif

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include "ustring.hxx"
#endif

#define KAB_FIELD_REVISION  0
#define KAB_DATA_FIELDS     1

namespace connectivity
{
    namespace kab
    {
        QString valueOfKabField(const ::KABC::Addressee &aAddressee, sal_Int32 nFieldNumber);
        sal_uInt32 findKabField(const ::rtl::OUString& columnName) throw(::com::sun::star::sdbc::SQLException);
    }
}

#endif
