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

#ifndef INCLUDED_SVX_SVXNUMOPTIONSTABPAGEHELPER_HXX
#define INCLUDED_SVX_SVXNUMOPTIONSTABPAGEHELPER_HXX

#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>
#include <svx/svxdllapi.h>

using namespace css::uno;
using namespace css::text;

class SVX_DLLPUBLIC SvxNumOptionsTabPageHelper
{
public:
    static Reference<XDefaultNumberingProvider> GetNumberingProvider();

    /** Get the numberings provided by the i18n framework (CTL, Asian, ...) and
        add them to the listbox. Extended numbering schemes present in the
        resource and already in the listbox but not offered by the i18n
        framework per configuration are removed.

        @param nDoNotRemove
            A value that shall not be removed, i.e. the ugly 0x88
            (SVX_NUM_BITMAP|0x80)
            Pass ::std::numeric_limits<sal_uInt16>::max() if there is no such
            restriction.
    */
    static void GetI18nNumbering(ListBox& rFmtLB, sal_uInt16 nDoNotRemove);
    static void GetI18nNumbering(weld::ComboBox& rFmtLB, sal_uInt16 nDoNotRemove);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
