/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_FMTOOLS_HXX
#define INCLUDED_SVX_FMTOOLS_HXX

#include <svx/svxdllapi.h>

#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
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
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>

#include <tools/wintypes.hxx>
#include <cppuhelper/weakref.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/implbase1.hxx>

#include <set>

class Window;


// common types

// displaying a database exception for the user
// display info about a simple ::com::sun::star::sdbc::SQLException
void displayException(const ::com::sun::star::sdbc::SQLException&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::sdbc::SQLWarning&, Window* _pParent = NULL);
SVX_DLLPUBLIC void displayException(const ::com::sun::star::sdb::SQLContext&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::sdb::SQLErrorEvent&, Window* _pParent = NULL);
void displayException(const ::com::sun::star::uno::Any&, Window* _pParent = NULL);

sal_Int32 getElementPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement);

SVX_DLLPUBLIC OUString getLabelName(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xControlModel);


// = class CursorWrapper - a helper class which works in common with a ::com::sun::star::uno::Reference<XDatabaseUpdateCursor>,
//                          XDatabaseBookmarkCursor and XDatabaseDirectCursor each


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
    CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor, bool bUseCloned = false);
    SVX_DLLPUBLIC CursorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, bool bUseCloned = false);
        // if bUseCloned == sal_True, the cursor is first doubled over the XCloneable interface (which it must implement)
        // and then used

    friend bool operator==(const CursorWrapper& lhs, const CursorWrapper& rhs)
    {
        return lhs.m_xGeneric.get() == rhs.m_xGeneric.get();
    }

    bool is() const { return m_xMoveOperations.is(); }
    bool Is() const { return m_xMoveOperations.is(); }

    CursorWrapper* operator ->() { return this; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& () const{ return m_xGeneric; }

    // 'Conversions'
    const CursorWrapper& operator=(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& xCursor);
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& () const          { return m_xMoveOperations; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate>& () const         { return m_xBookmarkOperations; }
    operator const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& () const   { return m_xColumnsSupplier; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&        getPropertySet() const      { return m_xPropertyAccess; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >&           getResultSet() const        { return m_xMoveOperations; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate >&          getRowLocate() const        { return m_xBookmarkOperations; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >&    getColumnsSupplier() const  { return m_xColumnsSupplier; }

    // the usual queryInterface
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
    void ImplConstruct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, bool bUseCloned);
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



class FmXDisposeMultiplexer :public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>       m_xObject;
    FmXDisposeListener* m_pListener;
    sal_Int16           m_nId;

    virtual ~FmXDisposeMultiplexer();
public:
    FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>& _rxObject, sal_Int16 _nId = -1);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    void dispose();
};



sal_Int16       getControlTypeByObject(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>& _rxObject);
    // get the object type (OBJ_FM_...) from the services the object supports


bool isRowSetAlive(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxRowSet);
    // checks if the ::com::sun::star::sdbcx::XColumnsSupplier provided by _rxRowSet supllies any columns


typedef ::std::set  < ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    , ::comphelper::OInterfaceCompare< ::com::sun::star::uno::XInterface >
                    > InterfaceBag;

#endif // INCLUDED_SVX_FMTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
