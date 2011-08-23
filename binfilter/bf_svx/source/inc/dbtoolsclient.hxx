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

#ifndef SVX_DBTOOLSCLIENT_HXX
#define SVX_DBTOOLSCLIENT_HXX

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef _OSL_MODULE_H_ 
#include <osl/module.h>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {
//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= ODbtoolsClient
    //====================================================================
    /** base class for classes which want to use dbtools features with load-on-call
        of the dbtools lib.
    */
    class ODbtoolsClient
    {
    private:
        static	::osl::Mutex	s_aMutex;
        static	sal_Int32		s_nClients;
        static oslModule		s_hDbtoolsModule;
        static ::connectivity::simple::createDataAccessToolsFactoryFunction
                                s_pFactoryCreationFunc;
        //add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)
        mutable BOOL            m_bCreateAlready;

    private:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessToolsFactory >	m_xDataAccessFactory;

    protected:
        const ::rtl::Reference< ::connectivity::simple::IDataAccessToolsFactory >&
            getFactory() const { return m_xDataAccessFactory; }

    protected:
        ODbtoolsClient();
        ~ODbtoolsClient();
        //add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)

    private:
    };

    //====================================================================
    //= OStaticDataAccessTools
    //====================================================================
    class OStaticDataAccessTools : public ODbtoolsClient
    {
    protected:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessTools >	m_xDataAccessTools;
        //add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)
        void create() const;

        void checkIfLoaded() const;

    public:
        OStaticDataAccessTools();

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getRowSetConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet) 
                const SAL_THROW ( (::com::sun::star::uno::RuntimeException) );

        // ------------------------------------------------

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
                const ::rtl::OUString& _rsRegisteredName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            ) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
            @param		_rxCursorSet	the property set
        */
        sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
            @param		_rxCursorSet	the property set
        */
        sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::DELETE
            @param		_rxCursorSet	the property set
        */
        sal_Bool canDelete(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getFieldsByCommandDescriptor(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rCommand,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxKeepFieldsAlive,
                ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
            )	SAL_THROW( ( ) );

        // ------------------------------------------------
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
            getFieldNamesByCommandDescriptor(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rCommand,
                ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
            )	SAL_THROW( ( ) );
    };
//........................................................................
}	// namespace svxform
//........................................................................

}//end of namespace binfilter
#endif // SVX_DBTOOLSCLIENT_HXX


