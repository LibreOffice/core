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

#include <reportformula.hxx>

#include <rtl/ustrbuf.hxx>


namespace rptui
{
    namespace
    {

        const char sExpressionPrefix[] = "rpt:";
        const char sFieldPrefix[] =  "field:";
    }


    //= ReportFormula


    ReportFormula::ReportFormula( const OUString& _rFormula )
        :m_eType( Invalid )
    {
        m_sCompleteFormula = _rFormula;

        // is it an ordinary expression?
        if ( m_sCompleteFormula.startsWith( sExpressionPrefix, &m_sUndecoratedContent ) )
        {
            m_eType = Expression;
            return;
        }

        /// does it refer to a field?
        if ( m_sCompleteFormula.startsWith( sFieldPrefix ) )
        {
            sal_Int32 nPrefixLen = strlen(sFieldPrefix);
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


    ReportFormula::ReportFormula( const BindType _eType, const OUString& _rFieldOrExpression )
        :m_eType( _eType )
    {
        switch ( m_eType )
        {
        case Expression:
        {
            if ( _rFieldOrExpression.startsWith( sExpressionPrefix ) )
                m_sCompleteFormula = _rFieldOrExpression;
            else
                m_sCompleteFormula = sExpressionPrefix + _rFieldOrExpression;
        }
        break;

        case Field:
        {
            m_sCompleteFormula = sFieldPrefix + OUStringLiteral(u"[") + _rFieldOrExpression + "]";
        }
        break;
        default:
            OSL_FAIL( "ReportFormula::ReportFormula: illegal bind type!" );
            return;
        }

        m_sUndecoratedContent = _rFieldOrExpression;
    }

    ReportFormula::~ReportFormula()
    {
    }

    OUString ReportFormula::getBracketedFieldOrExpression() const
    {
        bool bIsField = ( getType() == Field );
        OUStringBuffer aFieldContent;
        if ( bIsField )
            aFieldContent.append( "[" );
        aFieldContent.append( getUndecoratedContent() );
        if ( bIsField )
            aFieldContent.append( "]" );

        return aFieldContent.makeStringAndClear();
    }

    bool                    ReportFormula::isValid() const { return getType() != Invalid; }

    OUString ReportFormula::getEqualUndecoratedContent() const
    {
        OUStringBuffer aBuffer;
        aBuffer.append( "=" );
        aBuffer.append( getUndecoratedContent() );
        return aBuffer.makeStringAndClear();
    }


} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
