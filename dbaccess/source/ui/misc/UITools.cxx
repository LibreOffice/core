
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

// .........................................................................
namespace dbaui
{
// .........................................................................
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui;


// -----------------------------------------------------------------------------
void composeTableName(  const Reference< XDatabaseMetaData >& _rxMetaData,
                        const Reference< XPropertySet >& _rxTable,
                        ::rtl::OUString& _rComposedName,
                        sal_Bool _bQuote)
{
    OSL_ENSURE(_rxTable.is(),"Table can not be null!");
    if(_rxTable.is())
    {
        Reference< XPropertySetInfo > xInfo = _rxTable->getPropertySetInfo();
        if(xInfo->hasPropertyByName(PROPERTY_CATALOGNAME) && xInfo->hasPropertyByName(PROPERTY_SCHEMANAME) && xInfo->hasPropertyByName(PROPERTY_NAME))
        {
            ::rtl::OUString aCatalog,aSchema,aTable;
            _rxTable->getPropertyValue(PROPERTY_CATALOGNAME)>>= aCatalog;
            _rxTable->getPropertyValue(PROPERTY_SCHEMANAME) >>= aSchema;
            _rxTable->getPropertyValue(PROPERTY_NAME)       >>= aTable;

            ::dbtools::composeTableName(_rxMetaData,aCatalog,aSchema,aTable,_rComposedName,_bQuote);
        }
    }
}
// -----------------------------------------------------------------------------

SQLExceptionInfo createConnection(  const ::rtl::OUString& _rsDataSourceName,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xDatabaseContext,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rMF,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rEvtLst,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rOUTConnection )
{
    Any aValue;
    try
    {
        aValue = _xDatabaseContext->getByName(_rsDataSourceName);
    }
    catch(Exception&)
    {
    }
    SQLExceptionInfo aInfo;
    Reference<XPropertySet> xProp;
    aValue >>= xProp;
    if (!xProp.is())
    {
        OSL_ENSURE(0,"createConnection: coult not retrieve the data source!");
        return aInfo;
    }

    ::rtl::OUString sPwd, sUser;
    sal_Bool bPwdReq = sal_False;
    try
    {
        xProp->getPropertyValue(PROPERTY_PASSWORD) >>= sPwd;
        bPwdReq = ::cppu::any2bool(xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED));
        xProp->getPropertyValue(PROPERTY_USER) >>= sUser;
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"createConnection: error while retrieving data source properties!");
    }


    try
    {
        if(bPwdReq && !sPwd.getLength())
        {   // password required, but empty -> connect using an interaction handler
            Reference<XCompletedConnection> xConnectionCompletion(xProp, UNO_QUERY);
            if (!xConnectionCompletion.is())
            {
                OSL_ENSURE(0,"createConnection: missing an interface ... need an error message here!");
            }
            else
            {   // instantiate the default SDB interaction handler
                Reference< XInteractionHandler > xHandler(_rMF->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
                if (!xHandler.is())
                {
                    // ShowServiceNotAvailableError(NULL, String(SERVICE_SDB_INTERACTION_HANDLER), sal_True);
                        // TODO: a real parent!
                }
                else
                    _rOUTConnection = xConnectionCompletion->connectWithCompletion(xHandler);
            }
        }
        else
        {
            Reference<XDataSource> xDataSource(xProp,UNO_QUERY);
            _rOUTConnection = xDataSource->getConnection(sUser, sPwd);
        }
        // be notified when connection is in disposing
        Reference< XComponent >  xComponent(_rOUTConnection, UNO_QUERY);
        if (xComponent.is() && _rEvtLst.is())
            xComponent->addEventListener(_rEvtLst);
    }
    catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
    catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
    catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
    catch(Exception&) { OSL_ENSURE(0,"SbaTableQueryBrowser::OnExpandEntry: could not connect - unknown exception!"); }

    //  showError(aInfo);

    return aInfo;
}
// -----------------------------------------------------------------------------
void showError(const SQLExceptionInfo& _rInfo,Window* _pParent,const Reference< XMultiServiceFactory >& _xFactory)
{
    if (_rInfo.isValid())
    {
        try
        {
            Sequence< Any > aArgs(2);
            aArgs[0] <<= PropertyValue(PROPERTY_SQLEXCEPTION, 0, _rInfo.get(), PropertyState_DIRECT_VALUE);
            aArgs[1] <<= PropertyValue(PROPERTY_PARENTWINDOW, 0, makeAny(VCLUnoHelper::GetInterface(_pParent)), PropertyState_DIRECT_VALUE);


            Reference< XExecutableDialog > xErrorDialog(
                _xFactory->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.sdb.ErrorMessageDialog"), aArgs), UNO_QUERY);
            if (xErrorDialog.is())
                xErrorDialog->execute();
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"showError: could not display the error message!");
        }
    }
}
// -----------------------------------------------------------------------------

// .........................................................................
}
// .........................................................................
