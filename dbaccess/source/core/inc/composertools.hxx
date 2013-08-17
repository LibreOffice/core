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

#ifndef DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX
#define DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX

#include <rtl/ustrbuf.hxx>

#include <functional>

namespace dbaccess
{

    // TokenComposer
    struct TokenComposer : public ::std::unary_function< OUString, void >
    {
    private:
        #ifdef DBG_UTIL
        bool                    m_bUsed;
        #endif

    protected:
        OUStringBuffer   m_aBuffer;

    public:
        OUString getComposedAndClear()
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

        void operator() (const OUString& lhs)
        {
            append(lhs);
        }

        void append( const OUString& lhs )
        {
            #ifdef DBG_UTIL
            OSL_ENSURE( !m_bUsed, "FilterCreator::append: already used up!" );
            #endif
            if ( !lhs.isEmpty() )
            {
                if ( !m_aBuffer.isEmpty() )
                    appendNonEmptyToNonEmpty( lhs );
                else
                    m_aBuffer.append( lhs );
            }
        }

        /// append the given part. Only to be called when both the part and our buffer so far are not empty
        virtual void appendNonEmptyToNonEmpty( const OUString& lhs ) = 0;
    };

    // FilterCreator
    struct FilterCreator : public TokenComposer
    {
        virtual void appendNonEmptyToNonEmpty( const OUString& lhs )
        {
            m_aBuffer.insert( 0, (sal_Unicode)' ' );
            m_aBuffer.insert( 0, (sal_Unicode)'(' );
            m_aBuffer.appendAscii( " ) AND ( " );
            m_aBuffer.append( lhs );
            m_aBuffer.appendAscii( " )" );
        }
    };

    // FilterCreator
    struct OrderCreator : public TokenComposer
    {
        virtual void appendNonEmptyToNonEmpty( const OUString& lhs )
        {
            m_aBuffer.appendAscii( ", " );
            m_aBuffer.append( lhs );
        }
    };

} // namespace dbaccess

#endif // DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
