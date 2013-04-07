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
#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_LICENSE_DIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_LICENSE_DIALOG_HXX

#include "dp_gui.h"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"

#include "boost/bind.hpp"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_gui {

class LicenseDialog
    : public ::cppu::WeakImplHelper1<ui::dialogs::XExecutableDialog>
{
    Reference<XComponentContext> const m_xComponentContext;
    Reference<awt::XWindow> /* const */ m_parent;
    OUString m_sExtensionName;
    OUString /* const */ m_sLicenseText;
    OUString m_initialTitle;

    sal_Int16 solar_execute();

public:
    LicenseDialog( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XExecutableDialog
    virtual void SAL_CALL setTitle( OUString const & title )
        throw (RuntimeException);
    virtual sal_Int16 SAL_CALL execute() throw (RuntimeException);
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
