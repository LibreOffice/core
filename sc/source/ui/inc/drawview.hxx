/*************************************************************************
 *
 *  $RCSfile: drawview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-30 19:12:06 $
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

#ifndef SC_DRAWVIEW_HXX
#define SC_DRAWVIEW_HXX

#ifndef _SVX_FMVIEW_HXX //autogen
#include <svx/fmview.hxx>
#endif

#include "global.hxx"

class ScDocument;
class ScViewData;
class SdrViewUserMarker;

class ScDrawView: public FmFormView
{
    ScViewData*             pViewData;
    OutputDevice*           pDev;                   //! noetig ?
    ScDocument*             pDoc;
    USHORT                  nTab;
    Fraction                aScaleX;                // Faktor fuer Drawing-MapMode
    Fraction                aScaleY;
    SdrViewUserMarker*      pDropMarker;
    SdrObject*              pDropMarkObj;
    BOOL                    bInConstruct;
    BOOL                    bDisableHdl;

    void            Construct();
    void            UpdateBrowser();

protected:
    virtual void    ModelHasChanged();
    virtual void    MakeVisible( const Rectangle& rRect, Window& rWin );

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

public:
                    ScDrawView( OutputDevice* pOut, ScViewData* pData );
                    ScDrawView( OutputDevice* pOut, ScDocument* pDocument, USHORT nTable );
    virtual         ~ScDrawView();

    virtual void    MarkListHasChanged();
    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );

    void            DrawMarks( OutputDevice* pOut ) const;

    void            MarkDropObj( SdrObject* pObj );

    BOOL            IsDisableHdl() const    { return bDisableHdl; }

    void            SetMarkedToLayer( BYTE nLayerNo );

    void            InvalidateAttribs();
    void            InvalidateDrawTextAttrs();

    BOOL            BeginDrag( Window* pWindow, const Point& rStartPos );
    void            DoCut();
    void            DoCopy();

    void            GetScale( Fraction& rFractX, Fraction& rFractY ) const;
    void            RecalcScale();
    void            UpdateWorkArea();
    USHORT          GetTab() const      { return nTab; }

    void            CalcNormScale( Fraction& rFractX, Fraction& rFractY ) const;

    void            PaintObject( SdrObject* pObject, OutputDevice* pDev ) const;

    void            SetAnchor( ScAnchorType );
    ScAnchorType    GetAnchor() const;

    void            VCAddWin( Window* pWin );
    void            VCRemoveWin( Window* pWin );

    void            UpdateIMap( SdrObject* pObj );

    USHORT          GetPopupMenuId();
    void            UpdateUserViewOptions();

    void            SetMarkedOriginalSize();

    BOOL            SelectObject( const String& rName );
    String          GetSelectedChartName() const;
    BOOL            HasMarkedControl() const;

    FASTBOOL        InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, ULONG nOptions=0);

    SdrEndTextEditKind  ScEndTextEdit();    // ruft SetDrawTextUndo(0)
};




#endif

