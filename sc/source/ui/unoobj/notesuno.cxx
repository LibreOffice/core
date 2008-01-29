/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: notesuno.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:45:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif


#include "rangelst.hxx"
#include <svx/unotext.hxx>

#include "notesuno.hxx"
#include "textuno.hxx"
#include "cellsuno.hxx"     // getParent
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "miscuno.hxx"

// setVisible:
#include <svx/svdundo.hxx>
#include "drwlayer.hxx"
#include "detfunc.hxx"
#include "undocell.hxx"
#include "unoguard.hxx"
#include "userdat.hxx"

#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#include <svx/svditer.hxx>

#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------------

//  keine Properties fuer Text in Notizen
const SfxItemPropertyMap* lcl_GetAnnotationPropertyMap()
{
    static SfxItemPropertyMap aAnnotationPropertyMap_Impl[] =
    {
        {0,0,0,0,0,0}
    };
    return aAnnotationPropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScAnnotationObj, "ScAnnotationObj", "com.sun.star.sheet.CellAnnotation" )
SC_SIMPLE_SERVICE_INFO( ScAnnotationShapeObj, "ScAnnotationShapeObj", "com.sun.star.sheet.CellAnnotationShape" )

//------------------------------------------------------------------------

ScAnnotationObj::ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    pUnoText( NULL )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

SvxUnoText& ScAnnotationObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertyMap(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

ScAnnotationObj::~ScAnnotationObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    if (pUnoText)
        pUnoText->release();
}

void ScAnnotationObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}


// XChild

uno::Reference<uno::XInterface> SAL_CALL ScAnnotationObj::getParent() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  Parent der Notiz ist die zugehoerige Zelle
    //! existierendes Objekt finden und zurueckgeben ???

    if (pDocShell)
        return (cppu::OWeakObject*)new ScCellObj( pDocShell, aCellPos );

    return NULL;
}

void SAL_CALL ScAnnotationObj::setParent( const uno::Reference<uno::XInterface>& /* Parent */ )
                                    throw(lang::NoSupportException, uno::RuntimeException)
{
    //  hamma nich
    //! Exception oder so ??!
}

// XSimpleText

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

rtl::OUString SAL_CALL ScAnnotationObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getString();
}

void SAL_CALL ScAnnotationObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().setString(aText);
}

void SAL_CALL ScAnnotationObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScAnnotationObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

uno::Reference<text::XText> SAL_CALL ScAnnotationObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getEnd();
}

// XSheetAnnotation

table::CellAddress SAL_CALL ScAnnotationObj::getPosition() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet  = aCellPos.Tab();
    aAdr.Column = aCellPos.Col();
    aAdr.Row    = aCellPos.Row();
    return aAdr;
}

rtl::OUString SAL_CALL ScAnnotationObj::getAuthor() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
        String aAuthor;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
            ScPostIt aNote(pDoc);
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
                aAuthor = aNote.GetAuthor();
    }
    return aAuthor;
}

rtl::OUString SAL_CALL ScAnnotationObj::getDate() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
        String aDate;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
            ScPostIt aNote(pDoc);
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
                aDate = aNote.GetDate();
    }
    return aDate;
}

sal_Bool SAL_CALL ScAnnotationObj::getIsVisible() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bShown = FALSE;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
            ScPostIt aNote(pDoc);
        pDoc->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote );
                bShown = aNote.IsShown();
    }
    return bShown;
}

void SAL_CALL ScAnnotationObj::setIsVisible( sal_Bool bIsVisible ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    BOOL bDone = FALSE;
    if ( pDocShell )
    {
        //! Funktion an docfunc oder so

        BOOL bSet = bIsVisible ? TRUE : FALSE;
        ScDocument* pDoc = pDocShell->GetDocument();
        BOOL bUndo(pDoc->IsUndoEnabled());
        SCCOL nCol = aCellPos.Col();
        SCROW nRow = aCellPos.Row();
        SCTAB nTab = aCellPos.Tab();
        ScPostIt aNote(pDoc);
        if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) )
        {
            BOOL bHad = pDoc->HasNoteObject( nCol, nRow, nTab );
            if ( bHad != bSet )
            {
                pDocShell->MakeDrawLayer();
                ScDrawLayer* pModel = pDoc->GetDrawLayer();

                if (bUndo)
                    pModel->BeginCalcUndo();
                ScDetectiveFunc aFunc( pDoc,nTab );
                if ( bSet )
                    bDone = ( aFunc.ShowComment( nCol, nRow, FALSE ) != NULL );
                else
                    bDone = aFunc.HideComment( nCol, nRow );
                SdrUndoGroup* pUndo = NULL;
                if (bUndo)
                    pUndo = pModel->GetCalcUndo();
                if (bDone)
                {
                    aNote.SetShown( bSet );
                    pDoc->SetNote( nCol, nRow, nTab, aNote );
                    if (pUndo)
                        pDocShell->GetUndoManager()->AddUndoAction(
                            new ScUndoNote( pDocShell, bSet, aCellPos, pUndo ) );

                    pDocShell->SetDocumentModified();
                }
                else
                    delete pUndo;
            }
        }
    }
}

// XSheetAnnotationShapeSupplier
uno::Reference < drawing::XShape > SAL_CALL ScAnnotationObj::getAnnotationShape()
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScAnnotationShapeObj(pDocShell, aCellPos);
}

//------------------------------------------------------------------------

ScAnnotationShapeObj::ScAnnotationShapeObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    pUnoText( NULL )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

SvxUnoText& ScAnnotationShapeObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertyMap(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

uno::Reference < drawing::XShape > ScAnnotationShapeObj::GetXShape()
{
    if (!xShape.is())
    {
        ScPostIt aNote(pDocShell->GetDocument());
        if ( pDocShell->GetDocument()->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote ) )
        {
            const SfxItemSet& rSet = aNote.GetItemSet();

            // In order to transform the SfxItemSet to an EscherPropertyContainer
            // and export the properties, we need to recreate the drawing object and
            // pass this to XclObjComment() for processing.
            SdrCaptionObj* pCaption = new SdrCaptionObj( aNote.GetRectangle() );
            pCaption->SetMergedItemSet(rSet);

            if(const EditTextObject* pEditText = aNote.GetEditTextObject())
            {
                OutlinerParaObject* pOPO = new OutlinerParaObject( *pEditText );
                pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
                pCaption->NbcSetOutlinerParaObject( pOPO );
                pOPO->SetVertical( FALSE );  // notes are always horizontal
            }

            aNote.InsertObject(pCaption, *pDocShell->GetDocument(), aCellPos.Tab(), sal_False);

            xShape.set(pCaption->getUnoShape(), uno::UNO_QUERY);
        }
    }
    return xShape;
}

SdrObject* ScAnnotationShapeObj::GetCaptionObj()
{
    SdrObject* pRet = NULL;

    ScDrawLayer* pModel = pDocShell->GetDocument()->GetDrawLayer();
    if (!pModel)
        return FALSE;
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(aCellPos.Tab()));
    DBG_ASSERT(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !pRet)
    {
        if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
        {
            ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
            if ( pData && aCellPos.Col() == pData->aStt.Col() && aCellPos.Row() == pData->aStt.Row() )
            {
                pRet = pObject;
            }
        }

        pObject = aIter.Next();
    }

    return pRet;
}

void ScAnnotationShapeObj::UpdateData()
{
    if (xShape.is())
    {
        SvxShape* pShapeImp = SvxShape::getImplementation(xShape);
        if (pShapeImp)
        {
            SdrObject *pSdrObj = pShapeImp->GetSdrObject();
            if (pSdrObj)
            {
                ScPostIt aNote(pDocShell->GetDocument());
                if ( pDocShell->GetDocument()->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote ) )
                {
                    aNote.SetItemSet(pSdrObj->GetMergedItemSet());
                    awt::Point aPos = xShape->getPosition();
                    awt::Size aSize = xShape->getSize();
                    Rectangle aRect(Point(aPos.X, aPos.Y), Size(aSize.Width, aSize.Height));
                    aNote.SetRectangle(aRect);

                    pDocShell->GetDocument()->SetNote(aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote);

                    if (aNote.IsShown())
                    {
                        SdrObject* pObj = GetCaptionObj();
                        if (pObj)
                        {
                            uno::Reference<drawing::XShape> xCaptionShape(pObj->getUnoShape(), uno::UNO_QUERY);
                            if (xCaptionShape.is())
                            {
                                xCaptionShape->setSize(aSize);
                                xCaptionShape->setPosition(aPos);
                            }
                            pObj->SetMergedItemSet(aNote.GetItemSet());
                            pObj->ActionChanged();
                        }
                    // not neccessary, because it should not be possible to change the text in the shape directly
//                    if((pSdrObj->GetOutlinerParaObject()))
//                        pMyAnnotation->pOPO = new OutlinerParaObject( *(pSdrObj->GetOutlinerParaObject()) );
                    }
                }
            }
        }
    }
}

ScAnnotationShapeObj::~ScAnnotationShapeObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    if (xShape.is() && pDocShell)
    {
        SvxShape* pShapeImp = SvxShape::getImplementation(xShape);
        if (pShapeImp)
        {
            SdrObject *pSdrObj = pShapeImp->GetSdrObject();
            if (pSdrObj)
            {
                if (pSdrObj->ISA( SdrCaptionObj ))
                {
                    ScPostIt aNote(pDocShell->GetDocument());
                    if ( pDocShell->GetDocument()->GetNote( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), aNote ) )
                    {
                        aNote.RemoveObject(static_cast< SdrCaptionObj* >(pSdrObj), *pDocShell->GetDocument(), aCellPos.Tab());
                    }
                }
                else
                {
                    DBG_ERROR("should be a Caption Shape");
                }
            }
        }
    }

    if (pUnoText)
        pUnoText->release();
}

void ScAnnotationShapeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}


// XChild

uno::Reference<uno::XInterface> SAL_CALL ScAnnotationShapeObj::getParent() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  Parent der Notiz ist die zugehoerige Zelle
    //! existierendes Objekt finden und zurueckgeben ???

    if (pDocShell)
        return (cppu::OWeakObject*)new ScCellObj( pDocShell, aCellPos );

    return NULL;
}

void SAL_CALL ScAnnotationShapeObj::setParent( const uno::Reference<uno::XInterface>& /* Parent */ )
                                    throw(lang::NoSupportException, uno::RuntimeException)
{
    //  hamma nich
    //! Exception oder so ??!
}

// XElementAccess
uno::Type SAL_CALL ScAnnotationShapeObj::getElementType(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return GetUnoText().getElementType();
}

sal_Bool SAL_CALL ScAnnotationShapeObj::hasElements(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return GetUnoText().hasElements();
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL ScAnnotationShapeObj::createEnumeration(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    return GetUnoText().createEnumeration();
}

// XTextRangeMover
void SAL_CALL ScAnnotationShapeObj::moveTextRange( const uno::Reference< text::XTextRange >& xRange, sal_Int16 nParagraphs )
                                throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    GetUnoText().moveTextRange( xRange, nParagraphs );
}

// XText
void SAL_CALL ScAnnotationShapeObj::insertTextContent( const uno::Reference< text::XTextRange >& xRange,
                                    const uno::Reference< text::XTextContent >& xContent, sal_Bool bAbsorb )
                                    throw (lang::IllegalArgumentException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    GetUnoText().insertTextContent( xRange, xContent, bAbsorb );
}

void SAL_CALL ScAnnotationShapeObj::removeTextContent( const uno::Reference< text::XTextContent >& xContent )
                                    throw (container::NoSuchElementException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    GetUnoText().removeTextContent( xContent );
}

// XSimpleText

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationShapeObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationShapeObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Notizen brauchen keine Extrawurst
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

rtl::OUString SAL_CALL ScAnnotationShapeObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getString();
}

void SAL_CALL ScAnnotationShapeObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().setString(aText);
}

void SAL_CALL ScAnnotationShapeObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScAnnotationShapeObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetUnoText().insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

uno::Reference<text::XText> SAL_CALL ScAnnotationShapeObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationShapeObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationShapeObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetUnoText().getEnd();
}

// XShapeDescriptor
::rtl::OUString SAL_CALL ScAnnotationShapeObj::getShapeType(  )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < drawing::XShapeDescriptor > xDesc(GetXShape(), uno::UNO_QUERY);
    if (xDesc.is())
        return xDesc->getShapeType();
    return rtl::OUString();
}

// XShape
awt::Point SAL_CALL ScAnnotationShapeObj::getPosition(  )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetXShape()->getPosition();
}

void SAL_CALL ScAnnotationShapeObj::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetXShape()->setPosition(aPosition);
    UpdateData();
}

awt::Size SAL_CALL ScAnnotationShapeObj::getSize(  )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetXShape()->getSize();
}

void SAL_CALL ScAnnotationShapeObj::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    GetXShape()->setSize(aSize);
}

// XPropertyState
beans::PropertyState SAL_CALL ScAnnotationShapeObj::getPropertyState( const rtl::OUString& PropertyName )
                                    throw (beans::UnknownPropertyException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        return xState->getPropertyState( PropertyName );
    return beans::PropertyState();
}

uno::Sequence< beans::PropertyState > SAL_CALL ScAnnotationShapeObj::getPropertyStates(
                                    const uno::Sequence< rtl::OUString >& aPropertyName )
                                    throw (beans::UnknownPropertyException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        return xState->getPropertyStates( aPropertyName );
    return uno::Sequence< beans::PropertyState >();
}

void SAL_CALL ScAnnotationShapeObj::setPropertyToDefault( const ::rtl::OUString& PropertyName )
                                    throw (::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
    {
        xState->setPropertyToDefault( PropertyName );
        UpdateData();
    }
}

uno::Any SAL_CALL ScAnnotationShapeObj::getPropertyDefault( const rtl::OUString& aPropertyName )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertyState > xState (GetXShape(), uno::UNO_QUERY);
    if (xState.is())
        return xState->getPropertyDefault( aPropertyName );
    return uno::Any();
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ScAnnotationShapeObj::getPropertySetInfo(  )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->getPropertySetInfo();
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL ScAnnotationShapeObj::setPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
                                    throw (beans::UnknownPropertyException,
                                        beans::PropertyVetoException,
                                        lang::IllegalArgumentException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
    {
        xProp->setPropertyValue( aPropertyName, aValue );
        UpdateData();
    }
}

uno::Any SAL_CALL ScAnnotationShapeObj::getPropertyValue( const rtl::OUString& PropertyName )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->getPropertyValue( PropertyName );
    return uno::Any();
}

void SAL_CALL ScAnnotationShapeObj::addPropertyChangeListener( const rtl::OUString& aPropertyName,
                                    const uno::Reference< beans::XPropertyChangeListener >& xListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL ScAnnotationShapeObj::removePropertyChangeListener( const rtl::OUString& aPropertyName,
                                    const uno::Reference< beans::XPropertyChangeListener >& aListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScAnnotationShapeObj::addVetoableChangeListener( const rtl::OUString& PropertyName,
                                    const uno::Reference< beans::XVetoableChangeListener >& aListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL ScAnnotationShapeObj::removeVetoableChangeListener( const rtl::OUString& PropertyName,
                                    const uno::Reference< beans::XVetoableChangeListener >& aListener )
                                    throw (beans::UnknownPropertyException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->removeVetoableChangeListener( PropertyName, aListener );
}

                            // XMultiPropertySet
void SAL_CALL ScAnnotationShapeObj::setPropertyValues( const uno::Sequence< rtl::OUString >& aPropertyNames,
                                    const uno::Sequence< uno::Any >& aValues )
                                    throw (beans::PropertyVetoException,
                                        lang::IllegalArgumentException,
                                        lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
    {
        xProp->setPropertyValues( aPropertyNames, aValues );
        UpdateData();
    }
}

uno::Sequence< uno::Any > SAL_CALL ScAnnotationShapeObj::getPropertyValues(
                                    const uno::Sequence< rtl::OUString >& aPropertyNames )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        return xProp->getPropertyValues( aPropertyNames );
    return uno::Sequence< uno::Any >();
}

void SAL_CALL ScAnnotationShapeObj::addPropertiesChangeListener( const uno::Sequence< rtl::OUString >& aPropertyNames,
                                    const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->addPropertiesChangeListener( aPropertyNames, xListener );
}

void SAL_CALL ScAnnotationShapeObj::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->removePropertiesChangeListener( xListener );
}

void SAL_CALL ScAnnotationShapeObj::firePropertiesChangeEvent( const uno::Sequence< rtl::OUString >& aPropertyNames,
                                    const uno::Reference< beans::XPropertiesChangeListener >& xListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < beans::XMultiPropertySet > xProp (GetXShape(), uno::UNO_QUERY);
    if (xProp.is())
        xProp->firePropertiesChangeEvent( aPropertyNames, xListener );
}

                            // XComponent
void SAL_CALL ScAnnotationShapeObj::dispose(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < lang::XComponent > xComp (GetXShape(), uno::UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    if (xShape.is())
        xShape.clear();
}

void SAL_CALL ScAnnotationShapeObj::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < lang::XComponent > xComp (GetXShape(), uno::UNO_QUERY);
    if (xComp.is())
        xComp->addEventListener( xListener );
}

void SAL_CALL ScAnnotationShapeObj::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < lang::XComponent > xComp (GetXShape(), uno::UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener( aListener );
}

//------------------------------------------------------------------------




