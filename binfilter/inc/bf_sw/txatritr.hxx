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
#ifndef _TXATRITR_HXX
#define _TXATRITR_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include <tools/debug.hxx>
class String; 
namespace binfilter {

class SfxPoolItem;
class SwTxtNode;
class SwTxtAttr;



class SwScriptIterator
{

public:
    SwScriptIterator( const String& rStr, xub_StrLen nStart = 0,
                      sal_Bool bFrwrd = sal_True ){DBG_BF_ASSERT(0, "STRIP");};


};


class SwTxtAttrIterator
{
    const SfxPoolItem *pParaItem, *pCurItem;

public:
SwTxtAttrIterator( const SwTxtNode& rTxtNd, USHORT nWhichId,//STRIP001 	SwTxtAttrIterator( const SwTxtNode& rTxtNd, USHORT nWhichId,
xub_StrLen nStart = 0 ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 						xub_StrLen nStart = 0 );
    const SfxPoolItem& GetAttr() const	{ return *pCurItem; }
};


#ifdef ITEMID_LANGUAGE

class SwLanguageIterator : public SwTxtAttrIterator
{
public:
    SwLanguageIterator( const SwTxtNode& rTxtNd, xub_StrLen nStart = 0 )
        : SwTxtAttrIterator( rTxtNd, RES_CHRATR_LANGUAGE, nStart )
    {}

    sal_uInt16 GetLanguage() const
        { return ((SvxLanguageItem&)GetAttr()).GetValue(); }
};

#endif


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
