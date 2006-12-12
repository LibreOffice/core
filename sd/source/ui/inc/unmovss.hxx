/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unmovss.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:51:37 $
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

#ifndef _SD_UNMOVSS_HXX
#define _SD_UNMOVSS_HXX

#include "sdundo.hxx"

class List;
class SdDrawDocument;

class SdMoveStyleSheetsUndoAction : public SdUndoAction
{
    List* pStyles;
    List* pListOfChildLists;
    BOOL  bMySheets;

public:
    TYPEINFO();
    SdMoveStyleSheetsUndoAction(SdDrawDocument* pTheDoc,
                                List*           pTheStyles,
                                BOOL            bInserted);

    virtual ~SdMoveStyleSheetsUndoAction();
    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

#endif      // _SD_UNMOVSS_HXX

