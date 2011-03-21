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

#ifndef SVTOOLS_PICKER_CALLBACKS_HXX
#define SVTOOLS_PICKER_CALLBACKS_HXX

#include <tools/string.hxx>
#include <sal/types.h>

class Control;

//.........................................................................
namespace svt
{
//.........................................................................

    // --------------------------------------------------------------------
    class IFilePickerController
    {
    public:
        virtual Control*    getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl = sal_False ) const = 0;
        virtual void        enableControl( sal_Int16 _nControlId, sal_Bool _bEnable ) = 0;
        virtual String      getCurFilter( ) const = 0;
    };

    // --------------------------------------------------------------------
    class IFilePickerListener
    {
    public:
        virtual void    notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) = 0;
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_PICKER_CALLBACKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
