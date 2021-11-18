/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SdtHelper.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <editeng/unoprnms.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/sequence.hxx>
#include <xmloff/odffields.hxx>
#include <com/sun/star/text/XTextField.hpp>
#include "DomainMapper_Impl.hxx"
#include "StyleSheetTable.hxx"
#include <officecfg/Office/Writer.hxx>

namespace writerfilter::dmapper
{
using namespace ::com::sun::star;

/// w:sdt's w:dropDownList doesn't have width, so guess the size based on the longest string.
static awt::Size lcl_getOptimalWidth(const StyleSheetTablePtr& pStyleSheet,
                                     OUString const& rDefault, std::vector<OUString>& rItems)
{
    OUString aLongest = rDefault;
    sal_Int32 nHeight = 0;
    for (const OUString& rItem : rItems)
        if (rItem.getLength() > aLongest.getLength())
            aLongest = rItem;

    MapMode aMap(MapUnit::Map100thMM);
    OutputDevice* pOut = Application::GetDefaultDevice();
    pOut->Push(vcl::PushFlags::FONT | vcl::PushFlags::MAPMODE);

    PropertyMapPtr pDefaultCharProps = pStyleSheet->GetDefaultCharProps();
    vcl::Font aFont(pOut->GetFont());
    std::optional<PropertyMap::Property> aFontName
        = pDefaultCharProps->getProperty(PROP_CHAR_FONT_NAME);
    if (aFontName)
        aFont.SetFamilyName(aFontName->second.get<OUString>());
    std::optional<PropertyMap::Property> aHeight = pDefaultCharProps->getProperty(PROP_CHAR_HEIGHT);
    if (aHeight)
    {
        nHeight = aHeight->second.get<double>() * 35; // points -> mm100
        aFont.SetFontSize(Size(0, nHeight));
    }
    pOut->SetFont(aFont);
    pOut->SetMapMode(aMap);
    sal_Int32 nWidth = pOut->GetTextWidth(aLongest);

    pOut->Pop();

    // Border: see PDFWriterImpl::drawFieldBorder(), border size is font height / 4,
    // so additional width / height needed is height / 2.
    sal_Int32 nBorder = nHeight / 2;

    // Width: space for the text + the square having the dropdown arrow.
    return { nWidth + nBorder + nHeight, nHeight + nBorder };
}

SdtHelper::SdtHelper(DomainMapper_Impl& rDM_Impl)
    : m_rDM_Impl(rDM_Impl)
    , m_aControlType(SdtControlType::unknown)
    , m_bHasElements(false)
    , m_bOutsideAParagraph(false)
{
}

SdtHelper::~SdtHelper() = default;

void SdtHelper::createDropDownControl()
{
    assert(getControlType() == SdtControlType::dropDown);

    const bool bDropDown
        = officecfg::Office::Writer::Filter::Import::DOCX::ImportComboBoxAsDropDown::get();
    const OUString aDefaultText = m_aSdtTexts.makeStringAndClear();

    if (bDropDown)
    {
        // create field
        uno::Reference<css::text::XTextField> xControlModel(
            m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.text.TextField.DropDown"),
            uno::UNO_QUERY);

        const auto it = std::find_if(
            m_aDropDownItems.begin(), m_aDropDownItems.end(),
            [aDefaultText](const OUString& item) -> bool { return !item.compareTo(aDefaultText); });

        if (m_aDropDownItems.end() == it)
        {
            m_aDropDownItems.push_back(aDefaultText);
        }

        // set properties
        uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);
        xPropertySet->setPropertyValue("SelectedItem", uno::makeAny(aDefaultText));
        xPropertySet->setPropertyValue(
            "Items", uno::makeAny(comphelper::containerToSequence(m_aDropDownItems)));

        // add it into document
        m_rDM_Impl.appendTextContent(xControlModel, uno::Sequence<beans::PropertyValue>());

        m_bHasElements = true;
    }
    else
    {
        // create control
        uno::Reference<awt::XControlModel> xControlModel(
            m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.form.component.ComboBox"),
            uno::UNO_QUERY);

        // set properties
        uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);
        xPropertySet->setPropertyValue("DefaultText", uno::makeAny(aDefaultText));
        xPropertySet->setPropertyValue("Dropdown", uno::makeAny(true));
        xPropertySet->setPropertyValue(
            "StringItemList", uno::makeAny(comphelper::containerToSequence(m_aDropDownItems)));

        // add it into document
        createControlShape(
            lcl_getOptimalWidth(m_rDM_Impl.GetStyleSheetTable(), aDefaultText, m_aDropDownItems),
            xControlModel, uno::Sequence<beans::PropertyValue>());
    }

    // clean up
    m_aDropDownItems.clear();
    setControlType(SdtControlType::unknown);
}

void SdtHelper::createDateContentControl()
{
    if (!m_xDateFieldStartRange.is())
        return;

    uno::Reference<text::XTextCursor> xCrsr;
    if (m_rDM_Impl.HasTopText())
    {
        uno::Reference<text::XTextAppend> xTextAppend = m_rDM_Impl.GetTopTextAppend();
        if (xTextAppend.is())
        {
            xCrsr = xTextAppend->createTextCursorByRange(xTextAppend);
        }
    }
    if (!xCrsr.is())
        return;

    try
    {
        xCrsr->gotoRange(m_xDateFieldStartRange, false);
        bool bIsInTable = (m_rDM_Impl.hasTableManager() && m_rDM_Impl.getTableManager().isInTable())
                          || (m_rDM_Impl.m_nTableDepth > 0);
        if (bIsInTable)
            xCrsr->goRight(1, false);
        xCrsr->gotoEnd(true);
    }
    catch (uno::Exception&)
    {
        OSL_ENSURE(false, "Cannot get the right text range for date field");
        return;
    }

    uno::Reference<uno::XInterface> xFieldInterface
        = m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.text.Fieldmark");
    uno::Reference<text::XFormField> xFormField(xFieldInterface, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xToInsert(xFormField, uno::UNO_QUERY);
    if (!(xFormField.is() && xToInsert.is()))
        return;

    xToInsert->attach(uno::Reference<text::XTextRange>(xCrsr, uno::UNO_QUERY_THROW));
    xFormField->setFieldType(ODF_FORMDATE);
    uno::Reference<container::XNameContainer> xNameCont = xFormField->getParameters();
    if (xNameCont.is())
    {
        OUString sDateFormat = m_sDateFormat.makeStringAndClear();
        // Replace quotation mark used for marking static strings in date format
        sDateFormat = sDateFormat.replaceAll("'", "\"");
        xNameCont->insertByName(ODF_FORMDATE_DATEFORMAT, uno::makeAny(sDateFormat));
        xNameCont->insertByName(ODF_FORMDATE_DATEFORMAT_LANGUAGE,
                                uno::makeAny(m_sLocale.makeStringAndClear()));
    }
    OUString sFullDate = m_sDate.makeStringAndClear();
    if (!sFullDate.isEmpty())
    {
        sal_Int32 nTimeSep = sFullDate.indexOf("T");
        if (nTimeSep != -1)
            sFullDate = sFullDate.copy(0, nTimeSep);
        xNameCont->insertByName(ODF_FORMDATE_CURRENTDATE, uno::makeAny(sFullDate));
    }

    setControlType(SdtControlType::unknown);

    // Store all unused sdt parameters from grabbag
    xNameCont->insertByName("SdtParams", uno::makeAny(getInteropGrabBagAndClear()));
}

void SdtHelper::createControlShape(awt::Size aSize,
                                   uno::Reference<awt::XControlModel> const& xControlModel,
                                   const uno::Sequence<beans::PropertyValue>& rGrabBag)
{
    uno::Reference<drawing::XControlShape> xControlShape(
        m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.drawing.ControlShape"),
        uno::UNO_QUERY);
    xControlShape->setSize(aSize);
    xControlShape->setControl(xControlModel);

    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("VertOrient", uno::makeAny(text::VertOrientation::CENTER));

    if (rGrabBag.hasElements())
        xPropertySet->setPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG, uno::makeAny(rGrabBag));

    uno::Reference<text::XTextContent> xTextContent(xControlShape, uno::UNO_QUERY);
    m_rDM_Impl.appendTextContent(xTextContent, uno::Sequence<beans::PropertyValue>());
    m_bHasElements = true;
}

void SdtHelper::appendToInteropGrabBag(const beans::PropertyValue& rValue)
{
    m_aGrabBag.push_back(rValue);
}

uno::Sequence<beans::PropertyValue> SdtHelper::getInteropGrabBagAndClear()
{
    uno::Sequence<beans::PropertyValue> aRet = comphelper::containerToSequence(m_aGrabBag);
    m_aGrabBag.clear();
    return aRet;
}

bool SdtHelper::isInteropGrabBagEmpty() const { return m_aGrabBag.empty(); }

sal_Int32 SdtHelper::getInteropGrabBagSize() const { return m_aGrabBag.size(); }

bool SdtHelper::containedInInteropGrabBag(const OUString& rValueName)
{
    return std::any_of(
        m_aGrabBag.begin(), m_aGrabBag.end(),
        [&rValueName](const beans::PropertyValue& i) { return i.Name == rValueName; });
}

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
