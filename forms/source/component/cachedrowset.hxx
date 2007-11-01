/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedrowset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:56:27 $
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

#ifndef CACHEDROWSET_HXX
#define CACHEDROWSET_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>

#include <memory>

//........................................................................
namespace frm
{
//........................................................................

    struct CachedRowSet_Data;
    //====================================================================
    //= CachedRowSet
    //====================================================================
    class CachedRowSet
    {
    public:
        CachedRowSet( const ::comphelper::ComponentContext& _rContext );
        ~CachedRowSet();

    public:
        /** executes the row set

            Since the class is called <code>CachedRowSet</code>, it will cache the
            row set between the calls. If none of the parameters for the row set
            changes inbetween, then the row set instance from the previous call will be returned,
            without re-execution.

            @throws ::com::sun::star::sdbc::SQLException
                if such an exception is thrown when executing the <code>XRowSet</code>
        */
        void    execute();

        /// determines whether the row set properties are dirty, i.e. have changed since the last call to execute
        bool    isDirty() const;

        /// disposes the row set instance, and frees all associated resources
        void    dispose();

        /** returns the row set represented by this instance

            If the row set has not been executed before, <NULL/> is returned. If the row set
            properties are dirty, an old (dirty) instance of the <code>XRowSet</code> is returned.
            The caller of the method is responsible for preventing those cases.
        */
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >&
                getRowSet() const;

        void    setDataSource       ( const ::rtl::OUString& _rDataSource );
        void    setCommand          ( const ::rtl::OUString& _rCommand );
        void    setCommandType      ( const sal_Int32 _nCommandType );
        void    setEscapeProcessing ( const sal_Bool _bEscapeProcessing );
        void    setConnection       ( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection );

        void    setDataSource       ( const ::com::sun::star::uno::Any& _rDataSourceValue );
        void    setCommand          ( const ::com::sun::star::uno::Any& _rCommandValue );
        void    setEscapeProcessing ( const ::com::sun::star::uno::Any& _rEscapeProcessingValue );
        void    setConnection       ( const ::com::sun::star::uno::Any& _rConnectionValue );

    private:
        ::std::auto_ptr< CachedRowSet_Data >    m_pData;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // CACHEDROWSET_HXX
