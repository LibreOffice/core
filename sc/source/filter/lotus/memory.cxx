/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */







#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>

#include "attrib.hxx"

#include "tool.h"

extern ScDocument*      pDoc;

extern FormCache*       pValueFormCache;            

extern SvxHorJustifyItem    *pAttrRight, *pAttrLeft, *pAttrCenter,
                            *pAttrRepeat, *pAttrStandard;   
extern ScProtectionAttr*    pAttrUnprot;   



sal_Bool MemNew( void )
{
    pValueFormCache = new FormCache( pDoc );

    
    pAttrUnprot = new ScProtectionAttr( true );
    pAttrRight = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY );
    pAttrLeft = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY );
    pAttrCenter = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY );
    pAttrRepeat = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_REPEAT, ATTR_HOR_JUSTIFY );
    pAttrStandard = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY );

    return sal_True;
}


void MemDelete( void )
{
    delete pValueFormCache;
    delete pAttrRight;
    delete pAttrLeft;
    delete pAttrCenter;
    delete pAttrRepeat;
    delete pAttrStandard;
    delete pAttrUnprot;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
