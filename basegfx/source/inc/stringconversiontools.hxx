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

#ifndef _STRINGCONVERSIONTOOLS_HXX
#define _STRINGCONVERSIONTOOLS_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace basegfx
{
    namespace internal
    {
        void lcl_skipSpaces(sal_Int32&              io_rPos,
                            const ::rtl::OUString&  rStr,
                            const sal_Int32         nLen);

        void lcl_skipSpacesAndCommas(sal_Int32&             io_rPos,
                                        const ::rtl::OUString& rStr,
                                        const sal_Int32         nLen);

        inline bool lcl_isOnNumberChar(const sal_Unicode aChar, bool bSignAllowed = true)
        {
            const bool bPredicate( (sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                                    || (bSignAllowed && sal_Unicode('+') == aChar)
                                    || (bSignAllowed && sal_Unicode('-') == aChar) );

            return bPredicate;
        }

        inline bool lcl_isOnNumberChar(const ::rtl::OUString& rStr, const sal_Int32 nPos, bool bSignAllowed = true)
        {
            return lcl_isOnNumberChar(rStr[nPos],
                                        bSignAllowed);
        }

        bool lcl_getDoubleChar(double&                  o_fRetval,
                                sal_Int32&              io_rPos,
                                const ::rtl::OUString&  rStr);

        bool lcl_importDoubleAndSpaces( double&                 o_fRetval,
                                        sal_Int32&              io_rPos,
                                        const ::rtl::OUString&  rStr,
                                        const sal_Int32         nLen );

        bool lcl_importNumberAndSpaces(sal_Int32&                o_nRetval,
                                        sal_Int32&              io_rPos,
                                        const ::rtl::OUString&  rStr,
                                        const sal_Int32         nLen);

        void lcl_skipNumber(sal_Int32&              io_rPos,
                            const ::rtl::OUString&  rStr,
                            const sal_Int32         nLen);

        void lcl_skipDouble(sal_Int32&              io_rPos,
                            const ::rtl::OUString&  rStr);

        inline void lcl_skipNumberAndSpacesAndCommas(sal_Int32&                 io_rPos,
                                                const ::rtl::OUString&  rStr,
                                                const sal_Int32             nLen)
        {
            lcl_skipNumber(io_rPos, rStr, nLen);
            lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);
        }

        // #100617# Allow to skip doubles, too.
        inline void lcl_skipDoubleAndSpacesAndCommas(sal_Int32&                 io_rPos,
                                                const ::rtl::OUString&  rStr,
                                                const sal_Int32             nLen)
        {
            lcl_skipDouble(io_rPos, rStr);
            lcl_skipSpacesAndCommas(io_rPos, rStr, nLen);
        }

        inline void lcl_putNumberChar( ::rtl::OUStringBuffer& rStr,
                                double                 fValue )
        {
            rStr.append( fValue );
        }

        void lcl_putNumberCharWithSpace( ::rtl::OUStringBuffer& rStr,
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

#endif /* _STRINGCONVERSIONTOOLS_HXX */
