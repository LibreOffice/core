/*************************************************************************
 *
 *  $RCSfile: numberingtypelistbox.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2001-03-02 14:06:02 $
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

#ifndef _NUMBERINGTYPELISTBOX_HXX
#include <numberingtypelistbox.hxx>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _CNTTAB_HXX
#include <cnttab.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEFAULTNUMBERINGPROVIDER_HPP_
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XNUMBERINGTYPEINFO_HPP_
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::text;
using namespace com::sun::star::lang;
using namespace com::sun::star::style;
using namespace rtl;

#define C2S(cChar) UniString::CreateFromAscii(cChar)
/* -----------------------------01.03.01 16:04--------------------------------

 ---------------------------------------------------------------------------*/
struct SwNumberingTypeListBox_Impl
{
    Reference<XNumberingTypeInfo> xInfo;
};
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
SwNumberingTypeListBox::SwNumberingTypeListBox( Window* pWin, const ResId& rResId,
        USHORT nTypeFlags ) :
    ListBox(pWin, rResId),
    pImpl(new SwNumberingTypeListBox_Impl)
{
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xI = xMSF->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
    Reference<XDefaultNumberingProvider> xDefNum(xI, UNO_QUERY);
    DBG_ASSERT(xDefNum.is(), "service missing: \"com.sun.star.text.DefaultNumberingProvider\"")

    pImpl->xInfo = Reference<XNumberingTypeInfo>(xDefNum, UNO_QUERY);
    Reload(nTypeFlags);
}
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
SwNumberingTypeListBox::~SwNumberingTypeListBox()
{
    delete pImpl;
}
/* -----------------------------01.03.01 16:02--------------------------------

 ---------------------------------------------------------------------------*/
void SwNumberingTypeListBox::Reload(USHORT nTypeFlags)
{
    Clear();
    SwOLENames aNames(SW_RES(STRRES_NUMTYPES));
    ResStringArray& rNames = aNames.GetNames();
    for(sal_uInt16 i = 0; i < rNames.Count(); i++)
    {
        long nValue = rNames.GetValue(i);
        sal_Bool bInsert = sal_True;
        switch(nValue)
        {
            case  NumberingType::NUMBER_NONE:   bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_NO_NUMBERING); break;
            case  NumberingType::CHAR_SPECIAL:  bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_BULLET); break;
            case  NumberingType::PAGE_DESCRIPTOR:bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING); break;
            case  NumberingType::BITMAP:bInsert = 0 != (nTypeFlags&INSERT_NUM_TYPE_BITMAP ); break;
        }
        if(bInsert)
        {
            InsertEntry(rNames.GetString(i));
            SetEntryData( i, (void*)nValue );
        }
    }
    if(0 != (nTypeFlags&INSERT_NUM_EXTENDED_TYPES))
    {
        if(pImpl->xInfo.is())
        {
            Sequence<sal_Int16> aTypes = pImpl->xInfo->getSupportedNumberingTypes(  );
            const sal_Int16* pTypes = aTypes.getConstArray();
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                {
                    if(LISTBOX_ENTRY_NOTFOUND == GetEntryPos((void*)(ULONG)nCurrent))
                    {
                        OUString aIdent = pImpl->xInfo->getNumberingIdentifier( nCurrent );
                        USHORT nPos = InsertEntry(aIdent);
                        SetEntryData(nPos,(void*)(ULONG)nCurrent);
                    }
                }
            }
        }
        SelectEntryPos(0);
    }
}
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16   SwNumberingTypeListBox::GetSelectedNumberingType()
{
    sal_Int16 nRet = 0;
    USHORT nSelPos = GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        nRet = (sal_Int16)(ULONG)GetEntryData(nSelPos);
#ifdef DBG_UTIL
    else
        DBG_ERROR("SwNumberingTypeListBox not selected")
#endif
    return nRet;
}
/* -----------------------------01.03.01 14:46--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwNumberingTypeListBox::SelectNumberingType(sal_Int16 nType)
{
    USHORT nPos = GetEntryPos((void*)(ULONG)nType);
    SelectEntryPos( nPos );
    return LISTBOX_ENTRY_NOTFOUND != nPos;
}

