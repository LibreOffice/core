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

#include "WriteProtection.hxx"
#include "TagLogger.hxx"

#include <comphelper/propertyvalue.hxx>
#include <ooxml/resourceids.hxx>

using namespace com::sun::star;

namespace writerfilter::dmapper
{
WriteProtection::WriteProtection()
    : LoggedProperties("WriteProtection")
    , m_nCryptProviderType(NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES)
    , m_CryptSpinCount(0)
    , m_bRecommended(false)
{
}

WriteProtection::~WriteProtection() {}

void WriteProtection::lcl_attribute(Id nName, Value& val)
{
    int nIntValue = val.getInt();
    OUString sStringValue = val.getString();

    switch (nName)
    {
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
        {
            sal_Int32 nCryptAlgorithmSid = sStringValue.toInt32();
            switch (nCryptAlgorithmSid)
            {
                case 1:
                    m_sAlgorithmName = "MD2";
                    break;
                case 2:
                    m_sAlgorithmName = "MD4";
                    break;
                case 3:
                    m_sAlgorithmName = "MD5";
                    break;
                case 4:
                    m_sAlgorithmName = "SHA-1";
                    break;
                case 5:
                    m_sAlgorithmName = "MAC";
                    break;
                case 6:
                    m_sAlgorithmName = "RIPEMD";
                    break;
                case 7:
                    m_sAlgorithmName = "RIPEMD-160";
                    break;
                case 9:
                    m_sAlgorithmName = "HMAC";
                    break;
                case 12:
                    m_sAlgorithmName = "SHA-256";
                    break;
                case 13:
                    m_sAlgorithmName = "SHA-384";
                    break;
                case 14:
                    m_sAlgorithmName = "SHA-512";
                    break;
                default:; // 8, 10, 11, any other value: Undefined.
            }
        }
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
        case NS_ooxml::LN_CT_WriteProtection_recommended:
            m_bRecommended = nIntValue;
            break;
        default:
        {
#ifdef DBG_UTIL
            TagLogger::getInstance().element("unhandled");
#endif
        }
    }
}

void WriteProtection::lcl_sprm(Sprm& /*rSprm*/) {}

uno::Sequence<beans::PropertyValue> WriteProtection::toSequence() const
{
    uno::Sequence<beans::PropertyValue> aResult;
    if (!m_sAlgorithmName.isEmpty() && !m_sSalt.isEmpty() && !m_sHash.isEmpty()
        && m_sCryptAlgorithmClass == "hash" && m_sCryptAlgorithmType == "typeAny")
    {
        aResult = { comphelper::makePropertyValue("algorithm-name", m_sAlgorithmName),
                    comphelper::makePropertyValue("salt", m_sSalt),
                    comphelper::makePropertyValue("iteration-count", m_CryptSpinCount),
                    comphelper::makePropertyValue("hash", m_sHash) };
    }

    return aResult;
}

} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
