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

#include "reportformula.hxx"

#include <rtl/ustrbuf.hxx>

//........................................................................
namespace rptui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/

    namespace
    {
        //----------------------------------------------------------------
        const ::rtl::OUString&  lcl_getExpressionPrefix( sal_Int32* _pTakeLengthOrNull = NULL )
        {
            static ::rtl::OUString s_sPrefix( RTL_CONSTASCII_USTRINGPARAM( "rpt:" ) );
            if ( _pTakeLengthOrNull )
                *_pTakeLengthOrNull = s_sPrefix.getLength();
            return s_sPrefix;
        }

        //----------------------------------------------------------------
        const ::rtl::OUString&  lcl_getFieldPrefix( sal_Int32* _pTakeLengthOrNull = NULL )
        {
            static ::rtl::OUString s_sPrefix( RTL_CONSTASCII_USTRINGPARAM( "field:" ) );
            if ( _pTakeLengthOrNull )
                *_pTakeLengthOrNull = s_sPrefix.getLength();
            return s_sPrefix;
        }
    }

    //====================================================================
    //= ReportFormula
    //====================================================================
    //--------------------------------------------------------------------
    ReportFormula::ReportFormula( const ::rtl::OUString& _rFormula )
        :m_eType( Invalid )
    {
        impl_construct( _rFormula );
    }

    //--------------------------------------------------------------------
    ReportFormula::ReportFormula( const BindType _eType, const ::rtl::OUString& _rFieldOrExpression )
        :m_eType( _eType )
    {
        switch ( m_eType )
        {
        case Expression:
        {
            if ( _rFieldOrExpression.indexOf( lcl_getExpressionPrefix() ) == 0 )
                m_sCompleteFormula = _rFieldOrExpression;
            else
                m_sCompleteFormula = lcl_getExpressionPrefix() + _rFieldOrExpression;
        }
        break;

        case Field:
        {
            ::rtl::OUStringBuffer aBuffer;
            aBuffer.append( lcl_getFieldPrefix() );
            aBuffer.appendAscii( "[" );
            aBuffer.append( _rFieldOrExpression );
            aBuffer.appendAscii( "]" );
            m_sCompleteFormula = aBuffer.makeStringAndClear();
        }
        break;
        default:
            OSL_FAIL( "ReportFormula::ReportFormula: illegal bind type!" );
            return;
        }

        m_sUndecoratedContent = _rFieldOrExpression;
    }
    //--------------------------------------------------------------------
    ReportFormula::~ReportFormula()
    {
    }
    //--------------------------------------------------------------------
    void ReportFormula::impl_construct( const ::rtl::OUString& _rFormula )
    {
        m_sCompleteFormula = _rFormula;

        sal_Int32 nPrefixLen( -1 );
        // is it an ordinary expression?
        if ( m_sCompleteFormula.indexOf( lcl_getExpressionPrefix( &nPrefixLen ) ) == 0 )
        {
            m_eType = Expression;
            m_sUndecoratedContent = m_sCompleteFormula.copy( nPrefixLen );
            return;
        }

        /// does it refer to a field?
        if ( m_sCompleteFormula.indexOf( lcl_getFieldPrefix( &nPrefixLen ) ) == 0 )
        {
            if  (   ( m_sCompleteFormula.getLength() >= nPrefixLen + 2 )
                &&  ( m_sCompleteFormula[ nPrefixLen ] == '[' )
                &&  ( m_sCompleteFormula[ m_sCompleteFormula.getLength() - 1 ] == ']' )
                )
            {
                m_eType = Field;
                m_sUndecoratedContent = m_sCompleteFormula.copy( nPrefixLen + 1, m_sCompleteFormula.getLength() - nPrefixLen - 2 );
                return;
            }
        }

        m_eType = Invalid;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ReportFormula::getBracketedFieldOrExpression() const
    {
        bool bIsField = ( getType() == Field );
        ::rtl::OUStringBuffer aFieldContent;
        if ( bIsField )
            aFieldContent.appendAscii( "[" );
        aFieldContent.append( getUndecoratedContent() );
        if ( bIsField )
            aFieldContent.appendAscii( "]" );

        return aFieldContent.makeStringAndClear();
    }
    //--------------------------------------------------------------------
    const ::rtl::OUString& ReportFormula::getUndecoratedContent() const
    {
        return m_sUndecoratedContent;
    }
    const ::rtl::OUString&  ReportFormula::getCompleteFormula() const { return m_sCompleteFormula; }
    bool                    ReportFormula::isValid() const { return getType() != Invalid; }
    ReportFormula& ReportFormula::operator=(class ReportFormula const & _rHd)
    {
        if ( this == &_rHd )
            return *this;
        m_eType                 = _rHd.m_eType;
        m_sCompleteFormula      = _rHd.m_sCompleteFormula;
        m_sUndecoratedContent   = _rHd.m_sUndecoratedContent;
        return *this;
    }
    //--------------------------------------------------------------------
    ::rtl::OUString ReportFormula::getEqualUndecoratedContent() const
    {
        ::rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii( "=" );
        aBuffer.append( getUndecoratedContent() );
        return aBuffer.makeStringAndClear();
    }

//........................................................................
} // namespace rptui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
