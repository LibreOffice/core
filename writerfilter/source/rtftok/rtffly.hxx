/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFFLY_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFFLY_HXX

namespace writerfilter
{
namespace rtftok
{
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
        switch (GetOrient())
        {
        case css::text::VertOrientation::CENTER:
            nAlign = NS_ooxml::LN_Value_doc_ST_YAlign_center;
            break;
        case css::text::VertOrientation::TOP:
            nAlign = NS_ooxml::LN_Value_doc_ST_YAlign_top;
            break;
        case css::text::VertOrientation::BOTTOM:
            nAlign = NS_ooxml::LN_Value_doc_ST_YAlign_bottom;
            break;
        }

        return nAlign;
    }

    sal_Int32 GetAnchor() const
    {
        sal_Int32 nAnchor = 0;
        switch (GetRelation())
        {
        case css::text::RelOrientation::FRAME:
            nAnchor = NS_ooxml::LN_Value_doc_ST_VAnchor_text;
            break;
        case css::text::RelOrientation::PAGE_FRAME:
            nAnchor = NS_ooxml::LN_Value_doc_ST_VAnchor_page;
            break;
        case css::text::RelOrientation::PAGE_PRINT_AREA:
            nAnchor = NS_ooxml::LN_Value_doc_ST_VAnchor_margin;
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
        switch (GetOrient())
        {
        case css::text::HoriOrientation::CENTER:
            nAlign = NS_ooxml::LN_Value_doc_ST_XAlign_center;
            break;
        case css::text::HoriOrientation::RIGHT:
            nAlign = NS_ooxml::LN_Value_doc_ST_XAlign_right;
            break;
        case css::text::HoriOrientation::LEFT:
            nAlign = NS_ooxml::LN_Value_doc_ST_XAlign_left;
            break;
        case css::text::HoriOrientation::INSIDE:
            nAlign = NS_ooxml::LN_Value_doc_ST_XAlign_inside;
            break;
        case css::text::HoriOrientation::OUTSIDE:
            nAlign = NS_ooxml::LN_Value_doc_ST_XAlign_outside;
            break;
        }

        return nAlign;
    }

    sal_Int32 GetAnchor() const
    {
        sal_Int32 nAnchor = 0;
        switch (GetRelation())
        {
        case css::text::RelOrientation::FRAME:
            nAnchor = NS_ooxml::LN_Value_doc_ST_HAnchor_text;
            break;
        case css::text::RelOrientation::PAGE_FRAME:
            nAnchor = NS_ooxml::LN_Value_doc_ST_HAnchor_page;
            break;
        case css::text::RelOrientation::PAGE_PRINT_AREA:
            nAnchor = NS_ooxml::LN_Value_doc_ST_HAnchor_margin;
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
}
}

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFFLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
