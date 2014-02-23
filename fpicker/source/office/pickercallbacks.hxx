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

#ifndef SVTOOLS_PICKER_CALLBACKS_HXX
#define SVTOOLS_PICKER_CALLBACKS_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

class Control;


namespace svt
{



    class IFilePickerController
    {
    public:
        virtual Control*    getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl = sal_False ) const = 0;
        virtual void        enableControl( sal_Int16 _nControlId, sal_Bool _bEnable ) = 0;
        virtual OUString    getCurFilter( ) const = 0;

    protected:
        ~IFilePickerController() {}
    };


    class IFilePickerListener
    {
    public:
        virtual void    notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) = 0;

    protected:
        ~IFilePickerListener() {}
    };


}   // namespace svt


#endif // SVTOOLS_PICKER_CALLBACKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
