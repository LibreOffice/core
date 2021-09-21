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

#include "DocumentProtection.hxx"
#include "TagLogger.hxx"
#include <vector>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

namespace writerfilter::dmapper
{
DocumentProtection::DocumentProtection()
    : LoggedProperties("DocumentProtection")
    , m_nEdit(
          NS_ooxml::
              LN_Value_doc_ST_DocProtect_none) // Specifies that no editing restrictions have been applied to the document
    , m_bProtectForm(false)
    , m_bRedlineProtection(false)
    , m_bReadOnly(false)
    , m_bEnforcement(false)
    , m_bFormatting(false)
    , m_nCryptProviderType(NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES)
    , m_sCryptAlgorithmClass("hash")
    , m_sCryptAlgorithmType("typeAny")
    , m_CryptSpinCount(0)
{
}

DocumentProtection::~DocumentProtection() {}

void DocumentProtection::lcl_attribute(Id nName, Value& val)
{
    int nIntValue = val.getInt();
    OUString sStringValue = val.getString();

    switch (nName)
    {
        case NS_ooxml::LN_CT_DocProtect_edit: // 92037
            m_nEdit = nIntValue;
            // multiple DocProtect_edits should not exist. If they do, last one wins
            m_bRedlineProtection = false;
            m_bProtectForm = false;
            m_bReadOnly = false;
            switch (nIntValue)
            {
                case NS_ooxml::LN_Value_doc_ST_DocProtect_trackedChanges:
                {
                    m_bRedlineProtection = true;
                    m_sRedlineProtectionKey = m_sHash;
                    break;
                }
                case NS_ooxml::LN_Value_doc_ST_DocProtect_forms:
                    m_bProtectForm = true;
                    break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_readOnly:
                    m_bReadOnly = true;
                    break;
            }
            break;
        case NS_ooxml::LN_CT_DocProtect_enforcement: // 92039
            m_bEnforcement = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_DocProtect_formatting: // 92038
            m_bFormatting = (nIntValue != 0);
            break;
        case NS_ooxml::LN_AG_Password_cryptProviderType: // 92025
            m_nCryptProviderType = nIntValue;
            break;
        case NS_ooxml::LN_AG_Password_cryptAlgorithmClass: // 92026
            if (nIntValue == NS_ooxml::LN_Value_doc_ST_AlgClass_hash) // 92023
                m_sCryptAlgorithmClass = "hash";
            break;
        case NS_ooxml::LN_AG_Password_cryptAlgorithmType: // 92027
            if (nIntValue == NS_ooxml::LN_Value_doc_ST_AlgType_typeAny) // 92024
                m_sCryptAlgorithmType = "typeAny";
            break;
        case NS_ooxml::LN_AG_Password_cryptAlgorithmSid: // 92028
            m_sCryptAlgorithmSid = sStringValue;
            break;
        case NS_ooxml::LN_AG_Password_cryptSpinCount: // 92029
            m_CryptSpinCount = nIntValue;
            break;
        case NS_ooxml::LN_AG_Password_hash: // 92035
            m_sHash = sStringValue;
            break;
        case NS_ooxml::LN_AG_Password_salt: // 92036
            m_sSalt = sStringValue;
            break;
        default:
        {
#ifdef DBG_UTIL
            TagLogger::getInstance().element("unhandled");
#endif
        }
    }
}

void DocumentProtection::lcl_sprm(Sprm& /*rSprm*/) {}

uno::Sequence<beans::PropertyValue> DocumentProtection::toSequence() const
{
    std::vector<beans::PropertyValue> documentProtection;

    if (enabled())
    {
        // w:edit
        {
            beans::PropertyValue aValue;
            aValue.Name = "edit";

            switch (m_nEdit)
            {
                case NS_ooxml::LN_Value_doc_ST_DocProtect_none:
                    aValue.Value <<= OUString("none");
                    break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_readOnly:
                    aValue.Value <<= OUString("readOnly");
                    break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_comments:
                    aValue.Value <<= OUString("comments");
                    break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_trackedChanges:
                    aValue.Value <<= OUString("trackedChanges");
                    break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_forms:
                    aValue.Value <<= OUString("forms");
                    break;
                default:
                {
#ifdef DBG_UTIL
                    TagLogger::getInstance().element("unhandled");
#endif
                }
            }

            documentProtection.push_back(aValue);
        }

        // w:enforcement
        if (m_bEnforcement)
        {
            beans::PropertyValue aValue;
            aValue.Name = "enforcement";
            aValue.Value <<= OUString("1");
            documentProtection.push_back(aValue);
        }

        // w:formatting
        if (m_bFormatting)
        {
            beans::PropertyValue aValue;
            aValue.Name = "formatting";
            aValue.Value <<= OUString("1");
            documentProtection.push_back(aValue);
        }

        // w:cryptProviderType
        {
            beans::PropertyValue aValue;
            aValue.Name = "cryptProviderType";
            if (m_nCryptProviderType == NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES)
                aValue.Value <<= OUString("rsaAES");
            else if (m_nCryptProviderType == NS_ooxml::LN_Value_doc_ST_CryptProv_rsaFull)
                aValue.Value <<= OUString("rsaFull");
            documentProtection.push_back(aValue);
        }

        // w:cryptAlgorithmClass
        {
            beans::PropertyValue aValue;
            aValue.Name = "cryptAlgorithmClass";
            aValue.Value <<= m_sCryptAlgorithmClass;
            documentProtection.push_back(aValue);
        }

        // w:cryptAlgorithmType
        {
            beans::PropertyValue aValue;
            aValue.Name = "cryptAlgorithmType";
            aValue.Value <<= m_sCryptAlgorithmType;
            documentProtection.push_back(aValue);
        }

        // w:cryptAlgorithmSid
        {
            beans::PropertyValue aValue;
            aValue.Name = "cryptAlgorithmSid";
            aValue.Value <<= m_sCryptAlgorithmSid;
            documentProtection.push_back(aValue);
        }

        // w:cryptSpinCount
        {
            beans::PropertyValue aValue;
            aValue.Name = "cryptSpinCount";
            aValue.Value <<= OUString::number(m_CryptSpinCount);
            documentProtection.push_back(aValue);
        }

        // w:hash
        {
            beans::PropertyValue aValue;
            aValue.Name = "hash";
            aValue.Value <<= m_sHash;
            documentProtection.push_back(aValue);
        }

        // w:salt
        {
            beans::PropertyValue aValue;
            aValue.Name = "salt";
            aValue.Value <<= m_sSalt;
            documentProtection.push_back(aValue);
        }
    }

    return comphelper::containerToSequence(documentProtection);
}

} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
