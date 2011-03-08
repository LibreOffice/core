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

#ifndef _SD_UNDOPAGE_HXX
#define _SD_UNDOPAGE_HXX

#include <tools/gen.hxx>
#include <vcl/prntypes.hxx>

#include "sdundo.hxx"

class SdDrawDocument;
class SdPage;

/************************************************************************/

class SdPageFormatUndoAction : public SdUndoAction
{
    SdPage*     mpPage;

    Size        maOldSize;
    INT32       mnOldLeft;
    INT32       mnOldRight;
    INT32       mnOldUpper;
    INT32       mnOldLower;
    BOOL        mbOldScale;
    Orientation meOldOrientation;
    USHORT      mnOldPaperBin;
    BOOL        mbOldFullSize;

    Size        maNewSize;
    INT32       mnNewLeft;
    INT32       mnNewRight;
    INT32       mnNewUpper;
    INT32       mnNewLower;
    BOOL        mbNewScale;
    Orientation meNewOrientation;
    USHORT      mnNewPaperBin;
    BOOL        mbNewFullSize;

public:
    TYPEINFO();
    SdPageFormatUndoAction( SdDrawDocument* pDoc,
                            SdPage*         pThePage,
                            const Size&     rOldSz,
                            INT32           nOldLft,
                            INT32           nOldRgt,
                            INT32           nOldUpr,
                            INT32           nOldLwr,
                            BOOL            bOldScl,
                            Orientation     eOldOrient,
                            USHORT          nOPaperBin,
                            BOOL            bOFullSize,

                            const Size&     rNewSz,
                            INT32           nNewLft,
                            INT32           nNewRgt,
                            INT32           nNewUpr,
                            INT32           nNewLwr,
                            BOOL            bNewScl,
                            Orientation     eNewOrient,
                            USHORT          nNPaperBin,
                            BOOL            bNFullSize
                            ) :
        SdUndoAction(pDoc),
        mpPage      (pThePage),
        maOldSize   (rOldSz),
        mnOldLeft   (nOldLft),
        mnOldRight  (nOldRgt),
        mnOldUpper  (nOldUpr),
        mnOldLower  (nOldLwr),
        mbOldScale   (bOldScl),
        meOldOrientation(eOldOrient),
        mnOldPaperBin (nOPaperBin),
        mbOldFullSize (bOFullSize),


        maNewSize   (rNewSz),
        mnNewLeft   (nNewLft),
        mnNewRight  (nNewRgt),
        mnNewUpper  (nNewUpr),
        mnNewLower   (nNewLwr),
        mbNewScale   (bNewScl),
        meNewOrientation(eNewOrient),
        mnNewPaperBin (nNPaperBin),
        mbNewFullSize (bNFullSize)

        {}
    virtual ~SdPageFormatUndoAction();

    virtual void Undo();
    virtual void Redo();
};

/************************************************************************/

class SdPageLRUndoAction : public SdUndoAction
{
    SdPage* mpPage;

    INT32   mnOldLeft;
    INT32   mnOldRight;
    INT32   mnNewLeft;
    INT32   mnNewRight;

public:
    TYPEINFO();
    SdPageLRUndoAction( SdDrawDocument* pDoc, SdPage* pThePage,
                        INT32 nOldLft, INT32 nOldRgt,
                        INT32 nNewLft, INT32 nNewRgt ) :
        SdUndoAction(pDoc),
        mpPage      (pThePage),
        mnOldLeft   (nOldLft),
        mnOldRight  (nOldRgt),
        mnNewLeft   (nNewLft),
        mnNewRight  (nNewRgt)
        {}
    virtual ~SdPageLRUndoAction();

    virtual void Undo();
    virtual void Redo();
};

/************************************************************************/

class SdPageULUndoAction : public SdUndoAction
{
    SdPage* mpPage;

    INT32   mnOldUpper;
    INT32   mnOldLower;
    INT32   mnNewUpper;
    INT32   mnNewLower;

public:
    TYPEINFO();
    SdPageULUndoAction( SdDrawDocument* pDoc, SdPage* pThePage,
                        INT32 nOldUpr, INT32 nOldLwr,
                        INT32 nNewUpr, INT32 nNewLwr ) :
        SdUndoAction(pDoc),
        mpPage      (pThePage),
        mnOldUpper  (nOldUpr),
        mnOldLower  (nOldLwr),
        mnNewUpper  (nNewUpr),
        mnNewLower  (nNewLwr)
        {}
    virtual ~SdPageULUndoAction();

    virtual void Undo();
    virtual void Redo();
};



#endif      // _SD_UNDOPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
