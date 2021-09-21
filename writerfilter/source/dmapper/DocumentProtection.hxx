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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <ooxml/resourceids.hxx>

namespace writerfilter::dmapper
{
/** Document protection restrictions
     *
     * This element specifies the set of document protection restrictions which have been applied to the contents of a
     * WordprocessingML document.These restrictions should be enforced by applications editing this document
     * when the enforcement attribute is turned on, and ignored(but persisted) otherwise.Document protection is a
     * set of restrictions used to prevent unintentional changes to all or part of a WordprocessingML document.
     */
class DocumentProtection : public LoggedProperties
{
private:
    /** Document Editing Restrictions
         *
         * Possible values:
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_none
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_readOnly
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_comments
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_trackedChanges
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_forms
         */
    sal_Int32 m_nEdit;
    bool m_bProtectForm;
    bool m_bRedlineProtection;
    OUString m_sRedlineProtectionKey;
    bool m_bReadOnly;
    bool m_bEnforcement;
    bool m_bFormatting;

    /** Provider type
         *
         * Possible values:
         *  "rsaAES"  - NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES
         *  "rsaFull" - NS_ooxml::LN_Value_doc_ST_CryptProv_rsaFull
         */
    sal_Int32 m_nCryptProviderType;
    OUString m_sCryptAlgorithmClass;
    OUString m_sCryptAlgorithmType;
    OUString m_sCryptAlgorithmSid;
    sal_Int32 m_CryptSpinCount;
    OUString m_sHash;
    OUString m_sSalt;

    virtual void lcl_attribute(Id Name, Value& val) override;
    virtual void lcl_sprm(Sprm& sprm) override;

    bool enabled() const { return !isNone(); }
    bool isNone() const { return m_nEdit == NS_ooxml::LN_Value_doc_ST_DocProtect_none; };

public:
    DocumentProtection();
    virtual ~DocumentProtection() override;

    css::uno::Sequence<css::beans::PropertyValue> toSequence() const;

    bool getProtectForm() const { return m_bProtectForm; }
    bool getRedlineProtection() const { return m_bRedlineProtection; }
    bool getReadOnly() const { return m_bReadOnly; }
    bool getEnforcement() const { return m_bEnforcement; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
