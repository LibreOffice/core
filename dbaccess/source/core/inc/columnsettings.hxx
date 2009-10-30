/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef COLUMNSETTINGS_HXX
#define COLUMNSETTINGS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

//........................................................................
namespace dbaccess
{
//........................................................................

    //************************************************************
    //  OColumnSettings
    //************************************************************
    class OColumnSettings
    {
    //  <properties>
        ::com::sun::star::uno::Any  m_aWidth;               // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aFormatKey;           // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aRelativePosition;    // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aAlignment;           // sal_Int32 (::com::sun::star::awt::TextAlign) or void
        ::com::sun::star::uno::Any  m_aHelpText;            // the description of the column which is visible in the helptext of the column
        ::com::sun::star::uno::Any  m_aControlDefault;      // the default value which should be displayed as by a control when moving to a new row
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xControlModel;

        sal_Bool                    m_bHidden;
    //  </properties>

    // Setting of values
    public:
        OColumnSettings();
        virtual ~OColumnSettings();

        sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);
        void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;


        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    public:

        /** check if the persistent settings have their default value
        */
        sal_Bool    isDefaulted() const;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // COLUMNSETTINGS_HXX
