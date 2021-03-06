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

#pragma once

#include <tools/gen.hxx>
#include <vcl/prntypes.hxx>

#include <sdundo.hxx>

class SdDrawDocument;
class SdPage;

/************************************************************************/

class SdPageFormatUndoAction : public SdUndoAction
{
    SdPage*     mpPage;

    Size        maOldSize;
    sal_Int32       mnOldLeft;
    sal_Int32       mnOldRight;
    sal_Int32       mnOldUpper;
    sal_Int32       mnOldLower;
    Orientation meOldOrientation;
    sal_uInt16      mnOldPaperBin;
    bool        mbOldFullSize;

    Size        maNewSize;
    sal_Int32       mnNewLeft;
    sal_Int32       mnNewRight;
    sal_Int32       mnNewUpper;
    sal_Int32       mnNewLower;
    bool        mbNewScale;
    Orientation meNewOrientation;
    sal_uInt16      mnNewPaperBin;
    bool        mbNewFullSize;

public:
    SdPageFormatUndoAction( SdDrawDocument* pDoc,
                            SdPage*         pThePage,
                            const Size&     rOldSz,
                            sal_Int32           nOldLft,
                            sal_Int32           nOldRgt,
                            sal_Int32           nOldUpr,
                            sal_Int32           nOldLwr,
                            Orientation     eOldOrient,
                            sal_uInt16          nOPaperBin,
                            bool            bOFullSize,

                            const Size&     rNewSz,
                            sal_Int32           nNewLft,
                            sal_Int32           nNewRgt,
                            sal_Int32           nNewUpr,
                            sal_Int32           nNewLwr,
                            bool            bNewScl,
                            Orientation     eNewOrient,
                            sal_uInt16          nNPaperBin,
                            bool            bNFullSize
                            ) :
        SdUndoAction(pDoc),
        mpPage      (pThePage),
        maOldSize   (rOldSz),
        mnOldLeft   (nOldLft),
        mnOldRight  (nOldRgt),
        mnOldUpper  (nOldUpr),
        mnOldLower  (nOldLwr),
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
    virtual ~SdPageFormatUndoAction() override;

    virtual void Undo() override;
    virtual void Redo() override;
};

/************************************************************************/

class SdPageLRUndoAction : public SdUndoAction
{
    SdPage* mpPage;

    sal_Int32   mnOldLeft;
    sal_Int32   mnOldRight;
    sal_Int32   mnNewLeft;
    sal_Int32   mnNewRight;

public:
    SdPageLRUndoAction( SdDrawDocument* pDoc, SdPage* pThePage,
                        sal_Int32 nOldLft, sal_Int32 nOldRgt,
                        sal_Int32 nNewLft, sal_Int32 nNewRgt ) :
        SdUndoAction(pDoc),
        mpPage      (pThePage),
        mnOldLeft   (nOldLft),
        mnOldRight  (nOldRgt),
        mnNewLeft   (nNewLft),
        mnNewRight  (nNewRgt)
        {}
    virtual ~SdPageLRUndoAction() override;

    virtual void Undo() override;
    virtual void Redo() override;
};

/************************************************************************/

class SdPageULUndoAction : public SdUndoAction
{
    SdPage* mpPage;

    sal_Int32   mnOldUpper;
    sal_Int32   mnOldLower;
    sal_Int32   mnNewUpper;
    sal_Int32   mnNewLower;

public:
    SdPageULUndoAction( SdDrawDocument* pDoc, SdPage* pThePage,
                        sal_Int32 nOldUpr, sal_Int32 nOldLwr,
                        sal_Int32 nNewUpr, sal_Int32 nNewLwr ) :
        SdUndoAction(pDoc),
        mpPage      (pThePage),
        mnOldUpper  (nOldUpr),
        mnOldLower  (nOldLwr),
        mnNewUpper  (nNewUpr),
        mnNewLower  (nNewLwr)
        {}
    virtual ~SdPageULUndoAction() override;

    virtual void Undo() override;
    virtual void Redo() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
