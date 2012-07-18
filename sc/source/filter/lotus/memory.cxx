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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
//  Bemerkung:  Variablen nicht ueber Headerfile, Module muessen sich
//              selbst per extern ihre Sachen besorgen!




#include "scitems.hxx"
#include <svx/algitem.hxx>

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



