/*************************************************************************
 *
 *  $RCSfile: navicfg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 08:05:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>  // fuer Pathfinder
#endif
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif
#ifndef _SWCONT_HXX
#include <swcont.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

/* -----------------------------08.09.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwNavigationConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "RootType",             //0
        "SelectedPosition",     //1
        "OutlineLevel",         //2
        "InsertMode",           //3
        "ActiveBlock",          //4
        "ShowListBox",          //5
        "GlobalDocMode"         //6
    };
    const int nCount = 7;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-----------------13.11.96 11.03-------------------

--------------------------------------------------*/

SwNavigationConfig::SwNavigationConfig() :
    utl::ConfigItem(C2U("Office.Writer/Navigator")),
    bIsGlobalActive(TRUE),
    bIsSmall(FALSE),
    nRootType(0xffff),
    nSelectedPos(0),
    nOutlineLevel(MAXLEVEL),
    nRegionMode(REGION_MODE_NONE),
    nActiveBlock(0)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
//  EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case 0: pValues[nProp] >>= nRootType;      break;
                    case 1: pValues[nProp] >>= nSelectedPos;   break;
                    case 2: pValues[nProp] >>= nOutlineLevel;  break;
                    case 3: pValues[nProp] >>= nRegionMode;    break;
                    case 4: pValues[nProp] >>= nActiveBlock;    break;
                    case 5: bIsSmall        = *(sal_Bool*)pValues[nProp].getValue();  break;
                    case 6: bIsGlobalActive = *(sal_Bool*)pValues[nProp].getValue();  break;
                }
            }
        }
    }
}
/* -----------------------------08.09.00 16:35--------------------------------

 ---------------------------------------------------------------------------*/
SwNavigationConfig::~SwNavigationConfig()
{}
/* -----------------------------08.09.00 16:35--------------------------------

 ---------------------------------------------------------------------------*/
void SwNavigationConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    const Type& rType = ::getBooleanCppuType();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= nRootType;     break;
            case 1: pValues[nProp] <<= nSelectedPos;  break;
            case 2: pValues[nProp] <<= nOutlineLevel; break;
            case 3: pValues[nProp] <<= nRegionMode;   break;
            case 4: pValues[nProp] <<= nActiveBlock;    break;
            case 5: pValues[nProp].setValue(&bIsSmall, rType);          break;
            case 6: pValues[nProp].setValue(&bIsGlobalActive, rType);   break;
        }
    }
    PutProperties(aNames, aValues);
}

