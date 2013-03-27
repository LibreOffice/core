/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
                {
                    Value.nVal = nValue;
                }

                sal_uInt16 GetOrient() const
                {
                    return Value.Flags.nOrient;
                }

                sal_uInt16 GetRelation() const
                {
                    return Value.Flags.nRelOrient;
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
                    return Value.nVal;
                }

            private:
                union
                {
                    struct
                    {
                        sal_uInt16 nOrient : 4;
                        sal_uInt16 nRelOrient : 4;
                    } Flags;
                    sal_uInt16 nVal;
                } Value;
        };

        /// Stores the horizontal orientation properties of an RTF fly frame.
        class RTFHoriOrient
        {
            public:
                RTFHoriOrient(sal_uInt16 nValue)
                {
                    Value.nVal = nValue;
                }

                sal_uInt16 GetOrient() const
                {
                    return Value.Flags.nOrient;
                }

                sal_uInt16 GetRelation() const
                {
                    return Value.Flags.nRelOrient;
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
                    return Value.nVal;
                }

            private:
                union
                {
                    struct
                    {
                        sal_uInt16 nOrient : 4;
                        sal_uInt16 nRelAnchor : 4;
                        sal_uInt16 nRelOrient : 4;
                    } Flags;
                    sal_uInt16 nVal;
                } Value;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFFLY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
