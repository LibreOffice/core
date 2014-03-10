/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SDTHELPER_HXX
#define INCLUDED_SDTHELPER_HXX

#include <boost/optional.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ustrbuf.hxx>

#include <WriterFilterDllApi.hxx>

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
class SdtHelper
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

    bool m_bHasElements;

    /// Create and append the drawing::XControlShape, containing the various models.
    void createControlShape(com::sun::star::awt::Size aSize, com::sun::star::uno::Reference<com::sun::star::awt::XControlModel>);
    void createControlShape(com::sun::star::awt::Size aSize, com::sun::star::uno::Reference<com::sun::star::awt::XControlModel>,
                            com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> rGrabBag);
public:
    SdtHelper(DomainMapper_Impl& rDM_Impl);
    virtual ~SdtHelper();

    std::vector<OUString>& getDropDownItems();
    OUStringBuffer& getSdtTexts();
    OUStringBuffer& getDate();
    OUStringBuffer& getDateFormat();
    /// If createControlShape() was ever called.
    bool hasElements();

    /// Create drop-down control from w:sdt's w:dropDownList.
    void createDropDownControl();
    /// Create date control from w:sdt's w:date.
    void createDateControl(OUString& rContentText);
};

} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
