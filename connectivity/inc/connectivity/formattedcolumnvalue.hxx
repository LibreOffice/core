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

#ifndef CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX
#define CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <memory>
#include "connectivity/dbtoolsdllapi.hxx"

namespace comphelper { class ComponentContext; }

//........................................................................
namespace dbtools
{
//........................................................................

    struct FormattedColumnValue_Data;
    //====================================================================
    //= FormattedColumnValue
    //====================================================================
    /** a class which helps retrieving and setting the value of a database column
        as formatted string.
    */
    class OOO_DLLPUBLIC_DBTOOLS FormattedColumnValue : public ::boost::noncopyable
    {
    public:
        /** constructs an instance

            The format key for the string value exchange is taken from the given column object.
            If it has a non-<NULL/> property value <code>FormatKey</code>, this key is taken.
            Otherwise, a default format matching the column type is determined.

            The locale of this fallback format is the current system locale.

            The number formats supplier is determined from the given <code>RowSet</code>, by
            examining its <code>ActiveConnection</code>.
        */
        FormattedColumnValue(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );

        /** constructs an instance

            The format key for the string value exchange is taken from the given column object.
            If it has a non-<NULL/> property value <code>FormatKey</code>, this key is taken.
            Otherwise, a default format matching the column type is determined.

            The locale of this fallback format is the current system locale.
        */
        FormattedColumnValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& i_rNumberFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_rColumn
        );

        // note that all methods of this class need to be virtual, since it's
        // used in a load-on-demand context in module SVX

        virtual ~FormattedColumnValue();

        virtual void clear();

        // access to the details of the formatting we determined
        virtual sal_Int32   getFormatKey() const;
        virtual sal_Int32   getFieldType() const;
        virtual sal_Int16   getKeyType() const;
        virtual bool        isNumericField() const;
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >&
                            getColumn() const;
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate >&
                            getColumnUpdate() const;

        virtual bool            setFormattedValue( const ::rtl::OUString& _rFormattedStringValue ) const;
        virtual ::rtl::OUString getFormattedValue() const;

    private:
        ::std::auto_ptr< FormattedColumnValue_Data >    m_pData;
    };

//........................................................................
} // namespace dbtools
//........................................................................

#endif // CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
