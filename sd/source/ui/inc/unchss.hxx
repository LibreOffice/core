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

#ifndef _SD_UNCHSS_HXX
#define _SD_UNCHSS_HXX

#include "sdundo.hxx"

class SfxItemSet;
class SfxStyleSheet;
class SdDrawDocument;

class StyleSheetUndoAction : public SdUndoAction
{
    SfxStyleSheet*  pStyleSheet;

    SfxItemSet*     pNewSet;
    SfxItemSet*     pOldSet;
    String          aComment;

public:
    TYPEINFO();
    StyleSheetUndoAction(SdDrawDocument* pTheDoc,
                         SfxStyleSheet*  pTheStyleSheet,
                         const SfxItemSet* pTheNewItemSet);

    virtual ~StyleSheetUndoAction();
    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

#endif      // _SD_UNCHSS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
