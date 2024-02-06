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

#ifndef INCLUDED_SVX_SELECTIONCONTROLLER_HXX
#define INCLUDED_SVX_SELECTIONCONTROLLER_HXX

#include <svx/svxdllapi.h>
#include <cppuhelper/weak.hxx>
#include <rtl/ref.hxx>

class KeyEvent;
class MouseEvent;
namespace vcl { class Window; }
class SfxItemSet;
class SfxRequest;
class SfxStyleSheet;
class SdrModel;
class Point;
class FontList;
class SdrObject;

namespace sdr
{

namespace table { struct CellPos; }

class SVXCORE_DLLPUBLIC SelectionController: public cppu::OWeakObject
{
public:
    virtual bool onKeyInput(const KeyEvent& rKEvt, vcl::Window* pWin);
    virtual bool onMouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin);
    virtual bool onMouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin);
    virtual bool onMouseMove(const MouseEvent& rMEvt, vcl::Window* pWin);

    virtual void onSelectionHasChanged();

    virtual void onSelectAll();

    virtual void GetState( SfxItemSet& rSet );
    virtual void Execute( SfxRequest& rReq );

    virtual bool DeleteMarked();

    virtual bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;
    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);

    virtual bool GetStyleSheet( SfxStyleSheet* &rpStyleSheet ) const;
    virtual bool SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr );

    virtual rtl::Reference<SdrObject> GetMarkedSdrObjClone( SdrModel& rTargetModel );
    virtual bool PasteObjModel( const SdrModel& rModel );

    /** applies a format paint brush set from the current selection.
        if bNoCharacterFormats is true, no character attributes are changed.
        if bNoParagraphFormats is true, no paragraph attributes are changed.
    */
    virtual bool ApplyFormatPaintBrush( SfxItemSet& rFormatSet, sal_Int16 nDepth, bool bNoCharacterFormats, bool bNoParagraphFormats );
    /// This is a table object, and one or more of its cells are selected.
    virtual bool hasSelectedCells() const;
    /// Allows adjusting the point or mark of the selection to a document coordinate.
    virtual bool setCursorLogicPosition(const Point& rPosition, bool bPoint);
    /// Get the position of the first and the last selected cell.
    virtual void getSelectedCells(table::CellPos& rFirstPos, table::CellPos& rLastPos);
    /// Changes the font (grow/shrink) according to the input parameters.
    virtual bool ChangeFontSize(bool bGrow, const FontList* pFontList);
};

}

#endif // INCLUDED_SVX_SELECTIONCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
