/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdtext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:31:12 $
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

#ifndef _SVDTEXT_HXX
#define _SVDTEXT_HXX

#include <sal/types.h>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// --------------------------------------------------------------------

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SdrModel;

namespace sdr { namespace properties {
    class TextProperties;
}}

/** This class stores information about one text inside a shape.
*/

class SVX_DLLPUBLIC SdrText
{
public:
    SdrText( SdrTextObj* pObject, OutlinerParaObject* pOutlinerParaObject = 0 );
    virtual ~SdrText();

    virtual void SetModel(SdrModel* pNewModel);
    virtual void ForceOutlinerParaObject( USHORT nOutlMode );

    virtual void SetOutlinerParaObject( OutlinerParaObject* pTextObject );
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual void CheckPortionInfo( SdrOutliner& rOutliner );
    virtual void ReformatText();

    SdrModel* GetModel() const { return mpModel; }
    SdrTextObj* GetObject() const { return mpObject; }

    /** returns the current OutlinerParaObject and removes it from this instance */
    OutlinerParaObject* RemoveOutlinerParaObject();

protected:
    virtual const SfxItemSet& GetObjectItemSet();
    virtual void SetObjectItem(const SfxPoolItem& rItem);
    virtual SfxStyleSheet* GetStyleSheet() const;

private:
    OutlinerParaObject* mpOutlinerParaObject;
    SdrTextObj* mpObject;
    SdrModel* mpModel;
    bool mbPortionInfoChecked;
};

#endif //_SVDTEXT_HXX

