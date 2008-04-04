/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppElementType.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:01:31 $
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
#ifndef DBAUI_APPELEMENTTYPE_HXX
#define DBAUI_APPELEMENTTYPE_HXX
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
//........................................................................
namespace dbaui
{
//........................................................................

    enum ElementType
    {
        E_TABLE     = ::com::sun::star::sdb::application::DatabaseObject::TABLE,
        E_QUERY     = ::com::sun::star::sdb::application::DatabaseObject::QUERY,
        E_FORM      = ::com::sun::star::sdb::application::DatabaseObject::FORM,
        E_REPORT    = ::com::sun::star::sdb::application::DatabaseObject::REPORT,

        E_NONE      = 4,
        E_ELEMENT_TYPE_COUNT = E_NONE
    };

    enum PreviewMode
    {
        E_PREVIEWNONE   = 0,
        E_DOCUMENT      = 1,
        E_DOCUMENTINFO  = 2
    };
//........................................................................
} // namespace dbaui
//........................................................................
#endif // DBAUI_APPELEMENTTYPE_HXX

