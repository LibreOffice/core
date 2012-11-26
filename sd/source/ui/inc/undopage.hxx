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



#ifndef _SD_UNDOPAGE_HXX
#define _SD_UNDOPAGE_HXX

#include <tools/gen.hxx>
#include <vcl/prntypes.hxx>
#include "sdundo.hxx"
#include <basegfx/vector/b2dvector.hxx>

class SdDrawDocument;
class SdPage;

/************************************************************************/

class SdPageFormatUndoAction : public SdUndoAction
{
    SdPage*     mpPage;

    basegfx::B2DVector  maOldSize;
    double              mfOldLeft;
    double              mfOldRight;
    double              mfOldTop;
    double              mfOldBottom;
    bool                mbOldScale;
    Orientation meOldOrientation;
    sal_uInt16      mnOldPaperBin;
    bool                mbOldFullSize;

    basegfx::B2DVector  maNewSize;
    double              mfNewLeft;
    double              mfNewRight;
    double              mfNewTop;
    double              mfNewBottom;
    bool                mbNewScale;
    Orientation meNewOrientation;
    sal_uInt16      mnNewPaperBin;
    bool                mbNewFullSize;

public:
    SdPageFormatUndoAction(
        SdDrawDocument* pDoc,
                            SdPage*         pThePage,
        const basegfx::B2DVector& rOldSz,
        double fOldLeft,
        double fOldRight,
        double fOldTop,
        double fOldBottom,
        bool bOldScl,
                            Orientation     eOldOrient,
                            sal_uInt16          nOPaperBin,
        bool bOFullSize,

        const basegfx::B2DVector& rNewSz,
        double fNewLeft,
        double fNewRight,
        double fNewTop,
        double fNewBottom,
        bool bNewScl,
                            Orientation     eNewOrient,
                            sal_uInt16          nNPaperBin,
        bool bNFullSize)

    :   SdUndoAction(pDoc),
        mpPage      (pThePage),
        maOldSize   (rOldSz),
        mfOldLeft(fOldLeft),
        mfOldRight(fOldRight),
        mfOldTop(fOldTop),
        mfOldBottom(fOldBottom),
        mbOldScale   (bOldScl),
        meOldOrientation(eOldOrient),
        mnOldPaperBin (nOPaperBin),
        mbOldFullSize (bOFullSize),

        maNewSize   (rNewSz),
        mfNewLeft(fNewLeft),
        mfNewRight(fNewRight),
        mfNewTop(fNewTop),
        mfNewBottom(fNewBottom),
        mbNewScale   (bNewScl),
        meNewOrientation(eNewOrient),
        mnNewPaperBin (nNPaperBin),
        mbNewFullSize (bNFullSize)
    {
    }

    virtual ~SdPageFormatUndoAction();

    virtual void Undo();
    virtual void Redo();
};

/************************************************************************/

class SdPageLRUndoAction : public SdUndoAction
{
    SdPage* mpPage;

    double  mfOldLeft;
    double  mfOldRight;
    double  mfNewLeft;
    double  mfNewRight;

public:
    SdPageLRUndoAction(
        SdDrawDocument* pDoc,
        SdPage* pThePage,
        double fOldLeft,
        double fOldRight,
        double fNewLeft,
        double fNewRight )
    :   SdUndoAction(pDoc),
        mpPage      (pThePage),
        mfOldLeft   (fOldLeft),
        mfOldRight  (fOldRight),
        mfNewLeft   (fNewLeft),
        mfNewRight  (fNewRight)
    {
    }

    virtual ~SdPageLRUndoAction();

    virtual void Undo();
    virtual void Redo();
};

/************************************************************************/

class SdPageULUndoAction : public SdUndoAction
{
    SdPage* mpPage;

    double  mfOldTop;
    double  mfOldBottom;
    double  mfNewTop;
    double  mfNewBottom;

public:
    SdPageULUndoAction(
        SdDrawDocument* pDoc,
        SdPage* pThePage,
        double fOldTop,
        double fOldBottom,
        double fNewTop,
        double fNewBottom )
    :   SdUndoAction(pDoc),
        mpPage      (pThePage),
        mfOldTop    (fOldTop),
        mfOldBottom (fOldBottom),
        mfNewTop    (fNewTop),
        mfNewBottom (fNewBottom)
    {
    }

    virtual ~SdPageULUndoAction();

    virtual void Undo();
    virtual void Redo();
};



#endif      // _SD_UNDOPAGE_HXX

