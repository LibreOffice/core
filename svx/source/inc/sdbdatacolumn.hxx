/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SOURCE_INC_SDBDATACOLUMN_HXX
#define INCLUDED_SVX_SOURCE_INC_SDBDATACOLUMN_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <osl/diagnose.h>


namespace svxform
{



    //= DataColumn - a class wrapping an object implementing a sdb::DataColumn service

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
        operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> () const{ return m_xColumn.get(); }

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
        ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
        { return m_xColumn->queryInterface(type); }

        // ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> getPropertySetInfo() const throw( ::com::sun::star::uno::RuntimeException );
        inline void setPropertyValue(const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline ::com::sun::star::uno::Any getPropertyValue(const OUString& PropertyName) const throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void addPropertyChangeListener(const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& xListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void removePropertyChangeListener(const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void addVetoableChangeListener(const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
        inline void removeVetoableChangeListener(const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& aListener) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::sdb::XColumn
        inline sal_Bool wasNull() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline OUString getString() throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
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
        inline void updateString(const OUString& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateBytes(const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateDate(const com::sun::star::util::Date& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateTime(const ::com::sun::star::util::Time& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateTimestamp(const ::com::sun::star::util::DateTime& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateBinaryStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateCharacterStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateObject(const ::com::sun::star::uno::Any& x) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        inline void updateNumericObject(const ::com::sun::star::uno::Any& x, sal_Int32 scale) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
    };

#endif // INCLUDED_SVX_SOURCE_INC_SDBDATACOLUMN_HXX


}   // namespace svxform


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
