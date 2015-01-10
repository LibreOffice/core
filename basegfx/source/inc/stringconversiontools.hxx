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
#ifndef INCLUDED_BASEGFX_SOURCE_INC_STRINGCONVERSIONTOOLS_HXX
#define INCLUDED_BASEGFX_SOURCE_INC_STRINGCONVERSIONTOOLS_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace basegfx
{
    namespace internal
    {
        void lcl_skipSpaces(sal_Int32&              io_rPos,
                            const OUString&  rStr,
                            const sal_Int32         nLen);

        void lcl_skipSpacesAndCommas(sal_Int32&             io_rPos,
                                        const OUString& rStr,
                                        const sal_Int32         nLen);

        inline bool lcl_isOnNumberChar(const sal_Unicode aChar, bool bSignAllowed = true, bool bDotAllowed = true)
        {
            const bool bPredicate( (sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                                    || (bSignAllowed && sal_Unicode('+') == aChar)
                                    || (bSignAllowed && sal_Unicode('-') == aChar)
                                    || (bDotAllowed && sal_Unicode('.') == aChar));

            return bPredicate;
        }

        inline bool lcl_isOnNumberChar(const OUString& rStr, const sal_Int32 nPos, bool bSignAllowed = true, bool bDotAllowed = true)
        {
            return lcl_isOnNumberChar(rStr[nPos], bSignAllowed, bDotAllowed);
        }

        bool lcl_getDoubleChar(double&                  o_fRetval,
                                sal_Int32&              io_rPos,
                                const OUString&  rStr);

        bool lcl_importDoubleAndSpaces( double&                 o_fRetval,
                                        sal_Int32&              io_rPos,
                                        const OUString&  rStr,
                                        const sal_Int32         nLen );

        bool lcl_importFlagAndSpaces(sal_Int32&         o_nRetval,
                                     sal_Int32&         io_rPos,
                                     const OUString&    rStr,
                                     const sal_Int32    nLen);

        void lcl_skipNumber(sal_Int32&              io_rPos,
                            const OUString&  rStr,
                            const sal_Int32         nLen);

        void lcl_skipDouble(sal_Int32&              io_rPos,
                            const OUString&  rStr);

        void lcl_putNumberCharWithSpace( OUStringBuffer& rStr,
                                            double              fValue,
                                            double              fOldValue,
                                            bool                    bUseRelativeCoordinates );

        inline sal_Unicode lcl_getCommand( sal_Char cUpperCaseCommand,
                                            sal_Char cLowerCaseCommand,
                                            bool    bUseRelativeCoordinates )
        {
            return bUseRelativeCoordinates ? cLowerCaseCommand : cUpperCaseCommand;
        }
    } // namespace internal
} // namespace basegfx

#endif // INCLUDED_BASEGFX_SOURCE_INC_STRINGCONVERSIONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
