/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IUpdateHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-04-19 13:20:44 $
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
#ifndef DBAUI_IUPDATEHELPER_HXX
#define DBAUI_IUPDATEHELPER_HXX

namespace dbaui
{
    class SAL_NO_VTABLE IUpdateHelper
    {
    public:
        virtual void updateString(sal_Int32 _nPos, const ::rtl::OUString& _sValue) = 0;
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue) = 0;
        virtual void updateInt(sal_Int32 _nPos,const sal_Int32& _nValue) = 0;
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32 sqlType) = 0;
        virtual void moveToInsertRow() = 0;
        virtual void insertRow() = 0;
    };
}

#endif // DBAUI_IUPDATEHELPER_HXX

