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


#include "fmprop.hrc"
#include "fmservs.hxx"
#include "svx/fmtools.hxx"
#include "svx/dbtoolsclient.hxx"
#include "svx/fmglob.hxx"

#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdb/ErrorMessageDialog.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/Language.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>

#include <basic/sbxvar.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <comphelper/container.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/math.hxx>
#include <sfx2/bindings.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::form;
using namespace ::svxform;
using namespace ::connectivity::simple;

//  ------------------------------------------------------------------------------
namespace
{
    static bool lcl_shouldDisplayError( const Any& _rError )
    {
        SQLException aError;
        if ( !( _rError >>= aError ) )
            return true;

        if ( aError.Message.indexOfAsciiL( RTL_CONSTASCII_STRINGPARAM( "[OOoBase]" ) ) != 0 )
            // it is an exception *not* thrown by an OOo Base core component
            return true;

        // the only exception we do not display ATM is a RowSetVetoException, which
        // has been raised because an XRowSetApprovalListener vetoed a change
        if ( aError.ErrorCode + ErrorCondition::ROW_SET_OPERATION_VETOED == 0 )
            return false;

        // everything else is to be displayed
        return true;
    }
}

//  ------------------------------------------------------------------------------
void displayException(const Any& _rExcept, Window* _pParent)
{
    // check whether we need to display it
    if ( !lcl_shouldDisplayError( _rExcept ) )
        return;

    try
    {
        // the parent window
        Window* pParentWindow = _pParent ? _pParent : GetpApp()->GetDefDialogParent();
        Reference< XWindow > xParentWindow = VCLUnoHelper::GetInterface(pParentWindow);

        Reference< XExecutableDialog > xErrorDialog = ErrorMessageDialog::create(::comphelper::getProcessComponentContext(), "", xParentWindow, _rExcept);
        xErrorDialog->execute();
    }
    catch(const Exception&)
    {
        OSL_FAIL("displayException: could not display the error message!");
    }
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdbc::SQLException& _rExcept, Window* _pParent)
{
    displayException(makeAny(_rExcept), _pParent);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdbc::SQLWarning& _rExcept, Window* _pParent)
{
    displayException(makeAny(_rExcept), _pParent);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdb::SQLContext& _rExcept, Window* _pParent)
{
    displayException(makeAny(_rExcept), _pParent);
}

//  ------------------------------------------------------------------------------
void displayException(const ::com::sun::star::sdb::SQLErrorEvent& _rEvent, Window* _pParent)
{
    displayException(_rEvent.Reason, _pParent);
}

//------------------------------------------------------------------------------
sal_Int32 getElementPos(const Reference< ::com::sun::star::container::XIndexAccess>& xCont, const Reference< XInterface >& xElement)
{
    sal_Int32 nIndex = -1;
    if (!xCont.is())
        return nIndex;


    Reference< XInterface > xNormalized( xElement, UNO_QUERY );
    DBG_ASSERT( xElement.is(), "getElementPos: invalid element!" );
    if ( xNormalized.is() )
    {
        // Feststellen an welcher Position sich das Kind befindet
        nIndex = xCont->getCount();
        while (nIndex--)
        {
            try
            {
                Reference< XInterface > xCurrent(xCont->getByIndex( nIndex ),UNO_QUERY);
                DBG_ASSERT( xCurrent.get() == Reference< XInterface >( xCurrent, UNO_QUERY ).get(),
                    "getElementPos: container element not normalized!" );
                if ( xNormalized.get() == xCurrent.get() )
                    break;
            }
            catch(Exception&)
            {
                OSL_FAIL( "getElementPos: caught an exception!" );
            }

        }
    }
    return nIndex;
}

//------------------------------------------------------------------
OUString getLabelName(const Reference< ::com::sun::star::beans::XPropertySet>& xControlModel)
{
    if (!xControlModel.is())
        return OUString();

    if (::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xControlModel))
    {
        Reference< ::com::sun::star::beans::XPropertySet> xLabelSet;
        xControlModel->getPropertyValue(FM_PROP_CONTROLLABEL) >>= xLabelSet;
        if (xLabelSet.is() && ::comphelper::hasProperty(FM_PROP_LABEL, xLabelSet))
        {
            Any aLabel( xLabelSet->getPropertyValue(FM_PROP_LABEL) );
            if ((aLabel.getValueTypeClass() == TypeClass_STRING) && !::comphelper::getString(aLabel).isEmpty())
                return ::comphelper::getString(aLabel);
        }
    }

    return ::comphelper::getString(xControlModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
}

//========================================================================
// = CursorWrapper
//------------------------------------------------------------------------
CursorWrapper::CursorWrapper(const Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor, sal_Bool bUseCloned)
{
    ImplConstruct(Reference< ::com::sun::star::sdbc::XResultSet>(_rxCursor, UNO_QUERY), bUseCloned);
}

//------------------------------------------------------------------------
CursorWrapper::CursorWrapper(const Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned)
{
    ImplConstruct(_rxCursor, bUseCloned);
}

//------------------------------------------------------------------------
void CursorWrapper::ImplConstruct(const Reference< ::com::sun::star::sdbc::XResultSet>& _rxCursor, sal_Bool bUseCloned)
{
    if (bUseCloned)
    {
        Reference< ::com::sun::star::sdb::XResultSetAccess> xAccess(_rxCursor, UNO_QUERY);
        try
        {
            m_xMoveOperations = xAccess.is() ? xAccess->createResultSet() : Reference< ::com::sun::star::sdbc::XResultSet>();
        }
        catch(Exception&)
        {
        }
    }
    else
        m_xMoveOperations   = _rxCursor;

    m_xBookmarkOperations   = m_xBookmarkOperations.query( m_xMoveOperations );
    m_xColumnsSupplier      = m_xColumnsSupplier.query( m_xMoveOperations );
    m_xPropertyAccess       = m_xPropertyAccess.query( m_xMoveOperations );

    if ( !m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is() || !m_xPropertyAccess.is() )
    {   // all or nothing !!
        m_xMoveOperations = NULL;
        m_xBookmarkOperations = NULL;
        m_xColumnsSupplier = NULL;
    }
    else
        m_xGeneric = m_xMoveOperations.get();
}

//------------------------------------------------------------------------
const CursorWrapper& CursorWrapper::operator=(const Reference< ::com::sun::star::sdbc::XRowSet>& _rxCursor)
{
    m_xMoveOperations = Reference< ::com::sun::star::sdbc::XResultSet>(_rxCursor, UNO_QUERY);
    m_xBookmarkOperations = Reference< ::com::sun::star::sdbcx::XRowLocate>(_rxCursor, UNO_QUERY);
    m_xColumnsSupplier = Reference< ::com::sun::star::sdbcx::XColumnsSupplier>(_rxCursor, UNO_QUERY);
    if (!m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is())
    {   // all or nothing !!
        m_xMoveOperations = NULL;
        m_xBookmarkOperations = NULL;
        m_xColumnsSupplier = NULL;
    }
    return *this;
}

//------------------------------------------------------------------------------
FmXDisposeListener::~FmXDisposeListener()
{
    setAdapter(NULL);
}

//------------------------------------------------------------------------------
void FmXDisposeListener::setAdapter(FmXDisposeMultiplexer* pAdapter)
{
    if (m_pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter->release();
        m_pAdapter = NULL;
    }

    if (pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter = pAdapter;
        m_pAdapter->acquire();
    }
}

//==============================================================================
DBG_NAME(FmXDisposeMultiplexer);
//------------------------------------------------------------------------------
FmXDisposeMultiplexer::FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const Reference< ::com::sun::star::lang::XComponent>& _rxObject, sal_Int16 _nId)
    :m_xObject(_rxObject)
    ,m_pListener(_pListener)
    ,m_nId(_nId)
{
    DBG_CTOR(FmXDisposeMultiplexer, NULL);
    m_pListener->setAdapter(this);

    if (m_xObject.is())
        m_xObject->addEventListener(this);
}

//------------------------------------------------------------------------------
FmXDisposeMultiplexer::~FmXDisposeMultiplexer()
{
    DBG_DTOR(FmXDisposeMultiplexer, NULL);
}

// ::com::sun::star::lang::XEventListener
//------------------------------------------------------------------
void FmXDisposeMultiplexer::disposing(const ::com::sun::star::lang::EventObject& _Source) throw( RuntimeException )
{
    Reference< ::com::sun::star::lang::XEventListener> xPreventDelete(this);

    if (m_pListener)
    {
        m_pListener->disposing(_Source, m_nId);
        m_pListener->setAdapter(NULL);
        m_pListener = NULL;
    }
    m_xObject = NULL;
}

//------------------------------------------------------------------
void FmXDisposeMultiplexer::dispose()
{
    if (m_xObject.is())
    {
        Reference< ::com::sun::star::lang::XEventListener> xPreventDelete(this);

        m_xObject->removeEventListener(this);
        m_xObject = NULL;

        m_pListener->setAdapter(NULL);
        m_pListener = NULL;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
sal_Int16 getControlTypeByObject(const Reference< ::com::sun::star::lang::XServiceInfo>& _rxObject)
{
    // ask for the persistent service name
    Reference< ::com::sun::star::io::XPersistObject> xPersistence(_rxObject, UNO_QUERY);
    DBG_ASSERT(xPersistence.is(), "::getControlTypeByObject : argument shold be an ::com::sun::star::io::XPersistObject !");
    if (!xPersistence.is())
        return OBJ_FM_CONTROL;

    OUString sPersistentServiceName = xPersistence->getServiceName();
    if (sPersistentServiceName.equals(FM_COMPONENT_EDIT))   // 5.0-Name
    {
        // may be a simple edit field or a formatted field, dependent of the supported services
        if (_rxObject->supportsService(FM_SUN_COMPONENT_FORMATTEDFIELD))
            return OBJ_FM_FORMATTEDFIELD;
        return OBJ_FM_EDIT;
    }
    if (sPersistentServiceName.equals(FM_COMPONENT_TEXTFIELD))
        return OBJ_FM_EDIT;
    if (sPersistentServiceName.equals(FM_COMPONENT_COMMANDBUTTON))
        return OBJ_FM_BUTTON;
    if (sPersistentServiceName.equals(FM_COMPONENT_FIXEDTEXT))
        return OBJ_FM_FIXEDTEXT;
    if (sPersistentServiceName.equals(FM_COMPONENT_LISTBOX))
        return OBJ_FM_LISTBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_CHECKBOX))
        return OBJ_FM_CHECKBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_RADIOBUTTON))
        return OBJ_FM_RADIOBUTTON;
    if (sPersistentServiceName.equals(FM_COMPONENT_GROUPBOX))
        return OBJ_FM_GROUPBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_COMBOBOX))
        return OBJ_FM_COMBOBOX;
    if (sPersistentServiceName.equals(FM_COMPONENT_GRID))   // 5.0-Name
        return OBJ_FM_GRID;
    if (sPersistentServiceName.equals(FM_COMPONENT_GRIDCONTROL))
        return OBJ_FM_GRID;
    if (sPersistentServiceName.equals(FM_COMPONENT_IMAGEBUTTON))
        return OBJ_FM_IMAGEBUTTON;
    if (sPersistentServiceName.equals(FM_COMPONENT_FILECONTROL))
        return OBJ_FM_FILECONTROL;
    if (sPersistentServiceName.equals(FM_COMPONENT_DATEFIELD))
        return OBJ_FM_DATEFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_TIMEFIELD))
        return OBJ_FM_TIMEFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_NUMERICFIELD))
        return OBJ_FM_NUMERICFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_CURRENCYFIELD))
        return OBJ_FM_CURRENCYFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_PATTERNFIELD))
        return OBJ_FM_PATTERNFIELD;
    if (sPersistentServiceName.equals(FM_COMPONENT_HIDDEN)) // 5.0-Name
        return OBJ_FM_HIDDEN;
    if (sPersistentServiceName.equals(FM_COMPONENT_HIDDENCONTROL))
        return OBJ_FM_HIDDEN;
    if (sPersistentServiceName.equals(FM_COMPONENT_IMAGECONTROL))
        return OBJ_FM_IMAGECONTROL;
    if (sPersistentServiceName.equals(FM_COMPONENT_FORMATTEDFIELD))
    {
        OSL_FAIL("::getControlTypeByObject : suspicious persistent service name (formatted field) !");
            // objects with that service name should exist as they aren't compatible with older versions
        return OBJ_FM_FORMATTEDFIELD;
    }
    if ( sPersistentServiceName.equals( FM_SUN_COMPONENT_SCROLLBAR ) )
        return OBJ_FM_SCROLLBAR;
    if ( sPersistentServiceName.equals( FM_SUN_COMPONENT_SPINBUTTON) )
        return OBJ_FM_SPINBUTTON;
    if (sPersistentServiceName.equals(FM_SUN_COMPONENT_NAVIGATIONBAR))
        return OBJ_FM_NAVIGATIONBAR;

    OSL_FAIL("::getControlTypeByObject : unknown object type !");
    return OBJ_FM_CONTROL;
}

//------------------------------------------------------------------------------
sal_Bool isRowSetAlive(const Reference< XInterface >& _rxRowSet)
{
    sal_Bool bIsAlive = sal_False;
    Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xSupplyCols(_rxRowSet, UNO_QUERY);
    Reference< ::com::sun::star::container::XIndexAccess> xCols;
    if (xSupplyCols.is())
        xCols = Reference< ::com::sun::star::container::XIndexAccess>(xSupplyCols->getColumns(), UNO_QUERY);
    if (xCols.is() && (xCols->getCount() > 0))
        bIsAlive = sal_True;

    return bIsAlive;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
