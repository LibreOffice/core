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

#include <ooxml/resourceids.hxx>
#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

/************************
 * class: FFDataHandler *
 ************************/

FFDataHandler::FFDataHandler() :
LoggedProperties(dmapper_logger, "FFDataHandler"),
m_nCheckboxHeight(0),
m_bCheckboxAutoHeight(false),
m_nCheckboxChecked(-1),
m_nCheckboxDefault(-1)
{
}


FFDataHandler::~FFDataHandler()
{
}

const OUString & FFDataHandler::getName() const
{
    return m_sName;
}

const OUString & FFDataHandler::getHelpText() const
{
    return m_sHelpText;
}

const OUString & FFDataHandler::getStatusText() const
{
    return m_sStatusText;
}

sal_uInt32 FFDataHandler::getCheckboxHeight() const
{
    return m_nCheckboxHeight;
}

bool FFDataHandler::getCheckboxAutoHeight() const
{
    return m_bCheckboxAutoHeight;
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

const OUString & FFDataHandler::getDropDownResult() const
{
    return m_sDropDownResult;
}

const FFDataHandler::DropDownEntries_t & FFDataHandler::getDropDownEntries() const
{
    return m_DropDownEntries;
}

const OUString & FFDataHandler::getTextDefault() const
{
    return m_sTextDefault;
}

void FFDataHandler::lcl_sprm(Sprm & r_Sprm)
{
    switch(r_Sprm.getId())
    {
    case NS_ooxml::LN_CT_FFData_name:
        {
            m_sName = r_Sprm.getValue()->getString();
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
    case NS_ooxml::LN_CT_FFCheckBox_size:
        {
            m_nCheckboxHeight = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_sizeAuto:
        {
            m_bCheckboxAutoHeight = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_checked:
        {
            m_nCheckboxChecked = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_default:
        {
            m_nCheckboxDefault = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFData_checkBox:
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_result:
        {
            m_sDropDownResult = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_listEntry:
        {
            m_DropDownEntries.push_back(r_Sprm.getValue()->getString());
        }
        break;
    case NS_ooxml::LN_CT_FFData_ddList:
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_default:
        {
            m_sTextDefault = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFData_textInput:
        {
            resolveSprm(r_Sprm);
        }
        break;
    default:
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->element("unhandled");
#endif
        break;
    }
}

void FFDataHandler::resolveSprm(Sprm & r_Sprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = r_Sprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void FFDataHandler::lcl_attribute(Id name, Value & val)
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
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->element("unhandled");
#endif
        break;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
