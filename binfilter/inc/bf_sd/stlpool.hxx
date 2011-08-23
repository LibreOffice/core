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

#ifndef _SD_STLPOOL_HXX
#define _SD_STLPOOL_HXX


#include <bf_svtools/style.hxx>
namespace binfilter {

class SdStyleSheet;
class SdDrawDocument;


class SdStyleSheetPool : public SfxStyleSheetPool
{
private:
    SfxStyleSheetBase* pActualStyleSheet;
    SdDrawDocument*    pDoc;

protected:
    virtual SfxStyleSheetBase* Create(const String& rName,
                                      SfxStyleFamily eFamily,
                                      USHORT nMask);

public:
                        SdStyleSheetPool(SfxItemPool& rPool, SdDrawDocument* pDocument);
    virtual 			~SdStyleSheetPool();

    void				SetActualStyleSheet(SfxStyleSheetBase* pActStyleSheet)
                                       { pActualStyleSheet = pActStyleSheet; }
    SfxStyleSheetBase*	GetActualStyleSheet()	 { return pActualStyleSheet; }

    SfxStyleSheetBase*  GetTitleSheet(const String& rLayoutName);

                            // Caller muss Liste loeschen
    List*               CreateOutlineSheetList(const String& rLayoutName);

    /** creates all layout style sheets for the givin layout name if they
        don't exist yet.

        @param rLayoutName	Must be the name of a master page
        @param bCheck		If set to true, the debug version will assert if a style
                            had to be created. This is used to assert errors in documents
                            when styles are missing.
    */
    void                CreateLayoutStyleSheets(const String& rLayoutName, sal_Bool bCheck = sal_False );

    void                CreatePseudosIfNecessary();
    void				UpdateStdNames();
    void 				AdjustLRSpaceItems();
    static void			PutNumBulletItem( SfxStyleSheetBase* pSheet, Font& rBulletFont );
    Font                GetBulletFont() const;

    SdDrawDocument* 	GetDoc() const { return pDoc; }
};

} //namespace binfilter
#endif	   // _SD_STLPOOL_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
