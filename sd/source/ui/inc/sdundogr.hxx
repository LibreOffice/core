/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdundogr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:48:23 $
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

#ifndef _SD_SDUNDOGR_HXX
#define _SD_SDUNDOGR_HXX

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#ifndef _SD_SDUNDO_HXX
#include "sdundo.hxx"
#endif

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

class SD_DLLPUBLIC SdUndoGroup : public SdUndoAction
{
    Container      aCtn;
public:
    TYPEINFO();
                   SdUndoGroup(SdDrawDocument* pSdDrawDocument)
                              : SdUndoAction(pSdDrawDocument),
                                aCtn(16, 16, 16) {}
    virtual       ~SdUndoGroup();

    virtual BOOL   Merge( SfxUndoAction* pNextAction );

    virtual void   Undo();
    virtual void   Redo();

    void           AddAction(SdUndoAction* pAction);
    SdUndoAction*  GetAction(ULONG nAction) const;
    ULONG          Count() const { return aCtn.Count(); }

};

#endif     // _SD_SDUNDOGR_HXX
