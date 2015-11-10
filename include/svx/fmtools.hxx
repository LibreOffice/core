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

namespace vcl { class Window; }


// common types

// displaying a database exception for the user
// display info about a simple css::sdbc::SQLException
void displayException(const css::sdbc::SQLException&, vcl::Window* _pParent = nullptr);
SVX_DLLPUBLIC void displayException(const css::sdb::SQLContext&, vcl::Window* _pParent = nullptr);
void displayException(const css::sdb::SQLErrorEvent&, vcl::Window* _pParent = nullptr);
void displayException(const css::uno::Any&, vcl::Window* _pParent = nullptr);

sal_Int32 getElementPos(const css::uno::Reference< css::container::XIndexAccess>& xCont, const css::uno::Reference< css::uno::XInterface>& xElement);

SVX_DLLPUBLIC OUString getLabelName(const css::uno::Reference< css::beans::XPropertySet>& xControlModel);


// = class CursorWrapper - a helper class which works in common with a css::uno::Reference<XDatabaseUpdateCursor>,
//                          XDatabaseBookmarkCursor and XDatabaseDirectCursor each


class SAL_WARN_UNUSED CursorWrapper
{
private:
    css::uno::Reference< css::uno::XInterface>                m_xGeneric;
    css::uno::Reference< css::sdbc::XResultSet>               m_xMoveOperations;
    css::uno::Reference< css::sdbcx::XRowLocate>              m_xBookmarkOperations;
    css::uno::Reference< css::sdbcx::XColumnsSupplier>        m_xColumnsSupplier;
    css::uno::Reference< css::beans::XPropertySet>            m_xPropertyAccess;

public:
    // Construction/Destruction
    CursorWrapper() { }
    CursorWrapper(const css::uno::Reference< css::sdbc::XRowSet>& _rxCursor, bool bUseCloned = false);
    SVX_DLLPUBLIC CursorWrapper(const css::uno::Reference< css::sdbc::XResultSet>& _rxCursor, bool bUseCloned = false);
        // if bUseCloned == sal_True, the cursor is first doubled over the XCloneable interface (which it must implement)
        // and then used

    friend bool operator==(const CursorWrapper& lhs, const CursorWrapper& rhs)
    {
        return lhs.m_xGeneric.get() == rhs.m_xGeneric.get();
    }

    bool is() const { return m_xMoveOperations.is(); }
    bool Is() const { return m_xMoveOperations.is(); }

    CursorWrapper* operator ->() { return this; }
    operator const css::uno::Reference< css::uno::XInterface>& () const{ return m_xGeneric; }

    // 'Conversions'
    const CursorWrapper& operator=(const css::uno::Reference< css::sdbc::XRowSet>& xCursor);
    operator const css::uno::Reference< css::sdbc::XResultSet>& () const          { return m_xMoveOperations; }
    operator const css::uno::Reference< css::sdbcx::XRowLocate>& () const         { return m_xBookmarkOperations; }
    operator const css::uno::Reference< css::sdbcx::XColumnsSupplier>& () const   { return m_xColumnsSupplier; }

    const css::uno::Reference< css::beans::XPropertySet >&        getPropertySet() const      { return m_xPropertyAccess; }

    // css::uno::Reference< css::sdbcx::XRowLocate>
    css::uno::Any getBookmark()
        throw( css::sdbc::SQLException, css::uno::RuntimeException )
    { return m_xBookmarkOperations->getBookmark(); }
    bool moveToBookmark(const css::uno::Any& bookmark) throw( css::sdbc::SQLException, css::uno::RuntimeException ) { return m_xBookmarkOperations->moveToBookmark(bookmark); }

    // css::sdbc::XResultSet
    bool isBeforeFirst() const              { return m_xMoveOperations->isBeforeFirst(); }
    bool isAfterLast() const                { return m_xMoveOperations->isAfterLast(); }
    bool isFirst() const                    { return m_xMoveOperations->isFirst(); }
    bool isLast() const                     { return m_xMoveOperations->isLast(); }
    void beforeFirst()                          { m_xMoveOperations->beforeFirst(); }
    bool first()                            { return m_xMoveOperations->first(); }
    bool last()                             { return m_xMoveOperations->last(); }
    sal_Int32 getRow() const                    { return m_xMoveOperations->getRow(); }
    bool absolute(sal_Int32 nPosition)          { return m_xMoveOperations->absolute(nPosition); }
    bool relative(sal_Int32 nCount)             { return m_xMoveOperations->relative(nCount); }
    bool previous()                         { return m_xMoveOperations->previous(); }
    bool next()                             { return m_xMoveOperations->next(); }
    void refreshRow()                       { m_xMoveOperations->refreshRow(); }
    bool rowDeleted()                       { return m_xMoveOperations->rowDeleted(); }
    // css::sdbcx::XColumnsSupplier
    css::uno::Reference< css::container::XNameAccess> getColumns() const throw( css::uno::RuntimeException ) { return m_xColumnsSupplier->getColumns(); }
private:
    void ImplConstruct(const css::uno::Reference< css::sdbc::XResultSet>& _rxCursor, bool bUseCloned);
};


class FmXDisposeMultiplexer;
class SAL_WARN_UNUSED FmXDisposeListener
{
    friend class FmXDisposeMultiplexer;

    FmXDisposeMultiplexer*  m_pAdapter;
    ::osl::Mutex&   m_rMutex;

public:
    FmXDisposeListener(::osl::Mutex& _rMutex) : m_pAdapter(nullptr), m_rMutex(_rMutex) { }
    virtual ~FmXDisposeListener();

    virtual void disposing(const css::lang::EventObject& _rEvent, sal_Int16 _nId) throw( css::uno::RuntimeException ) = 0;

protected:
    void setAdapter(FmXDisposeMultiplexer* pAdapter);
};

class SAL_WARN_UNUSED FmXDisposeMultiplexer : public ::cppu::WeakImplHelper1< css::lang::XEventListener>
{
    css::uno::Reference< css::lang::XComponent>       m_xObject;
    FmXDisposeListener* m_pListener;
    sal_Int16           m_nId;

    virtual ~FmXDisposeMultiplexer();
public:
    FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const css::uno::Reference< css::lang::XComponent>& _rxObject, sal_Int16 _nId = -1);

// css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    void dispose();
};

sal_Int16       getControlTypeByObject(const css::uno::Reference< css::lang::XServiceInfo>& _rxObject);
    // get the object type (OBJ_FM_...) from the services the object supports


bool isRowSetAlive(const css::uno::Reference< css::uno::XInterface>& _rxRowSet);
    // checks if the css::sdbcx::XColumnsSupplier provided by _rxRowSet supplies any columns


typedef ::std::set  < css::uno::Reference< css::uno::XInterface >
                    , ::comphelper::OInterfaceCompare< css::uno::XInterface >
                    > InterfaceBag;

#endif // INCLUDED_SVX_FMTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
