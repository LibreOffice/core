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

#ifndef INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_EMFPSTRINGFORMAT_HXX
#define INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_EMFPSTRINGFORMAT_HXX

namespace cppcanvas
{
    namespace internal
    {
        struct EMFPStringFormat : public EMFPObject
        {
            sal_uInt32 header;
            sal_uInt32 stringFormatFlags;
            sal_uInt32 language;
            sal_uInt32 stringAlignment;
            sal_uInt32 lineAlign;
            sal_uInt32 digitSubstitution;
            sal_uInt32 digitLanguage;
            float firstTabOffset;
            sal_Int32 hotkeyPrefix;
            float leadingMargin;
            float trailingMargin;
            float tracking;
            sal_Int32 trimming;
            sal_Int32 tabStopCount;
            sal_Int32 rangeCount;

            EMFPStringFormat();
            void Read(SvMemoryStream &s);
        };
    }
}


#endif // INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_EMFPSTRINGFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
