/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
m_bEnabled(false),
m_bCalcOnExit(false),
m_nHelpTextType(0),
m_nStatusTextType(0),
m_nCheckboxHeight(0),
m_bCheckboxAutoHeight(false),
m_bCheckboxDefault(false),
m_bCheckboxChecked(false),
m_nTextType(0),
m_nTextMaxLength(0)
{
}


FFDataHandler::~FFDataHandler()
{
}

// member: FFDataHandler::name
void FFDataHandler::setName(const rtl::OUString & r_sName)
{
    m_sName = r_sName;
}

const rtl::OUString & FFDataHandler::getName() const
{
    return m_sName;
}

// member: FFDataHandler::enabled
void FFDataHandler::setEnabled(bool r_enabled)
{
    m_bEnabled = r_enabled;
}

bool FFDataHandler::getEnabled() const
{
    return m_bEnabled;
}

// member: FFDataHandler::calcOnExit
void FFDataHandler::setCalcOnExit(bool r_calcOnExit)
{
    m_bCalcOnExit = r_calcOnExit;
}

bool FFDataHandler::getCalcOnExit() const
{
    return m_bCalcOnExit;
}

// member: FFDataHandler::entryMacro
void FFDataHandler::setEntryMacro(const rtl::OUString & r_sEntryMacro)
{
    m_sEntryMacro = r_sEntryMacro;
}

const rtl::OUString & FFDataHandler::getEntryMacro() const
{
    return m_sEntryMacro;
}

// member: FFDataHandler::exitMacro
void FFDataHandler::setExitMacro(const rtl::OUString & r_sExitMacro)
{
    m_sExitMacro = r_sExitMacro;
}

const rtl::OUString & FFDataHandler::getExitMacro() const
{
    return m_sExitMacro;
}

// member: FFDataHandler::helpTextType
void FFDataHandler::setHelpTextType(sal_uInt32 r_helpTextType)
{
    m_nHelpTextType = r_helpTextType;
}

sal_uInt32 FFDataHandler::getHelpTextType() const
{
    return m_nHelpTextType;
}

// member: FFDataHandler::helpText
void FFDataHandler::setHelpText(const rtl::OUString & r_sHelpText)
{
    m_sHelpText = r_sHelpText;
}

const rtl::OUString & FFDataHandler::getHelpText() const
{
    return m_sHelpText;
}

// member: FFDataHandler::statusTextType
void FFDataHandler::setStatusTextType(sal_uInt32 r_statusTextType)
{
    m_nStatusTextType = r_statusTextType;
}

sal_uInt32 FFDataHandler::getStatusTextType() const
{
    return m_nStatusTextType;
}

// member: FFDataHandler::statusText
void FFDataHandler::setStatusText(const rtl::OUString & r_sStatusText)
{
    m_sStatusText = r_sStatusText;
}

const rtl::OUString & FFDataHandler::getStatusText() const
{
    return m_sStatusText;
}

// member: FFDataHandler::checkboxHeight
void FFDataHandler::setCheckboxHeight(sal_uInt32 r_checkboxHeight)
{
    m_nCheckboxHeight = r_checkboxHeight;
}

sal_uInt32 FFDataHandler::getCheckboxHeight() const
{
    return m_nCheckboxHeight;
}

// member: FFDataHandler::checkboxAutoHeight
void FFDataHandler::setCheckboxAutoHeight(bool r_checkboxAutoHeight)
{
    m_bCheckboxAutoHeight = r_checkboxAutoHeight;
}

bool FFDataHandler::getCheckboxAutoHeight() const
{
    return m_bCheckboxAutoHeight;
}

// member: FFDataHandler::checkboxDefault
void FFDataHandler::setCheckboxDefault(bool r_checkboxDefault)
{
    m_bCheckboxDefault = r_checkboxDefault;
}

bool FFDataHandler::getCheckboxDefault() const
{
    return m_bCheckboxDefault;
}

// member: FFDataHandler::checkboxChecked
void FFDataHandler::setCheckboxChecked(bool r_checkboxChecked)
{
    m_bCheckboxChecked = r_checkboxChecked;
}

bool FFDataHandler::getCheckboxChecked() const
{
    return m_bCheckboxChecked;
}

// member: FFDataHandler::dropDownResult
void FFDataHandler::setDropDownResult(const rtl::OUString & r_sDropDownResult)
{
    m_sDropDownResult = r_sDropDownResult;
}

const rtl::OUString & FFDataHandler::getDropDownResult() const
{
    return m_sDropDownResult;
}

// member: FFDataHandler::dropDownDefault
void FFDataHandler::setDropDownDefault(const rtl::OUString & r_sDropDownDefault)
{
    m_sDropDownDefault = r_sDropDownDefault;
}

const rtl::OUString & FFDataHandler::getDropDownDefault() const
{
    return m_sDropDownDefault;
}

// member: FFDataHandler::dropDownEntries
void FFDataHandler::setDropDownEntries(const FFDataHandler::DropDownEntries_t & r_dropDownEntries)
{
    m_DropDownEntries = r_dropDownEntries;
}

const FFDataHandler::DropDownEntries_t & FFDataHandler::getDropDownEntries() const
{
    return m_DropDownEntries;
}

void FFDataHandler::dropDownEntriesPushBack(const rtl::OUString & r_Element)
{
    m_DropDownEntries.push_back(r_Element);
}

// member: FFDataHandler::textType
void FFDataHandler::setTextType(sal_uInt32 r_textType)
{
    m_nTextType = r_textType;
}

sal_uInt32 FFDataHandler::getTextType() const
{
    return m_nTextType;
}

// member: FFDataHandler::textMaxLength
void FFDataHandler::setTextMaxLength(sal_uInt32 r_textMaxLength)
{
    m_nTextMaxLength = r_textMaxLength;
}

sal_uInt32 FFDataHandler::getTextMaxLength() const
{
    return m_nTextMaxLength;
}

// member: FFDataHandler::textDefault
void FFDataHandler::setTextDefault(const rtl::OUString & r_sTextDefault)
{
    m_sTextDefault = r_sTextDefault;
}

const rtl::OUString & FFDataHandler::getTextDefault() const
{
    return m_sTextDefault;
}

// member: FFDataHandler::textFormat
void FFDataHandler::setTextFormat(const rtl::OUString & r_sTextFormat)
{
    m_sTextFormat = r_sTextFormat;
}

const rtl::OUString & FFDataHandler::getTextFormat() const
{
    return m_sTextFormat;
}


void FFDataHandler::lcl_sprm(Sprm & r_Sprm)
{
    switch(r_Sprm.getId())
    {
    case NS_ooxml::LN_CT_FFData_name:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sName = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFData_enabled:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_bEnabled = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFData_calcOnExit:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_bCalcOnExit = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFData_entryMacro:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sEntryMacro = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFData_exitMacro:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sExitMacro = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFData_helpText:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFData_statusText:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_size:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_nCheckboxHeight = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_sizeAuto:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_bCheckboxAutoHeight = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_default:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_bCheckboxDefault = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFCheckBox_checked:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_bCheckboxChecked = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFData_checkBox:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_result:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sDropDownResult = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_default:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sDropDownDefault = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFDDList_listEntry:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_DropDownEntries.push_back(r_Sprm.getValue()->getString());;
        }
        break;
    case NS_ooxml::LN_CT_FFData_ddList:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            resolveSprm(r_Sprm);
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_type:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_nTextType = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_default:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sTextDefault = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_maxLength:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_nTextMaxLength = r_Sprm.getValue()->getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFTextInput_format:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sTextFormat = r_Sprm.getValue()->getString();
        }
        break;
    case NS_ooxml::LN_CT_FFData_textInput:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            resolveSprm(r_Sprm);
        }
        break;
    default:
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->element("FFDataHandler.unhandled");
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
    case NS_ooxml::LN_CT_FFHelpText_type:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_nHelpTextType = val.getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFHelpText_val:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sHelpText = val.getString();
        }
        break;
    case NS_ooxml::LN_CT_FFStatusText_type:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_nStatusTextType = val.getInt();
        }
        break;
    case NS_ooxml::LN_CT_FFStatusText_val:
    /* WRITERFILTERSTATUS done: 100, planned: 2, spent: 0 */
        {
            m_sStatusText = val.getString();
        }
        break;
    default:
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->element("FFDataHandler.unhandled");
#endif
        break;
    }
}

}}
