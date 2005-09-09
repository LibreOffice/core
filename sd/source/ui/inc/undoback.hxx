/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoback.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:57:23 $
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

#ifndef _SD_UNDOBACK_HXX
#define _SD_UNDOBACK_HXX

#include "sdundo.hxx"

class SdDrawDocument;
class SdPage;
class SdrObject;

// -----------------------------
// - SdBackgroundObjUndoAction -
// -----------------------------

class SdBackgroundObjUndoAction : public SdUndoAction
{
private:

    SdPage&                 mrPage;
    SdrObject*              mpBackgroundObj;

    void                    ImplRestoreBackgroundObj();

public:

                            TYPEINFO();

                            SdBackgroundObjUndoAction( SdDrawDocument& rDoc, SdPage& rPage, const SdrObject* pBackgroundObj );
    virtual                 ~SdBackgroundObjUndoAction();

    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat();

    virtual BOOL            CanRepeat( SfxRepeatTarget& ) const;

    virtual SdUndoAction*   Clone() const;
};

#endif // _SD_UNDOBACK_HXX
