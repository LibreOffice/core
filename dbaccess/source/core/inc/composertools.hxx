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

#ifndef DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX
#define DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <rtl/ustrbuf.hxx>

#include <functional>

namespace dbaccess
{

    //====================================================================
    //= TokenComposer
    //====================================================================
    struct TokenComposer : public ::std::unary_function< ::rtl::OUString, void >
    {
    private:
        #ifdef DBG_UTIL
        bool                    m_bUsed;
        #endif

    protected:
        ::rtl::OUStringBuffer   m_aBuffer;

    public:
        ::rtl::OUString getComposedAndClear()
        {
            #ifdef DBG_UTIL
            m_bUsed = true;
            #endif
            return m_aBuffer.makeStringAndClear();
        }

        void clear()
        {
            #ifdef DBG_UTIL
            m_bUsed = false;
            #endif
            m_aBuffer.makeStringAndClear();
        }

    public:
        TokenComposer()
        #ifdef DBG_UTIL
            :m_bUsed( false )
        #endif
        {
        }

        virtual ~TokenComposer()
        {
        }

        void operator() (const ::rtl::OUString& lhs)
        {
            append(lhs);
        }

        void append( const ::rtl::OUString& lhs )
        {
            #ifdef DBG_UTIL
            OSL_ENSURE( !m_bUsed, "FilterCreator::append: already used up!" );
            #endif
            if ( lhs.getLength() )
            {
                if ( m_aBuffer.getLength() )
                    appendNonEmptyToNonEmpty( lhs );
                else
                    m_aBuffer.append( lhs );
            }
        }

        /// append the given part. Only to be called when both the part and our buffer so far are not empty
        virtual void appendNonEmptyToNonEmpty( const ::rtl::OUString& lhs ) = 0;
    };

    //====================================================================
    //= FilterCreator
    //====================================================================
    struct FilterCreator : public TokenComposer
    {
        virtual void appendNonEmptyToNonEmpty( const ::rtl::OUString& lhs )
        {
            m_aBuffer.insert( 0, (sal_Unicode)' ' );
            m_aBuffer.insert( 0, (sal_Unicode)'(' );
            m_aBuffer.appendAscii( " ) AND ( " );
            m_aBuffer.append( lhs );
            m_aBuffer.appendAscii( " )" );
        }
    };

    //====================================================================
    //= FilterCreator
    //====================================================================
    struct OrderCreator : public TokenComposer
    {
        virtual void appendNonEmptyToNonEmpty( const ::rtl::OUString& lhs )
        {
            m_aBuffer.appendAscii( ", " );
            m_aBuffer.append( lhs );
        }
    };

} // namespace dbaccess

#endif // DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
