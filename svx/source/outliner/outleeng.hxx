/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outleeng.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 13:25:43 $
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
#ifndef _OUTLEENG_HXX
#define _OUTLEENG_HXX

#ifndef _OUTLINER_HXX
#include <outliner.hxx>
#endif
#ifndef _EDITENG_HXX
#include <editeng.hxx>
#endif

typedef EENotify* EENotifyPtr;
SV_DECL_PTRARR_DEL( NotifyList, EENotifyPtr, 1, 1 );

class OutlinerEditEng : public EditEngine
{
    Outliner*           pOwner;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        ~OutlinerEditEng();

    virtual void        PaintingFirstLine( USHORT nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );

    virtual void        ParagraphInserted( USHORT nNewParagraph );
    virtual void        ParagraphDeleted( USHORT nDeletedParagraph );

    // #101498#
    virtual void        DrawingText(const Point& rStartPos, const XubString& rText, USHORT nTextStart, USHORT nTextLen, const sal_Int32* pDXArray, const SvxFont& rFont, USHORT nPara, USHORT nIndex, BYTE nRightToLeft);

    virtual void        StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void        ParaAttribsChanged( USHORT nPara );
    virtual void        ParagraphHeightChanged( USHORT nPara );
    virtual BOOL        SpellNextDocument();
    virtual XubString   GetUndoComment( USHORT nUndoId ) const;

    // for text conversion
    virtual BOOL        ConvertNextDocument();

    virtual void        FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual void        FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual XubString   CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );

    virtual Rectangle   GetBulletArea( USHORT nPara );

    // belongs into class Outliner, move there before incompatible update!
    Link                aOutlinerNotifyHdl;
    NotifyList          aNotifyCache;
};

#endif

