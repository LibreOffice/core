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
#include "LoggedResources.hxx"
#include <rtl/ustring.hxx>
#include <vector>
namespace writerfilter::dmapper
{
class FFDataHandler : public LoggedProperties
{
public:
    // typedefs
    typedef ::tools::SvRef<FFDataHandler> Pointer_t;
    typedef ::std::vector<OUString> DropDownEntries_t;

    // constructor
    FFDataHandler();
    // destructor
    virtual ~FFDataHandler() override;

    // member: name
    const OUString& getName() const { return m_sName; }

    // member: helpText
    const OUString& getHelpText() const { return m_sHelpText; }

    // member: statusText
    const OUString& getStatusText() const { return m_sStatusText; }

    const OUString& getEntryMacro() const { return m_sEntryMacro; }
    const OUString& getExitMacro() const { return m_sExitMacro; }

    // member: checkboxHeight
    sal_uInt32 getCheckboxHeight() const { return m_nCheckboxHeight; }

    // member: checkboxAutoHeight
    bool getCheckboxAutoHeight() const { return m_bCheckboxAutoHeight; }

    // member: checkboxChecked or checkboxDefault (if the previous is not set)
    bool getCheckboxChecked() const;

    // member: dropDownResult
    const OUString& getDropDownResult() const { return m_sDropDownResult; }

    // member: dropDownEntries
    const DropDownEntries_t& getDropDownEntries() const { return m_DropDownEntries; }

    // member: textDefault
    const OUString& getTextDefault() const { return m_sTextDefault; }

    const OUString& getTextType() const { return m_sTextType; }
    const OUString& getTextFormat() const { return m_sTextFormat; }
    sal_uInt16 getTextMaxLength() const { return m_nTextMaxLength; }

    // sprm
    void resolveSprm(Sprm& r_sprm);

private:
    OUString m_sName;
    OUString m_sHelpText;
    OUString m_sStatusText;
    OUString m_sEntryMacro;
    OUString m_sExitMacro;
    sal_uInt32 m_nCheckboxHeight;
    bool m_bCheckboxAutoHeight;
    int m_nCheckboxChecked;
    int m_nCheckboxDefault;
    OUString m_sDropDownResult;
    DropDownEntries_t m_DropDownEntries;
    OUString m_sTextDefault;
    OUString m_sTextType;
    OUString m_sTextFormat;
    sal_uInt16 m_nTextMaxLength;

    // sprm
    void lcl_sprm(Sprm& r_sprm) override;

    // attribute
    void lcl_attribute(Id name, Value& val) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
