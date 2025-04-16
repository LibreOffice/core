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
#include "FFDataHandler.hxx"
#include "TagLogger.hxx"

#include <ooxml/resourceids.hxx>

namespace writerfilter::dmapper {

/************************
 * class: FFDataHandler *
 ************************/

FFDataHandler::FFDataHandler() :
LoggedProperties("FFDataHandler"),
m_nCheckboxHeight(0),
m_bCheckboxAutoHeight(false),
m_nCheckboxChecked(-1),
m_nCheckboxDefault(-1),
m_nTextMaxLength(0)
{
}


FFDataHandler::~FFDataHandler()
{
}


bool FFDataHandler::getCheckboxChecked() const
{
    if (m_nCheckboxChecked != -1)
        return m_nCheckboxChecked;
    else if (m_nCheckboxDefault != -1)
        return m_nCheckboxDefault;
    else
        return false;
}


void FFDataHandler::lcl_sprm(Sprm & r_Sprm)
{
    const Value* pValue = r_Sprm.getValue();
    sal_Int32 nIntValue = (pValue ? pValue->getInt() : 0);
    OUString sStringValue = pValue ? pValue->getString() : OUString();
    switch(r_Sprm.getId())
    {
    case NS_ooxml::LN_CT_FFData_name:
        {
            m_sName = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFData_helpText:
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFData_statusText:
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFData_entryMacro:
        {
            m_sEntryMacro = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFData_exitMacro:
        {
            m_sExitMacro = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_size:
        {
            m_nCheckboxHeight = nIntValue;
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_sizeAuto:
        {
            m_bCheckboxAutoHeight = nIntValue;
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_checked:
        {
            m_nCheckboxChecked = nIntValue;
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_default:
        {
            m_nCheckboxDefault = nIntValue;
        }
        break;
    case NS_ooxml::LN_CT_FFData_checkBox:
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_result:
        {
            m_sDropDownResult = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_listEntry:
        {
            m_DropDownEntries.push_back(sStringValue);
        }
        break;
    case NS_ooxml::LN_CT_FFData_ddList:
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_type:
        {
            m_sTextType = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_default:
        {
            m_sTextDefault = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_maxLength:
        {
            m_nTextMaxLength = nIntValue;
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_format:
        {
            m_sTextFormat = sStringValue;
        }
        break;
    case NS_ooxml::LN_CT_FFData_textInput:
        {
            resolveSprm(r_Sprm);
        }
        break;
    default:
#ifdef DBG_UTIL
        TagLogger::getInstance().element("unhandled");
#endif
        break;
    }
}

void FFDataHandler::resolveSprm(Sprm & r_Sprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = r_Sprm.getProps();
    if( pProperties)
        pProperties->resolve(*this);
}

void FFDataHandler::lcl_attribute(Id name, const Value & val)
{
    switch (name)
    {
    case NS_ooxml::LN_CT_FFHelpText_val:
        {
            m_sHelpText = val.getString();
        }
        break;
    case NS_ooxml::LN_CT_FFStatusText_val:
        {
            m_sStatusText = val.getString();
        }
        break;
    default:
#ifdef DBG_UTIL
        TagLogger::getInstance().element("unhandled");
#endif
        break;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
