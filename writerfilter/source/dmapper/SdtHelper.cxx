/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <unotools/datetime.hxx>

#include <DomainMapper_Impl.hxx>
#include <StyleSheetTable.hxx>
#include <SdtHelper.hxx>

namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

/// w:sdt's w:dropDownList doesn't have width, so guess the size based on the longest string.
awt::Size lcl_getOptimalWidth(StyleSheetTablePtr pStyleSheet, OUString& rDefault, std::vector<OUString>& rItems)
{
    OUString aLongest = rDefault;
    sal_Int32 nHeight = 0;
    for (size_t i = 0; i < rItems.size(); ++i)
        if (rItems[i].getLength() > aLongest.getLength())
            aLongest = rItems[i];

    MapMode aMap(MAP_100TH_MM);
    OutputDevice* pOut = Application::GetDefaultDevice();
    pOut->Push(PUSH_FONT | PUSH_MAPMODE);

    PropertyMapPtr pDefaultCharProps = pStyleSheet->GetDefaultCharProps();
    Font aFont(pOut->GetFont());
    PropertyMap::iterator aFontName = pDefaultCharProps->find(PROP_CHAR_FONT_NAME);
    if (aFontName != pDefaultCharProps->end())
        aFont.SetName(aFontName->second.getValue().get<OUString>());
    PropertyMap::iterator aHeight = pDefaultCharProps->find(PROP_CHAR_HEIGHT);
    if (aHeight != pDefaultCharProps->end())
    {
        nHeight = aHeight->second.getValue().get<double>() * 35; // points -> mm100
        aFont.SetSize(Size(0, nHeight));
    }
    pOut->SetFont(aFont);
    pOut->SetMapMode(aMap);
    sal_Int32 nWidth = pOut->GetTextWidth(aLongest);

    pOut->Pop();

    // Border: see PDFWriterImpl::drawFieldBorder(), border size is font height / 4,
    // so additional width / height needed is height / 2.
    sal_Int32 nBorder = nHeight / 2;

    // Width: space for the text + the square having the dropdown arrow.
    return awt::Size(nWidth + nBorder + nHeight, nHeight + nBorder);
}

SdtHelper::SdtHelper(DomainMapper_Impl& rDM_Impl)
    : m_rDM_Impl(rDM_Impl)
    , m_bHasElements(false)
{
}

SdtHelper::~SdtHelper()
{
}

void SdtHelper::createDropDownControl()
{
    OUString aDefaultText = m_aSdtTexts.makeStringAndClear();
    uno::Reference<awt::XControlModel> xControlModel(m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.form.component.ComboBox"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("DefaultText", uno::makeAny(aDefaultText));
    xPropertySet->setPropertyValue("Dropdown", uno::makeAny(sal_True));
    uno::Sequence<OUString> aItems(m_aDropDownItems.size());
    for (size_t i = 0; i < m_aDropDownItems.size(); ++i)
        aItems[i] = m_aDropDownItems[i];
    xPropertySet->setPropertyValue("StringItemList", uno::makeAny(aItems));

    createControlShape(lcl_getOptimalWidth(m_rDM_Impl.GetStyleSheetTable(), aDefaultText, m_aDropDownItems), xControlModel);
    m_aDropDownItems.clear();
}

void SdtHelper::createDateControl(OUString& rDefaultText)
{
    uno::Reference<awt::XControlModel> xControlModel(m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.form.component.DateField"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("HelpText", uno::makeAny(rDefaultText));
    xPropertySet->setPropertyValue("Dropdown", uno::makeAny(sal_True));
    xPropertySet->setPropertyValue("DateFormat", uno::makeAny(*m_oDateFormat));
    m_oDateFormat.reset();

    util::Date aDate;
    util::DateTime aDateTime;
    if(utl::ISO8601parseDateTime(m_sDate.makeStringAndClear(), aDateTime))
    {
        utl::extractDate(aDateTime, aDate);
        xPropertySet->setPropertyValue("Date", uno::makeAny(aDate));
    }

    std::vector<OUString> aItems;
    createControlShape(lcl_getOptimalWidth(m_rDM_Impl.GetStyleSheetTable(), rDefaultText, aItems), xControlModel);
}

void SdtHelper::createControlShape(awt::Size aSize, uno::Reference<awt::XControlModel> xControlModel)
{
    uno::Reference<drawing::XControlShape> xControlShape(m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.drawing.ControlShape"), uno::UNO_QUERY);
    xControlShape->setSize(aSize);
    xControlShape->setControl(xControlModel);

    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("VertOrient", uno::makeAny(text::VertOrientation::CENTER));

    uno::Reference<text::XTextContent> xTextContent(xControlShape, uno::UNO_QUERY);
    m_rDM_Impl.appendTextContent(xTextContent, uno::Sequence< beans::PropertyValue >());
    m_bHasElements = true;
}

std::vector<OUString>& SdtHelper::getDropDownItems()
{
    return m_aDropDownItems;
}

OUStringBuffer& SdtHelper::getSdtTexts()
{
    return m_aSdtTexts;
}

OUStringBuffer& SdtHelper::getDate()
{
    return m_sDate;
}

boost::optional<sal_Int16>& SdtHelper::getDateFormat()
{
    return m_oDateFormat;
}

bool SdtHelper::hasElements()
{
    return m_bHasElements;
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
