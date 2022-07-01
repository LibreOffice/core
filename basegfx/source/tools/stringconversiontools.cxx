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

#include <stringconversiontools.hxx>
#include <rtl/math.hxx>

namespace basegfx::internal
{
        void skipSpaces(sal_Int32&      io_rPos,
                        std::u16string_view rStr,
                        const sal_Int32 nLen)
        {
            while( io_rPos < nLen &&
                    rStr[io_rPos] == ' ' )
            {
                ++io_rPos;
            }
        }

        static void skipSpacesAndCommas(sal_Int32&      io_rPos,
                                 std::u16string_view rStr,
                                 const sal_Int32 nLen)
        {
            while(io_rPos < nLen
                    && (rStr[io_rPos] == ' ' || rStr[io_rPos] == ','))
            {
                ++io_rPos;
            }
        }

        static bool getDoubleChar(double&         o_fRetval,
                           sal_Int32&      io_rPos,
                           std::u16string_view rStr)
        {
            const sal_Int64 nStrSize = rStr.size();
            sal_Unicode aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;
            const sal_Int32 nStartPos = io_rPos;

            // sign
            if(aChar == '+' || aChar == '-')
            {
                aChar = rStr[++io_rPos];
            }

            // numbers before point
            while('0' <= aChar && '9' >= aChar)
            {
                io_rPos++;
                aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;
            }

            // point
            if(aChar == '.')
            {
                io_rPos++;
                aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;
            }

            // numbers after point
            while ('0' <= aChar && '9' >= aChar)
            {
                io_rPos++;
                aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;
            }

            // 'e'
            if(aChar == 'e' || aChar == 'E')
            {
                io_rPos++;
                aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;

                // sign for 'e'
                if(aChar == '+' || aChar == '-')
                {
                    io_rPos++;
                    aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;
                }

                // number for 'e'
                while('0' <= aChar && '9' >= aChar)
                {
                    io_rPos++;
                    aChar = io_rPos < nStrSize ? rStr[io_rPos] : 0;
                }
            }

            const sal_Int32 nLen = io_rPos - nStartPos;
            if(nLen)
            {
                rStr = rStr.substr(nStartPos, nLen);
                rtl_math_ConversionStatus eStatus;
                o_fRetval = ::rtl::math::stringToDouble( rStr,
                                                            '.',
                                                            ',',
                                                            &eStatus );
                return ( eStatus == rtl_math_ConversionStatus_Ok );
            }

            return false;
        }

        bool importDoubleAndSpaces(double&         o_fRetval,
                                   sal_Int32&      io_rPos,
                                   std::u16string_view rStr,
                                   const sal_Int32 nLen )
        {
            if( !getDoubleChar(o_fRetval, io_rPos, rStr) )
                return false;

            skipSpacesAndCommas(io_rPos, rStr, nLen);

            return true;
        }

        bool importFlagAndSpaces(sal_Int32&      o_nRetval,
                                 sal_Int32&      io_rPos,
                                 std::u16string_view rStr,
                                 const sal_Int32 nLen)
        {
            sal_Unicode aChar( rStr[io_rPos] );

            if(aChar == '0')
            {
                o_nRetval = 0;
                ++io_rPos;
            }
            else if (aChar == '1')
            {
                o_nRetval = 1;
                ++io_rPos;
            }
            else
                return false;

            skipSpacesAndCommas(io_rPos, rStr, nLen);

            return true;
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
