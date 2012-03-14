/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "DumpHelper.hxx"

#include <iostream>
#include <rtl/oustringostreaminserter.hxx>

using namespace com::sun::star;

namespace chart
{
/*
void DumpHelper::writeStripe(const Stripe& rStripe)
{
    std::cout << "Stripe" << std::endl;
}*/

void DumpHelper::writeElement(const char* pName)
{
    std::cout << pName << std::endl;
}

void DumpHelper::writeAttribute(const char* pAttrName, const rtl::OUString& rName)
{
    std::cout << pAttrName << " " << rName << std::endl;
}

void DumpHelper::writeAttribute(const char* pAttrName, const char* pAttrValue)
{
    std::cout << pAttrName << " " << pAttrValue << std::endl;
}

void DumpHelper::writeAttribute(const char* pAttrName, const drawing::Position3D& rPos)
{
    std::cout << pAttrName << " " << rPos.PositionX << "," << rPos.PositionY << "," << rPos.PositionZ << std::endl;
}

void DumpHelper::writeAttribute(const char* pAttrName, const drawing::Direction3D& rDirection)
{
    std::cout << pAttrName << " " << rDirection.DirectionX << "," << rDirection.DirectionY << "," << rDirection.DirectionZ << std::endl;
}

void DumpHelper::writeAttribute(const char* pAttrName, const sal_Int32 nValue)
{
    std::cout << pAttrName << " " << nValue << std::endl;
}

void DumpHelper::writePointElement(const awt::Point& rPoint)
{
    std::cout << "Point" << " " << rPoint.X << "," << rPoint.Y << std::endl;
}

void DumpHelper::writeDoubleSequence(const char* pName, const drawing::DoubleSequenceSequence& rSequence)
{
    writeElement(pName);
    writeElement("OuterSequence");
    sal_Int32 nLength1 = rSequence.getLength();
    for (sal_Int32 i = 0; i < nLength1; ++i)
    {
        writeElement("InnerSequence");
        const uno::Sequence<double>& aInnerSequence = rSequence[i];
        sal_Int32 nLength2 = aInnerSequence.getLength();
        for( sal_Int32 j = 0; j < nLength2; ++j)
        {
            std::cout << "Value: " << aInnerSequence[j];
        }
        endElement();
    }
    endElement();
}

void DumpHelper::writeAttribute(const char* pAttrName, const drawing::PointSequenceSequence& rSequence)
{
    std::cout << pAttrName << " " << std::endl;
    sal_Int32 nLength1 = rSequence.getLength();
    writeElement("OuterSequence");
    for (sal_Int32 i = 0; i < nLength1; ++i)
    {
        writeElement("InnerSequence");
        const uno::Sequence<awt::Point>& aInnerSequence = rSequence[i];
        sal_Int32 nLength2 = aInnerSequence.getLength();
        for( sal_Int32 j = 0; j < nLength2; ++j)
        {
            writePointElement(aInnerSequence[j]);
        }
        endElement();
    }
    endElement();
}

void DumpHelper::writeAttribute(const char* pAttrName, const drawing::PolyPolygonShape3D& rShape)
{
    std::cout << pAttrName << " " << std::endl;
    writeDoubleSequence("SequenceX", rShape.SequenceX);
    writeDoubleSequence("SequenceY", rShape.SequenceY);
    writeDoubleSequence("SequenceZ", rShape.SequenceZ);
}

void DumpHelper::writeAttribute(const char* pAttrName, const drawing::PolyPolygonBezierCoords& )
{
    std::cout << pAttrName << " " << std::endl;
}

void DumpHelper::endElement()
{
    std::cout << "EndElement" << std::endl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
