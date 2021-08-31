/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <config_options.h>

#include <vcl/dllapi.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>
#include <unordered_map>

namespace weld
{
class Window;
}

namespace vcl
{
class PrinterController;
}

namespace vcl::print
{
class PrinterOptions;
class ImplPrinterControllerData;

enum class NupOrderType
{
    LRTB,
    TBLR,
    TBRL,
    RLTB
};

class VCL_DLLPUBLIC PrinterOptionsHelper
{
protected:
    std::unordered_map<OUString, css::uno::Any> m_aPropertyMap;
    std::vector<css::beans::PropertyValue> m_aUIProperties;

public:
    /// Create without ui properties
    PrinterOptionsHelper() {}

    /** Process a new set of properties

        merges changed properties and returns "true" if any occurred
    */
    bool processProperties(const css::uno::Sequence<css::beans::PropertyValue>& i_rNewProp);

    /** Append to a sequence of property values the ui property sequence passed at creation

        as the "ExtraPrintUIOptions" property. if that sequence was empty, no "ExtraPrintUIOptions" property
        will be appended.
    */
    void appendPrintUIOptions(css::uno::Sequence<css::beans::PropertyValue>& io_rProps) const;

    /** @return An empty Any for not existing properties */
    css::uno::Any getValue(const OUString& i_rPropertyName) const;

    bool getBoolValue(const OUString& i_rPropertyName, bool i_bDefault) const;
    // convenience for fixed strings
    bool getBoolValue(const char* i_pPropName, bool i_bDefault = false) const
    {
        return getBoolValue(OUString::createFromAscii(i_pPropName), i_bDefault);
    }

    sal_Int64 getIntValue(const OUString& i_rPropertyName, sal_Int64 i_nDefault) const;
    // convenience for fixed strings
    sal_Int64 getIntValue(const char* i_pPropName, sal_Int64 i_nDefault) const
    {
        return getIntValue(OUString::createFromAscii(i_pPropName), i_nDefault);
    }

    OUString getStringValue(const OUString& i_rPropertyName) const;
    // convenience for fixed strings
    OUString getStringValue(const char* i_pPropName) const
    {
        return getStringValue(OUString::createFromAscii(i_pPropName));
    }

    // helper functions for user to create a single control
    struct UIControlOptions
    {
        OUString maDependsOnName;
        sal_Int32 mnDependsOnEntry;
        bool mbAttachToDependency;
        OUString maGroupHint;
        bool mbInternalOnly;
        bool mbEnabled;
        std::vector<css::beans::PropertyValue> maAddProps;

        UIControlOptions(const OUString& i_rDependsOnName = OUString(),
                         sal_Int32 i_nDependsOnEntry = -1, bool i_bAttachToDependency = false)
            : maDependsOnName(i_rDependsOnName)
            , mnDependsOnEntry(i_nDependsOnEntry)
            , mbAttachToDependency(i_bAttachToDependency)
            , mbInternalOnly(false)
            , mbEnabled(true)
        {
        }
    };

    // note: in the following helper functions HelpIds are expected as an OUString
    // the normal HelpId form is OString (byte string instead of UTF16 string)
    // this is because the whole interface is base on UNO properties; in fact the structures
    // are passed over UNO interfaces. UNO does not know a byte string, hence the string is
    // transported via UTF16 strings.

    /// Show general control
    static css::uno::Any
    setUIControlOpt(const css::uno::Sequence<OUString>& i_rIDs, const OUString& i_rTitle,
                    const css::uno::Sequence<OUString>& i_rHelpId, const OUString& i_rType,
                    const css::beans::PropertyValue* i_pValue = nullptr,
                    const UIControlOptions& i_rControlOptions = UIControlOptions());

    /// Show and set the title of a TagPage of id i_rID
    static css::uno::Any setGroupControlOpt(const OUString& i_rID, const OUString& i_rTitle,
                                            const OUString& i_rHelpId);

    /// Show and set the label of a VclFrame of id i_rID
    static css::uno::Any setSubgroupControlOpt(const OUString& i_rID, const OUString& i_rTitle,
                                               const OUString& i_rHelpId,
                                               const UIControlOptions& i_rControlOptions
                                               = UIControlOptions());

    /// Show a bool option as a checkbox
    static css::uno::Any
    setBoolControlOpt(const OUString& i_rID, const OUString& i_rTitle, const OUString& i_rHelpId,
                      const OUString& i_rProperty, bool i_bValue,
                      const UIControlOptions& i_rControlOptions = UIControlOptions());

    /// Show a set of choices in a list box
    static css::uno::Any setChoiceListControlOpt(
        const OUString& i_rID, const OUString& i_rTitle,
        const css::uno::Sequence<OUString>& i_rHelpId, const OUString& i_rProperty,
        const css::uno::Sequence<OUString>& i_rChoices, sal_Int32 i_nValue,
        const css::uno::Sequence<sal_Bool>& i_rDisabledChoices = css::uno::Sequence<sal_Bool>(),
        const UIControlOptions& i_rControlOptions = UIControlOptions());

    /// Show a set of choices as radio buttons
    static css::uno::Any setChoiceRadiosControlOpt(
        const css::uno::Sequence<OUString>& i_rIDs, const OUString& i_rTitle,
        const css::uno::Sequence<OUString>& i_rHelpId, const OUString& i_rProperty,
        const css::uno::Sequence<OUString>& i_rChoices, sal_Int32 i_nValue,
        const css::uno::Sequence<sal_Bool>& i_rDisabledChoices = css::uno::Sequence<sal_Bool>(),
        const UIControlOptions& i_rControlOptions = UIControlOptions());

    /** Show an integer range (e.g. a spin field)

        note: max value < min value means do not apply min/max values
    */
    static css::uno::Any setRangeControlOpt(const OUString& i_rID, const OUString& i_rTitle,
                                            const OUString& i_rHelpId, const OUString& i_rProperty,
                                            sal_Int32 i_nValue, sal_Int32 i_nMinValue,
                                            sal_Int32 i_nMaxValue,
                                            const UIControlOptions& i_rControlOptions);

    /** Show a string field

        note: max value < min value means do not apply min/max values
    */
    static css::uno::Any setEditControlOpt(const OUString& i_rID, const OUString& i_rTitle,
                                           const OUString& i_rHelpId, const OUString& i_rProperty,
                                           const OUString& i_rValue,
                                           const UIControlOptions& i_rControlOptions);
}; // class PrinterOptionsHelper

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
