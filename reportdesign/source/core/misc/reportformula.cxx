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
#include "precompiled_reportdesign.hxx"
#include "reportformula.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

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
