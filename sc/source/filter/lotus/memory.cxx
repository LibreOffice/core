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

#include "scitems.hxx"
#include "scmem.h"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>
#include "attrib.hxx"
#include "lotfilter.hxx"
#include "tool.h"

bool MemNew(LotusContext &rContext)
{
    rContext.pValueFormCache = new FormCache(rContext.pDoc);

    // fuer tool.cxx::PutFormString()
    rContext.pAttrUnprot = new ScProtectionAttr( true );
    rContext.pAttrRight = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY );
    rContext.pAttrLeft = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY );
    rContext.pAttrCenter = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY );
    rContext.pAttrRepeat = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_REPEAT, ATTR_HOR_JUSTIFY );
    rContext.pAttrStandard = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY );

    return true;
}

void MemDelete(LotusContext &rContext)
{
    delete rContext.pValueFormCache;
    delete rContext.pAttrRight;
    delete rContext.pAttrLeft;
    delete rContext.pAttrCenter;
    delete rContext.pAttrRepeat;
    delete rContext.pAttrStandard;
    delete rContext.pAttrUnprot;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
