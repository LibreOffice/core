/*************************************************************************
 *
 *  $RCSfile: AccessibleText.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2002-03-01 15:09:56 $
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

#include "AccessibleText.hxx"
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef _SVX_UNOFORED_HXX
#include <svx/unofored.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif

class ScViewForwarder : public SvxViewForwarder
{
    ScTabViewShell*     mpViewShell;
    ScSplitPos          meSplitPos;
public:
                        ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos);
    virtual             ~ScViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint ) const;
    virtual Point       PixelToLogic( const Point& rPoint ) const;
};

ScViewForwarder::ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos)
    :
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos)
{
}

ScViewForwarder::~ScViewForwarder()
{
}

BOOL ScViewForwarder::IsValid() const
{
    return mpViewShell != NULL;
}

Rectangle ScViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            aVisArea.SetSize(pWindow->GetSizePixel());
        if(mpViewShell->GetViewData())
            aVisArea.SetPos(Point(mpViewShell->GetViewData()->GetPixPos(meSplitPos)));
    }
    return aVisArea;
}

Point ScViewForwarder::LogicToPixel( const Point& rPoint ) const
{
    return rPoint;
}

Point ScViewForwarder::PixelToLogic( const Point& rPoint ) const
{
    MapMode aMapMode(MAP_100TH_MM);
    Point aPoint(rPoint);
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            aPoint = pWindow->PixelToLogic( rPoint, aMapMode );
    }
    return rPoint;
}

class ScEditViewForwarder : public SvxEditViewForwarder
{
public:
                        ScEditViewForwarder();
    virtual             ~ScEditViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint ) const;
    virtual Point       PixelToLogic( const Point& rPoint ) const;
    virtual sal_Bool    GetSelection( ESelection& rSelection ) const;
    virtual sal_Bool    SetSelection( const ESelection& rSelection );
    virtual sal_Bool    Copy();
    virtual sal_Bool    Cut();
    virtual sal_Bool    Paste();
};

ScEditViewForwarder::ScEditViewForwarder()
{
}

ScEditViewForwarder::~ScEditViewForwarder()
{
}

BOOL ScEditViewForwarder::IsValid() const
{
    return sal_False;
}

Rectangle ScEditViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    return aVisArea;
}

Point ScEditViewForwarder::LogicToPixel( const Point& rPoint ) const
{
    return rPoint;
}

Point ScEditViewForwarder::PixelToLogic( const Point& rPoint ) const
{
    return rPoint;
}

sal_Bool ScEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    return sal_False;
}

sal_Bool ScEditViewForwarder::SetSelection( const ESelection& rSelection )
{
    return sal_False;
}

sal_Bool ScEditViewForwarder::Copy()
{
    return sal_False;
}

sal_Bool ScEditViewForwarder::Cut()
{
    return sal_False;
}

sal_Bool ScEditViewForwarder::Paste()
{
    return sal_False;
}

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

ScAccessibleCellTextData::ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos)
    : ScCellTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpViewForwarder(NULL)
{
}

ScAccessibleCellTextData::~ScAccessibleCellTextData()
{
    if (mpViewForwarder)
        delete mpViewForwarder;
}

void ScAccessibleCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpViewShell = NULL;                     // invalid now
        }
    }
    ScCellTextData::Notify(rBC, rHint);
}

ScAccessibleCellTextData* ScAccessibleCellTextData::Clone() const
{
    return new ScAccessibleCellTextData(mpViewShell, aCellPos, meSplitPos);
}

SvxTextForwarder* ScAccessibleCellTextData::GetTextForwarder()
{
    SvxEditEngineForwarder* pTempForwarder = NULL;
    if (mpViewShell && mpViewShell->GetViewData() &&
        (mpViewShell->GetViewData()->GetCurPos() == aCellPos) &&
        (mpViewShell->GetViewData()->HasEditView(meSplitPos)) &&
        (mpViewShell->GetViewData()->GetEditViewCol() == aCellPos.Col()) &&
        (mpViewShell->GetViewData()->GetEditViewRow() == aCellPos.Row()))
    {
        if (pForwarder)
            DELETEZ( pForwarder );
        if (pEditEngine)
            DELETEZ( pEditEngine );

        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        if (pEditView)
        {
            pTempForwarder = new SvxEditEngineForwarder(*(pEditView->GetEditEngine()));
            pForwarder = pTempForwarder;
        }
    }

    if (pTempForwarder)
        return pTempForwarder;
    else
        return ScCellTextData::GetTextForwarder();
}

SvxViewForwarder* ScAccessibleCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScViewForwarder(mpViewShell, meSplitPos);
    return mpViewForwarder;
}

SvxEditViewForwarder* ScAccessibleCellTextData::GetEditViewForwarder( sal_Bool bCreate )
{
    return new ScEditViewForwarder;
}

ScDocShell* ScAccessibleCellTextData::GetDocShell(ScTabViewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell && pViewShell->GetViewData() && pViewShell->GetViewData()->GetDocument())
        pDocSh = (ScDocShell*) pViewShell->GetViewData()->GetDocument()->GetDocumentShell();
    return pDocSh;
}

