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
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequence.hxx>
#include <xmloff/odffields.hxx>
#include <com/sun/star/text/XTextField.hpp>
#include "DomainMapper_Impl.hxx"
#include "StyleSheetTable.hxx"
#include <officecfg/Office/Writer.hxx>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <ooxml/OOXMLDocument.hxx>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>

namespace writerfilter::dmapper
{
using namespace ::com::sun::star;
using namespace ::css::xml::xpath;
using namespace ::comphelper;

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

SdtHelper::SdtHelper(DomainMapper_Impl& rDM_Impl,
                     css::uno::Reference<css::uno::XComponentContext> const& xContext)
    : m_rDM_Impl(rDM_Impl)
    , m_xComponentContext(xContext)
    , m_aControlType(SdtControlType::unknown)
    , m_bHasElements(false)
    , m_bOutsideAParagraph(false)
    , m_bPropertiesXMLsLoaded(false)
{
}

SdtHelper::~SdtHelper() = default;

void SdtHelper::loadPropertiesXMLs()
{
    // Initialize properties xml storage (m_xPropertiesXMLs)
    uno::Reference<uno::XInterface> xTemp
        = m_xComponentContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.document.OOXMLDocumentPropertiesImporter", m_xComponentContext);
    uno::Reference<document::XOOXMLDocumentPropertiesImporter> xImporter(xTemp, uno::UNO_QUERY);
    if (!xImporter.is())
        return;

    uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(
        xml::dom::DocumentBuilder::create(m_xComponentContext));
    if (!xDomBuilder.is())
        return;

    std::vector<uno::Reference<xml::dom::XDocument>> aPropDocs;

    // Load core properties
    try
    {
        auto xCorePropsStream = xImporter->getCorePropertiesStream(m_rDM_Impl.m_xDocumentStorage);
        aPropDocs.push_back(xDomBuilder->parse(xCorePropsStream));
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("writerfilter",
                 "SdtHelper::loadPropertiesXMLs: failed loading core properties XML");
    }

    // Load extended properties
    try
    {
        auto xExtPropsStream
            = xImporter->getExtendedPropertiesStream(m_rDM_Impl.m_xDocumentStorage);
        aPropDocs.push_back(xDomBuilder->parse(xExtPropsStream));
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("writerfilter",
                 "SdtHelper::loadPropertiesXMLs: failed loading extended properties XML");
    }

    // TODO: some other property items?

    // Add custom XMLs
    uno::Sequence<uno::Reference<xml::dom::XDocument>> aCustomXmls
        = m_rDM_Impl.getDocumentReference()->getCustomXmlDomList();
    for (const auto& xDoc : aCustomXmls)
    {
        aPropDocs.push_back(xDoc);
    }

    m_xPropertiesXMLs = comphelper::containerToSequence(aPropDocs);
    m_bPropertiesXMLsLoaded = true;
}

static void lcl_registerNamespaces(std::u16string_view sNamespaceString,
                                   const uno::Reference<XXPathAPI>& xXPathAPI)
{
    // Split namespaces and register it in XPathAPI
    auto aNamespaces = string::split(sNamespaceString, ' ');
    for (const auto& sNamespace : aNamespaces)
    {
        // Here we have just one namespace in format "xmlns:ns0='http://someurl'"
        auto aNamespace = string::split(sNamespace, '=');
        if (aNamespace.size() < 2)
        {
            SAL_WARN("writerfilter",
                     "SdtHelper::getValueFromDataBinding: invalid namespace: " << sNamespace);
            continue;
        }

        auto aNamespaceId = string::split(aNamespace[0], ':');
        if (aNamespaceId.size() < 2)
        {
            SAL_WARN("writerfilter",
                     "SdtHelper::getValueFromDataBinding: invalid namespace: " << aNamespace[0]);
            continue;
        }

        OUString sNamespaceURL = aNamespace[1];
        sNamespaceURL = string::strip(sNamespaceURL, ' ');
        sNamespaceURL = string::strip(sNamespaceURL, '\'');

        xXPathAPI->registerNS(aNamespaceId[1], sNamespaceURL);
    }
}

std::optional<OUString> SdtHelper::getValueFromDataBinding()
{
    // No xpath - nothing to do
    if (m_sDataBindingXPath.isEmpty())
        return {};

    // Load properties XMLs
    if (!m_bPropertiesXMLsLoaded)
        loadPropertiesXMLs();

    uno::Reference<XXPathAPI> xXpathAPI = XPathAPI::create(m_xComponentContext);

    lcl_registerNamespaces(m_sDataBindingPrefixMapping, xXpathAPI);

    // Iterate all properties xml documents and try to fetch data
    for (const auto& xDocument : m_xPropertiesXMLs)
    {
        uno::Reference<XXPathObject> xResult = xXpathAPI->eval(xDocument, m_sDataBindingXPath);

        if (xResult.is() && xResult->getNodeList() && xResult->getNodeList()->getLength()
            && xResult->getString().getLength())
        {
            return xResult->getString();
        }
    }

    // No data
    return {};
}

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
        xPropertySet->setPropertyValue("SelectedItem", uno::Any(aDefaultText));
        xPropertySet->setPropertyValue("Items",
                                       uno::Any(comphelper::containerToSequence(m_aDropDownItems)));

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
        xPropertySet->setPropertyValue("DefaultText", uno::Any(aDefaultText));
        xPropertySet->setPropertyValue("Dropdown", uno::Any(true));
        xPropertySet->setPropertyValue("StringItemList",
                                       uno::Any(comphelper::containerToSequence(m_aDropDownItems)));

        // add it into document
        createControlShape(
            lcl_getOptimalWidth(m_rDM_Impl.GetStyleSheetTable(), aDefaultText, m_aDropDownItems),
            xControlModel, uno::Sequence<beans::PropertyValue>());
    }

    // clean up
    clear();
}

void SdtHelper::createPlainTextControl()
{
    assert(getControlType() == SdtControlType::plainText);

    OUString aDefaultText = m_aSdtTexts.makeStringAndClear();

    // create field
    uno::Reference<css::text::XTextField> xControlModel(
        m_rDM_Impl.GetTextFactory()->createInstance("com.sun.star.text.TextField.Input"),
        uno::UNO_QUERY);

    // set properties
    uno::Reference<beans::XPropertySet> xPropertySet(xControlModel, uno::UNO_QUERY);

    std::optional<OUString> oData = getValueFromDataBinding();
    if (oData.has_value())
        aDefaultText = *oData;

    xPropertySet->setPropertyValue("Content", uno::Any(aDefaultText));

    // add it into document
    m_rDM_Impl.appendTextContent(xControlModel, uno::Sequence<beans::PropertyValue>());

    // Store all unused sdt parameters from grabbag
    xPropertySet->setPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG,
                                   uno::Any(getInteropGrabBagAndClear()));

    // clean up
    clear();
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
        xNameCont->insertByName(ODF_FORMDATE_DATEFORMAT, uno::Any(sDateFormat));
        xNameCont->insertByName(ODF_FORMDATE_DATEFORMAT_LANGUAGE,
                                uno::Any(m_sLocale.makeStringAndClear()));
    }
    OUString sFullDate = m_sDate.makeStringAndClear();

    std::optional<OUString> oData = getValueFromDataBinding();
    if (oData.has_value())
        sFullDate = *oData;

    if (!sFullDate.isEmpty())
    {
        sal_Int32 nTimeSep = sFullDate.indexOf("T");
        if (nTimeSep != -1)
            sFullDate = sFullDate.copy(0, nTimeSep);
        xNameCont->insertByName(ODF_FORMDATE_CURRENTDATE, uno::Any(sFullDate));
    }

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(m_rDM_Impl.GetTextDocument(),
                                                                  uno::UNO_QUERY);
    uno::Reference<util::XRefreshable> xRefreshable(xTextFieldsSupplier->getTextFields(),
                                                    uno::UNO_QUERY);
    xRefreshable->refresh();

    // Store all unused sdt parameters from grabbag
    xNameCont->insertByName(UNO_NAME_MISC_OBJ_INTEROPGRABBAG,
                            uno::Any(getInteropGrabBagAndClear()));

    clear();
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
    xPropertySet->setPropertyValue("VertOrient", uno::Any(text::VertOrientation::CENTER));

    if (rGrabBag.hasElements())
        xPropertySet->setPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG, uno::Any(rGrabBag));

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

void SdtHelper::SetShowingPlcHdr() { m_bShowingPlcHdr = true; }

bool SdtHelper::GetShowingPlcHdr() const { return m_bShowingPlcHdr; }

void SdtHelper::SetChecked() { m_bChecked = true; }

bool SdtHelper::GetChecked() const { return m_bChecked; }

void SdtHelper::SetCheckedState(const OUString& rCheckedState) { m_aCheckedState = rCheckedState; }

OUString SdtHelper::GetCheckedState() const { return m_aCheckedState; }

void SdtHelper::SetUncheckedState(const OUString& rUncheckedState)
{
    m_aUncheckedState = rUncheckedState;
}

OUString SdtHelper::GetUncheckedState() const { return m_aUncheckedState; }

void SdtHelper::clear()
{
    m_aDropDownItems.clear();
    m_aDropDownDisplayTexts.clear();
    setControlType(SdtControlType::unknown);
    m_sDataBindingPrefixMapping.clear();
    m_sDataBindingXPath.clear();
    m_sDataBindingStoreItemID.clear();
    m_aGrabBag.clear();
    m_bShowingPlcHdr = false;
    m_bChecked = false;
    m_aCheckedState.clear();
    m_aUncheckedState.clear();
}

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
