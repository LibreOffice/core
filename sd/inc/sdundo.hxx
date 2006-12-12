/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdundo.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:26:18 $
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
public:
    TYPEINFO();
                            SdUndoAction(SdDrawDocument* pSdDrawDocument)
                                : mpDoc(pSdDrawDocument)  {}
    virtual                 ~SdUndoAction() {}

    void                    SetComment(String& rStr) { maComment = rStr; }
    virtual String          GetComment() const { return maComment; }
    virtual SdUndoAction*   Clone() const { return NULL; }

protected:
    SdDrawDocument* mpDoc;
    String maComment;
};

#endif     // _SD_SDUNDO_HXX
