/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlobj.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:26:44 $
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
#include <svtools/itemset.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <svx/eeitem.hxx>

class Outliner;
class EditTextObject;
class SvStream;
class SfxItemPool;
class SfxStyleSheetPool;

class SVX_DLLPUBLIC OutlinerParaObject
{
    friend class Outliner;

    EditTextObject*             pText;
    USHORT*                     pDepthArr;
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
    USHORT                      GetDepth( USHORT nPara ) const  { return pDepthArr[nPara]; }
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

    void                        SetLRSpaceItemFlags( BOOL bOutlineMode );
    void                        MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart = EE_CHAR_START, USHORT nEnd = EE_CHAR_END );

/* cl removed because not needed anymore since binfilter
    void                        PrepareStore( SfxStyleSheetPool* pStyleSheetPool );
    void                        FinishStore();
    void                        FinishLoad( SfxStyleSheetPool* pStyleSheetPool );
*/
};

#endif
