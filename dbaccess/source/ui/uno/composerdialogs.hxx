/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: composerdialogs.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 08:44:52 $
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

#ifndef DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX
#define DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include <svtools/genericunodialog.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= ComposerDialog
    //=====================================================================
    class ComposerDialog;
    typedef ::svt::OGenericUnoDialog                                    ComposerDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< ComposerDialog >  ComposerDialog_PBASE;

    class ComposerDialog
            :public ComposerDialog_BASE
            ,public ComposerDialog_PBASE
    {
        OModuleClient m_aModuleClient;
    protected:
        // <properties>
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                        m_xComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                        m_xRowSet;
        // </properties>

    protected:
        ComposerDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        ~ComposerDialog();

    public:
        DECLARE_IMPLEMENTATION_ID( );

        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    protected:
        // own overridables
        virtual Dialog* createComposerDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns
        ) = 0;

    private:
        // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
    };

    //=====================================================================
    //= RowsetFilterDialog
    //=====================================================================
    class RowsetFilterDialog : public ComposerDialog
    {
    public:
        RowsetFilterDialog(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        DECLARE_SERVICE_INFO_STATIC( );

    protected:
        // own overridables
        virtual Dialog* createComposerDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns
        );

        // OGenericUnoDialog overridables
        virtual void executedDialog( sal_Int16 _nExecutionResult );
    };

    //=====================================================================
    //= RowsetOrderDialog
    //=====================================================================
    class RowsetOrderDialog : public ComposerDialog
    {
    public:
        RowsetOrderDialog(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        DECLARE_SERVICE_INFO_STATIC( );

    protected:
        // own overridables
        virtual Dialog* createComposerDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns
        );

        // OGenericUnoDialog overridables
        virtual void executedDialog( sal_Int16 _nExecutionResult );
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX


