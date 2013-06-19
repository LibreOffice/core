/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFFLY_HXX_
#define _RTFFLY_HXX_

namespace writerfilter {
    namespace rtftok {
        /// Stores the vertical orientation properties of an RTF fly frame.
        class RTFVertOrient
        {
            public:
                RTFVertOrient(sal_uInt16 nValue)
                    : nVal(nValue)
                {
                }

                sal_uInt16 GetOrient() const
                {
                    return OSL_LONIBBLE(OSL_LOBYTE(nVal));
                }

                sal_uInt16 GetRelation() const
                {
                    return OSL_HINIBBLE(OSL_LOBYTE(nVal));
                }

                sal_Int32 GetAlign() const
                {
                    sal_Int32 nAlign = 0;
                    switch( GetOrient( ) )
                    {
                        case text::VertOrientation::CENTER:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_center;
                            break;
                        case text::VertOrientation::TOP:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_top;
                            break;
                        case text::VertOrientation::BOTTOM:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_bottom;
                            break;
                    }

                    return nAlign;
                }

                sal_Int32 GetAnchor() const
                {
                    sal_Int32 nAnchor = 0;
                    switch( GetRelation( ) )
                    {
                        case text::RelOrientation::FRAME:
                            nAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_text;
                            break;
                        case text::RelOrientation::PAGE_FRAME:
                            nAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_page;
                            break;
                        case text::RelOrientation::PAGE_PRINT_AREA:
                            nAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_margin;
                            break;
                    }

                    return nAnchor;
                }

                sal_uInt16 GetValue() const
                {
                    return nVal;
                }

            private:
                sal_uInt16 nVal;
        };

        /// Stores the horizontal orientation properties of an RTF fly frame.
        class RTFHoriOrient
        {
            public:
                RTFHoriOrient(sal_uInt16 nValue)
                    : nVal(nValue)
                {
                }

                sal_uInt16 GetOrient() const
                {
                    return OSL_LONIBBLE(OSL_LOBYTE(nVal));
                }

                sal_uInt16 GetRelation() const
                {
                    return OSL_LONIBBLE(OSL_HIBYTE(nVal));
                }

                sal_uInt16 GetRelAnchor() const
                {
                    return OSL_HINIBBLE(OSL_LOBYTE(nVal));
                }

                sal_Int32 GetAlign() const
                {
                    sal_Int32 nAlign = 0;
                    switch( GetOrient( ) )
                    {
                        case text::HoriOrientation::CENTER:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_center;
                            break;
                        case text::HoriOrientation::RIGHT:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_right;
                            break;
                        case text::HoriOrientation::LEFT:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_left;
                            break;
                        case text::HoriOrientation::INSIDE:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_inside;
                            break;
                        case text::HoriOrientation::OUTSIDE:
                            nAlign = NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_outside;
                            break;
                    }

                    return nAlign;
                }

                sal_Int32 GetAnchor() const
                {
                    sal_Int32 nAnchor = 0;
                    switch( GetRelation( ) )
                    {
                        case text::RelOrientation::FRAME:
                            nAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_text;
                            break;
                        case text::RelOrientation::PAGE_FRAME:
                            nAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_page;
                            break;
                        case text::RelOrientation::PAGE_PRINT_AREA:
                            nAnchor = NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_margin;
                            break;
                    }

                    return nAnchor;
                }

                sal_uInt16 GetValue() const
                {
                    return nVal;
                }

            private:
                sal_uInt16 nVal;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFFLY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
