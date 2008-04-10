/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"




// INCLUDE ---------------------------------------------------------------
#include "postit.hxx"
#include <svtools/itemset.hxx>
#include <sfx2/objsh.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/editobj.hxx>
#include <unotools/charclass.hxx>
#include <tools/urlobj.hxx>
#include "scitems.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "attrib.hxx"
#include "detfunc.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "drwlayer.hxx"
#include "docuno.hxx"
#include <svx/svdpage.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnstit.hxx>
#ifndef _SVX_XLNSWIT_HXX
#include <svx/xlnstwit.hxx>
#endif
#include <com/sun/star/uno/Reference.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XModel;

//========================================================================
// class ScPostIt
//========================================================================

ScPostIt::ScPostIt(ScDocument* pDoc):
    mpDoc(pDoc),
    mbShown(FALSE),
    maItemSet(pDoc->GetNoteItemPool(), SDRATTR_START,  SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END, 0,0)
{
    // this 'default' ctor  does not set the Author & Date stamp.
    // maRectangle is not initialised as it can be tested using IsEmpty().
}

ScPostIt::ScPostIt( const String& rText, ScDocument* pDoc ):
    mpDoc(pDoc),
    mbShown(FALSE),
    maItemSet(pDoc->GetNoteItemPool(), SDRATTR_START,  SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END, 0,0)
{
    // maRectangle is not initialised as it can be tested using IsEmpty().
    SetText( rText);
    AutoStamp( );
}

ScPostIt::ScPostIt( const EditTextObject* pTextObj, ScDocument* pDoc ):
    mpDoc ( pDoc ),
    mbShown(FALSE),
    maItemSet(pDoc->GetNoteItemPool(), SDRATTR_START,  SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END, 0,0)
{
    // maRectangle is not initialised as it can be tested using IsEmpty().
    SetEditTextObject( pTextObj);
    AutoStamp( );
}

ScPostIt::ScPostIt( const ScPostIt& rNote, ScDocument* pDoc ):
    mpDoc ( pDoc ),
    maItemSet(pDoc->GetNoteItemPool(), SDRATTR_START,  SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END, 0,0)
{
    SetEditTextObject( rNote.mpEditObj.get());
    maStrDate   = rNote.maStrDate;
    maStrAuthor = rNote.maStrAuthor;
    mbShown = rNote.mbShown;
    maRectangle = rNote.maRectangle;
    maItemSet.PutExtended(rNote.maItemSet,SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
}

ScPostIt::~ScPostIt()
{
}

const ScPostIt& ScPostIt::operator=( const ScPostIt& rCpy )
{
    mpDoc       = rCpy.mpDoc;
    SetEditTextObject( rCpy.mpEditObj.get());
    maStrDate   = rCpy.maStrDate;
    maStrAuthor = rCpy.maStrAuthor;
    mbShown = rCpy.mbShown;
    maRectangle = rCpy.maRectangle;
    maItemSet.PutExtended(rCpy.maItemSet,SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);

    return *this;
}

// Support existing functionality - create simple text string from the
// EditTextObject.
String ScPostIt::GetText() const
{
    String aText;
    const EditTextObject* pEditText;
    if(mpDoc && ((pEditText = GetEditTextObject()) != 0))
    {
        ScNoteEditEngine& rEE = mpDoc->GetNoteEngine();
        rEE.SetText( *pEditText );
        aText = rEE.GetText( LINEEND_LF );
        aText.SearchAndReplaceAll(CHAR_CR, ' ');
    }
    return aText;
}

void ScPostIt::SetEditTextObject( const EditTextObject* pTextObj )
{
    if(pTextObj && mpDoc)
    {
        ScNoteEditEngine& rEE = mpDoc->GetNoteEngine();
        rEE.SetText( *pTextObj );
        sal_uInt16 nCount = pTextObj->GetParagraphCount();
        for( sal_uInt16 nPara = 0; nPara < nCount; ++nPara )
        {
            String aParaText( rEE.GetText( nPara ) );
            if( aParaText.Len() )
            {
                SfxItemSet aSet( pTextObj->GetParaAttribs( nPara));
                rEE.SetParaAttribs(nPara, aSet);
            }
        }
        mpEditObj.reset(rEE.CreateTextObject());
    }
    else
        mpEditObj.reset();
}

// Support existing functionality - create EditTextObject from a simple
// text string
void ScPostIt::SetText( const String& rText)
{
    if(mpDoc && rText.Len())
    {
        ScNoteEditEngine& rEE = mpDoc->GetNoteEngine();
        rEE.SetText( rText );
        mpEditObj.reset(rEE.CreateTextObject());
    }
    else
        mpEditObj.reset();

}

void ScPostIt::AutoStamp( )
{
    maStrDate   = ScGlobal::pLocaleData->getDate( Date() );

    SvtUserOptions aUserOpt;
    maStrAuthor = aUserOpt.GetID();
}

BOOL ScPostIt::IsEmpty() const
{
    return (!mpEditObj.get());
}

Rectangle ScPostIt::DefaultRectangle(const ScAddress& rPos) const
{
    BOOL bNegativePage = mpDoc->IsNegativePage( rPos.Tab() );
    long nPageSign = bNegativePage ? -1 : 1;

    SCCOL nNextCol = rPos.Col()+1;
    const ScMergeAttr* pMerge = (const ScMergeAttr*) mpDoc->GetAttr( rPos.Col(), rPos.Row(), rPos.Tab(), ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 )
        nNextCol = rPos.Col() + pMerge->GetColMerge();

    Point aRectPos = ScDetectiveFunc(mpDoc, rPos.Tab()).GetDrawPos( nNextCol, rPos.Row(), FALSE );

    aRectPos.X() += 600 * nPageSign;
    aRectPos.Y() -= 1500;
    if ( aRectPos.Y() < 0 ) aRectPos.Y() = 0;

    String aText = GetText();
    long nDefWidth = ( aText.Len() > SC_NOTE_SMALLTEXT ) ? 5800 : 2900;
    Size aRectSize( nDefWidth, 1800 );

    if ( bNegativePage )
        aRectPos.X() -= aRectSize.Width();
    else
        if ( aRectPos.X() < 0 ) aRectPos.X() = 0;

    return Rectangle(aRectPos, aRectSize);
}

Rectangle ScPostIt::MimicOldRectangle(const ScAddress& rPos) const
{
    // Mimic the functionality prior to the support for note positioning:
    // The DefaultRectangle() is modified once it is inserted in the
    // DrawPage. The Bottom part of the rectangle is modified against
    // the text.  Thus using the DefaultRectangle() does not always return
    // the previous cell note appearance [see #i38350#] of sxc docs
    // containing notes created without a stored note position.
    Rectangle aDefaultRect(DefaultRectangle(rPos));
    SCCOL nNextCol = rPos.Col()+1;
    Point aTailPos = ScDetectiveFunc(mpDoc, rPos.Tab()).GetDrawPos( nNextCol, rPos.Row(), FALSE );

    SdrCaptionObj* pCaption = new SdrCaptionObj(aDefaultRect, aTailPos );
    InsertObject( pCaption, *mpDoc, rPos.Tab(), sal_False );
    pCaption->SetText( GetText() );
    Rectangle aRect = pCaption->GetLogicRect();
    if ( aRect.Bottom() > aDefaultRect.Bottom() )
    {
        long nDif = aRect.Bottom() - aDefaultRect.Bottom();
        aRect.Bottom() = aDefaultRect.Bottom();
        aRect.Top() = Max( aDefaultRect.Top(), static_cast<long>((aRect.Top() - nDif)) );
    }
    RemoveObject( pCaption,*mpDoc,rPos.Tab() );
    delete pCaption;
    return aRect;
}

void ScPostIt::SetRectangle(const Rectangle& aRect)
{
    maRectangle = aRect;
}

SfxItemSet ScPostIt::DefaultItemSet() const
{
    SfxItemSet  aCaptionSet( mpDoc->GetNoteItemPool(), SDRATTR_START,  SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END, 0,0);

    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);

    aCaptionSet.Put( XLineStartItem( EMPTY_STRING, basegfx::B2DPolyPolygon(aTriangle) ) );
    aCaptionSet.Put( XLineStartWidthItem( 200 ) );
    aCaptionSet.Put( XLineStartCenterItem( FALSE ) );
    aCaptionSet.Put( XFillStyleItem( XFILL_SOLID ) );
    Color aYellow( ScDetectiveFunc::GetCommentColor() );
    aCaptionSet.Put( XFillColorItem( String(), aYellow ) );

    //  shadow
    //  SdrShadowItem has FALSE, instead the shadow is set for the rectangle
    //  only with SetSpecialTextBoxShadow when the object is created
    //  (item must be set to adjust objects from older files)
    aCaptionSet.Put( SdrShadowItem( FALSE ) );
    aCaptionSet.Put( SdrShadowXDistItem( 100 ) );
    aCaptionSet.Put( SdrShadowYDistItem( 100 ) );

    //  text attributes
    aCaptionSet.Put( SdrTextLeftDistItem( 100 ) );
    aCaptionSet.Put( SdrTextRightDistItem( 100 ) );
    aCaptionSet.Put( SdrTextUpperDistItem( 100 ) );
    aCaptionSet.Put( SdrTextLowerDistItem( 100 ) );

    //  #78943# do use the default cell style, so the user has a chance to
    //  modify the font for the annotations
    ((const ScPatternAttr&)mpDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
    FillEditItemSet( &aCaptionSet );

    // support the best position for the tail connector now that
    // that notes can be resized and repositioned.
    aCaptionSet.Put( SdrCaptionEscDirItem( SDRCAPT_ESCBESTFIT) );

    return aCaptionSet;
}

void ScPostIt::SetItemSet(const SfxItemSet& rItemSet)
{
    maItemSet.PutExtended(rItemSet, SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
}

// Called from Excel import - Excel supports alignment on a per note
// basis while Calc uses a per paragraph alignment. Thus we need to
// apply the Note alignment to all paragraphs.
void ScPostIt::SetAndApplyItemSet(const SfxItemSet& rItemSet)
{
    SetItemSet(rItemSet);
    if(mpEditObj.get() && mpDoc)
    {
        ScNoteEditEngine& rEE = mpDoc->GetNoteEngine();
        rEE.SetText( *mpEditObj);
        sal_uInt16 nCount = mpEditObj.get()->GetParagraphCount();
        for( sal_uInt16 nPara = 0; nPara < nCount; ++nPara )
        {
            String aParaText( rEE.GetText( nPara ) );
            if( aParaText.Len() )
                rEE.SetParaAttribs(nPara, rItemSet);
        }
        mpEditObj.reset(rEE.CreateTextObject());
    }
}

void ScPostIt::InsertObject(SdrCaptionObj* pObj, ScDocument& rDoc, SCTAB nTab, sal_Bool bVisible) const
{
    SdrPage* pPage = NULL;
    ScDrawLayer* pDrawModel = rDoc.GetDrawLayer();
    // Copying note cells between documents is one user scenario where the
    // DrawLayer may not exist. Use the ScModelObj() to create
    // and ensure there is a DrawLayer.
    if(!pDrawModel)
    {
        ScModelObj* pScModelObj = NULL;
        SfxObjectShell* pDocShell = rDoc.GetDocumentShell();
        if(pDocShell)
            pScModelObj = ScModelObj::getImplementation( Reference< XModel >( pDocShell->GetModel() ) );
        if(pScModelObj)
            pDrawModel= pScModelObj->MakeDrawLayer();
        if(pDrawModel)
            pPage = pDrawModel->GetPage(nTab);
    }
    else
        pPage = pDrawModel->GetPage(nTab);

    if(pPage && pObj)
    {
        if (!bVisible)
        {
            pObj->NbcSetLayer(SC_LAYER_HIDDEN);
        }
        pPage->InsertObject(pObj);
    }
}

void ScPostIt::RemoveObject(SdrCaptionObj* pObj, ScDocument& rDoc, SCTAB nTab) const
{
    SdrPage* pPage = NULL;
    ScDrawLayer* pDrawModel = rDoc.GetDrawLayer();
    if(pDrawModel)
        pPage = pDrawModel->GetPage(nTab);
    if(pPage && pObj)
        pPage->RemoveObject(pObj->GetOrdNum());
}

//========================================================================
