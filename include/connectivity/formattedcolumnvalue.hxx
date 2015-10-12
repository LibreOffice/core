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

#ifndef INCLUDED_CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX
#define INCLUDED_CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <connectivity/dbtoolsdllapi.hxx>
#include <memory>

namespace dbtools
{
    struct FormattedColumnValue_Data;

    //= FormattedColumnValue

    /** a class which helps retrieving and setting the value of a database column
        as formatted string.
    */
    class OOO_DLLPUBLIC_DBTOOLS FormattedColumnValue
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
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

        void        clear();

        // access to the details of the formatting we determined
        sal_Int16   getKeyType() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >&
                    getColumn() const;

        bool        setFormattedValue( const OUString& _rFormattedStringValue ) const;
        OUString    getFormattedValue() const;

    private:
        FormattedColumnValue(const FormattedColumnValue&) = delete;
        FormattedColumnValue& operator=(const FormattedColumnValue&) = delete;
        std::unique_ptr< FormattedColumnValue_Data >    m_pData;
    };


} // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_FORMATTEDCOLUMNVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
