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


#include <fmprop.hxx>
#include <fmservs.hxx>
#include <svx/fmtools.hxx>
#include <svx/svdobjkind.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdb/ErrorMessageDialog.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/Language.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::svxform;

namespace
{
    bool lcl_shouldDisplayError( const Any& _rError )
    {
        SQLException aError;
        if ( !( _rError >>= aError ) )
            return true;

        if ( ! aError.Message.startsWith( "[OOoBase]" ) )
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

void displayException(const Any& _rExcept, const css::uno::Reference<css::awt::XWindow>& rParent)
{
    // check whether we need to display it
    if ( !lcl_shouldDisplayError( _rExcept ) )
        return;

    try
    {
        Reference< XExecutableDialog > xErrorDialog = ErrorMessageDialog::create(::comphelper::getProcessComponentContext(), u""_ustr, rParent, _rExcept);
        xErrorDialog->execute();
    }
    catch(const Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx.form", "could not display the error message!");
    }
}

void displayException(const css::sdbc::SQLException& _rExcept, const css::uno::Reference<css::awt::XWindow>& rParent)
{
    displayException(Any(_rExcept), rParent);
}

void displayException(const css::sdb::SQLContext& _rExcept, const css::uno::Reference<css::awt::XWindow>& rParent)
{
    displayException(Any(_rExcept), rParent);
}

void displayException(const css::sdb::SQLErrorEvent& _rEvent, const css::uno::Reference<css::awt::XWindow>& rParent)
{
    displayException(_rEvent.Reason, rParent);
}

sal_Int32 getElementPos(const Reference< css::container::XIndexAccess>& xCont, const Reference< XInterface >& xElement)
{
    sal_Int32 nIndex = -1;
    if (!xCont.is())
        return nIndex;


    Reference< XInterface > xNormalized( xElement, UNO_QUERY );
    DBG_ASSERT( xNormalized.is(), "getElementPos: invalid element!" );
    if ( xNormalized.is() )
    {
        // find child position
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
                TOOLS_WARN_EXCEPTION( "svx", "getElementPos" );
            }

        }
    }
    return nIndex;
}


OUString getLabelName(const Reference< css::beans::XPropertySet>& xControlModel)
{
    if (!xControlModel.is())
        return OUString();

    if (::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xControlModel))
    {
        Reference< css::beans::XPropertySet> xLabelSet;
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


// = CursorWrapper

CursorWrapper::CursorWrapper(const Reference< css::sdbc::XRowSet>& _rxCursor, bool bUseCloned)
{
    ImplConstruct(Reference< css::sdbc::XResultSet>(_rxCursor), bUseCloned);
}


CursorWrapper::CursorWrapper(const Reference< css::sdbc::XResultSet>& _rxCursor, bool bUseCloned)
{
    ImplConstruct(_rxCursor, bUseCloned);
}


void CursorWrapper::ImplConstruct(const Reference< css::sdbc::XResultSet>& _rxCursor, bool bUseCloned)
{
    if (bUseCloned)
    {
        Reference< css::sdb::XResultSetAccess> xAccess(_rxCursor, UNO_QUERY);
        try
        {
            m_xMoveOperations = xAccess.is() ? xAccess->createResultSet() : Reference< css::sdbc::XResultSet>();
        }
        catch(Exception&)
        {
        }
    }
    else
        m_xMoveOperations   = _rxCursor;

    m_xBookmarkOperations.set(m_xMoveOperations, css::uno::UNO_QUERY);
    m_xColumnsSupplier.set(m_xMoveOperations, css::uno::UNO_QUERY);
    m_xPropertyAccess.set(m_xMoveOperations, css::uno::UNO_QUERY);

    if ( !m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is() || !m_xPropertyAccess.is() )
    {   // all or nothing !!
        m_xMoveOperations = nullptr;
        m_xBookmarkOperations = nullptr;
        m_xColumnsSupplier = nullptr;
    }
    else
        m_xGeneric = m_xMoveOperations.get();
}

CursorWrapper& CursorWrapper::operator=(const Reference< css::sdbc::XRowSet>& _rxCursor)
{
    m_xMoveOperations.set(_rxCursor);
    m_xBookmarkOperations.set(_rxCursor, UNO_QUERY);
    m_xColumnsSupplier.set(_rxCursor, UNO_QUERY);
    if (!m_xMoveOperations.is() || !m_xBookmarkOperations.is() || !m_xColumnsSupplier.is())
    {   // all or nothing !!
        m_xMoveOperations = nullptr;
        m_xBookmarkOperations = nullptr;
        m_xColumnsSupplier = nullptr;
    }
    return *this;
}

FmXDisposeListener::~FmXDisposeListener()
{
    setAdapter(nullptr);
}

void FmXDisposeListener::setAdapter(FmXDisposeMultiplexer* pAdapter)
{
    std::scoped_lock aGuard(m_aMutex);
    m_pAdapter = pAdapter;
}

FmXDisposeMultiplexer::FmXDisposeMultiplexer(FmXDisposeListener* _pListener, const Reference< css::lang::XComponent>& _rxObject)
    :m_xObject(_rxObject)
    ,m_pListener(_pListener)
{
    m_pListener->setAdapter(this);

    if (m_xObject.is())
        m_xObject->addEventListener(this);
}

FmXDisposeMultiplexer::~FmXDisposeMultiplexer()
{
}

// css::lang::XEventListener

void FmXDisposeMultiplexer::disposing(const css::lang::EventObject& /*Source*/)
{
    Reference< css::lang::XEventListener> xPreventDelete(this);

    if (m_pListener)
    {
        m_pListener->disposing(0);
        m_pListener->setAdapter(nullptr);
        m_pListener = nullptr;
    }
    m_xObject = nullptr;
}


void FmXDisposeMultiplexer::dispose()
{
    if (m_xObject.is())
    {
        Reference< css::lang::XEventListener> xPreventDelete(this);

        m_xObject->removeEventListener(this);
        m_xObject = nullptr;

        m_pListener->setAdapter(nullptr);
        m_pListener = nullptr;
    }
}


SdrObjKind getControlTypeByObject(const Reference< css::lang::XServiceInfo>& _rxObject)
{
    // ask for the persistent service name
    Reference< css::io::XPersistObject> xPersistence(_rxObject, UNO_QUERY);
    DBG_ASSERT(xPersistence.is(), "::getControlTypeByObject : argument should be a css::io::XPersistObject !");
    if (!xPersistence.is())
        return SdrObjKind::FormControl;

    OUString sPersistentServiceName = xPersistence->getServiceName();
    if (sPersistentServiceName == FM_COMPONENT_EDIT)   // 5.0-Name
    {
        // may be a simple edit field or a formatted field, dependent of the supported services
        if (_rxObject->supportsService(FM_SUN_COMPONENT_FORMATTEDFIELD))
            return SdrObjKind::FormFormattedField;
        return SdrObjKind::FormEdit;
    }
    if (sPersistentServiceName == FM_COMPONENT_TEXTFIELD)
        return SdrObjKind::FormEdit;
    if (sPersistentServiceName == FM_COMPONENT_COMMANDBUTTON)
        return SdrObjKind::FormButton;
    if (sPersistentServiceName == FM_COMPONENT_FIXEDTEXT)
        return SdrObjKind::FormFixedText;
    if (sPersistentServiceName == FM_COMPONENT_LISTBOX)
        return SdrObjKind::FormListbox;
    if (sPersistentServiceName == FM_COMPONENT_CHECKBOX)
        return SdrObjKind::FormCheckbox;
    if (sPersistentServiceName == FM_COMPONENT_RADIOBUTTON)
        return SdrObjKind::FormRadioButton;
    if (sPersistentServiceName == FM_COMPONENT_GROUPBOX)
        return SdrObjKind::FormGroupBox;
    if (sPersistentServiceName == FM_COMPONENT_COMBOBOX)
        return SdrObjKind::FormCombobox;
    if (sPersistentServiceName == FM_COMPONENT_GRID)   // 5.0-Name
        return SdrObjKind::FormGrid;
    if (sPersistentServiceName == FM_COMPONENT_GRIDCONTROL)
        return SdrObjKind::FormGrid;
    if (sPersistentServiceName == FM_COMPONENT_IMAGEBUTTON)
        return SdrObjKind::FormImageButton;
    if (sPersistentServiceName == FM_COMPONENT_FILECONTROL)
        return SdrObjKind::FormFileControl;
    if (sPersistentServiceName == FM_COMPONENT_DATEFIELD)
        return SdrObjKind::FormDateField;
    if (sPersistentServiceName == FM_COMPONENT_TIMEFIELD)
        return SdrObjKind::FormTimeField;
    if (sPersistentServiceName == FM_COMPONENT_NUMERICFIELD)
        return SdrObjKind::FormNumericField;
    if (sPersistentServiceName == FM_COMPONENT_CURRENCYFIELD)
        return SdrObjKind::FormCurrencyField;
    if (sPersistentServiceName == FM_COMPONENT_PATTERNFIELD)
        return SdrObjKind::FormPatternField;
    if (sPersistentServiceName == FM_COMPONENT_HIDDEN) // 5.0-Name
        return SdrObjKind::FormHidden;
    if (sPersistentServiceName == FM_COMPONENT_HIDDENCONTROL)
        return SdrObjKind::FormHidden;
    if (sPersistentServiceName == FM_COMPONENT_IMAGECONTROL)
        return SdrObjKind::FormImageControl;
    if (sPersistentServiceName == FM_COMPONENT_FORMATTEDFIELD)
    {
        OSL_FAIL("::getControlTypeByObject : suspicious persistent service name (formatted field) !");
            // objects with that service name should exist as they aren't compatible with older versions
        return SdrObjKind::FormFormattedField;
    }
    if ( sPersistentServiceName == FM_SUN_COMPONENT_SCROLLBAR )
        return SdrObjKind::FormScrollbar;
    if ( sPersistentServiceName == FM_SUN_COMPONENT_SPINBUTTON )
        return SdrObjKind::FormSpinButton;
    if ( sPersistentServiceName == FM_SUN_COMPONENT_NAVIGATIONBAR )
        return SdrObjKind::FormNavigationBar;

    OSL_FAIL("::getControlTypeByObject : unknown object type !");
    return SdrObjKind::FormControl;
}


bool isRowSetAlive(const Reference< XInterface >& _rxRowSet)
{
    bool bIsAlive = false;
    Reference< css::sdbcx::XColumnsSupplier> xSupplyCols(_rxRowSet, UNO_QUERY);
    Reference< css::container::XIndexAccess> xCols;
    if (xSupplyCols.is())
        xCols.set(xSupplyCols->getColumns(), UNO_QUERY);
    if (xCols.is() && (xCols->getCount() > 0))
        bIsAlive = true;

    return bIsAlive;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
