/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outlobj.hxx,v $
 * $Revision: 1.5.78.1 $
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

#include <tools/solar.h>
#include <tools/string.hxx>
#include <rsc/rscsfx.hxx>
#include <svtools/itemset.hxx>
#include "svx/svxdllapi.h"

#include <svx/eeitem.hxx>

class Outliner;
class EditTextObject;
class SvStream;
class SfxItemPool;
class SfxStyleSheetPool;
class ParagraphData;

class SVX_DLLPUBLIC OutlinerParaObject
{
    friend class Outliner;

    EditTextObject*             pText;
    ParagraphData*              pParagraphDataArr;
    sal_uInt32                  nCount;
    BOOL                        bIsEditDoc;
                                OutlinerParaObject( USHORT nParaCount );

public:
                                OutlinerParaObject( const OutlinerParaObject& );
                                OutlinerParaObject( const EditTextObject& rTObj );
                                ~OutlinerParaObject();

    OutlinerParaObject*         Clone() const;

    USHORT                      GetOutlinerMode() const;
    void                        SetOutlinerMode( USHORT n );

    BOOL                        IsVertical() const;
    void                        SetVertical( BOOL bVertical );

    sal_uInt32                  Count() const                   { return nCount; }
    sal_Int16                   GetDepth( USHORT nPara ) const;
    const EditTextObject&       GetTextObject() const           { return *pText; }
    void                        ClearPortionInfo();
    BOOL                        IsEditDoc() const               { return bIsEditDoc; }
    void                        Store( SvStream& ) const;
    static OutlinerParaObject*  Create( SvStream&, SfxItemPool* pTextObjectPool = 0 );

    BOOL                        ChangeStyleSheets(  const XubString& rOldName, SfxStyleFamily eOldFamily,
                                                    const XubString& rNewName, SfxStyleFamily eNewFamily );
    void                        ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );
    void                        SetStyleSheets( USHORT nLevel, const XubString rNewName, const SfxStyleFamily& rNewFamily );

    BOOL                        RemoveCharAttribs( USHORT nWhich = 0 );
    BOOL                        RemoveParaAttribs( USHORT nWhich = 0 );

    void                        MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart = EE_CHAR_START, USHORT nEnd = EE_CHAR_END );
};

#endif
