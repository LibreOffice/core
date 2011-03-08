/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SELECTIONCONTROLLER_HXX
#define _SELECTIONCONTROLLER_HXX

#include "svx/svxdllapi.h"

#include <boost/shared_ptr.hpp>

#include <cppuhelper/weak.hxx>

class KeyEvent;
class MouseEvent;
class Window;
class SfxItemSet;
class SfxRequest;
class SfxStyleSheet;
class SdrPage;
class SdrModel;

namespace sdr
{

class SVX_DLLPUBLIC SelectionController: public cppu::OWeakObject
{
public:
    virtual bool onKeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual bool onMouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual bool onMouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual bool onMouseMove(const MouseEvent& rMEvt, Window* pWin);

    virtual void onSelectionHasChanged();

    virtual void GetState( SfxItemSet& rSet );
    virtual void Execute( SfxRequest& rReq );

    virtual bool DeleteMarked();

    virtual bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;
    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);

    virtual bool GetStyleSheet( SfxStyleSheet* &rpStyleSheet ) const;
    virtual bool SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr );

    virtual bool GetMarkedObjModel( SdrPage* pNewPage );
    virtual bool PasteObjModel( const SdrModel& rModel );

    /** returns a format paint brush set from the current selection */
    virtual bool TakeFormatPaintBrush( boost::shared_ptr< SfxItemSet >& rFormatSet  );

    /** applies a format paint brush set from the current selection.
        if bNoCharacterFormats is true, no character attributes are changed.
        if bNoParagraphFormats is true, no paragraph attributes are changed.
    */
    virtual bool ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats );
};

}

#endif //_SELECTIONCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
