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

#ifndef INCLUDED_VCL_OSX_HTMLFMTFLT_HXX
#define INCLUDED_VCL_OSX_HTMLFMTFLT_HXX

#include <com/sun/star/uno/Sequence.hxx>

/* Transform plain HTML into the format expected by MS Office.
 */
css::uno::Sequence<sal_Int8> TextHtmlToHTMLFormat(css::uno::Sequence<sal_Int8>& aTextHtml);

/* Transform the MS Office HTML format into plain HTML.
 */
css::uno::Sequence<sal_Int8> HTMLFormatToTextHtml(const css::uno::Sequence<sal_Int8>& aHTMLFormat);

/* Detects whether the given byte sequence contains the MS Office Html format.

   @returns True if the MS Office Html format will be detected False otherwise.
 */
bool isHTMLFormat (const css::uno::Sequence<sal_Int8>& aHtmlSequence);

#endif // INCLUDED_VCL_OSX_HTMLFMTFLT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
