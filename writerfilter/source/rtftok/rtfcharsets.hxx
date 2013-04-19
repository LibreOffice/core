/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFCHARSETS_HXX_
#define _RTFCHARSETS_HXX_

namespace writerfilter {
    namespace rtftok {
        /// RTF legacy charsets
        typedef struct
        {
            int charset;
            int codepage;
        } RTFEncoding;
        extern RTFEncoding aRTFEncodings[];
        extern int nRTFEncodings;
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFCHARSETS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
