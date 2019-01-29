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

#ifndef INCLUDED_SVTOOLS_BINDABLECONTROLHELPER_HXX
#define INCLUDED_SVTOOLS_BINDABLECONTROLHELPER_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/uno/Reference.hxx>

namespace com :: sun :: star :: frame { class XModel; }
namespace com :: sun :: star :: uno { class XInterface; }

namespace svt
{


    //= GraphicAccess

    /** helper class for obtaining streams (which also can be used with the ImageProducer)
        from a resource
    */
    namespace BindableControlHelper
    {
        SVT_DLLPUBLIC void ApplyListSourceAndBindableData( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::uno::XInterface >& rObj, const OUString& rsCtrlSource, const OUString& rsRowSource, sal_uInt16 nRefTab = 0 );
    }


} // namespace svt


#endif // INCLUDED_SVTOOLS_BINDABLECONTROLHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
