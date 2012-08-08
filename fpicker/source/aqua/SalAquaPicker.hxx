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

#ifndef _SALAQUAFPICKER_HXX_
#define _SALAQUAFPICKER_HXX_

#include <osl/mutex.hxx>

#include <rtl/ustring.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/uno/RuntimeException.hpp>
#include "ControlHelper.hxx"

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalAquaPicker
{
public:
    // constructor
    SalAquaPicker();
    virtual ~SalAquaPicker();

    int run();
    int runandwaitforresult();

    inline rtl::OUString getDisplayDirectory() { return m_sDisplayDirectory; }

    inline ControlHelper* getControlHelper() const {
        return m_pControlHelper;
    }

protected:

    rtl::OUString m_sDisplayDirectory;

    NSSavePanel *m_pDialog;

    ControlHelper *m_pControlHelper;

    osl::Mutex m_rbHelperMtx;

    // The type of dialog
    enum NavigationServices_DialogType {
        NAVIGATIONSERVICES_OPEN,
        NAVIGATIONSERVICES_SAVE,
        NAVIGATIONSERVICES_DIRECTORY
    };

    NavigationServices_DialogType m_nDialogType;

    void implsetTitle( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    void implsetDisplayDirectory( const rtl::OUString& rDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    rtl::OUString implgetDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    void implInitialize( );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
