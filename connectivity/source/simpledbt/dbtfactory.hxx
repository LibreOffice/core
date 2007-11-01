/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtfactory.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:53:04 $
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

#ifndef CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX
#define CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef CONNECTIVITY_DBTOOLS_REFBASE_HXX
#include "refbase.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ODataAccessToolsFactory
    //================================================================
    class ODataAccessToolsFactory
            :public simple::IDataAccessToolsFactory
            ,public ORefBase
    {
    protected:
        ::rtl::Reference< simple::IDataAccessTypeConversion >   m_xTypeConversionHelper;
        ::rtl::Reference< simple::IDataAccessTools >            m_xToolsHelper;

    public:
        ODataAccessToolsFactory();

        // IDataAccessToolsFactory
        virtual ::rtl::Reference< simple::ISQLParser >  createSQLParser(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceFactory,
            const IParseContext* _pContext
        ) const;

        virtual ::rtl::Reference< simple::IDataAccessCharSet > createCharsetHelper( ) const;

        virtual ::rtl::Reference< simple::IDataAccessTypeConversion > getTypeConversionHelper();

        virtual ::rtl::Reference< simple::IDataAccessTools > getDataAccessTools();

        virtual ::std::auto_ptr< ::dbtools::FormattedColumnValue >  createFormattedColumnValue(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX

