/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableFieldInfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:29:56 $
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
#ifndef DBAUI_TABLEFIELDINFO_HXX
#define DBAUI_TABLEFIELDINFO_HXX

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace dbaui
{
    class OTableFieldInfo
    {
    private:
        ETableFieldType m_eFieldType;
        sal_Int32       m_eDataType;

    public:
        OTableFieldInfo();
        ~OTableFieldInfo();

        inline ETableFieldType  GetKeyType() const { return m_eFieldType; }
        inline void             SetKey(ETableFieldType bKey=TAB_NORMAL_FIELD) { m_eFieldType = bKey; }
        inline sal_Int32        GetDataType() const { return m_eDataType; }
        inline void             SetDataType(sal_Int32 eTyp) { m_eDataType = eTyp; }
    };
}
#endif // DBAUI_TABLEFIELDINFO_HXX


