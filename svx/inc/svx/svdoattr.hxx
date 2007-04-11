/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoattr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:22:44 $
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

#ifndef _SVDOATTR_HXX
#define _SVDOATTR_HXX

#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif

#ifndef _SVX_XFLASIT_HXX //autogen
#include <svx/xflasit.hxx>
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif

#ifndef _SVX_XLNASIT_HXX //autogen
#include <svx/xlnasit.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#ifndef _SVDATTR_HXX
#include <svx/svdattr.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxPoolItem;
class SfxSetItem;
class SdrOutliner;
class SfxItemSet;
class SfxItemPool;

//************************************************************
//   SdrAttrObj
//************************************************************

class SVX_DLLPUBLIC SdrAttrObj : public SdrObject
{
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    friend class                SdrOutliner;

protected:
    Rectangle                   maSnapRect;

protected:
    // Strichstaerke ermitteln. Keine Linie -> 0.
    sal_Int32 ImpGetLineWdt() const;

    // Feststellen, wieviel wegen der Linienenden auf's BoundRect draufaddiert werden muss.
    INT32 ImpGetLineEndAdd() const;

    // Schattenabstand ermitteln. FALSE=Kein Schatten.
    FASTBOOL ImpGetShadowDist(sal_Int32& nXDist, sal_Int32& nYDist) const;

    // ggf. Schattenversatz zum BoundRect draufaddieren
    void ImpAddShadowToBoundRect();

    // Line und Fill Attribute fuer Schatten setzen.
    // Return=FALSE: kein Schatten attributiert.
    FASTBOOL ImpSetShadowAttributes( const SfxItemSet& rSet, SfxItemSet& rShadowSet ) const;

    // Zuhoeren, ob sich ein StyleSheet aendert
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

    SdrAttrObj();
    virtual ~SdrAttrObj();

public:
    TYPEINFO();

    // Feststellen, ob bFilledObj && Fuellung!=FillNone
    BOOL HasFill() const;

    // Feststellen, ob Linie!=LineNone
    BOOL HasLine() const;

    virtual const Rectangle& GetSnapRect() const;

    virtual void SetModel(SdrModel* pNewModel);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOATTR_HXX

