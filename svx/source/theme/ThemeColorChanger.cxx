/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/theme/ThemeColorChanger.hxx>

#include <sal/config.h>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <editeng/unoprnms.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/theme/ColorSet.hxx>

#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XComplexColor.hpp>

using namespace css;

namespace svx
{
namespace theme
{
namespace
{
/// Updates text portion property colors
void updateTextPortionColorSet(model::ColorSet const& rColorSet,
                               const uno::Reference<beans::XPropertySet>& xPortion)
{
    if (!xPortion->getPropertySetInfo()->hasPropertyByName(UNO_NAME_EDIT_CHAR_COMPLEX_COLOR))
    {
        return;
    }

    uno::Reference<util::XComplexColor> xComplexColor;
    xPortion->getPropertyValue(UNO_NAME_EDIT_CHAR_COMPLEX_COLOR) >>= xComplexColor;
    if (!xComplexColor.is())
        return;

    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);

    if (aComplexColor.getSchemeType() == model::ThemeColorType::Unknown)
        return;

    Color aColor = rColorSet.resolveColor(aComplexColor);
    xPortion->setPropertyValue(UNO_NAME_EDIT_CHAR_COLOR, uno::Any(static_cast<sal_Int32>(aColor)));
}

/// Updates the fill property colors
void updateFillColorSet(model::ColorSet const& rColorSet,
                        const uno::Reference<beans::XPropertySet>& xShape)
{
    if (!xShape->getPropertySetInfo()->hasPropertyByName(UNO_NAME_FILL_COMPLEX_COLOR))
        return;

    uno::Reference<util::XComplexColor> xComplexColor;
    xShape->getPropertyValue(UNO_NAME_FILL_COMPLEX_COLOR) >>= xComplexColor;
    if (!xComplexColor.is())
        return;

    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);

    if (aComplexColor.getSchemeType() == model::ThemeColorType::Unknown)
        return;

    Color aColor = rColorSet.resolveColor(aComplexColor);
    xShape->setPropertyValue(UNO_NAME_FILLCOLOR, uno::Any(static_cast<sal_Int32>(aColor)));
}

/// Updates the line property colors
void updateLineColorSet(model::ColorSet const& rColorSet,
                        const uno::Reference<beans::XPropertySet>& xShape)
{
    if (!xShape->getPropertySetInfo()->hasPropertyByName(UNO_NAME_LINE_COMPLEX_COLOR))
        return;

    uno::Reference<util::XComplexColor> xComplexColor;
    xShape->getPropertyValue(UNO_NAME_LINE_COMPLEX_COLOR) >>= xComplexColor;
    if (!xComplexColor.is())
        return;

    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);

    if (aComplexColor.getSchemeType() == model::ThemeColorType::Unknown)
        return;

    Color aColor = rColorSet.resolveColor(aComplexColor);
    xShape->setPropertyValue(UNO_NAME_LINECOLOR, uno::Any(static_cast<sal_Int32>(aColor)));
}

} // end anonymous namespace

/// Updates properties of the SdrObject
void updateSdrObject(model::ColorSet const& rColorSet, SdrObject* pObject)
{
    uno::Reference<drawing::XShape> xShape = pObject->getUnoShape();
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    if (xShapeText.is())
    {
        // E.g. group shapes have no text.
        uno::Reference<container::XEnumerationAccess> xText(xShapeText->getText(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
        while (xParagraphs->hasMoreElements())
        {
            uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                                     uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
            while (xPortions->hasMoreElements())
            {
                uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(),
                                                             uno::UNO_QUERY);
                updateTextPortionColorSet(rColorSet, xPortion);
            }
        }
    }

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    updateFillColorSet(rColorSet, xShapeProps);
    updateLineColorSet(rColorSet, xShapeProps);
}

} // end theme

ThemeColorChanger::ThemeColorChanger(SdrPage* pPage)
    : mpPage(pPage)
{
}

ThemeColorChanger::~ThemeColorChanger() = default;

void ThemeColorChanger::apply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
    for (size_t nObject = 0; nObject < mpPage->GetObjCount(); ++nObject)
    {
        SdrObject* pObject = mpPage->GetObj(nObject);
        theme::updateSdrObject(*pColorSet, pObject);

        // update child objects
        SdrObjList* pList = pObject->GetSubList();
        if (pList)
        {
            SdrObjListIter aIter(pList, SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                theme::updateSdrObject(*pColorSet, aIter.Next());
            }
        }
    }
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
