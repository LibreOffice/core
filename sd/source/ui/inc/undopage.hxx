/*************************************************************************
 *
 *  $RCSfile: undopage.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    SdPage*     pPage;

    Size        aOldSize;
    INT32       nOldLeft;
    INT32       nOldRight;
    INT32       nOldUpper;
    INT32       nOldLower;
    BOOL        bOldScale;
    Orientation eOldOrientation;

    Size        aNewSize;
    INT32       nNewLeft;
    INT32       nNewRight;
    INT32       nNewUpper;
    INT32       nNewLower;
    BOOL        bNewScale;
    Orientation eNewOrientation;

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

                            const Size&     rNewSz,
                            INT32           nNewLft,
                            INT32           nNewRgt,
                            INT32           nNewUpr,
                            INT32           nNewLwr,
                            BOOL            bNewScl,
                            Orientation     eNewOrient ) :
        SdUndoAction(pDoc),
        pPage       (pThePage),
        aOldSize    (rOldSz),
        nOldLeft    (nOldLft),
        nOldRight   (nOldRgt),
        nOldUpper   (nOldUpr),
        nOldLower   (nOldLwr),
        bOldScale   (bOldScl),
        eOldOrientation(eOldOrient),

        aNewSize    (rNewSz),
        nNewLeft    (nNewLft),
        nNewRight   (nNewRgt),
        nNewUpper   (nNewUpr),
        nNewLower   (nNewLwr),
        bNewScale   (bNewScl),
        eNewOrientation(eNewOrient)
        {}
    virtual ~SdPageFormatUndoAction();

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat();
};

/************************************************************************/

class SdPageLRUndoAction : public SdUndoAction
{
    SdPage* pPage;

    INT32   nOldLeft;
    INT32   nOldRight;
    INT32   nNewLeft;
    INT32   nNewRight;

public:
    TYPEINFO();
    SdPageLRUndoAction( SdDrawDocument* pDoc, SdPage* pThePage,
                        INT32 nOldLft, INT32 nOldRgt,
                        INT32 nNewLft, INT32 nNewRgt ) :
        SdUndoAction(pDoc),
        pPage       (pThePage),
        nOldLeft    (nOldLft),
        nOldRight   (nOldRgt),
        nNewLeft    (nNewLft),
        nNewRight   (nNewRgt)
        {}
    virtual ~SdPageLRUndoAction();

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat();
};

/************************************************************************/

class SdPageULUndoAction : public SdUndoAction
{
    SdPage* pPage;

    INT32   nOldUpper;
    INT32   nOldLower;
    INT32   nNewUpper;
    INT32   nNewLower;

public:
    TYPEINFO();
    SdPageULUndoAction( SdDrawDocument* pDoc, SdPage* pThePage,
                        INT32 nOldUpr, INT32 nOldLwr,
                        INT32 nNewUpr, INT32 nNewLwr ) :
        SdUndoAction(pDoc),
        pPage       (pThePage),
        nOldUpper   (nOldUpr),
        nOldLower   (nOldLwr),
        nNewUpper   (nNewUpr),
        nNewLower   (nNewLwr)
        {}
    virtual ~SdPageULUndoAction();

    virtual void Undo();
    virtual void Redo();
    virtual void Repeat();
};



#endif      // _SD_UNDOPAGE_HXX

