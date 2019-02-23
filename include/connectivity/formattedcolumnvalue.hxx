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

#include <connectivity/dbtoolsdllapi.hxx>
#include <rtl/ustring.hxx>
#include <memory>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::sdbc { class XRowSet; }
namespace com::sun::star::sdb { class XColumn; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { template <typename > class Reference; }
namespace com::sun::star::util { class XNumberFormatter; }

namespace dbtools
{
    struct FormattedColumnValue_Data;

    //= FormattedColumnValue

    /** a class which helps retrieving and setting the value of a database column
        as formatted string.
    */
    class OOO_DLLPUBLIC_DBTOOLS FormattedColumnValue final
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
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const css::uno::Reference< css::sdbc::XRowSet >& _rxRowSet,
            const css::uno::Reference< css::beans::XPropertySet >& _rxColumn
        );

        /** constructs an instance

            The format key for the string value exchange is taken from the given column object.
            If it has a non-<NULL/> property value <code>FormatKey</code>, this key is taken.
            Otherwise, a default format matching the column type is determined.

            The locale of this fallback format is the current system locale.
        */
        FormattedColumnValue(
            const css::uno::Reference< css::util::XNumberFormatter >& i_rNumberFormatter,
            const css::uno::Reference< css::beans::XPropertySet >& i_rColumn
        );

        ~FormattedColumnValue();

        // access to the details of the formatting we determined
        sal_Int16   getKeyType() const;
        const css::uno::Reference< css::sdb::XColumn >&
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
