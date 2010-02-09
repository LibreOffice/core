/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: undoback.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SD_UNDOBACK_HXX
#define _SD_UNDOBACK_HXX

#define NEWPBG

#include "sdundo.hxx"

class SdDrawDocument;
class SdPage;

#ifdef NEWPBG
class SfxItemSet;
#else
class SdrObject;
#endif

// -----------------------------
// - SdBackgroundObjUndoAction -
// -----------------------------

class SdBackgroundObjUndoAction : public SdUndoAction
{
private:

    SdPage&                 mrPage;
#ifdef NEWPBG
    SfxItemSet*             mpItemSet;
#else
    SdrObject*              mpBackgroundObj;
#endif

    void                    ImplRestoreBackgroundObj();

public:

                            TYPEINFO();

#ifdef NEWPBG
                            SdBackgroundObjUndoAction(
                                SdDrawDocument& rDoc,
                                SdPage& rPage,
                                const SfxItemSet& rItenSet);
#else
                            SdBackgroundObjUndoAction( SdDrawDocument& rDoc, SdPage& rPage, const SdrObject* pBackgroundObj );
#endif
    virtual                 ~SdBackgroundObjUndoAction();

    virtual void            Undo();
    virtual void            Redo();

    virtual SdUndoAction*   Clone() const;
};

#endif // _SD_UNDOBACK_HXX
