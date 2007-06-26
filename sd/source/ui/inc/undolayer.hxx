/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undolayer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 13:41:31 $
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

#ifndef _SD_UNDOLAYER_HXX
#define _SD_UNDOLAYER_HXX

#include "sdundo.hxx"

class SdDrawDocument;
class SdrLayer;

/************************************************************************/

class SdLayerModifyUndoAction : public SdUndoAction
{

public:
    TYPEINFO();
    SdLayerModifyUndoAction( SdDrawDocument* _pDoc, SdrLayer* pLayer,
    const String& rOldLayerName, const String& rOldLayerTitle, const String& rOldLayerDesc, bool bOldIsVisible, bool bOldIsLocked, bool bOldIsPrintable,
    const String& rNewLayerName, const String& rNewLayerTitle, const String& rNewLayerDesc, bool bNewIsVisible, bool bNewIsLocked, bool bNewIsPrintable );

    virtual void Undo();
    virtual void Redo();

private:
    SdrLayer* mpLayer;
    String maOldLayerName;
    String maOldLayerTitle;
    String maOldLayerDesc;
    bool mbOldIsVisible;
    bool mbOldIsLocked;
    bool mbOldIsPrintable;
    String maNewLayerName;
    String maNewLayerTitle;
    String maNewLayerDesc;
    bool mbNewIsVisible;
    bool mbNewIsLocked;
    bool mbNewIsPrintable;
};

#endif      // _SD_UNDOLAYER_HXX

