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

#ifndef _SALAQUAFPICKER_HXX_
#define _SALAQUAFPICKER_HXX_

//_____________________________________________________________________________
//  includes of other projects
//_____________________________________________________________________________

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
