/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <notesuno.hxx>

#include <vcl/svapp.hxx>
#include <svl/hint.hxx>
#include <editeng/unotext.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdocapt.hxx>

#include <postit.hxx>
#include <cellsuno.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <hints.hxx>
#include <editsrc.hxx>
#include <miscuno.hxx>

using namespace com::sun::star;

static const SvxItemPropertySet* lcl_GetAnnotationPropertySet()
{
    static const SfxItemPropertyMapEntry aAnnotationPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_NUMBERING_PROPERTIE,    // for completeness of service ParagraphProperties
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aAnnotationPropertySet_Impl( aAnnotationPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aAnnotationPropertySet_Impl;
}

SC_SIMPLE_SERVICE_INFO( ScAnnotationObj, "ScAnnotationObj", "com.sun.star.sheet.CellAnnotation" )
//SC_SIMPLE_SERVICE_INFO( ScAnnotationShapeObj, "ScAnnotationShapeObj", "com.sun.star.sheet.CellAnnotationShape" )

ScAnnotationObj::ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos )
{
    pDocShell->GetDocument().AddUnoObject(*this);

    //  pUnoText is allocated on demand (GetUnoText)
    //  can't be aggregated because getString/setString is handled here
}

ScAnnotationObj::~ScAnnotationObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScAnnotationObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XChild

uno::Reference<uno::XInterface> SAL_CALL ScAnnotationObj::getParent()
{
    SolarMutexGuard aGuard;

    //  parent of note is the related cell
    //! find and reset existing object ???

    if (pDocShell)
        return static_cast<cppu::OWeakObject*>(new ScCellObj( pDocShell, aCellPos ));

    return nullptr;
}

void SAL_CALL ScAnnotationObj::setParent( const uno::Reference<uno::XInterface>& /* Parent */ )
{
    //  ain't there
    //! exception or what ??!
}

// XSimpleText

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursor()
{
    SolarMutexGuard aGuard;
    //  notes does not need special treatment
    return GetUnoText().createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL ScAnnotationObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
{
    SolarMutexGuard aGuard;
    //  notes does not need special treatment
    return GetUnoText().createTextCursorByRange(aTextPosition);
}

OUString SAL_CALL ScAnnotationObj::getString()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getString();
}

void SAL_CALL ScAnnotationObj::setString( const OUString& aText )
{
    SolarMutexGuard aGuard;
    GetUnoText().setString(aText);
}

void SAL_CALL ScAnnotationObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const OUString& aString, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;
    GetUnoText().insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScAnnotationObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;
    GetUnoText().insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

uno::Reference<text::XText> SAL_CALL ScAnnotationObj::getText()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getStart()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScAnnotationObj::getEnd()
{
    SolarMutexGuard aGuard;
    return GetUnoText().getEnd();
}

// XSheetAnnotation

table::CellAddress SAL_CALL ScAnnotationObj::getPosition()
{
    SolarMutexGuard aGuard;
    table::CellAddress aAdr;
    aAdr.Sheet  = aCellPos.Tab();
    aAdr.Column = aCellPos.Col();
    aAdr.Row    = aCellPos.Row();
    return aAdr;
}

OUString SAL_CALL ScAnnotationObj::getAuthor()
{
    SolarMutexGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetAuthor() : OUString();
}

OUString SAL_CALL ScAnnotationObj::getDate()
{
    SolarMutexGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote ? pNote->GetDate() : OUString();
}

sal_Bool SAL_CALL ScAnnotationObj::getIsVisible()
{
    SolarMutexGuard aGuard;
    const ScPostIt* pNote = ImplGetNote();
    return pNote && pNote->IsCaptionShown();
}

void SAL_CALL ScAnnotationObj::setIsVisible( sal_Bool bIsVisible )
{
    SolarMutexGuard aGuard;
    // show/hide note with undo action
    if( pDocShell )
        pDocShell->GetDocFunc().ShowNote( aCellPos, bIsVisible );
}

// XSheetAnnotationShapeSupplier
uno::Reference < drawing::XShape > SAL_CALL ScAnnotationObj::getAnnotationShape()
{
    SolarMutexGuard aGuard;
    uno::Reference < drawing::XShape > xShape;
    if( const ScPostIt* pNote = ImplGetNote() )
        if( SdrObject* pCaption = pNote->GetOrCreateCaption( aCellPos ) )
            xShape.set( pCaption->getUnoShape(), uno::UNO_QUERY );
    return xShape;
}

SvxUnoText& ScAnnotationObj::GetUnoText()
{
    if (!pUnoText.is())
    {
        ScAnnotationEditSource aEditSource( pDocShell, aCellPos );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetAnnotationPropertySet(),
                                    uno::Reference<text::XText>() );
    }
    return *pUnoText;
}

const ScPostIt* ScAnnotationObj::ImplGetNote() const
{
    return pDocShell ? pDocShell->GetDocument().GetNote(aCellPos) : nullptr;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
