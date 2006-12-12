/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undopage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:50:52 $
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

#ifndef _SD_UNDOPAGE_HXX
#define _SD_UNDOPAGE_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SV_PRNTYPES_HXX //autogen
#include <vcl/prntypes.hxx>
#endif

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

