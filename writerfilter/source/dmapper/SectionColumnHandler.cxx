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
#include <SectionColumnHandler.hxx>
#include <PropertyMap.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <osl/diagnose.h>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;


SectionColumnHandler::SectionColumnHandler()
    : LoggedProperties(dmapper_logger, "SectionColumnHandler")
    , bEqualWidth(false)
    , nSpace(0)
    , nNum(0)
    , bSep(false)
{
    aTempColumn.nWidth = aTempColumn.nSpace = 0;
}

SectionColumnHandler::~SectionColumnHandler()
{
}

void SectionColumnHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch( rName )
    {
        case NS_ooxml::LN_CT_Columns_equalWidth:
            bEqualWidth = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_Columns_space:
            nSpace = ConversionHelper::convertTwipToMM100( nIntValue );
            break;
        case NS_ooxml::LN_CT_Columns_num:
            nNum = nIntValue;
            break;
        case NS_ooxml::LN_CT_Columns_sep:
            bSep = (nIntValue != 0);
            break;

        case NS_ooxml::LN_CT_Column_w:
            aTempColumn.nWidth = ConversionHelper::convertTwipToMM100( nIntValue );
            break;
        case NS_ooxml::LN_CT_Column_space:
            aTempColumn.nSpace = ConversionHelper::convertTwipToMM100( nIntValue );
            break;
        default:
            OSL_FAIL( "SectionColumnHandler: unknown attribute");
    }
}

void SectionColumnHandler::lcl_sprm(Sprm & rSprm)
{
    switch( rSprm.getId())
    {
        case NS_ooxml::LN_CT_Columns_col:
        {
            aTempColumn.nWidth = aTempColumn.nSpace = 0;
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
                aCols.push_back(aTempColumn);
            }
        }
        break;
        default:
            OSL_FAIL( "SectionColumnHandler: unknown sprm");
    }
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
