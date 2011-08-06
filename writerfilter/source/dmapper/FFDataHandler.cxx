/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
m_bCheckboxChecked(false)
{
}


FFDataHandler::~FFDataHandler()
{
}

const rtl::OUString & FFDataHandler::getName() const
{
    return m_sName;
}

const rtl::OUString & FFDataHandler::getHelpText() const
{
    return m_sHelpText;
}

const rtl::OUString & FFDataHandler::getStatusText() const
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
    return m_bCheckboxChecked;
}

const rtl::OUString & FFDataHandler::getDropDownResult() const
{
    return m_sDropDownResult;
}

const FFDataHandler::DropDownEntries_t & FFDataHandler::getDropDownEntries() const
{
    return m_DropDownEntries;
}

const rtl::OUString & FFDataHandler::getTextDefault() const
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
            m_bCheckboxChecked = r_Sprm.getValue()->getInt();
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
            m_DropDownEntries.push_back(r_Sprm.getValue()->getString());;
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
