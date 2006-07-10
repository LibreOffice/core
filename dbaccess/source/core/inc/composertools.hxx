/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: composertools.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:12:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX
#define DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <functional>

//........................................................................
namespace dbaccess
{
//........................................................................

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

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX

