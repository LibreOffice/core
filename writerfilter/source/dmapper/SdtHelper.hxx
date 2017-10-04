/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_SDTHELPER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_SDTHELPER_HXX

#include <vector>

#include <boost/optional.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ustrbuf.hxx>

#include "DomainMapper_Impl.hxx"

namespace com
{
namespace sun
{
namespace star
{
namespace awt
{
struct Size;
class XControlModel;
}
}
}
}

namespace writerfilter
{
namespace dmapper
{

/**
 * Helper to create form controls from w:sdt tokens.
 *
 * w:sdt tokens can't be imported as form fields, as w:sdt supports
 * e.g. date picking as well.
 */
class SdtHelper final
{
    DomainMapper_Impl& m_rDM_Impl;

    /// Items of the drop-down control.
    std::vector<OUString> m_aDropDownItems;
    /// Pieces of the default text -- currently used only by the dropdown control.
    OUStringBuffer m_aSdtTexts;
    /// Date ISO string contained in the w:date element, used by the date control.
    OUStringBuffer m_sDate;
    /// Date format string as it comes from the ooxml document.
    OUStringBuffer m_sDateFormat;
    /// Locale string as it comes from the ooxml document.
    OUStringBuffer m_sLocale;
    /// Grab bag to store unsupported SDTs, aiming to save them back on export.
    std::vector<css::beans::PropertyValue> m_aGrabBag;

    bool m_bHasElements;
    /// The last stored SDT element is outside paragraphs.
    bool m_bOutsideAParagraph;

    /// Create and append the drawing::XControlShape, containing the various models.
    void createControlShape(css::awt::Size aSize, css::uno::Reference<css::awt::XControlModel> const& xControlModel, const css::uno::Sequence<css::beans::PropertyValue>& rGrabBag);
public:
    explicit SdtHelper(DomainMapper_Impl& rDM_Impl);
    ~SdtHelper();

    std::vector<OUString>& getDropDownItems()
    {
        return m_aDropDownItems;
    }
    OUStringBuffer& getSdtTexts()
    {
        return m_aSdtTexts;
    }
    OUStringBuffer& getDate()
    {
        return m_sDate;
    }
    OUStringBuffer& getDateFormat()
    {
        return m_sDateFormat;
    }
    OUStringBuffer& getLocale()
    {
        return m_sLocale;
    }
    /// If createControlShape() was ever called.
    bool hasElements()
    {
        return m_bHasElements;
    }

    void setOutsideAParagraph(bool bOutsideAParagraph)
    {
        m_bOutsideAParagraph = bOutsideAParagraph;
    }

    bool isOutsideAParagraph()
    {
        return m_bOutsideAParagraph;
    }

    /// Create drop-down control from w:sdt's w:dropDownList.
    void createDropDownControl();
    /// Create date control from w:sdt's w:date.
    void createDateControl(OUString const& rContentText, const css::beans::PropertyValue& rCharFormat);

    void appendToInteropGrabBag(const css::beans::PropertyValue& rValue);
    css::uno::Sequence<css::beans::PropertyValue> getInteropGrabBagAndClear();
    bool isInteropGrabBagEmpty();
    bool containedInInteropGrabBag(const OUString& rValueName);
    sal_Int32 getInteropGrabBagSize();
};

} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
