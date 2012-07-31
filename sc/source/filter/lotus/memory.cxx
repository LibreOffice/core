/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

//  Bemerkung:  Variablen nicht ueber Headerfile, Module muessen sich
//              selbst per extern ihre Sachen besorgen!




#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>

#include "attrib.hxx"

#include "decl.h"
#include "tool.h"

extern ScDocument*      pDoc;

extern FormCache*       pValueFormCache;            // -> tool.cxx

extern SvxHorJustifyItem    *pAttrRight, *pAttrLeft, *pAttrCenter,
                            *pAttrRepeat, *pAttrStandard;   // -> tool.cxx, fuer GetFormAttr()
extern ScProtectionAttr*    pAttrUnprot;   // -> tool.cxx, fuer PutFormString()



sal_Bool MemNew( void )
{
    pValueFormCache = new FormCache( pDoc );

    // fuer tool.cxx::PutFormString()
    pAttrUnprot = new ScProtectionAttr( sal_True );
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
