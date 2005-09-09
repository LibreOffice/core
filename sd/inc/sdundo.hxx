/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdundo.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:02:08 $
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

#ifndef _SD_SDUNDO_HXX
#define _SD_SDUNDO_HXX

#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

class SdDrawDocument;

class SD_DLLPUBLIC SdUndoAction : public SfxUndoAction
{
    String aComment;
protected:
    SdDrawDocument* pDoc;
public:
    TYPEINFO();
                            SdUndoAction(SdDrawDocument* pSdDrawDocument)
                                : pDoc(pSdDrawDocument)  {}
    virtual                 ~SdUndoAction() {}

    virtual BOOL            CanRepeat(SfxRepeatTarget& rView) const;
    virtual void            Repeat(SfxRepeatTarget& rView);
    void                    SetComment(String& rStr) { aComment = rStr; }
    virtual String          GetComment() const { return aComment; }
    virtual SdUndoAction*   Clone() const { return NULL; }
};

#endif     // _SD_SDUNDO_HXX
