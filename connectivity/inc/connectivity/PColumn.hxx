/*************************************************************************
 *
 *  $RCSfile: PColumn.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 13:17:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif

namespace connectivity
{
    namespace parse
    {
        class OParseColumn;

        typedef sdbcx::OColumn OParseColumn_BASE;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OParseColumn> OParseColumn_PROP;

        class OParseColumn :    public OParseColumn_BASE,
                                public OParseColumn_PROP
        {
            ::rtl::OUString m_aRealName;
            ::rtl::OUString m_aTableName;
            sal_Bool        m_bFunction;
            sal_Bool        m_bDbasePrecisionChanged;
        protected:
            DECLARE_CTY_PROPERTY(OParseColumn_PROP,OParseColumn)
        public:
            OParseColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,sal_Bool _bCase);
            OParseColumn(const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase);

            ~OParseColumn();

            virtual void construct();

            void setRealName(const ::rtl::OUString& _rName)  { m_aRealName  = _rName; }
            void setTableName(const ::rtl::OUString& _rName) { m_aTableName = _rName; }
            void setFunction(sal_Bool _bFunction)            { m_bFunction = _bFunction; }
            void setDbasePrecisionChanged(sal_Bool _bDbasePrecisionChanged) { m_bDbasePrecisionChanged = _bDbasePrecisionChanged; }

            ::rtl::OUString getRealName() const { return  m_aRealName; }
            ::rtl::OUString getTableName() const { return  m_aTableName; }
            sal_Bool        getFunction() const { return  m_bFunction; }
            sal_Bool        getDbasePrecisionChanged()  const { return  m_bDbasePrecisionChanged; }
        };
    }
}

