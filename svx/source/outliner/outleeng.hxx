/*************************************************************************
 *
 *  $RCSfile: outleeng.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:23 $
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

#ifndef _OUTLEENG_HXX
#define _OUTLEENG_HXX

#ifndef _OUTLINER_HXX
#include <outliner.hxx>
#endif
#ifndef _EDITENG_HXX
#include <editeng.hxx>
#endif


class OutlinerEditEng : public EditEngine
{
    Outliner*           pOwner;

public:
                        OutlinerEditEng( Outliner* pOwner, SfxItemPool* pPool );
                        ~OutlinerEditEng();

    virtual void        PaintingFirstLine( USHORT nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev );

    virtual void        ParagraphInserted( USHORT nNewParagraph );
    virtual void        ParagraphDeleted( USHORT nDeletedParagraph );
    virtual void        DrawingText(const Point& rStartPos, const XubString& rText, const long* pDXArray, const SvxFont& rFont, USHORT nPara, USHORT nIndex );
    virtual void        StyleSheetChanged( SfxStyleSheet* pStyle );
    virtual void        ParaAttribsChanged( USHORT nPara );
    virtual void        ParagraphHeightChanged( USHORT nPara );
    virtual BOOL        SpellNextDocument();
    virtual XubString   GetUndoComment( USHORT nUndoId ) const;

    virtual void        FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual void        FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos );
    virtual XubString   CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor );

    virtual Rectangle   GetBulletArea( USHORT nPara );
};

#endif

