/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selectioncontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:26:57 $
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

#ifndef _SELECTIONCONTROLLER_HXX
#define _SELECTIONCONTROLLER_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <cppuhelper/weak.hxx>

class KeyEvent;
class MouseEvent;
class Window;
class SfxItemSet;
class SfxRequest;
class SfxStyleSheet;
class SdrPage;
class SdrModel;

namespace sdr
{

class SVX_DLLPUBLIC SelectionController: public cppu::OWeakObject
{
public:
    virtual bool onKeyInput(const KeyEvent& rKEvt, Window* pWin);
    virtual bool onMouseButtonDown(const MouseEvent& rMEvt, Window* pWin);
    virtual bool onMouseButtonUp(const MouseEvent& rMEvt, Window* pWin);
    virtual bool onMouseMove(const MouseEvent& rMEvt, Window* pWin);

    virtual void onSelectionHasChanged();

    virtual void GetState( SfxItemSet& rSet );
    virtual void Execute( SfxRequest& rReq );

    virtual bool DeleteMarked();

    virtual bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;
    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);

    virtual bool GetStyleSheet( SfxStyleSheet* &rpStyleSheet ) const;
    virtual bool SetStyleSheet( SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr );

    virtual bool GetMarkedObjModel( SdrPage* pNewPage );
    virtual bool PasteObjModel( const SdrModel& rModel );
};

}

#endif //_SELECTIONCONTROLLER_HXX

