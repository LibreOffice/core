/*************************************************************************
 *
 *  $RCSfile: sdbdatacolumn.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:03:20 $
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

#ifndef SVX_FORM_SDBDATACOLUMN_HXX
#define SVX_FORM_SDBDATACOLUMN_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//..............................................................................
namespace svxform
{
//..............................................................................

    //==========================================================================
    //= DataColumn - a class wrapping an object implementing a sdb::DataColumn service
    //==========================================================================
    class DataColumn
    {
        // interfaces needed for sddb::Column
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xPropertySet;
        // interfaces needed for sdb::DataColumn
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>           m_xColumn;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>     m_xColumnUpdate;

    public:
        DataColumn() { };
        DataColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxIFace);
        // if the object behind _rxIFace doesn't fully support the DataColumn service,
        // (which is checked via the supported interfaces) _all_ members will be set to
        // void !, even if the object has some of the needed interfaces.

        sal_Bool is() const { return m_xColumn.is(); }
        sal_Bool Is() const { return m_xColumn.is(); }
        sal_Bool supportsUpdate() const { return m_xColumnUpdate.is(); }

        DataColumn* operator ->() { return this; }
        //  operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> () const { return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> m_xColumn; }
        operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> () const{ return m_xColumn;; }

        // 'conversions'
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& getPropertySet() const
        {
            return m_xPropertySet;
        }
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& getColumn() const
        {
            return m_xColumn;
        }
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>& getColumnUpdate() const
        {
            OSL_ENSURE(m_xColumnUpdate.is() , "DataColumn::getColumnUpdate: NULL!");
            return m_xColumnUpdate;
        }

        // das normale queryInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
        { return m_xColumn->queryInterface(type); }

        // ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> getPropertySetInfo() const throw( ::com::sun::star::uno::RuntimeException );
        inline void setPropertyValue(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Any getPropertyValue(const ::rtl::OUString& PropertyName) const throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& xListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void addVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void removeVetoableChangeListener(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::sdb::XColumn
        inline sal_Bool wasNull() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::rtl::OUString getString() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline sal_Bool getBoolean() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline sal_Int8 getByte() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline sal_Int16 getShort() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline sal_Int32 getInt() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline sal_Int64 getLong() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline float getFloat() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline double getDouble() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Sequence< sal_Int8 > getBytes() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::util::Date getDate() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::util::Time getTime() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::util::DateTime  getTimestamp() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> getBinaryStream() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> getCharacterStream() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Any getObject(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& typeMap) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef> getRef() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob> getBlob() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob> getClob() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray> getArray() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );

        // XColumnUpdate
        inline void updateNull(void) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateBoolean(sal_Bool x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateByte(sal_Int8 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateShort(sal_Int16 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateInt(sal_Int32 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateLong(sal_Int64 x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateFloat(float x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateDouble(double x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateString(const ::rtl::OUString& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateBytes(const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateDate(const com::sun::star::util::Date& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateTime(const ::com::sun::star::util::Time& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateTimestamp(const ::com::sun::star::util::DateTime& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateBinaryStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateCharacterStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateObject(const ::com::sun::star::uno::Any& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateNumericObject(const ::com::sun::star::uno::Any& x, sal_Int32 scale) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    };

#endif // SVX_FORM_SDBDATACOLUMN_HXX

//..............................................................................
}   // namespace svxform
//..............................................................................
