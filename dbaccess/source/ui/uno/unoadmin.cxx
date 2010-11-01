/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbustrings.hrc"
#include <toolkit/awt/vclxwindow.hxx>
#include "dbu_reghelper.hxx"
#ifndef _DBAUI_UNOADMIN_
#include "unoadmin.hxx"
#endif
#include "dbadmin.hxx"
#include <comphelper/extract.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>

// --- needed because of the solar mutex
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
// ---

#define THISREF()   static_cast< XServiceInfo* >(this)

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

//=========================================================================
DBG_NAME(ODatabaseAdministrationDialog)
//-------------------------------------------------------------------------
ODatabaseAdministrationDialog::ODatabaseAdministrationDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :ODatabaseAdministrationDialogBase(_rxORB)
    ,m_pDatasourceItems(NULL)
    ,m_pItemPool(NULL)
    ,m_pItemPoolDefaults(NULL)
    ,m_pCollection(NULL)
{
    DBG_CTOR(ODatabaseAdministrationDialog,NULL);

    m_pCollection = new ::dbaccess::ODsnTypeCollection(_rxORB);
    ODbAdminDialog::createItemSet(m_pDatasourceItems, m_pItemPool, m_pItemPoolDefaults, m_pCollection);
}

//-------------------------------------------------------------------------
ODatabaseAdministrationDialog::~ODatabaseAdministrationDialog()
{
    // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
    // so this virtual-method-call the base class does does not work, we're already dead then ...
    if (m_pDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pDialog)
            destroyDialog();
    }

    delete m_pCollection;
    m_pCollection = NULL;

    DBG_DTOR(ODatabaseAdministrationDialog,NULL);
}
//-------------------------------------------------------------------------
void ODatabaseAdministrationDialog::destroyDialog()
{
    ODatabaseAdministrationDialogBase::destroyDialog();
    ODbAdminDialog::destroyItemSet(m_pDatasourceItems, m_pItemPool, m_pItemPoolDefaults);
}
//------------------------------------------------------------------------------
void ODatabaseAdministrationDialog::implInitialize(const Any& _rValue)
{
    PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        if (0 == aProperty.Name.compareToAscii("InitialSelection"))
        {
            m_aInitialSelection = aProperty.Value;
        }
        else if (0 == aProperty.Name.compareToAscii("ActiveConnection"))
        {
            m_xActiveConnection.set(aProperty.Value,UNO_QUERY);
        }
        else
            ODatabaseAdministrationDialogBase::implInitialize(_rValue);
    }
    else
        ODatabaseAdministrationDialogBase::implInitialize(_rValue);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
