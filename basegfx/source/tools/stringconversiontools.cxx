/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"

#include <stringconversiontools.hxx>
#include <rtl/math.hxx>

namespace basegfx
{
    namespace internal
    {
        void lcl_skipSpaces(sal_Int32&              io_rPos,
                            const ::rtl::OUString&  rStr,
                            const sal_Int32         nLen)
        {
            while( io_rPos < nLen &&
                    sal_Unicode(' ') == rStr[io_rPos] )
            {
                ++io_rPos;
            }
        }

        void lcl_skipSpacesAndCommas(sal_Int32&             io_rPos,
                                        const ::rtl::OUString& rStr,
                                        const sal_Int32         nLen)
        {
            while(io_rPos < nLen
                    && (sal_Unicode(' ') == rStr[io_rPos] || sal_Unicode(',') == rStr[io_rPos]))
            {
                ++io_rPos;
            }
        }

        bool lcl_getDoubleChar(double&                  o_fRetval,
                                sal_Int32&              io_rPos,
                                const ::rtl::OUString&  rStr)
        {
            sal_Unicode aChar( rStr[io_rPos] );
            ::rtl::OUStringBuffer sNumberString;

            if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                aChar = rStr[++io_rPos];
            }

            while((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                    || sal_Unicode('.') == aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                aChar = rStr[++io_rPos];
            }

            if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                aChar = rStr[++io_rPos];

                if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    aChar = rStr[++io_rPos];
                }

                while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                {
                    sNumberString.append(rStr[io_rPos]);
                    aChar = rStr[++io_rPos];
                }
            }

            if(sNumberString.getLength())
            {
                rtl_math_ConversionStatus eStatus;
                o_fRetval = ::rtl::math::stringToDouble( sNumberString.makeStringAndClear(),
                                                            (sal_Unicode)('.'),
                                                            (sal_Unicode)(','),
                                                            &eStatus,
                                                            NULL );
                return ( eStatus == rtl_math_ConversionStatus_Ok );
            }

            return false;
        }

        bool lcl_importDoubleAndSpaces( double&                 o_fRetval,
                                        sal_Int32&              io_rPos,
                                        const ::rtl::OUString&  rStr,
                                        const sal_Int32         nLen )
        {
            if( !lcl_getDoubleChar(o_fRetval, io_rPos, rStr) )
                return false;

            lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);

            return true;
        }

        bool lcl_importNumberAndSpaces(sal_Int32&                o_nRetval,
                                        sal_Int32&              io_rPos,
                                        const ::rtl::OUString&  rStr,
                                        const sal_Int32         nLen)
        {
            sal_Unicode aChar( rStr[io_rPos] );
            ::rtl::OUStringBuffer sNumberString;

            if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                aChar = rStr[++io_rPos];
            }

            while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
            {
                sNumberString.append(rStr[io_rPos]);
                aChar = rStr[++io_rPos];
            }

            if(sNumberString.getLength())
            {
                o_nRetval = sNumberString.makeStringAndClear().toInt32();
                lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);

                return true;
            }

            return false;
        }

        void lcl_skipNumber(sal_Int32&              io_rPos,
                            const ::rtl::OUString&  rStr,
                            const sal_Int32         nLen)
        {
            bool bSignAllowed(true);

            while(io_rPos < nLen && lcl_isOnNumberChar(rStr, io_rPos, bSignAllowed))
            {
                bSignAllowed = false;
                ++io_rPos;
            }
        }

        void lcl_skipDouble(sal_Int32&              io_rPos,
                            const ::rtl::OUString&  rStr)
        {
            sal_Unicode aChar( rStr[io_rPos] );

            if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                aChar = rStr[++io_rPos];

            while((sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                    || sal_Unicode('.') == aChar)
            {
                aChar = rStr[++io_rPos];
            }

            if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
            {
                aChar = rStr[++io_rPos];

                if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
                    aChar = rStr[++io_rPos];

                while(sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                {
                    aChar = rStr[++io_rPos];
                }
            }
        }

        void lcl_putNumberCharWithSpace( ::rtl::OUStringBuffer& rStr,
                                            double              fValue,
                                            double              fOldValue,
                                            bool                    bUseRelativeCoordinates )
        {
            if( bUseRelativeCoordinates )
                fValue -= fOldValue;

            const sal_Int32 aLen( rStr.getLength() );
            if(aLen)
            {
                if( lcl_isOnNumberChar(rStr.charAt(aLen - 1), false) &&
                    fValue >= 0.0 )
                {
                    rStr.append( sal_Unicode(' ') );
                }
            }

            lcl_putNumberChar(rStr, fValue);
        }
    } // namespace internal
}

// eof
