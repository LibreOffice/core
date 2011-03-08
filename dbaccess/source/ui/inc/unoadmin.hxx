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

#ifndef _DBAUI_UNOADMIN_
#define _DBAUI_UNOADMIN_

#include <svtools/genericunodialog.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include "dsntypes.hxx"
#include "moduledbu.hxx"

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

//.........................................................................
namespace dbaui
{
//.........................................................................

class IDatabaseSettingsDialog;

//=========================================================================
//= ODatabaseAdministrationDialog
//=========================================================================
typedef ::svt::OGenericUnoDialog ODatabaseAdministrationDialogBase;
class ODatabaseAdministrationDialog
        :public ODatabaseAdministrationDialogBase
{
    OModuleClient    m_aModuleClient;
protected:
    SfxItemSet*             m_pDatasourceItems;     // item set for the dialog
    SfxItemPool*            m_pItemPool;            // item pool for the item set for the dialog
    SfxPoolItem**           m_pItemPoolDefaults;    // pool defaults
    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;          // datasource type collection

    ::com::sun::star::uno::Any          m_aInitialSelection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xActiveConnection;

protected:
    ODatabaseAdministrationDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
    virtual ~ODatabaseAdministrationDialog();
protected:
// OGenericUnoDialog overridables
    virtual void destroyDialog();
    virtual void implInitialize(const com::sun::star::uno::Any& _rValue);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_UNOADMIN_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
