/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoutl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:26:14 $
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

#ifndef _SVDOUTL_HXX
#define _SVDOUTL_HXX

#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

class SdrTextObj;
class SdrPaintInfoRec;

class SVX_DLLPUBLIC SdrOutliner : public Outliner
{
protected:
    SdrObjectWeakRef mpTextObj;
    const SdrPaintInfoRec* mpPaintInfoRec;
public:
    SdrOutliner( SfxItemPool* pItemPool, USHORT nMode );
    virtual ~SdrOutliner() ;

    void  SetTextObj( const SdrTextObj* pObj );
    void SetTextObjNoInit( const SdrTextObj* pObj );

    const SdrTextObj* GetTextObj() const;

    void SetPaintInfoRec( const SdrPaintInfoRec* pPaintInfoRec ) { mpPaintInfoRec = pPaintInfoRec; }
    const SdrPaintInfoRec* GetPaintInfoRec() const { return mpPaintInfoRec; }
    void ClearPaintInfoRec() { mpPaintInfoRec = NULL; }

    virtual String  CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor);
};

#endif //_SVDOUTL_HXX

