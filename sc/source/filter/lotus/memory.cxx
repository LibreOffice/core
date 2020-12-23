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

//  Remark:  Variables are not declared in header file! Modules have to declare
//              their variables via extern!

#include <scitems.hxx>
#include <scmem.h>
#include <editeng/justifyitem.hxx>
#include "lotfilter.hxx"
#include <tool.h>
#include <editeng/svxenum.hxx>

bool MemNew(LotusContext& rContext)
{
    rContext.xValueFormCache.reset(new FormCache(&rContext.rDoc));

    // for tool.cxx::PutFormString()
    rContext.xAttrRight.reset(new SvxHorJustifyItem(SvxCellHorJustify::Right, ATTR_HOR_JUSTIFY));
    rContext.xAttrLeft.reset(new SvxHorJustifyItem(SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY));
    rContext.xAttrCenter.reset(new SvxHorJustifyItem(SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY));
    rContext.xAttrRepeat.reset(new SvxHorJustifyItem(SvxCellHorJustify::Repeat, ATTR_HOR_JUSTIFY));
    rContext.xAttrStandard.reset(
        new SvxHorJustifyItem(SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY));

    return true;
}

void MemDelete(LotusContext& rContext)
{
    rContext.xValueFormCache.reset();
    rContext.xAttrRight.reset();
    rContext.xAttrLeft.reset();
    rContext.xAttrCenter.reset();
    rContext.xAttrRepeat.reset();
    rContext.xAttrStandard.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
