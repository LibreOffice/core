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

#ifndef REPORTFORMULA_HXX
#define REPORTFORMULA_HXX

#include "dllapi.h"

#include <com/sun/star/uno/Any.hxx>

#include <osl/diagnose.h>

//........................................................................
namespace rptui
{
//........................................................................

    //====================================================================
    //= ReportFormula
    //====================================================================
    class REPORTDESIGN_DLLPUBLIC ReportFormula
    {
    public:
        enum BindType
        {
            Expression,
            Field,

            Invalid
        };

    private:
        BindType            m_eType;
        ::rtl::OUString     m_sCompleteFormula;
        ::rtl::OUString     m_sUndecoratedContent;

    public:
        /// constructs a ReportFormula object from a string
        ReportFormula( const ::rtl::OUString& _rFormula );

        /// constructs a ReportFormula by BindType
        ReportFormula( const BindType _eType, const ::rtl::OUString& _rFieldOrExpression );
        ~ReportFormula();

        ReportFormula& operator=(class ReportFormula const &);

        /// returns whether the object denotes a valid formula
        bool        isValid() const;

        /// returns the type of the binding represented by the formula
        inline BindType    getType() const { return m_eType; }

        /// returns the complete formula represented by the object
        const ::rtl::OUString&
                    getCompleteFormula() const;

        /** gets the <em>undecorated formula</em> content

            If the formula denotes a field binding, the <em>undecorated content</em> is the
            field name.

            If the formula denotes an expression, then the <em>undecorated content</em> is the expression
            itself.
        */
        const ::rtl::OUString& getUndecoratedContent() const;

        /// convenience alias for <code>getUndecoratedContent</code>, which asserts (in a non-product build) when used on an expression
        inline ::rtl::OUString  getFieldName() const;

        /**
            @returns "=" + getFieldName()
        */
        ::rtl::OUString getEqualUndecoratedContent() const;

        /// convenience alias for <code>getUndecoratedContent</code>, which asserts (in a non-product build) when used on a field
        inline ::rtl::OUString  getExpression() const;

        /** returns a bracketed field name of the formula denotes a field reference,
            or the undecorated expression if the formula denotes an expression.

            Effectively, this means the method returns the complete formular, stripped by the prefix
            which indicates a field or a expression.
        */
        ::rtl::OUString getBracketedFieldOrExpression() const;

    private:
        void    impl_construct( const ::rtl::OUString& _rFormula );
    };

    //--------------------------------------------------------------------
    inline ::rtl::OUString ReportFormula::getFieldName() const
    {
        OSL_PRECOND( getType() == Field, "ReportFormula::getFieldName: not bound to a field!" );
        return getUndecoratedContent();
    }

    //--------------------------------------------------------------------
    inline ::rtl::OUString ReportFormula::getExpression() const
    {
        OSL_PRECOND( getType() == Expression, "ReportFormula::getExpression: not bound to an expression!" );
        return getUndecoratedContent();
    }

//........................................................................
} // namespace rptui
//........................................................................

#endif // REPORTFORMULA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
