/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

#ifndef _FORMS_LIMITED_FORMATS_HXX_
#define _FORMS_LIMITED_FORMATS_HXX_

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //= OLimitedFormats
    //=====================================================================
    /** maintains translation tables format key <-> enum value
        <p>Used for controls which provide a limited number for (standard) formats, which
        should be available as format keys.</p>
    */
    class OLimitedFormats
    {
    private:
        static sal_Int32	s_nInstanceCount;
        static ::osl::Mutex	s_aMutex;
        static ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                            s_xStandardFormats;

    protected:
        sal_Int32			m_nFormatEnumPropertyHandle;
        const sal_Int16		m_nTableId;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >
                            m_xAggregate;

    protected:
        /** ctor
            <p>The class id is used to determine the translation table to use. All instances which
            pass the same value here share one table.</p>
        */
        OLimitedFormats(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const sal_Int16 _nClassId
            );
        ~OLimitedFormats();

    protected:
        void setAggregateSet(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >& _rxAggregate,
            sal_Int32 _nOriginalPropertyHandle
        );

    protected:
        void		getFormatKeyPropertyValue( ::com::sun::star::uno::Any& _rValue ) const;
        sal_Bool	convertFormatKeyPropertyValue(
                        ::com::sun::star::uno::Any& _rConvertedValue,
                        ::com::sun::star::uno::Any& _rOldValue,
                const	::com::sun::star::uno::Any& _rNewValue
            );
        void		setFormatKeyPropertyValue( const ::com::sun::star::uno::Any& _rNewValue );
        // setFormatKeyPropertyValue should only be called with a value got from convertFormatKeyPropertyValue!

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                    getFormatsSupplier() const { return s_xStandardFormats; }

    private:	
        void acquireSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        void releaseSupplier();

        static void ensureTableInitialized(const sal_Int16 _nTableId);
        static void clearTable(const sal_Int16 _nTableId);
    };

//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_LIMITED_FORMATS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
