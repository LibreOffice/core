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

#include <config_options.h>
#include <svx/svxdllapi.h>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
#include <o3tl/sorted_vector.hxx>

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::container { class XIndexAccess; }
namespace com::sun::star::container { class XNameAccess; }
namespace com::sun::star::lang { class XComponent; }
namespace com::sun::star::lang { class XServiceInfo; }
namespace com::sun::star::sdbc { class SQLException; }
namespace com::sun::star::sdbc { class XRowSet; }
namespace com::sun::star::sdb { class SQLContext; }
namespace com::sun::star::sdb { struct SQLErrorEvent; }


// common types

// displaying a database exception for the user
// display info about a simple css::sdbc::SQLException
void displayException(const css::sdbc::SQLException&, const css::uno::Reference<css::awt::XWindow>& rParent);
SVXCORE_DLLPUBLIC void displayException(const css::sdb::SQLContext&, const css::uno::Reference<css::awt::XWindow>& rParent);
void displayException(const css::sdb::SQLErrorEvent&, const css::uno::Reference<css::awt::XWindow>& rParent);
void displayException(const css::uno::Any&, const css::uno::Reference<css::awt::XWindow>& rParent);

sal_Int32 getElementPos(const css::uno::Reference< css::container::XIndexAccess>& xCont, const css::uno::Reference< css::uno::XInterface>& xElement);

SVXCORE_DLLPUBLIC OUString getLabelName(const css::uno::Reference< css::beans::XPropertySet>& xControlModel);


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
    CursorWrapper(const css::uno::Reference< css::sdbc::XRowSet>& _rxCursor, bool bUseCloned = false);
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) CursorWrapper(const css::uno::Reference< css::sdbc::XResultSet>& _rxCursor, bool bUseCloned = false);
        // if bUseCloned == sal_True, the cursor is first doubled over the XCloneable interface (which it must implement)
        // and then used

    friend bool operator==(const CursorWrapper& lhs, const CursorWrapper& rhs)
    {
        return lhs.m_xGeneric.get() == rhs.m_xGeneric.get();
    }

    bool is() const { return m_xMoveOperations.is(); }
    bool Is() const { return m_xMoveOperations.is(); }

    operator const css::uno::Reference< css::uno::XInterface>& () const{ return m_xGeneric; }

    // 'Conversions'
    CursorWrapper& operator=(const css::uno::Reference< css::sdbc::XRowSet>& xCursor);
    operator const css::uno::Reference< css::sdbc::XResultSet>& () const          { return m_xMoveOperations; }

    const css::uno::Reference< css::beans::XPropertySet >&        getPropertySet() const      { return m_xPropertyAccess; }

    // css::uno::Reference< css::sdbcx::XRowLocate>
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    css::uno::Any getBookmark()
    { return m_xBookmarkOperations->getBookmark(); }
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    bool moveToBookmark(const css::uno::Any& bookmark) { return m_xBookmarkOperations->moveToBookmark(bookmark); }

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
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::container::XNameAccess> getColumns() const { return m_xColumnsSupplier->getColumns(); }
private:
    void ImplConstruct(const css::uno::Reference< css::sdbc::XResultSet>& _rxCursor, bool bUseCloned);
};


class FmXDisposeMultiplexer;
class SAL_WARN_UNUSED FmXDisposeListener
{
    friend class FmXDisposeMultiplexer;

    rtl::Reference<FmXDisposeMultiplexer> m_pAdapter;
    osl::Mutex   m_aMutex;

public:
    virtual ~FmXDisposeListener();

    /// @throws css::uno::RuntimeException
    virtual void disposing(sal_Int16 _nId) = 0;

protected:
    void setAdapter(FmXDisposeMultiplexer* pAdapter);
};

class SAL_WARN_UNUSED FmXDisposeMultiplexer : public cppu::WeakImplHelper< css::lang::XEventListener >
{
    css::uno::Reference< css::lang::XComponent>       m_xObject;
    FmXDisposeListener* m_pListener;

    virtual ~FmXDisposeMultiplexer() override;
public:
    FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const css::uno::Reference< css::lang::XComponent>& _rxObject);

// css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void dispose();
};

sal_Int16       getControlTypeByObject(const css::uno::Reference< css::lang::XServiceInfo>& _rxObject);
    // get the object type (OBJ_FM_...) from the services the object supports


bool isRowSetAlive(const css::uno::Reference< css::uno::XInterface>& _rxRowSet);
    // checks if the css::sdbcx::XColumnsSupplier provided by _rxRowSet supplies any columns


typedef ::o3tl::sorted_vector< css::uno::Reference< css::uno::XInterface > > InterfaceBag;

#endif // INCLUDED_SVX_FMTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
