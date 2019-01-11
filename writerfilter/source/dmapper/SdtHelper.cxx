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
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <sal/log.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <editeng/unoprnms.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <unotools/datetime.hxx>
#include <comphelper/sequence.hxx>

#include "DomainMapper_Impl.hxx"
#include "StyleSheetTable.hxx"

namespace
{
/// Maps OOXML <w:dateFormat> values to UNO date format values.
sal_Int16 getUNODateFormat(const OUString& rDateFormat)
{
    // See com/sun/star/awt/UnoControlDateFieldModel.idl, DateFormat; sadly
    // there are no constants.
    sal_Int16 nDateFormat = -1;

    if (rDateFormat == "M/d/yyyy" || rDateFormat == "M.d.yyyy")
        // MMDDYYYY
        nDateFormat = 8;
    else if (rDateFormat == "dd/MM/yyyy")
        // DDMMYYYY
        nDateFormat = 7;

    return nDateFormat;
}
}

namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

/// w:sdt's w:dropDownList doesn't have width, so guess the size based on the longest string.
static awt::Size lcl_getOptimalWidth(const StyleSheetTablePtr& pStyleSheet, OUString const& rDefault, std::vector<OUString>& rItems)
{
    OUString aLongest = rDefault;
    sal_Int32 nHeight = 0;
    for (const OUString& rItem : rItems)
        if (rItem.getLength() > aLongest.getLength())
            aLongest = rItem;

    MapMode aMap(MapUnit::Map100thMM);
    OutputDevice* pOut = Application::GetDefaultDevice();
    pOut->Push(PushFlags::FONT | PushFlags::MAPMODE);

    PropertyMapPtr pDefaultCharProps = pStyleSheet->GetDefaultCharProps();
    vcl::Font aFont(pOut->GetFont());
    boost::optional<PropertyMap::Property> aFontName = pDefaultCharProps->getProperty(PROP_CHAR_FONT_NAME);
    if (aFontName)
        aFont.SetFamilyName(aFontName->second.get<OUString>());
    boost::optional<PropertyMap::Property> aHeight = pDefaultCharProps->getProperty(PROP_CHAR_HEIGHT);
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
    return {nWidth + nBorder + nHeight, nHeight + nBorder};
}

SdtHelper::SdtHelper(DomainMapper_Impl& rDM_Impl)
    : m_rDM_Impl(rDM_Impl)
    , m_bHasElements(false)
    , m_bOutsideAParagraph(false)
{
}

SdtHelper::~SdtHelper() = default;

void SdtHelper::createDropDownControl()
{
    OUString aDefaultText = m_aSdtTexts.makeStringAndClear();
    uno::Reference<awt::XControlModel> xControlModel(m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.form.component.ComboBox"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("DefaultText", uno::makeAny(aDefaultText));
    xPropertySet->setPropertyValue("Dropdown", uno::makeAny(true));
    xPropertySet->setPropertyValue("StringItemList", uno::makeAny(comphelper::containerToSequence(m_aDropDownItems)));

    createControlShape(lcl_getOptimalWidth(m_rDM_Impl.GetStyleSheetTable(), aDefaultText, m_aDropDownItems),
                       xControlModel, uno::Sequence<beans::PropertyValue>());
    m_aDropDownItems.clear();
}

bool SdtHelper::validateDateFormat()
{
    bool bRet = !m_sDate.isEmpty() || getUNODateFormat(m_sDateFormat.toString()) != -1;
    if (!bRet)
        m_sDateFormat.setLength(0);
    return bRet;
}

void SdtHelper::createDateControl(OUString const& rContentText, const beans::PropertyValue& rCharFormat)
{
    uno::Reference<awt::XControlModel> xControlModel;
    try
    {
        xControlModel.set(m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.form.component.DateField"), uno::UNO_QUERY_THROW);
    }
    catch (css::uno::RuntimeException&)
    {
        throw;
    }
    catch (css::uno::Exception& e)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw css::lang::WrappedTargetRuntimeException("wrapped " + a.getValueTypeName() + ": " + e.Message, css::uno::Reference<css::uno::XInterface>(), a);
    }
    uno::Reference<beans::XPropertySet> xPropertySet(
        xControlModel, uno::UNO_QUERY_THROW);

    xPropertySet->setPropertyValue("Dropdown", uno::makeAny(true));

    // See com/sun/star/awt/UnoControlDateFieldModel.idl, DateFormat; sadly there are no constants
    OUString sDateFormat = m_sDateFormat.makeStringAndClear();
    sal_Int16 nDateFormat = getUNODateFormat(sDateFormat);
    if (nDateFormat == -1)
    {
        // Set default format, so at least the date picker is created.
        SAL_WARN("writerfilter", "unhandled w:dateFormat value");
        if (m_sDate.isEmpty())
            return;
        nDateFormat = 0;
    }
    xPropertySet->setPropertyValue("DateFormat", uno::makeAny(nDateFormat));

    util::Date aDate;
    util::DateTime aDateTime;
    if (utl::ISO8601parseDateTime(m_sDate.makeStringAndClear(), aDateTime))
    {
        utl::extractDate(aDateTime, aDate);
        xPropertySet->setPropertyValue("Date", uno::makeAny(aDate));
    }
    else
        xPropertySet->setPropertyValue("HelpText", uno::makeAny(rContentText.trim()));

    // append date format to grab bag
    comphelper::SequenceAsHashMap aGrabBag;
    aGrabBag["OriginalDate"] <<= aDate;
    aGrabBag["OriginalContent"] <<= rContentText;
    aGrabBag["DateFormat"] <<= sDateFormat;
    aGrabBag["Locale"] <<= m_sLocale.makeStringAndClear();
    aGrabBag["CharFormat"] = rCharFormat.Value;
    // merge in properties like ooxml:CT_SdtPr_alias and friends.
    aGrabBag.update(comphelper::SequenceAsHashMap(comphelper::containerToSequence(m_aGrabBag)));
    // and empty the property list, so they won't end up on the next sdt as well
    m_aGrabBag.clear();

    std::vector<OUString> aItems;
    createControlShape(lcl_getOptimalWidth(m_rDM_Impl.GetStyleSheetTable(), rContentText, aItems), xControlModel, aGrabBag.getAsConstPropertyValueList());
}

void SdtHelper::createControlShape(awt::Size aSize, uno::Reference<awt::XControlModel> const& xControlModel, const uno::Sequence<beans::PropertyValue>& rGrabBag)
{
    uno::Reference<drawing::XControlShape> xControlShape(m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.drawing.ControlShape"), uno::UNO_QUERY);
    xControlShape->setSize(aSize);
    xControlShape->setControl(xControlModel);

    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("VertOrient", uno::makeAny(text::VertOrientation::CENTER));

    if (rGrabBag.hasElements())
        xPropertySet->setPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG, uno::makeAny(rGrabBag));

    uno::Reference<text::XTextContent> xTextContent(xControlShape, uno::UNO_QUERY);
    m_rDM_Impl.appendTextContent(xTextContent, uno::Sequence< beans::PropertyValue >());
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

bool SdtHelper::isInteropGrabBagEmpty()
{
    return m_aGrabBag.empty();
}

sal_Int32 SdtHelper::getInteropGrabBagSize()
{
    return m_aGrabBag.size();
}

bool SdtHelper::containedInInteropGrabBag(const OUString& rValueName)
{
    for (beans::PropertyValue& i : m_aGrabBag)
        if (i.Name == rValueName)
            return true;

    return false;
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
