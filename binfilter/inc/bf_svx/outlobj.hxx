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

#ifndef _OUTLOBJ_HXX
#define _OUTLOBJ_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <bf_svtools/itemset.hxx>
#endif

#include <bf_svx/eeitem.hxx>
class SvStream;
namespace binfilter {

class SfxItemPool;
class SfxStyleSheetPool; 
class Outliner;
class EditTextObject;

class OutlinerParaObject
{
    friend class Outliner;

    EditTextObject* 			pText;
    USHORT* 					pDepthArr;
    sal_uInt32					nCount;
    BOOL						bIsEditDoc;
                                OutlinerParaObject( USHORT nParaCount );

public:
                                OutlinerParaObject( const OutlinerParaObject& );
                                ~OutlinerParaObject();

    OutlinerParaObject* 		Clone() const;

    USHORT						GetOutlinerMode() const;
    void						SetOutlinerMode( USHORT n );

    BOOL						IsVertical() const;
    void						SetVertical( BOOL bVertical );

    sal_uInt32					Count() const 					{ return nCount; }
    USHORT 						GetDepth( USHORT nPara ) const	{ return pDepthArr[nPara]; }
    const EditTextObject& 		GetTextObject() const 			{ return *pText; }
    void						ClearPortionInfo();
    BOOL						IsEditDoc() const 				{ return bIsEditDoc; }
    void 						Store( SvStream& ) const;
    static OutlinerParaObject*	Create( SvStream&, SfxItemPool* pTextObjectPool = 0 );

    BOOL						ChangeStyleSheets( 	const XubString& rOldName, SfxStyleFamily eOldFamily,
                                                    const XubString& rNewName, SfxStyleFamily eNewFamily );
    void						ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    void						MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart = EE_CHAR_START, USHORT nEnd = EE_CHAR_END );

    void 						PrepareStore( SfxStyleSheetPool* pStyleSheetPool );
    void 						FinishStore();
    void 						FinishLoad( SfxStyleSheetPool* pStyleSheetPool );
};

}//end of namespace binfilter
#endif
