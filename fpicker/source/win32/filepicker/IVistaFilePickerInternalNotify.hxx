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

#ifndef FPICKER_WIN32_VISTA_FILEPICKER_INTERNALNOTIFY_HXX
#define FPICKER_WIN32_VISTA_FILEPICKER_INTERNALNOTIFY_HXX

#include "comptr.hxx"
#include "vistatypes.h"

#include <cppuhelper/basemutex.hxx>
#include <osl/interlck.h>

#include <shobjidl.h>

#ifdef css
    #error "Clash on using CSS as namespace define."
#else
    #define css ::com::sun::star
#endif

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
// types, const etcpp.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/** todo document me
 */
class IVistaFilePickerInternalNotify
{
    public:

        virtual void onAutoExtensionChanged (bool bChecked) = 0;

        virtual bool onFileTypeChanged( UINT nTypeIndex ) = 0;
};

}}}

#undef css

#endif // FPICKER_WIN32_VISTA_FILEPICKER_INTERNALNOTIFY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
