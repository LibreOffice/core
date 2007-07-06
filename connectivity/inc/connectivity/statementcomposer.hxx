/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statementcomposer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 06:47:01 $
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

#ifndef CONNECTIVITY_STATEMENTCOMPOSER_HXX
#define CONNECTIVITY_STATEMENTCOMPOSER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <memory>

//........................................................................
namespace dbtools
{
//........................................................................

    //====================================================================
    //= StatementComposer
    //====================================================================
    struct StatementComposer_Data;
    /** a class which is able to compose queries (SELECT statements) from a command and a command type
    */
    class StatementComposer : public ::boost::noncopyable
    {
        ::std::auto_ptr< StatementComposer_Data >   m_pData;

    public:
        /** constructs an instance

            @param _rxConnection
                the connection to work with. Must not be <NULL/>.
        */
        StatementComposer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::rtl::OUString&  _rCommand,
            const sal_Int32         _nCommandType,
            const sal_Bool          _bEscapeProcessing
        );

        ~StatementComposer();

        /** controls whether or not the instance disposes its XSingleSelectQueryComposer upon
            destruction

            Unless you explicitly call this method with the parameter being <TRUE/>,
            the XSingleSelectQueryComposer will be disposed when the StatementComposer
            instance is destroyed.
        */
        void    setDisposeComposer( bool _bDoDispose );
        bool    getDisposeComposer() const;

        void    setFilter( const ::rtl::OUString& _rFilter );
        void    setOrder( const ::rtl::OUString& _rOrder );

        /** returns the composer which has been fed with the current settings

            @throws ::com::sun::star::sdbc::SQLException
                if such an exception occurs while creating the composer
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                getComposer();

        /** returns the composer statement

            Effectively, this is equivalent to calling getComposer, and asking the composer
            for its Query attribute.

            @throws ::com::sun::star::sdbc::SQLException
                if such an exception occurs while creating the composer
        */
        ::rtl::OUString
                getQuery();

    private:
        StatementComposer();    // not implemented
    };

//........................................................................
} // namespace dbtools
//........................................................................

#endif // CONNECTIVITY_STATEMENTCOMPOSER_HXX
