/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txatritr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 13:19:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _TXATRITR_HXX
#define _TXATRITR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

class String;
class SwTxtNode;
class SwTxtAttr;
class SfxPoolItem;


class SwScriptIterator
{
    const String& rText;
    xub_StrLen nChgPos;
    sal_uInt16 nCurScript;
    sal_Bool bForward;

public:
    SwScriptIterator( const String& rStr, xub_StrLen nStart = 0,
                      sal_Bool bFrwrd = sal_True );

    sal_Bool Next();

    sal_uInt16 GetCurrScript() const        { return nCurScript; }
    xub_StrLen GetScriptChgPos() const      { return nChgPos; }
    const String& GetText() const           { return rText; }
};


class SwTxtAttrIterator
{
    SwScriptIterator aSIter;
    SvPtrarr aStack;
    const SwTxtNode& rTxtNd;
    const SfxPoolItem *pParaItem, *pCurItem;
    xub_StrLen nChgPos;
    sal_uInt16 nAttrPos, nWhichId;
    sal_Bool bIsUseGetWhichOfScript;

    void AddToStack( const SwTxtAttr& rAttr );
    void SearchNextChg();

public:
    SwTxtAttrIterator( const SwTxtNode& rTxtNd, USHORT nWhichId,
                        xub_StrLen nStart = 0, sal_Bool bUseGetWhichOfScript = sal_True );

    sal_Bool Next();

    const SfxPoolItem& GetAttr() const  { return *pCurItem; }
    xub_StrLen GetChgPos() const        { return nChgPos; }
};


#ifdef ITEMID_LANGUAGE

class SwLanguageIterator : public SwTxtAttrIterator
{
public:
    SwLanguageIterator( const SwTxtNode& rTxtNd, xub_StrLen nStart = 0,
                        USHORT nWhichId = RES_CHRATR_LANGUAGE,
                        sal_Bool bUseGetWhichOfScript = sal_True )
        : SwTxtAttrIterator( rTxtNd, nWhichId, nStart, bUseGetWhichOfScript )
    {}

    sal_uInt16 GetLanguage() const
        { return ((SvxLanguageItem&)GetAttr()).GetValue(); }
};

#endif


#endif
