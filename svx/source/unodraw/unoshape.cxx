/*************************************************************************
 *
 *  $RCSfile: unoshape.cxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-19 09:13:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SVX_USE_UNOGLOBALS_

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef _FLTCALL_HXX
#include <svtools/fltcall.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SVX_UNOTEXT_HXX
#include <unotext.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef SVX_LIGHT
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#endif
#include <comphelper/extract.hxx>

#include <toolkit/unohlp.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef SVX_LIGHT
#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#endif

#ifndef _SVX_XFLBSTIT_HXX
#include "xflbstit.hxx"
#endif
#ifndef _SVX_XFLBMTIT_HXX
#include "xflbmtit.hxx"
#endif
#ifndef _SVX_XLNSTIT_HXX
#include "xlnstit.hxx"
#endif
#ifndef _SVX_XLNEDIT_HXX
#include "xlnedit.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif
#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif
#include "svdmodel.hxx"
#include "globl3d.hxx"
#include "fmglob.hxx"
#include "unopage.hxx"
#include "view3d.hxx"
#include "unoshape.hxx"
#include "svxids.hrc"
#include "unoshtxt.hxx"
#include "svdpage.hxx"
#include "unoshprp.hxx"
#include "svdoole2.hxx"
#include "sxciaitm.hxx" // todo: remove
#include "svdograf.hxx"
#include "unoapi.hxx"
#include "svdomeas.hxx"

#include <tools/shl.hxx>    //
#include "dialmgr.hxx"      // not nice, we need our own resources some day
#include "dialogs.hrc"      //

#ifndef _E3D_OBJ3D_HXX
#include <obj3d.hxx>
#endif

using namespace ::osl;
using namespace ::vos;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

const SfxItemPropertyMap* ImplGetSvxUnoOutlinerTextCursorPropertyMap()
{
    // Propertymap fuer einen Outliner Text
    static const SfxItemPropertyMap aSvxUnoOutlinerTextCursorPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {0,0}
    };

    return aSvxUnoOutlinerTextCursorPropertyMap;
}

const SfxItemPropertyMap* ImplGetSvxTextPortionPropertyMap()
{
    // Propertymap fuer einen Outliner Text
    static const SfxItemPropertyMap aSvxTextPortionPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {MAP_CHAR_LEN("TextField"),                     EE_FEATURE_FIELD,   &::getCppuType((const uno::Reference< text::XTextField >*)0),   beans::PropertyAttribute::READONLY, 0 },\
        {MAP_CHAR_LEN("TextPortionType"),               WID_PORTIONTYPE,    &::getCppuType((const ::rtl::OUString*)0), beans::PropertyAttribute::READONLY, 0 }, \
        {0,0}
    };

    return aSvxTextPortionPropertyMap;
}

class GDIMetaFile;
class SvStream;
sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              PFilterCallback pCallback=NULL, void * pCallerData=NULL,
                              sal_Bool bPlaceable=sal_True);

uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject );

DECLARE_LIST( SvxShapeList, SvxShape * );

#define GET_TEXT_INTERFACE( xint, xval ) \
    Reference< xint > xval; \
    if(!xTextAgg.is() ) { Reference< ::com::sun::star::text::XText > xText( (OWeakObject*)this, UNO_QUERY ); } \
    if(xTextAgg.is() ) \
    { \
        uno::Any aAny( \
            xTextAgg->queryAggregation( ::getCppuType((const Reference< xint >*)0))); \
        aAny >>= xval; \
    } \

#define INTERFACE_TYPE( xint ) \
    ::getCppuType((const Reference< xint >*)0)

/***********************************************************************
* class SvxShape                                                       *
***********************************************************************/

//UNO3_GETIMPLEMENTATION_IMPL( SvxShape );
uno::Sequence< uno::Type > SvxShape::maTypeSequence;

SvxShape::SvxShape( SdrObject* pObject ) throw()
:   aPropSet(aSvxMapProvider.GetMap(SVXMAP_SHAPE)),
    pObj    (pObject),
    bDisposing( sal_False ),
    pModel(NULL),
    aSize(100,100),
    aDisposeListeners( aDisposeContainerMutex )
{
    Init();
}

//----------------------------------------------------------------------
SvxShape::SvxShape( SdrObject* pObject, const SfxItemPropertyMap* pPropertyMap ) throw()
:   aPropSet(pPropertyMap),
    bDisposing( sal_False ),
    pObj    (pObject),
    pModel(NULL),
    aSize(100,100),
    aDisposeListeners( aDisposeContainerMutex )

{
    Init();
}

//----------------------------------------------------------------------
SvxShape::SvxShape() throw()
:   aPropSet(aSvxMapProvider.GetMap(SVXMAP_SHAPE)),
    bDisposing( sal_False ),
    pObj    (NULL),
    pModel(NULL),
    aSize(100,100),
    aDisposeListeners( aDisposeContainerMutex )

{
    Init();
}

//----------------------------------------------------------------------
SvxShape::~SvxShape() throw()
{
    if (xTextAgg.is())
        xTextAgg->setDelegator( Reference< uno::XInterface >() );

    xTextAgg = NULL;

    if( pModel )
        EndListening( *pModel );


    OGuard aGuard( Application::GetSolarMutex() );
}

//----------------------------------------------------------------------
const ::com::sun::star::uno::Sequence< sal_Int8 > & SvxShape::getUnoTunnelId() throw()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//----------------------------------------------------------------------
SvxShape* SvxShape::getImplementation( uno::Reference< uno::XInterface > xInt ) throw()
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, ::com::sun::star::uno::UNO_QUERY );
    if( xUT.is() )
        return (SvxShape*)xUT->getSomething( SvxShape::getUnoTunnelId() );
    else
        return NULL;
}

//----------------------------------------------------------------------
sal_Int64 SAL_CALL SvxShape::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    else
    {
        if( !xTextAgg.is() )
            queryAggregation( ::getCppuType((const Reference< text::XText >*)0));

        if( xTextAgg.is() )
        {
            uno::Any aAny( xTextAgg->queryAggregation( ::getCppuType((const Reference< lang::XUnoTunnel >*)0) ) );
            uno::Reference< lang::XUnoTunnel > xUT;
            aAny >>= xUT;
            if( xUT.is() )
                return xUT->getSomething(rId );
        }
    }

    return NULL;
}

//----------------------------------------------------------------------
SvxShape* SvxShape::GetShapeForSdrObj( SdrObject* pObj ) throw()
{
    return getImplementation( pObj->getUnoShape() );
}

//----------------------------------------------------------------------
void SvxShape::Init() throw()
{
    if(pObj == NULL)    // ab hier nur nocht mit Objekt
        return;

    if(!pObj->GetModel())
        return;

    pModel = pObj->GetModel();

    StartListening( *pModel );

    const sal_uInt32 nInventor = pObj->GetObjInventor();

    if( nInventor == SdrInventor ||
        nInventor == E3dInventor ||
        nInventor == FmFormInventor )
    {
        UHashMapEntry* pMap = pSdrShapeIdentifierMap;
        sal_uInt32 nObjId;

        if(nInventor == FmFormInventor)
            nObjId = OBJ_UNO;
        else
        {
            nObjId = pObj->GetObjIdentifier();
            if( nInventor == E3dInventor )
                nObjId |= E3D_INVENTOR_FLAG;
        }

        switch(nObjId)
        {
        case OBJ_CCUT:          // Kreisabschnitt
        case OBJ_CARC:          // Kreisbogen
        case OBJ_SECT:          // Kreissektor
            nObjId = OBJ_CIRC;
            break;

        case E3D_POLYSCENE_ID | E3D_INVENTOR_FLAG:
            nObjId = E3D_SCENE_ID | E3D_INVENTOR_FLAG;
            break;
        }

        while(pMap->aIdentifier.getLength() && ( pMap->nId != nObjId ) )
            pMap++;

        if(pMap->aIdentifier)
        {
            OUString aType(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing." ) );
            aType += pMap->aIdentifier;
            aShapeType = aType;
        }
        else
        {
            DBG_ASSERT(aShapeType.len() == 0, "[CL] unknown SdrObjekt identifier");
        }
    }
}

//----------------------------------------------------------------------
void SvxShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw()
{
    DBG_ASSERT( !pObj || pObj == pNewObj, "SvxShape::Create was called twice!" );

    pObj = pNewObj;

    Init();

    ObtainSettingsFromPropertySet( aPropSet );

    // save user call
    SdrObjUserCall* pUser = pObj->GetUserCall();
    pObj->SetUserCall(NULL);

    setPosition( aPosition );
    setSize( aSize );

    pObj->SetUserCall( pUser );

    if( aShapeName.getLength() )
    {
        pObj->SetName( aShapeName );
        aShapeName = OUString();
    }
}

//----------------------------------------------------------------------
void SvxShape::ForceMetricToItemPoolMetric(Pair& rPoint) const throw()
{
    if(pObj && pModel)
    {
        SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
        if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            switch(eMapUnit)
            {
                case SFX_MAPUNIT_TWIP :
                {
                    rPoint.A() = MM_TO_TWIPS(rPoint.A());
                    rPoint.B() = MM_TO_TWIPS(rPoint.B());
                    break;
                }
                default:
                {
                    DBG_ERROR("AW: Missing unit translation to PoolMetric!");
                }
            }
        }
    }
}

//----------------------------------------------------------------------
void SvxShape::ForceMetricTo100th_mm(Pair& rPoint) const throw()
{
    SfxMapUnit eMapUnit = SFX_MAPUNIT_100TH_MM;
    if(pObj && pModel)
    {
        eMapUnit = pModel->GetItemPool().GetMetric(0);
        if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            switch(eMapUnit)
            {
                case SFX_MAPUNIT_TWIP :
                {
                    rPoint.A() = TWIPS_TO_MM(rPoint.A());
                    rPoint.B() = TWIPS_TO_MM(rPoint.B());
                    break;
                }
                default:
                {
                    DBG_ERROR("AW: Missing unit translation to 100th mm!");
                }
            }
        }
    }
}

//----------------------------------------------------------------------
void SvxShape::ObtainSettingsFromPropertySet(SvxItemPropertySet& rPropSet) throw()
{
    if(pObj && rPropSet.AreThereOwnUsrAnys())
    {
        SfxItemSet aSet( pModel->GetItemPool(),
            SDRATTR_START, SDRATTR_END,
//-/            SID_ATTR_3D_START, SID_ATTR_3D_END,
            0);

        Reference< beans::XPropertySet > xShape( (OWeakObject*)this, UNO_QUERY );
        aPropSet.ObtainSettingsFromPropertySet(rPropSet, aSet, xShape);

//-/        pObj->SetAttributes( aSet, sal_False );
//-/        SdrBroadcastItemChange aItemChange(*pObj);
        pObj->SetItemSetAndBroadcast(aSet);
//-/        pObj->BroadcastItemChange(aItemChange);

        pObj->ApplyNotPersistAttr( aSet );
    }
}

uno::Any SvxShape::GetBitmap( sal_Bool bMetaFile /* = sal_False */ ) const throw()
{
    uno::Any aAny;

    if( pObj == NULL || pModel == NULL )
        return aAny;

    VirtualDevice aVDev;
    aVDev.SetMapMode(MapMode(MAP_100TH_MM));

    SdrModel* pModel = pObj->GetModel();
    SdrPage* pPage = pObj->GetPage();
    E3dView* pView = new E3dView( pModel, &aVDev );
    pView->SetMarkHdlHidden( sal_True );
    SdrPageView* pPageView = pView->ShowPage(pPage, Point());

    SdrObject *pTempObj = pObj;
    pView->MarkObj(pTempObj,pPageView);

    Rectangle aRect(pTempObj->GetBoundRect());
    aRect.Justify();
    Size aSize(aRect.GetSize());

    GDIMetaFile aMtf( pView->GetAllMarkedMetaFile() );
    if( bMetaFile )
    {
        SvMemoryStream aDestStrm( 65535, 65535 );
        ConvertGDIMetaFileToWMF( aMtf, aDestStrm, NULL, NULL, sal_False );
        uno::Sequence<sal_Int8> aSeq((sal_Int8*)aDestStrm.GetData(), aDestStrm.GetSize());
        aAny.setValue( &aSeq, ::getCppuType((const uno::Sequence< sal_Int8 >*)0) );
    }
    else
    {
        Graphic aGraph(aMtf);
        aGraph.SetPrefSize(aSize);
        aGraph.SetPrefMapMode(MAP_100TH_MM);

        Reference< awt::XBitmap > xBmp( VCLUnoHelper::CreateBitmap(aGraph.GetBitmapEx()) );
        aAny.setValue( &xBmp, ::getCppuType(( const uno::Reference< awt::XBitmap>*)0) );
    }

    pView->UnmarkAll();
    delete pView;

    return aAny;
}

// UserAggObject

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShape::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxShape::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aAny;

    if( rType == ::getCppuType((const Reference< beans::XPropertySet >*)0))
        aAny <<= Reference< beans::XPropertySet >(this);
    else if( rType == ::getCppuType((const Reference< drawing::XShape >*)0))
        aAny <<= Reference< drawing::XShape >(this);
    else if( rType == ::getCppuType((const Reference< lang::XComponent >*)0))
        aAny <<= Reference< lang::XComponent>(this);
    else if( rType == ::getCppuType((const Reference< drawing::XShapeDescriptor >*)0))
        aAny <<= Reference< drawing::XShapeDescriptor>(this);
    else if( rType == ::getCppuType((const Reference< beans::XPropertyState >*)0))
        aAny <<= Reference<beans::XPropertyState>(this);
    else if( rType == ::getCppuType((const Reference< lang::XTypeProvider >*)0))
        aAny <<= Reference<lang::XTypeProvider>(this);
    else if( rType == ::getCppuType((const Reference< XServiceInfo >*)0))
        aAny <<= Reference<XServiceInfo>(this);
    else if( rType == ::getCppuType((const Reference< XUnoTunnel >*)0))
        aAny <<= Reference<XUnoTunnel>(this);
    else if( rType == ::getCppuType((const Reference< drawing::XGluePointsSupplier >*)0))
        aAny <<= Reference<drawing::XGluePointsSupplier>(this);
    else if( rType == ::getCppuType((const Reference< container::XChild >*)0))
        aAny <<= Reference<container::XChild >(this);
    else if( rType == ::getCppuType((const Reference< container::XNamed >*)0))
        aAny <<= Reference<container::XNamed>(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    if( !aAny.hasValue() && pObj && pObj->GetModel() )
    {
        if( !xTextAgg.is() )
        {
            SvxTextEditSource aSvxTextEditSource( pObj );

            {
                Reference< ::com::sun::star::text::XText > xTextParent;
                Reference< ::com::sun::star::text::XText > xText =
                    new SvxUnoText( &aSvxTextEditSource, ImplGetSvxUnoOutlinerTextCursorPropertyMap(), xTextParent );
                if(xText.is())
                {
                    Reference< uno::XAggregation > xAgg( xText, UNO_QUERY );
                    xTextAgg = xAgg;
                }
            }

            if(xTextAgg.is())
            {
                Reference< uno::XInterface > xDelegator( (OWeakObject*)this, UNO_QUERY );
                xTextAgg->setDelegator(xDelegator);
            }
            else
                return uno::Any();
        }

        aAny <<= xTextAgg->queryAggregation( rType );
    }

    return aAny;
}

void SAL_CALL SvxShape::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShape::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

void SvxShape::addStaticTypes( sal_Int16 nNewTypes, /* uno::Type* */ ... ) throw()
{
    const sal_Int32 nOldCount = maTypeSequence.getLength();
    DBG_ASSERT( nOldCount, "illegal call of addStaticType() before SvxShape::getStaticTypes()!" );

    maTypeSequence.realloc( nOldCount + nNewTypes );
    uno::Type* pTypes = &maTypeSequence.getArray()[nOldCount];

    va_list marker;
    va_start( marker, nNewTypes );
    for( sal_Int32 i = 0 ; i < nNewTypes; i++ )
        *pTypes++ = *va_arg( marker, uno::Type*);
    va_end( marker );
}

uno::Sequence< uno::Type > SAL_CALL SvxShape::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SvxUnoText::getStaticTypes() );
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const sal_Int32 nOwnTypes = 8;      // !DANGER! Keep this updated!

        maTypeSequence.realloc( nBaseTypes  + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShape::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//----------------------------------------------------------------------

Reference< uno::XInterface > SvxShape_NewInstance()
{
    uno::Reference< drawing::XShape > xShape( (OWeakObject*)new SvxShape(), UNO_QUERY );
    return xShape;
}

// SfxListener

//----------------------------------------------------------------------
void SvxShape::Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw()
{
    Reference< drawing::XShape > rHoldMyself( (drawing::XShape*)this );

    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint && pObj)
    {
        if( pSdrHint->GetKind() == HINT_OBJREMOVED )
        {
            if( pObj == pSdrHint->GetObject() )
            {
                pObj = NULL;
            }
        }
        else if( pSdrHint->GetKind() == HINT_MODELCLEARED )
        {
            pModel = NULL;
        }
        else if( pSdrHint->GetKind() == HINT_OBJLISTCLEARED && pSdrHint->GetObjList() == pObj->GetObjList() )
        {
            pObj = NULL;
        }
    }

    if( pObj == NULL )
    {
        if(!bDisposing)
            dispose();
    }
}

// XShape

sal_Bool needLogicRectHack( SdrObject* pObj )
{
    if( pObj->GetObjInventor() == SdrInventor)
    {
        switch(pObj->GetObjIdentifier())
        {
        case OBJ_GRUP:
        case OBJ_LINE:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_SPLNLINE:
        case OBJ_SPLNFILL:
        case OBJ_EDGE:
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
        case OBJ_MEASURE:
            return sal_True;
        }
    }
    return sal_False;
}

Rectangle getLogicRectHack( SdrObject* pObj )
{
    if(needLogicRectHack(pObj))
    {
        return pObj->GetSnapRect();
    }
    else
    {
        return pObj->GetLogicRect();
    }
}

void setLogicRectHack( SdrObject* pObj, const Rectangle& rRect )
{
    if(needLogicRectHack(pObj))
    {
        pObj->SetSnapRect( rRect );
    }
    else
    {
        pObj->SetLogicRect( rRect );
    }
}

//----------------------------------------------------------------------
awt::Point SAL_CALL SvxShape::getPosition() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pObj && pModel)
    {
        Rectangle aRect( getLogicRectHack(pObj) );
        Point aPt( aRect.Left(), aRect.Top() );

        // Position ist relativ zum Anker, in absolute Position
        // umrechnen
        aPt -= pObj->GetAnchorPos();

        ForceMetricTo100th_mm(aPt);
        return ::com::sun::star::awt::Point( aPt.X(), aPt.Y() );
    }
    else
    {
        return aPosition;
    }
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pObj && pModel )
    {
        // do NOT move 3D objects, this would change the homogen
        // transformation matrix
        if(!pObj->ISA(E3dCompoundObject))
        {
            Rectangle aRect( getLogicRectHack(pObj) );
            Point aLocalPos( Position.X, Position.Y );
            ForceMetricToItemPoolMetric(aLocalPos);

            // Position ist absolut, relativ zum Anker stellen
            aLocalPos += pObj->GetAnchorPos();

            long nDX = aLocalPos.X() - aRect.Left();
            long nDY = aLocalPos.Y() - aRect.Top();

            pObj->Move( Size( nDX, nDY ) );
            pModel->SetChanged();
        }
    }

    aPosition = Position;
}

//----------------------------------------------------------------------
awt::Size SAL_CALL SvxShape::getSize() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pObj && pModel)
    {
        Rectangle aRect( getLogicRectHack(pObj) );
        Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
        ForceMetricTo100th_mm(aObjSize);
        return ::com::sun::star::awt::Size( aObjSize.getWidth(), aObjSize.getHeight() );
    }
    else
        return aSize;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pObj && pModel)
    {
        Rectangle aRect( getLogicRectHack(pObj) );
        Size aLocalSize( rSize.Width, rSize.Height );
        ForceMetricToItemPoolMetric(aLocalSize);
        if(pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_MEASURE )
        {
            Fraction aWdt(aLocalSize.Width(),aRect.Right()-aRect.Left());
            Fraction aHgt(aLocalSize.Height(),aRect.Bottom()-aRect.Top());
            Point aPt = pObj->GetSnapRect().TopLeft();
            pObj->Resize(aPt,aWdt,aHgt);
        }
        else
        {
            aRect.SetSize(aLocalSize);
            setLogicRectHack( pObj, aRect );
        }

        pModel->SetChanged();
    }
    aSize = rSize;
}


// XNamed
OUString SAL_CALL SvxShape::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    if( pObj )
    {
        return pObj->GetName();
    }
    else
    {
        return aShapeName;
    }
}

void SAL_CALL SvxShape::setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException)
{
    if( pObj )
    {
        pObj->SetName( aName );
    }
    else
    {
        aShapeName = aName;
    }
}

// XShapeDescriptor

//----------------------------------------------------------------------
OUString SAL_CALL SvxShape::getShapeType() throw(uno::RuntimeException)
{
    return aShapeType;
}

// XComponent

//----------------------------------------------------------------------
void SAL_CALL SvxShape::dispose() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( bDisposing )
        return; // catched a recursion

    bDisposing = sal_True;

    lang::EventObject aEvt;
    aEvt.Source = *(OWeakAggObject*) this;
    aDisposeListeners.disposeAndClear(aEvt);

    SdrObject* pObj = GetSdrObject();
    if(pObj)
    {
        SdrPage* pPage = pObj->GetPage();
        // SdrObject aus der Page loeschen
        sal_uInt32 nCount = pPage->GetObjCount();
        for( sal_uInt32 nNum = 0; nNum < nCount; nNum++ )
        {
            if(pPage->GetObj(nNum) == pObj)
            {
                delete pPage->RemoveObject(nNum);
                InvalidateSdrObject();
                break;
            }
        }
    }
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::addEventListener( const Reference< lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    aDisposeListeners.addInterface(xListener);
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::removeEventListener( const Reference< lang::XEventListener >& aListener ) throw(uno::RuntimeException)
{
   aDisposeListeners.removeInterface(aListener);
}

// XPropertySet

//----------------------------------------------------------------------
Reference< beans::XPropertySetInfo > SAL_CALL
    SvxShape::getPropertySetInfo() throw(uno::RuntimeException)
{
    return new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addPropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::addVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removeVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName )
{
    SfxItemSet aSet( pModel->GetItemPool(), (USHORT)nWID, (USHORT)nWID );
    if( !SetFillAttribute( nWID, rName, aSet ) )
        return sal_False;

    pObj->SetItemSetAndBroadcast(aSet);

    return sal_True;
}

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet )
{
    String aName;
    SvxUnogetInternalNameForItem( (sal_Int16)nWID, rName, aName );

    if( aName.Len() == 0 )
    {
        switch( nWID )
        {
        case XATTR_LINEEND:
        case XATTR_LINESTART:
            {
                const String aEmpty;
                const XPolygon aPoly;
                if( nWID == XATTR_LINEEND )
                    rSet.Put( XLineEndItem( aEmpty, aPoly ) );
                else
                    rSet.Put( XLineStartItem( aEmpty, aPoly ) );

                return sal_True;
            }
            break;
        }

        return sal_False;
    }

    const SfxItemPool* pPool = rSet.GetPool();

    const String aSearchName( aName );
    const USHORT nCount = pPool->GetItemCount((USHORT)nWID);
    const NameOrIndex *pItem;

    for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)pPool->GetItem((USHORT)nWID, nSurrogate);
        if( pItem && ( pItem->GetName() == aSearchName ) )
        {
            rSet.Put( *pItem );
            return sal_True;
        }
    }

    return sal_False;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::setPropertyValue( const OUString& rPropertyName, const uno::Any& rVal )
    throw(beans::UnknownPropertyException,
            beans::PropertyVetoException,
                lang::IllegalArgumentException,
                    lang::WrappedTargetException,
                        uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(rPropertyName);

    if( pObj && pModel )
    {
        if( pMap == NULL || ( pMap->nFlags & beans::PropertyAttribute::READONLY ) != 0 )
            throw beans::UnknownPropertyException();

        pModel->SetChanged();

        switch( pMap->nWID )
        {
        case OWN_ATTR_TRANSFORMATION:
        {
            drawing::HomogenMatrix3 aMatrix;
            if(rVal >>= aMatrix)
            {
                XPolyPolygon aEmptyPolygon;
                Matrix3D aMatrix3D;
                pObj->TRGetBaseGeometry(aMatrix3D, aEmptyPolygon);
                aMatrix3D[0] = Point3D( aMatrix.Line1.Column1, aMatrix.Line1.Column2, aMatrix.Line1.Column3 );
                aMatrix3D[1] = Point3D( aMatrix.Line2.Column1, aMatrix.Line2.Column2, aMatrix.Line2.Column3 );
                aMatrix3D[2] = Point3D( aMatrix.Line3.Column1, aMatrix.Line3.Column2, aMatrix.Line3.Column3 );
                pObj->TRSetBaseGeometry(aMatrix3D, aEmptyPolygon);
                return;
            }
            break;
        }

        case OWN_ATTR_ZORDER:
        {
            sal_Int32 nNewOrdNum;
            if(rVal >>= nNewOrdNum)
            {
                SdrPage * pPage = pObj->GetPage();
                if( pPage )
                {
                    SdrObject* pCheck =
                        pPage->SetObjectOrdNum( pObj->GetOrdNum(), (ULONG)nNewOrdNum );
                    DBG_ASSERT( pCheck == pObj, "GetOrdNum() failed!" );
                    return;
                }
            }
            break;
        }
        case OWN_ATTR_WRITINGMODE:
        {
            text::WritingMode eMode;
            if( pObj && pObj->ISA(SdrTextObj) && (rVal >>= eMode) )
            {
                SdrTextObj* pText = (SdrTextObj*)pObj;
                pText->SetVerticalWriting( eMode == text::WritingMode_TB_RL );
                return;
            }
        }
        case OWN_ATTR_FRAMERECT:
        {
            awt::Rectangle aUnoRect;
            if(rVal >>= aUnoRect)
            {
                Point aTopLeft( aUnoRect.X, aUnoRect.Y );
                Size aObjSize( aUnoRect.Width, aUnoRect.Height );
                ForceMetricToItemPoolMetric(aTopLeft);
                ForceMetricToItemPoolMetric(aObjSize);
                Rectangle aRect;
                aRect.SetPos(aTopLeft);
                aRect.SetSize(aObjSize);
                pObj->SetSnapRect(aRect);
                return;
            }
            break;
        }
        case OWN_ATTR_CLSID:
        {
#ifndef SVX_LIGHT
            OUString aCLSID;
            if( rVal >>= aCLSID )
            {
                // init a ole object with a global name
                if( pObj && pObj->ISA(SdrOle2Obj))
                {
                    const SvInPlaceObjectRef& rIPRef = ((SdrOle2Obj*)pObj)->GetObjRef();
                    if (!rIPRef.Is() )
                    {
                        SvGlobalName aClassName;
                        if( aClassName.MakeId( aCLSID ) )
                        {
                            SvGlobalName aClassName;
                            if( aClassName.MakeId( aCLSID ) )
                            {
                                // create storage and inplace object
                                String aEmptyStr;
                                SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
                                SvInPlaceObjectRef aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit( aClassName, aStor);

                                SvPersist* pPersist = pModel->GetPersist();

                                String aName = getName();

                                sal_Bool bOk = sal_False;
                                // if we already have a shape name check if its a unique
                                // storage name
                                if( aName.Len() && !pPersist->Find( aName ) )
                                {
                                    SvInfoObjectRef xSub = new SvEmbeddedInfoObject( aIPObj, aName );
                                    bOk = pPersist->Move( xSub, aName );
                                }
                                else
                                {
                                    // generate a unique name

                                    aName = String( RTL_CONSTASCII_USTRINGPARAM("Object ") );
                                    String aStr;
                                    sal_Int32 i = 1;
                                    HACK(Wegen Storage Bug 46033)
                                    // for-Schleife wegen Storage Bug 46033
                                    for( sal_Int16 n = 0; n < 100; n++ )
                                    {
                                        do
                                        {
                                            aStr = aName;
                                            aStr += String::CreateFromInt32( i );
                                            i++;
                                        } while ( pPersist->Find( aStr ) );

                                        SvInfoObjectRef xSub = new SvEmbeddedInfoObject( aIPObj, aStr );
                                        if( pPersist->Move( xSub, aStr ) ) // Eigentuemer Uebergang
                                        {
                                            bOk = sal_True;
                                            break;
                                        }
                                    }
                                }

                                DBG_ASSERT( bOk, "could not create move ole stream!" )

                                if( bOk )
                                    pObj->SetName( aName );

                                ((SdrOle2Obj*)pObj)->SetObjRef(aIPObj);

                                Rectangle aRect( ( (SdrOle2Obj*) pObj)->GetLogicRect() );
                                aIPObj->SetVisAreaSize( aRect.GetSize() );

                                return;
                            }
                        }
                    }
                }
            }
#endif
            break;
        }
        case OWN_ATTR_EDGE_START_OBJ:
        case OWN_ATTR_EDGE_END_OBJ:
        case OWN_ATTR_GLUEID_HEAD:
        case OWN_ATTR_GLUEID_TAIL:
        case OWN_ATTR_EDGE_START_POS:
        case OWN_ATTR_EDGE_END_POS:
        {
            SdrEdgeObj* pEdgeObj = PTR_CAST(SdrEdgeObj,pObj);
            if(pEdgeObj)
            {
                switch(pMap->nWID)
                {
                case OWN_ATTR_EDGE_START_OBJ:
                case OWN_ATTR_EDGE_END_OBJ:
                    {
                        Reference< drawing::XShape > xShape;
                        if( rVal >>= xShape )
                        {
                            SdrObject* pNode = GetSdrObjectFromXShape( xShape );
                            if( pNode )
                            {
                                pEdgeObj->ConnectToNode( pMap->nWID == OWN_ATTR_EDGE_START_OBJ, pNode );
                                pEdgeObj->setGluePointIndex( pMap->nWID == OWN_ATTR_EDGE_START_OBJ, -1 );
                                return;
                            }
                        }
                        break;
                    }

                case OWN_ATTR_EDGE_START_POS:
                case OWN_ATTR_EDGE_END_POS:
                    {
                        awt::Point aUnoPoint;
                        if( rVal >>= aUnoPoint )
                        {
                            Point aPoint( aUnoPoint.X, aUnoPoint.Y );
                            pEdgeObj->SetTailPoint( pMap->nWID == OWN_ATTR_EDGE_START_POS, aPoint );
                            return;
                        }
                        break;
                    }

                case OWN_ATTR_GLUEID_HEAD:
                case OWN_ATTR_GLUEID_TAIL:
                    {
                        sal_Int32 nId;
                        if( rVal >>= nId )
                        {
                            pEdgeObj->setGluePointIndex( pMap->nWID == OWN_ATTR_GLUEID_HEAD, nId );
                            return;
                        }
                    }
                }
            }
            break;
        }
        case OWN_ATTR_MEASURE_START_POS:
        case OWN_ATTR_MEASURE_END_POS:
        {
            SdrMeasureObj* pMeasureObj = PTR_CAST(SdrMeasureObj,pObj);
            awt::Point aUnoPoint;
            if(pMeasureObj && ( rVal >>= aUnoPoint ) )
            {
                Point aPoint( aUnoPoint.X, aUnoPoint.Y );

                pMeasureObj->NbcSetPoint( aPoint, pMap->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 );
                pMeasureObj->SendRepaintBroadcast();
                pMeasureObj->SetChanged();
                return;
            }
            break;
        }
        case OWN_ATTR_FILLBMP_MODE:
            do
            {
                drawing::BitmapMode eMode;
                if(!(rVal >>= eMode) )
                {
                    sal_Int32 nMode;
                    if(!(rVal >>= nMode))
                        break;

                    eMode = (drawing::BitmapMode)nMode;
                }
                pObj->SetItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                pObj->SetItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                return;
            }
            while(0);
            break;

        case SDRATTR_LAYERID:
        {
            sal_Int16 nLayerId;
            if( rVal >>= nLayerId )
            {
                SdrLayer* pLayer = pModel->GetLayerAdmin().GetLayerPerID((unsigned char)nLayerId);
                if( pLayer )
                {
                    pObj->SetLayer((unsigned char)nLayerId);
                    return;
                }
            }
            break;
        }

        case SDRATTR_LAYERNAME:
        {
            OUString aLayerName;
            if( rVal >>= aLayerName )
            {
                const SdrLayer* pLayer=pModel->GetLayerAdmin().GetLayer(aLayerName, TRUE);
                if( pLayer != NULL )
                {
                    pObj->SetLayer( pLayer->GetID() );
                    return;
                }
            }
            break;
        }
        case SDRATTR_ROTATEANGLE:
        {
            sal_Int32 nAngle;
            if( rVal >>= nAngle )
            {
                Point aRef1(pObj->GetSnapRect().Center());
                nAngle -= pObj->GetRotateAngle();
                if (nAngle!=0)
                {
                    double nSin=sin(nAngle*nPi180);
                    double nCos=cos(nAngle*nPi180);
                    pObj->Rotate(aRef1,nAngle,nSin,nCos);
                }
                return;
            }

            break;
        }

        case SDRATTR_SHEARANGLE:
        {
            sal_Int32 nShear;
            if( rVal >>= nShear )
            {
                nShear -= pObj->GetShearAngle();
                if(nShear != 0 )
                {
                    Point aRef1(pObj->GetSnapRect().Center());
                    double nTan=tan(nShear*nPi180);
                    pObj->Shear(aRef1,nShear,nTan,FALSE);
                    return;
                }
            }

            break;
        }

        case SDRATTR_OBJMOVEPROTECT:
        {
            sal_Bool bMoveProtect;
            if( rVal >>= bMoveProtect )
            {
                pObj->SetMoveProtect(bMoveProtect);
                return;
            }
            break;
        }
        case SDRATTR_OBJECTNAME:
        {
            OUString aName;
            if( rVal >>= aName )
            {
                pObj->SetName( aName );
                return;
            }
            break;
        }
        case SDRATTR_OBJPRINTABLE:
        {
            sal_Bool bPrintable;
            if( rVal >>= bPrintable )
            {
                pObj->SetPrintable(bPrintable);
                return;
            }
            break;
        }
        case SDRATTR_OBJSIZEPROTECT:
        {
            sal_Bool bResizeProtect;
            if( rVal >>= bResizeProtect )
            {
                pObj->SetResizeProtect(bResizeProtect);
                return;
            }
            break;
        }

        case XATTR_FILLBITMAP:
        case XATTR_FILLGRADIENT:
        case XATTR_FILLHATCH:
        case XATTR_FILLFLOATTRANSPARENCE:
        case XATTR_LINEEND:
        case XATTR_LINESTART:
        case XATTR_LINEDASH:
        {
            if( pMap->nMemberId == MID_NAME )
            {
                OUString aApiName;
                if( rVal >>= aApiName )
                {
                    if( SetFillAttribute( pMap->nWID, aApiName ) )
                        return;
                }
                break;
            }

            // warning, this fall-through is intended
        }
        default:
        {
            DBG_ASSERT( pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST, "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
//-/                pObj->TakeAttributes( aSet, sal_False, sal_False );
            aSet.Put(pObj->GetItem(pMap->nWID));

            if( SvxUnoTextRangeBase::SetPropertyValueHelper( aSet, pMap, rVal, aSet ))
                return;

            if(!aSet.Count())
            {
                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // Not-Persistant Attribute, hole diese extra
                    pObj->TakeNotPersistAttr(aSet, sal_False);
                }
            }

            if(!aSet.Count())
            {
//-/                    if(pMap->nWID >= SID_ATTR_3D_START && pMap->nWID <= SID_ATTR_3D_END)
//-/                    {
//-/                        // 3D-Attribut, eigenen Default
//-/                        // Diese sollten IMMER gesetzt sein, da TakeAttributes an
//-/                        // 3D-Objekten alle Items erzeugt und eintraegt
//-/                        DBG_ERROR("AW: Got NO default item from group SID_ATTR_3D_");
//-/                    }
//-/                    else
//-/                    {
                // Default aus ItemPool holen
                if(pModel->GetItemPool().IsWhich(pMap->nWID))
                    aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));
//-/                    }
            }

            if(aSet.Count())
            {
                SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(pMap->nWID);
                if(pMap->nMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
                {
                    // Umrechnen auf Metrik des ItemPools in 100stel mm
                    // vorkommende Typen: sal_Int32, sal_uInt32, sal_uInt16
                    uno::Any aVal( rVal );

                    switch(eMapUnit)
                    {
                        case SFX_MAPUNIT_TWIP :
                        {
                            if( rVal.getValueType() == ::getCppuType(( const sal_Int32 *)0))
                                aVal <<= (sal_Int32)(MM_TO_TWIPS(*(sal_Int32*)rVal.getValue()));
                            else if( rVal.getValueType() == ::getCppuType(( const sal_uInt32*)0))
                                aVal <<= (sal_uInt32)(MM_TO_TWIPS(*(sal_uInt32*)rVal.getValue()));
                            else if( rVal.getValueType() == ::getCppuType(( const sal_uInt16*)0))
                                aVal <<= (sal_uInt16)(MM_TO_TWIPS(*(sal_uInt16*)rVal.getValue()));
                            else
                                DBG_ERROR("AW: Missing unit translation to PoolMetrics!");
                            break;
                        }
                        default:
                        {
                            DBG_ERROR("AW: Missing unit translation to PoolMetrics!");
                        }
                    }
                    aPropSet.setPropertyValue( pMap, aVal, aSet );
                }
                else
                {
                    aPropSet.setPropertyValue( pMap, rVal, aSet );
                }

                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // Not-Persist Attribute extra setzen
                    pObj->ApplyNotPersistAttr( aSet );
                }
                else
                {
//-/                        pObj->SetAttributes( aSet, sal_False );
//-/                        SdrBroadcastItemChange aItemChange(*pObj);
                    pObj->SetItemSetAndBroadcast(aSet);
//-/                        pObj->BroadcastItemChange(aItemChange);
                }
            }
            return;
        }
        }

        throw lang::IllegalArgumentException();
    }
    else
    {
        // since we have no actual sdr object right now
        // remember all properties in a list. These
        // properties will be set when the sdr object is
        // created

        if(pMap && pMap->nWID)
// Fixme: We should throw a UnknownPropertyException here.
//        But since this class is aggregated from classes
//        that support additional properties that we don't
//        know here we silently store *all* properties, even
//        if they may be not supported after creation
            aPropSet.setPropertyValue( pMap, rVal );
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShape::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException,
            lang::WrappedTargetException,
                uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    uno::Any aAny;
    if( pObj && pModel )
    {
        if(pMap == NULL )
            throw beans::UnknownPropertyException();

        switch( pMap->nWID )
        {
            case OWN_ATTR_INTERNAL_OLE:
            {
#ifndef SVX_LIGHT
                sal_Bool bInternal = sal_False;
                if( pObj && pObj->ISA(SdrOle2Obj))
                {
                    const SvInPlaceObjectRef& rIPRef = ((SdrOle2Obj*)pObj)->GetObjRef();
                    if (rIPRef.Is() )
                    {
                        const SvGlobalName &rClassName = rIPRef->GetClassName();
                        bInternal = SvFactory::IsIntern( rClassName, 0 );
                    }
                }
                aAny = uno::Any( &bInternal, ::getBooleanCppuType() );
#endif
                break;
            }
            case OWN_ATTR_TRANSFORMATION:
            {
                Matrix3D aMatrix3D;
                XPolyPolygon aPolyPolygon;
                pObj->TRGetBaseGeometry( aMatrix3D, aPolyPolygon );

                drawing::HomogenMatrix3 aMatrix;
                aMatrix.Line1.Column1 = aMatrix3D[0].X();
                aMatrix.Line1.Column2 = aMatrix3D[0].Y();
                aMatrix.Line1.Column3 = aMatrix3D[0].W();

                aMatrix.Line2.Column1 = aMatrix3D[1].X();
                aMatrix.Line2.Column2 = aMatrix3D[1].Y();
                aMatrix.Line2.Column3 = aMatrix3D[1].W();

                aMatrix.Line3.Column1 = aMatrix3D[2].X();
                aMatrix.Line3.Column2 = aMatrix3D[2].Y();
                aMatrix.Line3.Column3 = aMatrix3D[2].W();
                aAny <<= aMatrix;
                break;
            }
            case OWN_ATTR_ZORDER:
            {
                aAny <<= (sal_Int32)pObj->GetOrdNum();
                break;
            }
            case OWN_ATTR_BITMAP:
            {
                aAny = GetBitmap();
                if(!aAny.hasValue())
                    throw uno::RuntimeException();

                break;
            }
            case OWN_ATTR_WRITINGMODE:
            {
                if( pObj && pObj->ISA(SdrTextObj) )
                {
                    SdrTextObj* pText = (SdrTextObj*)pObj;
                    text::WritingMode eMode = pText->IsVerticalWriting() ? text::WritingMode_TB_RL : text::WritingMode_LR_TB;
                    aAny <<= eMode;
                    break;
                }
            }
            case OWN_ATTR_ISFONTWORK:
            {
                sal_Bool bIsFontWork = pObj->ISA(SdrTextObj) && ((SdrTextObj*)pObj)->IsFontwork();
                aAny.setValue( &bIsFontWork, ::getBooleanCppuType() );
                break;
            }
            case OWN_ATTR_FRAMERECT:
            {
                Rectangle aRect( pObj->GetSnapRect() );
                Point aTopLeft( aRect.TopLeft() );
                Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
                ForceMetricTo100th_mm(aTopLeft);
                ForceMetricTo100th_mm(aObjSize);
                ::com::sun::star::awt::Rectangle aUnoRect(
                    aTopLeft.X(), aTopLeft.Y(),
                    aObjSize.getWidth(), aObjSize.getHeight() );
                aAny <<= aUnoRect;
                break;
            }
            case OWN_ATTR_BOUNDRECT:
            {
                Rectangle aRect( pObj->GetBoundRect() );
                Point aTopLeft( aRect.TopLeft() );
                Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
                ForceMetricTo100th_mm(aTopLeft);
                ForceMetricTo100th_mm(aObjSize);
                ::com::sun::star::awt::Rectangle aUnoRect(
                    aTopLeft.X(), aTopLeft.Y(),
                    aObjSize.getWidth(), aObjSize.getHeight() );
                aAny <<= aUnoRect;
                break;
            }
            case OWN_ATTR_LDNAME:
            {
                OUString aName( pObj->GetName() );
                aAny <<= aName;
                break;
            }
            case OWN_ATTR_LDBITMAP:
            {
                sal_uInt16 nId;
                if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
                {
                    nId = RID_UNODRAW_OLE2;
                }
                else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
                {
                    nId = RID_UNODRAW_GRAPHICS;
                }
                else
                {
                    nId = RID_UNODRAW_OBJECTS;
                }

                BitmapEx aBmp( SVX_RES(nId) );
                Reference< awt::XBitmap > xBmp( VCLUnoHelper::CreateBitmap( aBmp ) );

                aAny <<= xBmp;
                break;
            }
            case OWN_ATTR_OLESIZE:
            {
                awt::Size aSize;
#ifndef SVX_LIGHT
                if( pObj->ISA(SdrOle2Obj))
                {
                    SdrOle2Obj& aObj = *(SdrOle2Obj*)pObj;
                    const SvInPlaceObjectRef& xInplace = aObj.GetObjRef();
                    if( xInplace.Is() )
                    {
                        Size aTmpSize( xInplace->GetVisArea().GetSize() );
                        aSize = awt::Size( aTmpSize.Width(), aTmpSize.Height() );
                    }
                }
#endif
                aAny <<= aSize;
                break;
            }
            case OWN_ATTR_OLEMODEL:
            {
                uno::Reference< frame::XModel > xModel;
                if( pObj->ISA(SdrOle2Obj))
                {
                    SdrOle2Obj& aObj = *(SdrOle2Obj*)pObj;
                    xModel = ((SdrOle2Obj*)pObj)->getXModel();
                }
                aAny <<= xModel;
                break;
            }
            case OWN_ATTR_MIRRORED:
            {
                sal_Bool bMirror = sal_False;
                if( pObj && pObj->ISA(SdrGrafObj) )
                    bMirror = ((SdrGrafObj*)pObj)->IsMirrored();

                return uno::Any( &bMirror, ::getCppuBooleanType() );
            }
            case OWN_ATTR_CLSID:
            {
                OUString aCLSID;
#ifndef SVX_LIGHT
                if( pObj && pObj->ISA(SdrOle2Obj))
                {
                    const SvInPlaceObjectRef& rIPRef = ((SdrOle2Obj*)pObj)->GetObjRef();
                    if (rIPRef.Is() )
                    {
                        const SvGlobalName &rClassName = rIPRef->GetClassName();
                        aCLSID = rClassName.GetHexName();
                    }
                }
#endif
                aAny <<= aCLSID;
                break;
            }
            case OWN_ATTR_METAFILE:
            {
                if( pObj->ISA(SdrOle2Obj))
                {
                    SdrOle2Obj& aObj = *(SdrOle2Obj*)pObj;

                    if(aObj.HasGDIMetaFile() && aObj.GetGDIMetaFile())
                    {
                        SvMemoryStream aDestStrm( 65535, 65535 );

                        ConvertGDIMetaFileToWMF( *aObj.GetGDIMetaFile(), aDestStrm, NULL, NULL, sal_False );
                        uno::Sequence<sal_Int8> aSeq((sal_Int8*)aDestStrm.GetData(), aDestStrm.GetSize());
                        aAny <<= aSeq;
                    }
                }
                else
                {
                    aAny = GetBitmap( sal_True );
                }
                break;
            }
            case OWN_ATTR_EDGE_START_OBJ:
            case OWN_ATTR_EDGE_START_POS:
            case OWN_ATTR_EDGE_END_POS:
            case OWN_ATTR_EDGE_END_OBJ:
            case OWN_ATTR_GLUEID_HEAD:
            case OWN_ATTR_GLUEID_TAIL:
            {
                SdrEdgeObj* pEdgeObj = PTR_CAST(SdrEdgeObj,pObj);
                if(pEdgeObj)
                {
                    switch(pMap->nWID)
                    {
                    case OWN_ATTR_EDGE_START_OBJ:
                    case OWN_ATTR_EDGE_END_OBJ:
                        {
                            SdrObject* pNode = pEdgeObj->GetConnectedNode(pMap->nWID == OWN_ATTR_EDGE_START_OBJ);
                            if(pNode)
                            {
                                Reference< drawing::XShape > xShape( GetXShapeForSdrObject( pNode ) );
                                if(xShape.is())
                                    aAny <<= xShape;

                            }
                            break;
                        }

                    case OWN_ATTR_EDGE_START_POS:
                    case OWN_ATTR_EDGE_END_POS:
                        {
                            Point aPoint( pEdgeObj->GetTailPoint( pMap->nWID == OWN_ATTR_EDGE_START_POS ) );
                            awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );
                            aAny <<= aUnoPoint;
                            break;
                        }
                    case OWN_ATTR_GLUEID_HEAD:
                    case OWN_ATTR_GLUEID_TAIL:
                        {
                            aAny <<= pEdgeObj->getGluePointIndex( pMap->nWID == OWN_ATTR_GLUEID_HEAD );
                            break;
                        }
                    }
                }
                break;
            }
            case OWN_ATTR_MEASURE_START_POS:
            case OWN_ATTR_MEASURE_END_POS:
            {
                SdrMeasureObj* pMeasureObj = PTR_CAST(SdrMeasureObj,pObj);
                if(pMeasureObj)
                {
                    const Point& rPoint = pMeasureObj->GetPoint( pMap->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 );
                    awt::Point aUnoPoint( rPoint.X(), rPoint.Y() );
                    aAny <<= aUnoPoint;
                    break;
                }
                break;
            }
            case OWN_ATTR_FILLBMP_MODE:
            {
                XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)&pObj->GetItem(XATTR_FILLBMP_STRETCH);
                XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)&pObj->GetItem(XATTR_FILLBMP_TILE);

                if( pTileItem && pTileItem->GetValue() )
                {
                    aAny <<= drawing::BitmapMode_REPEAT;
                }
                else if( pStretchItem && pStretchItem->GetValue() )
                {
                    aAny <<= drawing::BitmapMode_STRETCH;
                }
                else
                {
                    aAny <<= drawing::BitmapMode_NO_REPEAT;
                }
                break;
            }
            case SDRATTR_LAYERID:
                aAny <<= (sal_Int16)pObj->GetLayer();
                break;
            case SDRATTR_LAYERNAME:
                {
                    SdrLayer* pLayer = pModel->GetLayerAdmin().GetLayerPerID(pObj->GetLayer());
                    if( pLayer )
                    {
                        OUString aName( pLayer->GetName() );
                        aAny <<= aName;
                    }
                }
                break;
            case SDRATTR_ROTATEANGLE:
                aAny <<= pObj->GetRotateAngle();
                break;
            case SDRATTR_SHEARANGLE:
                aAny <<= pObj->GetShearAngle();
                break;
            case SDRATTR_OBJMOVEPROTECT:
                aAny = bool2any( pObj->IsMoveProtect() );
                break;
            case SDRATTR_OBJECTNAME:
            {
                OUString aName( pObj->GetName() );
                aAny <<= aName;
                break;
            }
            case SDRATTR_OBJPRINTABLE:
                aAny = bool2any( pObj->IsPrintable() );
                break;
            case SDRATTR_OBJSIZEPROTECT:
                aAny = bool2any( pObj->IsResizeProtect() );
                break;

            default:
            {
                DBG_ASSERT( pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST, "Not persist item not handled!" );
                DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

                SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
//-/                pObj->TakeAttributes( aSet, sal_False, sal_False );
//-/                aSet.Put(pObj->GetItemSet());
                aSet.Put(pObj->GetItem(pMap->nWID));

                if(SvxUnoTextRangeBase::GetPropertyValueHelper(  aSet, pMap, aAny ))
                    return aAny;

                if(!aSet.Count())
                {
                    if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                    {
                        // Not-Persistant Attribute, hole diese extra
                        pObj->TakeNotPersistAttr(aSet, sal_False);
                    }
                }

                if(!aSet.Count())
                {
//-/                    if(pMap->nWID >= SID_ATTR_3D_START && pMap->nWID <= SID_ATTR_3D_END)
//-/                    {
//-/                        // 3D-Attribut, eigenen Default
//-/                        // Diese sollten IMMER gesetzt sein, da TakeAttributes an
//-/                        // 3D-Objekten alle Items erzeugt und eintraegt
//-/                        DBG_ERROR("AW: Got NO default item from group SID_ATTR_3D_");
//-/                    }
//-/                    else
//-/                    {
                    // Default aus ItemPool holen
                    if(pModel->GetItemPool().IsWhich(pMap->nWID))
                        aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));
//-/                    }
                }

                if(aSet.Count())
                    aAny = GetAnyForItem( aSet, pMap );
            }
        }
    }
    else
    {

// Fixme: we should return default values for OWN_ATTR !

        if(pMap && pMap->nWID)
//      FixMe: see setPropertyValue
            aAny = aPropSet.getPropertyValue( pMap );

    }
    return aAny;
}

uno::Any SvxShape::GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertyMap* pMap ) const
{
    uno::Any aAny;

    switch(pMap->nWID)
    {
    case SDRATTR_CIRCSTARTANGLE:
    {
        const SfxPoolItem* pPoolItem=NULL;
        if(aSet.GetItemState(SDRATTR_CIRCSTARTANGLE,sal_False,&pPoolItem)==SFX_ITEM_SET)
        {
            sal_Int32 nAngle = ((SdrCircStartAngleItem*)pPoolItem)->GetValue();
            aAny <<= nAngle;
        }
        break;
    }

    case SDRATTR_CIRCENDANGLE:
    {
        const SfxPoolItem* pPoolItem=NULL;
        if (aSet.GetItemState(SDRATTR_CIRCENDANGLE,sal_False,&pPoolItem)==SFX_ITEM_SET)
        {
            sal_Int32 nAngle = ((SdrCircEndAngleItem*)pPoolItem)->GetValue();
            aAny <<= nAngle;
        }
        break;
    }

    case SDRATTR_CIRCKIND:
    {
        if( pObj->GetObjInventor() == SdrInventor)
        {
            drawing::CircleKind eKind;
            switch(pObj->GetObjIdentifier())
            {
            case OBJ_CIRC:          // Kreis, Ellipse
                eKind = drawing::CircleKind_FULL;
                break;
            case OBJ_CCUT:          // Kreisabschnitt
                eKind = drawing::CircleKind_CUT;
                break;
            case OBJ_CARC:          // Kreisbogen
                eKind = drawing::CircleKind_ARC;
                break;
            case OBJ_SECT:          // Kreissektor
                eKind = drawing::CircleKind_SECTION;
                break;
            }
            aAny <<= eKind;
        }
        break;
    }
    default:
    {
        // Hole Wert aus ItemSet
        aAny = aPropSet.getPropertyValue( pMap, aSet );

        // eventuell umrechnen der Metrik auf 100stel mm noetig
        SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(pMap->nWID);
        if(pMap->nMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            // Umrechnen auf Metrik des ItemPools
            // vorkommende Typen: sal_Int32, sal_uInt32, sal_uInt16
            switch(eMapUnit)
            {
            case SFX_MAPUNIT_TWIP :
            {
                switch( aAny.getValueTypeClass() )
                {
                case uno::TypeClass_BYTE:
                    aAny <<= (sal_Int8)(TWIPS_TO_MM(*(sal_Int8*)aAny.getValue()));
                    break;
                case uno::TypeClass_SHORT:
                    aAny <<= (sal_Int16)(TWIPS_TO_MM(*(sal_Int16*)aAny.getValue()));
                    break;
                case uno::TypeClass_UNSIGNED_SHORT:
                    aAny <<= (sal_uInt16)(TWIPS_TO_MM(*(sal_uInt16*)aAny.getValue()));
                    break;
                case uno::TypeClass_LONG:
                    aAny <<= (sal_Int32)(TWIPS_TO_MM(*(sal_Int32*)aAny.getValue()));
                    break;
                case uno::TypeClass_UNSIGNED_LONG:
                    aAny <<= (sal_uInt32)(TWIPS_TO_MM(*(sal_uInt32*)aAny.getValue()));
                    break;
                default:
                    DBG_ERROR("AW: Missing unit translation to 100th mm!");
                }
                break;
            }
            default:
            {
                DBG_ERROR("AW: Missing unit translation to 100th mm!");
            }
            }
        }
    }
    }

    return aAny;
}

// XPropertyState
beans::PropertyState SAL_CALL SvxShape::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    if( pObj == NULL || pMap == NULL )
        throw beans::UnknownPropertyException();

    if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = pObj->GetItemSet();

        if( rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
            rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
        {
            return beans::PropertyState_DIRECT_VALUE;
        }
        else
        {
            return beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else if(( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        const SfxItemSet& rSet = pObj->GetItemSet();

        switch( rSet.GetItemState( pMap->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            return beans::PropertyState_DIRECT_VALUE;
        case SFX_ITEM_DEFAULT:
            return beans::PropertyState_DEFAULT_VALUE;
        case SFX_ITEM_UNKNOWN:
        case SFX_ITEM_DONTCARE:
        case SFX_ITEM_DISABLED:
        default:
            return beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
}

uno::Sequence< beans::PropertyState > SAL_CALL SvxShape::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    const sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aRet( nCount );
    beans::PropertyState* pState = aRet.getArray();;

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
        pState[nIdx] = getPropertyState( pNames[nIdx] );

    return aRet;
}

void SAL_CALL SvxShape::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    if( pObj == NULL || pModel == NULL || pMap == NULL )
        throw beans::UnknownPropertyException();

    if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        pObj->ClearItem( XATTR_FILLBMP_STRETCH );
        pObj->ClearItem( XATTR_FILLBMP_TILE );
    }
    else if((pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return;
    }
    else
    {
        pObj->ClearItem( pMap->nWID );
    }

    pModel->SetChanged();
}

uno::Any SAL_CALL SvxShape::getPropertyDefault( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pObj == NULL || pMap == NULL )
        throw beans::UnknownPropertyException();

    if(( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return getPropertyValue( aPropertyName );
    }
    else
    {
        // Default aus ItemPool holen
        if(!pModel->GetItemPool().IsWhich(pMap->nWID))
            throw beans::UnknownPropertyException();

        SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
        aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));

        return GetAnyForItem( aSet, pMap );
    }
}

//----------------------------------------------------------------------
// XServiceInfo
//----------------------------------------------------------------------
OUString SAL_CALL SvxShape::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxShape") );
}

#define STAR_NAMESPACE "com.sun.star."

const char* sUNO_service_style_ParagraphProperties          = STAR_NAMESPACE "style.ParagraphProperties";
const char* sUNO_service_style_CharacterProperties          = STAR_NAMESPACE "style.CharacterProperties";

const char* sUNO_service_drawing_FillProperties             = STAR_NAMESPACE "drawing.FillProperties";
const char* sUNO_service_drawing_TextProperties             = STAR_NAMESPACE "drawing.TextProperties";
const char* sUNO_service_drawing_LineProperties             = STAR_NAMESPACE "drawing.LineProperties";
const char* sUNO_service_drawing_ConnectorProperties        = STAR_NAMESPACE "drawing.ConnectorProperties";
const char* sUNO_service_drawing_MeasureProperties          = STAR_NAMESPACE "drawing.MeasureProperties";
const char* sUNO_service_drawing_ShadowProperties           = STAR_NAMESPACE "drawing.ShadowProperties";

const char* sUNO_service_drawing_RotationDescriptor         = STAR_NAMESPACE "drawing.RotationDescriptor";

const char* sUNO_service_drawing_Text                       = STAR_NAMESPACE "drawing.Text";
const char* sUNO_service_drawing_GroupShape                 = STAR_NAMESPACE "drawing.GroupShape";

const char* sUNO_service_drawing_PolyPolygonDescriptor      = STAR_NAMESPACE "drawing.PolyPolygonDescriptor";
const char* sUNO_service_drawing_PolyPolygonBezierDescriptor= STAR_NAMESPACE "drawing.PolyPolygonBezierDescriptor";

const char* sUNO_service_drawing_LineShape                  = STAR_NAMESPACE "drawing.LineShape";
const char* sUNO_service_drawing_Shape                      = STAR_NAMESPACE "drawing.Shape";
const char* sUNO_service_drawing_RectangleShape             = STAR_NAMESPACE "drawing.RectangleShape";
const char* sUNO_service_drawing_EllipseShape               = STAR_NAMESPACE "drawing.EllipseShape";
const char* sUNO_service_drawing_PolyPolygonShape           = STAR_NAMESPACE "drawing.PolyPolygonShape";
const char* sUNO_service_drawing_PolyLineShape              = STAR_NAMESPACE "drawing.PolyLineShape";
const char* sUNO_service_drawing_OpenBezierShape            = STAR_NAMESPACE "drawing.OpenBezierShape";
const char* sUNO_service_drawing_ClosedBezierShape          = STAR_NAMESPACE "drawing.ClosedBezierShape";
const char* sUNO_service_drawing_TextShape                  = STAR_NAMESPACE "drawing.TextShape";
const char* sUNO_service_drawing_GraphicObjectShape         = STAR_NAMESPACE "drawing.GraphicObjectShape";
const char* sUNO_service_drawing_OLE2Shape                  = STAR_NAMESPACE "drawing.OLE2Shape";
const char* sUNO_service_drawing_PageShape                  = STAR_NAMESPACE "drawing.PageShape";
const char* sUNO_service_drawing_CaptionShape               = STAR_NAMESPACE "drawing.CaptionShape";
const char* sUNO_service_drawing_MeasureShape               = STAR_NAMESPACE "drawing.MeasureShape";
const char* sUNO_service_drawing_FrameShape                 = STAR_NAMESPACE "drawing.FrameShape";
const char* sUNO_service_drawing_ControlShape               = STAR_NAMESPACE "drawing.ControlShape";
const char* sUNO_service_drawing_ConnectorShape             = STAR_NAMESPACE "drawing.ConnectorShape";

uno::Sequence< OUString > SAL_CALL SvxShape::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq;

    GET_TEXT_INTERFACE( XServiceInfo, xInfo );

    if( xInfo.is() )
        aSeq = xInfo->getSupportedServiceNames();

    if( pObj && pObj->GetObjInventor() == SdrInventor)
    {
        const UINT16 nIdent = pObj->GetObjIdentifier();

        switch(nIdent)
        {
        case OBJ_GRUP:
            addToSequence( aSeq, 2, sUNO_service_drawing_GroupShape,

                                    sUNO_service_drawing_Shape );
            break;
        case OBJ_LINE:
            addToSequence( aSeq,10, sUNO_service_drawing_LineShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_PolyPolygonDescriptor,
                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_RECT:
            addToSequence( aSeq,10, sUNO_service_drawing_RectangleShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_FillProperties,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
            addToSequence( aSeq,10, sUNO_service_drawing_EllipseShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_FillProperties,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_PATHPLIN:
        case OBJ_PLIN:
            addToSequence( aSeq,10, sUNO_service_drawing_PolyLineShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_PolyPolygonDescriptor,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_PATHPOLY:
        case OBJ_POLY:
            addToSequence( aSeq,11, sUNO_service_drawing_PolyPolygonShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,
                                    sUNO_service_drawing_FillProperties,

                                    sUNO_service_drawing_PolyPolygonDescriptor,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_FREELINE:
        case OBJ_PATHLINE:
            addToSequence( aSeq,11, sUNO_service_drawing_OpenBezierShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,
                                    sUNO_service_drawing_FillProperties,

                                    sUNO_service_drawing_PolyPolygonBezierDescriptor,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
            addToSequence( aSeq,11, sUNO_service_drawing_ClosedBezierShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,
                                    sUNO_service_drawing_FillProperties,

                                    sUNO_service_drawing_PolyPolygonBezierDescriptor,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_OUTLINETEXT:
        case OBJ_TITLETEXT:
        case OBJ_TEXT:
            addToSequence( aSeq,10, sUNO_service_drawing_TextShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_FillProperties,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
        break;

        case OBJ_GRAF:
            addToSequence( aSeq, 8, sUNO_service_drawing_GraphicObjectShape,

                                    sUNO_service_drawing_Shape,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_OLE2:
            addToSequence( aSeq, 2, sUNO_service_drawing_OLE2Shape,
                                    sUNO_service_drawing_Shape);
            break;

        case OBJ_CAPTION:
            addToSequence( aSeq,10, sUNO_service_drawing_CaptionShape,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_FillProperties,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_PAGE:
            addToSequence( aSeq, 2, sUNO_service_drawing_PageShape,
                                    sUNO_service_drawing_Shape );
            break;

        case OBJ_MEASURE:
            addToSequence( aSeq,11, sUNO_service_drawing_MeasureShape,

                                    sUNO_service_drawing_MeasureProperties,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_PolyPolygonDescriptor,
                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        case OBJ_FRAME:
            addToSequence( aSeq, 2, sUNO_service_drawing_FrameShape,
                                    sUNO_service_drawing_Shape );
            break;

        case OBJ_UNO:
            addToSequence( aSeq, 2, sUNO_service_drawing_ControlShape,
                                    sUNO_service_drawing_Shape );
            break;
        case OBJ_EDGE:
            addToSequence( aSeq,11, sUNO_service_drawing_ConnectorShape,

                                    sUNO_service_drawing_ConnectorProperties,

                                    sUNO_service_drawing_Shape,
                                    sUNO_service_drawing_LineProperties,

                                    sUNO_service_drawing_Text,
                                    sUNO_service_drawing_TextProperties,
                                    sUNO_service_style_ParagraphProperties,
                                    sUNO_service_style_CharacterProperties,

                                    sUNO_service_drawing_PolyPolygonDescriptor,
                                    sUNO_service_drawing_ShadowProperties,
                                    sUNO_service_drawing_RotationDescriptor);
            break;

        return aSeq;

        }
    }

    return aSeq;
}

// XGluePointsSupplier
uno::Reference< container::XIndexContainer > SAL_CALL SvxShape::getGluePoints()
    throw(uno::RuntimeException)
{
    uno::Reference< container::XIndexContainer > xGluePoints( mxGluePoints );

    if( !xGluePoints.is() )
    {
        uno::Reference< container::XIndexContainer > xNew( SvxUnoGluePointAccess_createInstance( pObj ), uno::UNO_QUERY );
        mxGluePoints = xGluePoints = xNew;
    }

    return xGluePoints;
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL SvxShape::getParent(  )
    throw(uno::RuntimeException)
{

    if( pObj && pObj->GetObjList() )
    {
        SdrObjList* pObjList = pObj->GetObjList();

        switch( pObjList->GetListKind() )
        {
        case SDROBJLIST_GROUPOBJ:
            if( pObjList->GetOwnerObj()->ISA( SdrObjGroup ) )
                return PTR_CAST( SdrObjGroup, pObjList->GetOwnerObj())->getUnoShape();
            else if( pObjList->GetOwnerObj()->ISA( E3dScene ) )
                return PTR_CAST( E3dScene, pObjList->GetOwnerObj())->getUnoShape();
            break;
        case SDROBJLIST_DRAWPAGE:
        case SDROBJLIST_MASTERPAGE:
            return PTR_CAST( SdrPage, pObjList )->getUnoPage();
        }


        DBG_ERROR( "SvxShape::getParent(  ): unexpected SdrObjListKind" );
    }

    uno::Reference< uno::XInterface > xParent;
    return xParent;
}

void SAL_CALL SvxShape::setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent )
    throw(lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}

/************6***********************************************************
* class SvxShapeRect                                                   *
***********************************************************************/
SvxShapeRect::SvxShapeRect( SdrObject* pObj ) throw()
: SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_SHAPE) )
{

}

SvxShapeRect::~SvxShapeRect() throw()
{
}

uno::Any SAL_CALL SvxShapeRect::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeRect::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return SvxShape::queryAggregation( rType );
}

void SAL_CALL SvxShapeRect::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShapeRect::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}
//----------------------------------------------------------------------
// XServiceInfo
//----------------------------------------------------------------------
uno::Sequence< OUString > SvxShapeRect::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/** returns a StarOffice API wrapper for the given SdrObject */
uno::Reference< drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ()
{
    uno::Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
    return xShape;
}

/** returns the SdrObject from the given StarOffice API wrapper */
SdrObject* GetSdrObjectFromXShape( uno::Reference< drawing::XShape > xShape ) throw()
{
    SvxShape* pShape = SvxShape::getImplementation( xShape );
    return pShape ? pShape->GetSdrObject() : NULL;
}