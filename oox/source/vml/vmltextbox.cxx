/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/vml/vmltextbox.hxx"

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/XTextAppend.hpp>

namespace oox {
namespace vml {

// ============================================================================

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace com::sun::star;

// ============================================================================

TextFontModel::TextFontModel()
{
}

// ============================================================================

TextPortionModel::TextPortionModel( const TextFontModel& rFont, const OUString& rText ) :
    maFont( rFont ),
    maText( rText )
{
}

TextBox::TextBox(ShapeTypeModel& rTypeModel)
    : mrTypeModel(rTypeModel),
    borderDistanceSet( false )
{
}

void TextBox::appendPortion( const TextFontModel& rFont, const OUString& rText )
{
    maPortions.push_back( TextPortionModel( rFont, rText ) );
}

const TextFontModel* TextBox::getFirstFont() const
{
    return maPortions.empty() ? 0 : &maPortions.front().maFont;
}

OUString TextBox::getText() const
{
    OUStringBuffer aBuffer;
    for( PortionVector::const_iterator aIt = maPortions.begin(), aEnd = maPortions.end(); aIt != aEnd; ++aIt )
        aBuffer.append( aIt->maText );
    return aBuffer.makeStringAndClear();
}

void TextBox::convert(uno::Reference<drawing::XShape> xShape) const
{
    uno::Reference<text::XTextAppend> xTextAppend(xShape, uno::UNO_QUERY);
    for (PortionVector::const_iterator aIt = maPortions.begin(), aEnd = maPortions.end(); aIt != aEnd; ++aIt)
    {
        beans::PropertyValue aPropertyValue;
        std::vector<beans::PropertyValue> aPropVec;
        const TextFontModel& rFont = aIt->maFont;
        if (rFont.mobBold.has())
        {
            aPropertyValue.Name = "CharWeight";
            aPropertyValue.Value = uno::makeAny(rFont.mobBold.get() ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL);
            aPropVec.push_back(aPropertyValue);
        }
        if (rFont.monSize.has())
        {
            aPropertyValue.Name = "CharHeight";
            aPropertyValue.Value = uno::makeAny(double(rFont.monSize.get()) / 2.);
            aPropVec.push_back(aPropertyValue);
        }
        uno::Sequence<beans::PropertyValue> aPropSeq(aPropVec.size());
        beans::PropertyValue* pValues = aPropSeq.getArray();
        for (std::vector<beans::PropertyValue>::iterator i = aPropVec.begin(); i != aPropVec.end(); ++i)
            *pValues++ = *i;
        xTextAppend->appendTextPortion(aIt->maText, aPropSeq);
    }
}

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
