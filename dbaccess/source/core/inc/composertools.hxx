/*************************************************************************
 *
 *  $RCSfile: composertools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-05 12:29:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        #if OSL_DEBUG_LEVEL > 1
        bool                    m_bUsed;
        #endif

    protected:
        ::rtl::OUStringBuffer   m_aBuffer;

    public:
        ::rtl::OUString getComposedAndClear()
        {
            #if OSL_DEBUG_LEVEL > 1
            m_bUsed = true;
            #endif
            return m_aBuffer.makeStringAndClear();
        }

        void clear()
        {
            #if OSL_DEBUG_LEVEL > 1
            m_bUsed = false;
            #endif
            m_aBuffer.makeStringAndClear();
        }

    public:
        TokenComposer()
        #if OSL_DEBUG_LEVEL > 1
            :m_bUsed( false )
        #endif
        {
        }

        void operator() (const ::rtl::OUString& lhs)
        {
            append(lhs);
        }

        void append( const ::rtl::OUString& lhs )
        {
            #if OSL_DEBUG_LEVEL > 1
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

