/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connectiontools.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:07:26 $
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

#ifndef DBACCESS_CONNECTIONTOOLS_HXX
#define DBACCESS_CONNECTIONTOOLS_HXX

#ifndef DBACCESS_MODULE_SDBT_HXX
#include "module_sdbt.hxx"
#endif

#ifndef DBACCESS_CONNECTION_DEPENDENT_HXX
#include "connectiondependent.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDB_TOOLS_XCONNECTIONTOOLS_HPP_
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= ConnectionTools
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::sdb::tools::XConnectionTools
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XInitialization
                                    >   ConnectionTools_Base;
    /** implements the com::sun::star::sdb::tools::XConnectionTools functionality
    */
    class ConnectionTools   :public ConnectionTools_Base
                            ,public ConnectionDependentComponent
    {
    private:
        SdbtClient                      m_aModuleClient;

    public:
        /** constructs a ConnectionTools instance

            @param _rxContext
                the context of the component
        */
        ConnectionTools( const ::comphelper::ComponentContext& _rContext );

        // XConnectionTools
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableName > SAL_CALL createTableName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XObjectNames > SAL_CALL getObjectNames() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const ::rtl::OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const ::rtl::OUString& command ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static();
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

        // XInitialization
        virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

    protected:
        ~ConnectionTools();

    private:
        ConnectionTools();                                      // never implemented
        ConnectionTools( const ConnectionTools& );              // never implemented
        ConnectionTools& operator=( const ConnectionTools& );   // never implemented
    };

//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_CONNECTIONTOOLS_HXX

