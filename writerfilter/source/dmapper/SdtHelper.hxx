/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include <optional>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/XDocument.hpp>

#include <rtl/ustrbuf.hxx>
#include <tools/ref.hxx>

namespace com::sun::star
{
namespace uno
{
class XComponentContext;
}
namespace awt
{
struct Size;
class XControlModel;
}
}

namespace writerfilter::dmapper
{
class DomainMapper_Impl;

enum class SdtControlType
{
    datePicker,
    dropDown,
    plainText,
    richText,
    checkBox,
    picture,
    unsupported, // Sdt block is defined, but we still do not support such type of field
    unknown
};

/**
 * Helper to create form controls from w:sdt tokens.
 *
 * w:sdt tokens can't be imported as form fields, as w:sdt supports
 * e.g. date picking as well.
 */
class SdtHelper final : public virtual SvRefBase
{
    DomainMapper_Impl& m_rDM_Impl;
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;

    /// Items of the drop-down control: <w:listItem w:value="...">.
    std::vector<OUString> m_aDropDownItems;
    /// Display texts of a drop-down control: <w:listItem w:displayText="...">.
    std::vector<OUString> m_aDropDownDisplayTexts;
    /// Type of sdt control
    SdtControlType m_aControlType;
    sal_uInt32 m_nSdtType = 0;
    /// Pieces of the default text -- currently used only by the dropdown control.
    OUStringBuffer m_aSdtTexts;
    /// Date ISO string contained in the w:date element, used by the date control.
    OUStringBuffer m_sDate;
    /// Date format string as it comes from the ooxml document.
    OUStringBuffer m_sDateFormat;

    /// <w:dataBinding w:prefixMappings="">
    OUString m_sDataBindingPrefixMapping;
    /// <w:dataBinding w:xpath="">
    OUString m_sDataBindingXPath;
    /// <w:dataBinding w:storeItemID="">
    OUString m_sDataBindingStoreItemID;

    /// Start range of the date field
    css::uno::Reference<css::text::XTextRange> m_xDateFieldStartRange;
    /// Locale string as it comes from the ooxml document.
    OUStringBuffer m_sLocale;
    /// Grab bag to store unsupported SDTs, aiming to save them back on export.
    std::vector<css::beans::PropertyValue> m_aGrabBag;

    bool m_bHasElements;
    /// The last stored SDT element is outside paragraphs.
    bool m_bOutsideAParagraph;

    /// Storage for all properties documents as xml::dom::XDocument for later querying xpath for data
    css::uno::Sequence<css::uno::Reference<css::xml::dom::XDocument>> m_xPropertiesXMLs;

    /// Check if m_xPropertiesXMLs is initialized and loaded (need extra flag to distinguish
    /// empty sequence from not yet initialized)
    bool m_bPropertiesXMLsLoaded;

    /// Current contents are placeholder text.
    bool m_bShowingPlcHdr = false;

    /// If this is a checkbox, is the checkbox checked?
    bool m_bChecked = false;

    /// If this is a checkbox, the value of a checked checkbox.
    OUString m_aCheckedState;

    /// If this is a checkbox, the value of an unchecked checkbox.
    OUString m_aUncheckedState;

    /// Create and append the drawing::XControlShape, containing the various models.
    void createControlShape(css::awt::Size aSize,
                            css::uno::Reference<css::awt::XControlModel> const& xControlModel,
                            const css::uno::Sequence<css::beans::PropertyValue>& rGrabBag);

    std::optional<OUString> getValueFromDataBinding();

    void loadPropertiesXMLs();

    /// <w:placeholder>'s <w:docPart w:val="...">.
    OUString m_aPlaceholderDocPart;

    /// <w:sdtPr>'s <w15:color w:val="...">.
    OUString m_aColor;

public:
    explicit SdtHelper(DomainMapper_Impl& rDM_Impl,
                       css::uno::Reference<css::uno::XComponentContext> const& xContext);
    ~SdtHelper() override;

    std::vector<OUString>& getDropDownItems() { return m_aDropDownItems; }
    std::vector<OUString>& getDropDownDisplayTexts() { return m_aDropDownDisplayTexts; }
    OUStringBuffer& getSdtTexts() { return m_aSdtTexts; }

    OUStringBuffer& getDate() { return m_sDate; }

    OUStringBuffer& getDateFormat() { return m_sDateFormat; }

    void setDataBindingPrefixMapping(const OUString& sValue)
    {
        m_sDataBindingPrefixMapping = sValue;
    }
    OUString GetDataBindingPrefixMapping() const { return m_sDataBindingPrefixMapping; }

    void setDataBindingXPath(const OUString& sValue) { m_sDataBindingXPath = sValue; }
    OUString GetDataBindingXPath() const { return m_sDataBindingXPath; }

    void setDataBindingStoreItemID(const OUString& sValue) { m_sDataBindingStoreItemID = sValue; }
    OUString GetDataBindingStoreItemID() const { return m_sDataBindingStoreItemID; }

    void setDateFieldStartRange(const css::uno::Reference<css::text::XTextRange>& xStartRange)
    {
        m_xDateFieldStartRange = xStartRange;
    }

    OUStringBuffer& getLocale() { return m_sLocale; }
    /// If createControlShape() was ever called.
    bool hasElements() const { return m_bHasElements; }

    void setOutsideAParagraph(bool bOutsideAParagraph)
    {
        m_bOutsideAParagraph = bOutsideAParagraph;
    }

    bool isOutsideAParagraph() const { return m_bOutsideAParagraph; }

    SdtControlType getControlType() { return m_aControlType; }
    void setControlType(SdtControlType aType) { m_aControlType = aType; }

    void SetSdtType(sal_uInt32 nSdtType) { m_nSdtType = nSdtType; }
    sal_uInt32 GetSdtType() const { return m_nSdtType; }

    /// Create drop-down control from w:sdt's w:dropDownList.
    void createDropDownControl();
    /// Create date control from w:sdt's w:date.
    void createDateContentControl();

    void createPlainTextControl();

    void appendToInteropGrabBag(const css::beans::PropertyValue& rValue);
    css::uno::Sequence<css::beans::PropertyValue> getInteropGrabBagAndClear();
    bool isInteropGrabBagEmpty() const;
    bool containedInInteropGrabBag(const OUString& rValueName);
    sal_Int32 getInteropGrabBagSize() const;

    void SetShowingPlcHdr();
    bool GetShowingPlcHdr() const;

    void SetChecked();
    bool GetChecked() const;
    void SetCheckedState(const OUString& rCheckedState);
    OUString GetCheckedState() const;
    void SetUncheckedState(const OUString& rUncheckedState);
    OUString GetUncheckedState() const;

    /// Clear all collected attributes for further reuse
    void clear();

    void SetPlaceholderDocPart(const OUString& rPlaceholderDocPart);
    OUString GetPlaceholderDocPart() const;

    void SetColor(const OUString& rColor);
    OUString GetColor() const;
};

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
