/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "notesuno.hxx"

#include <svl/smplhint.hxx>
#include <editeng/unotext.hxx>
#include <svx/globaldrawitempool.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdocapt.hxx>

#include "postit.hxx"
#include "cellsuno.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "miscuno.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  keine Properties fuer Text in Notizen
const SvxItemPropertySet* lcl_GetAnnotationPropertySet()
{
    static SfxItemPropertyMapEntry aAnnotationPropertyMap_Impl[] =
    {
        {0,0,0,0,0,0}
    };
    static SvxItemPropertySet aAnnotationPropertySet_Impl( aAnnotationPropertyMap_Impl, GetGlobalDrawObjectItemPool() );
    return &aAnnotationPropertySet_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScAnnotationObj, "ScAnnotationObj", "com.sun.star.sheet.CellAnnotation" )
//SC_SIMPLE_SERVICE_INFO( ScAnnotationShapeObj, "ScAnnotationShapeObj", "com.sun.star.sheet.CellAnnotationShape" )

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

ScAnnotationObj::~ScAnnotationObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    if (pUnoText)
        pUnoText->release();
}

void ScAnnotationObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const ScUpdateRefHint* pScUpdateRefHint = dynamic_cast< const ScUpdateRefHint* >(&rHint);

    if ( pScUpdateRefHint )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else
    {
        const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

        if ( pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId() )
        {
            pDocShell = NULL;       // ungueltig geworden
        }
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
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetAuthor() : rtl::OUString();
}

rtl::OUString SAL_CALL ScAnnotationObj::getDate() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetDate() : rtl::OUString();
}

sal_Bool SAL_CALL ScAnnotationObj::getIsVisible() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote && pNote->IsCaptionShown();
}

void SAL_CALL ScAnnotationObj::setIsVisible( sal_Bool bIsVisible ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    // show/hide note with undo action
    if( pDocShell )
        pDocShell->GetDocFunc().ShowNote( aCellPos, bIsVisible );
}

// XSheetAnnotationShapeSupplier
uno::Reference < drawing::XShape > SAL_CALL ScAnnotationObj::getAnnotationShape()
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < drawing::XShape > xShape;
    if( const ScPostIt* pNote = ImplGetNote() )
        if( SdrObject* pCaption = pNote->GetOrCreateCaption( aCellPos ) )
            xShape.set( pCaption->getUnoShape(), uno::UNO_QUERY );
    return xShape;
}

SvxUnoText& ScAnnotationObj::GetUnoText()
{
    if (!pUnoText)
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertySet(),
                                    uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
    return *pUnoText;
}

const ScPostIt* ScAnnotationObj::ImplGetNote() const
{
    return pDocShell ? pDocShell->GetDocument()->GetNote( aCellPos ) : 0;
}

//------------------------------------------------------------------------
