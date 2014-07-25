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

namespace basegfx
{
    namespace internal
    {
        void lcl_skipSpaces(sal_Int32&              io_rPos,
                            const OUString&  rStr,
                            const sal_Int32         nLen)
        {
            while( io_rPos < nLen &&
                    ' ' == rStr[io_rPos] )
            {
                ++io_rPos;
            }
        }

        void lcl_skipSpacesAndCommas(sal_Int32&             io_rPos,
                                        const OUString& rStr,
                                        const sal_Int32         nLen)
        {
            while(io_rPos < nLen
                    && (' ' == rStr[io_rPos] || ',' == rStr[io_rPos]))
            {
                ++io_rPos;
            }
        }

        bool lcl_getDoubleChar(double&                  o_fRetval,
                                sal_Int32&              io_rPos,
                                const OUString&  rStr)
        {
            sal_Unicode aChar( rStr[io_rPos] );
            OUStringBuffer sNumberString;
            bool separator_seen=false;

            if('+' == aChar || '-' == aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                aChar = rStr[++io_rPos];
            }

            while(('0' <= aChar && '9' >= aChar)
                     || ('.' == aChar))
            {
                if ('.' == aChar)
                {
                    /* for example: d="M 20 120 H 40.5.6"
                     * in the first H-run this method reads the '40.5' string
                     * in the next H-run we'll read '.6' which we should handle
                     * like it is '0.6' and add it up
                     * FIXME: currently we don't add it up, but just skip it to avoid
                     * warnings in importFromSvgD */
                    if (separator_seen == true)
                    {
                        while (('0' <= aChar && '9' >= aChar) || ('.' == aChar))
                        {
                            io_rPos++;
                            aChar = io_rPos < rStr.getLength() ? rStr[io_rPos] : 0;
                        }
                        break;
                    }
                    separator_seen = true;
                }
                sNumberString.append(rStr[io_rPos]);
                io_rPos++;
                aChar = io_rPos < rStr.getLength() ? rStr[io_rPos] : 0;
            }

            if('e' == aChar || 'E' == aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                io_rPos++;
                aChar = io_rPos < rStr.getLength() ? rStr[io_rPos] : 0;

                if('+' == aChar || '-' == aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    io_rPos++;
                    aChar = io_rPos < rStr.getLength() ? rStr[io_rPos] : 0;
                }

                while('0' <= aChar && '9' >= aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    io_rPos++;
                    aChar = io_rPos < rStr.getLength() ? rStr[io_rPos] : 0;
                }
            }

            if(sNumberString.getLength())
            {
                rtl_math_ConversionStatus eStatus;
                o_fRetval = ::rtl::math::stringToDouble( sNumberString.makeStringAndClear(),
                                                            '.',
                                                            ',',
                                                            &eStatus,
                                                            NULL );
                return ( eStatus == rtl_math_ConversionStatus_Ok );
            }

            return false;
        }

        bool lcl_importDoubleAndSpaces( double&                 o_fRetval,
                                        sal_Int32&              io_rPos,
                                        const OUString&  rStr,
                                        const sal_Int32         nLen )
        {
            if( !lcl_getDoubleChar(o_fRetval, io_rPos, rStr) )
                return false;

            lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);

            return true;
        }

        bool lcl_importFlagAndSpaces(sal_Int32&         o_nRetval,
                                     sal_Int32&         io_rPos,
                                     const OUString&    rStr,
                                     const sal_Int32    nLen)
        {
            sal_Unicode aChar( rStr[io_rPos] );

            if('0' == aChar)
            {
                o_nRetval = 0;
                ++io_rPos;
            }
            else if ('1' == aChar)
            {
                o_nRetval = 1;
                ++io_rPos;
            }
            else
                return false;

            lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);

            return true;
        }

        void lcl_putNumberCharWithSpace( OUStringBuffer& rStr,
                                            double              fValue,
                                            double              fOldValue,
                                            bool                    bUseRelativeCoordinates )
        {
            if( bUseRelativeCoordinates )
                fValue -= fOldValue;

            const sal_Int32 aLen( rStr.getLength() );
            if(aLen)
            {
                if( lcl_isOnNumberChar(rStr[aLen - 1], false) &&
                    fValue >= 0.0 )
                {
                    rStr.append( ' ' );
                }
            }

            lcl_putNumberChar(rStr, fValue);
        }

    } // namespace internal
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
