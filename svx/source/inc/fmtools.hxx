/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmtools.hxx,v $
 * $Revision: 1.27 $
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
#ifndef _SVX_FMTOOLS_HXX
#define _SVX_FMTOOLS_HXX

#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#ifndef _COM_SUN_STAR_SDBC_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vcl/wintypes.hxx>
#include <vos/mutex.hxx>

#ifndef _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif
#include <sfx2/ctrlitem.hxx>
#include <tools/link.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/weakref.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/component.hxx>
#include "fmstatic.hxx"

#include <set>

class Window;

//==================================================================
// allgemeine Typen
//==================================================================
// displaying a database exception for the user
// display info about a simple ::com::sun::star::sdbc::SQLException
void displayException(const ::com::sun::star::sdbc::SQLException&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::sdbc::SQLWarning&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::sdb::SQLContext&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::sdb::SQLErrorEvent&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::uno::Any&, Window* _pParent = NULL);

// StringConversion
namespace svxform
{
    // modes for the XModeSelector
    DECLARE_CONSTASCII_USTRING(DATA_MODE);
    DECLARE_CONSTASCII_USTRING(FILTER_MODE);

}   // namespace svxform


// Kopieren von Persistenten Objecten
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> cloneUsingProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>& _xObj);

sal_Int32 findPos(const ::rtl::OUString& aStr, const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rList);

// Suchen in einer Indexliste nach einem Element
sal_Bool  searchElement(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement);

sal_Int32 getElementPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement);
String getFormComponentAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xElement);
String getFormComponentAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xElement, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rTopLevelElement);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> getElementFromAccessPath(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xParent, const String& _rRelativePath);


::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel> getXModel(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xIface);

::rtl::OUString getLabelName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xControlModel);

// ===================================================================================================
// = class CursorWrapper - eine Hilfsklasse, die parallel mit je einem ::com::sun::star::uno::Reference<XDatabaseUpdateCursor>,
//                          XDatabaseBookmarkCursor und XDatabaseDirectCursor arbeitet
// ===================================================================================================

class CursorWrapper
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>                m_xGeneric;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>               m_xMoveOperations;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>              m_xBookmarkOperations;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>        m_xColumnsSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>            m_xPropertyAccess;

public:
    // Construction/Destruction
    CursorWrapper() { }
    CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor, sal_Bool bUseCloned = sal_False);
    CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned = sal_False);
        // bei bUseCloned == sal_True wird der Cursor ueber das XCloneable-Interface (dass er besitzen muss) gedoubled und
        // erst dann benutzt

    friend bool operator==(const CursorWrapper& lhs, const CursorWrapper& rhs)
    {
        return lhs.m_xGeneric.get() == rhs.m_xGeneric.get();
    }

    sal_Bool is() const { return m_xMoveOperations.is(); }
    sal_Bool Is() const { return m_xMoveOperations.is(); }

    CursorWrapper* operator ->() { return this; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& () const{ return m_xGeneric; }

    // 'Konvertierungen'
    const CursorWrapper& operator=(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xCursor);
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& () const          { return m_xMoveOperations; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>& () const         { return m_xBookmarkOperations; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& () const   { return m_xColumnsSupplier; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&        getPropertySet() const      { return m_xPropertyAccess; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >&           getResultSet() const        { return m_xMoveOperations; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate >&          getRowLocate() const        { return m_xBookmarkOperations; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >&    getColumnsSupplier() const  { return m_xColumnsSupplier; }

    // das normale queryInterface
    ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
    { return m_xMoveOperations->queryInterface(type); }

    // ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>
    ::com::sun::star::uno::Any getBookmark(void)
        throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException )
    { return m_xBookmarkOperations->getBookmark(); }
    sal_Bool moveToBookmark(const ::com::sun::star::uno::Any& bookmark) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->moveToBookmark(bookmark); }
    sal_Bool moveRelativeToBookmark(const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->moveRelativeToBookmark(bookmark, rows); }
    sal_Int32 compareBookmarks(const ::com::sun::star::uno::Any& lhs, const ::com::sun::star::uno::Any& rhs) const throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->compareBookmarks(lhs, rhs); }
    sal_Int32 hasOrderedBookmarks(void) const throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->hasOrderedBookmarks(); }
    sal_Int32 hashBookmark(const ::com::sun::star::uno::Any& bookmark) const throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) { return m_xBookmarkOperations->hashBookmark(bookmark); }

    // ::com::sun::star::sdbc::XResultSet
    sal_Bool isBeforeFirst() const              { return m_xMoveOperations->isBeforeFirst(); }
    sal_Bool isAfterLast() const                { return m_xMoveOperations->isAfterLast(); }
    sal_Bool isFirst() const                    { return m_xMoveOperations->isFirst(); }
    sal_Bool isLast() const                     { return m_xMoveOperations->isLast(); }
    void beforeFirst()                          { m_xMoveOperations->beforeFirst(); }
    sal_Bool first()                            { return m_xMoveOperations->first(); }
    sal_Bool last()                             { return m_xMoveOperations->last(); }
    sal_Int32 getRow() const                    { return m_xMoveOperations->getRow(); }
    sal_Bool absolute(sal_Int32 nPosition)          { return m_xMoveOperations->absolute(nPosition); }
    sal_Bool relative(sal_Int32 nCount)             { return m_xMoveOperations->relative(nCount); }
    sal_Bool previous()                         { return m_xMoveOperations->previous(); }
    sal_Bool next()                             { return m_xMoveOperations->next(); }
    void refreshRow()                       { m_xMoveOperations->refreshRow(); }
    sal_Bool rowUpdated()                       { return m_xMoveOperations->rowUpdated(); }
    sal_Bool rowInserted()                      { return m_xMoveOperations->rowInserted(); }
    sal_Bool rowDeleted()                       { return m_xMoveOperations->rowDeleted(); }
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> getStatement()         { return m_xMoveOperations->getStatement(); }
    // ::com::sun::star::sdbcx::XColumnsSupplier
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getColumns() const throw( ::com::sun::star::uno::RuntimeException ) { return m_xColumnsSupplier->getColumns(); }
private:
    void ImplConstruct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned);
};


class FmXDisposeMultiplexer;
class FmXDisposeListener
{
    friend class FmXDisposeMultiplexer;

    FmXDisposeMultiplexer*  m_pAdapter;
    ::osl::Mutex&   m_rMutex;

public:
    FmXDisposeListener(::osl::Mutex& _rMutex) : m_pAdapter(NULL), m_rMutex(_rMutex) { }
    virtual ~FmXDisposeListener();

    virtual void disposing(const ::com::sun::star::lang::EventObject& _rEvent, sal_Int16 _nId) throw( ::com::sun::star::uno::RuntimeException ) = 0;

protected:
    void setAdapter(FmXDisposeMultiplexer* pAdapter);
};

typedef ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener> FmXDisposeMultiplexer_x;
//==============================================================================


class FmXDisposeMultiplexer :public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       m_xObject;
    FmXDisposeListener* m_pListener;
    sal_Int16           m_nId;

    virtual ~FmXDisposeMultiplexer();
public:
    FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _rxObject, sal_Int16 _nId = -1);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void dispose();
};

//  ==================================================================

//========================================================================
//= dispatch interception helper classes
//========================================================================

//------------------------------------------------------------------------
//- FmDispatchInterceptor
//------------------------------------------------------------------------
class FmDispatchInterceptor
{
public:
    FmDispatchInterceptor() { }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> interceptedQueryDispatch(sal_uInt16 _nId,
        const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException ) = 0;

    virtual ::osl::Mutex* getInterceptorMutex() = 0;
};

//------------------------------------------------------------------------
//- FmXDispatchInterceptorImpl
//------------------------------------------------------------------------
typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::frame::XDispatchProviderInterceptor
                                        ,   ::com::sun::star::lang::XEventListener
                                        ,   ::com::sun::star::frame::XInterceptorInfo
                                        >   FmXDispatchInterceptorImpl_BASE;

class FmXDispatchInterceptorImpl : public FmXDispatchInterceptorImpl_BASE
{
    ::osl::Mutex                    m_aFallback;

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProviderInterception>
                    m_xIntercepted;
    sal_Bool        m_bListening;

    // the real interceptor
    FmDispatchInterceptor*          m_pMaster;

    // chaining
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xMasterDispatcher;

    // our id
    sal_Int16                       m_nId;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                    m_aInterceptedURLSchemes;

    virtual ~FmXDispatchInterceptorImpl();

public:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception> getIntercepted() const { return m_xIntercepted; }

public:
    FmXDispatchInterceptorImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>& _rToIntercept,
        FmDispatchInterceptor* _pMaster,
        sal_Int16 _nId,
        ::com::sun::star::uno::Sequence< ::rtl::OUString > _rInterceptedSchemes /// if not empty, this will be used for getInterceptedURLs
    );

    // StarOne
    DECLARE_UNO3_DEFAULTS(FmXDispatchInterceptorImpl, FmXDispatchInterceptorImpl_BASE);
    //  virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    //  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XInterceptorInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getInterceptedURLs(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

protected:
    void ImplDetach();

    ::osl::Mutex&       getAccessSafety()
    {
        if (m_pMaster && m_pMaster->getInterceptorMutex())
            return *m_pMaster->getInterceptorMutex();
        return m_aFallback;
    }
};

//==================================================================
// ...
//==================================================================
::rtl::OUString     getServiceNameByControlType(sal_Int16 nType);
    // get a service name to create a model of the given type (OBJ_FM_...)
sal_Int16       getControlTypeByObject(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>& _rxObject);
    // get the object type (OBJ_FM_...) from the services the object supports

void TransferEventScripts(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xModel, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor>& rTransferIfAvailable);

sal_Int16   GridView2ModelPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& rColumns, sal_Int16 nViewPos);

//==================================================================
sal_Bool isLoadable(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xLoad);
sal_Bool isRowSetAlive(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxRowSet);
    // checks if the ::com::sun::star::sdbcx::XColumnsSupplier provided by _rxRowSet supllies any columns

//==================================================================
typedef ::std::set  < ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    , ::comphelper::OInterfaceCompare< ::com::sun::star::uno::XInterface >
                    > InterfaceBag;

#endif // _SVX_FMTOOLS_HXX

