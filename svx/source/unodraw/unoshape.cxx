/*************************************************************************
 *
 *  $RCSfile: unoshape.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-27 10:42:32 $
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
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#include <toolkit/unohlp.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
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

#include <tools/shl.hxx>    //
#include "dialmgr.hxx"      // not nice, we need our own resources some day
#include "dialogs.hrc"      //

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

class GDIMetaFile;
class SvStream;
sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              PFilterCallback pCallback=NULL, void * pCallerData=NULL,
                              sal_Bool bPlaceable=sal_True);

uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject );

DECLARE_LIST( SvxShapeList, SvxShape * );

SvxShapeList* SvxShape::m_pGlobalShapeList = NULL;

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

    if( m_pGlobalShapeList != NULL )
    {
        m_pGlobalShapeList->Remove(this);
        if( m_pGlobalShapeList->Count() == 0 )
        {
            delete m_pGlobalShapeList;
            m_pGlobalShapeList = NULL;
        }
    }
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( m_pGlobalShapeList != NULL )
    {
        for( SvxShape* pShape = m_pGlobalShapeList->First();
                       pShape;
                       pShape = m_pGlobalShapeList->Next() )
        {
            if( pShape->pObj == pObj )
                return pShape;
        }
    }

    return NULL;
}

//----------------------------------------------------------------------
void SvxShape::Init() throw()
{
    if(pObj == NULL)    // ab hier nur nocht mit Objekt
        return;

    {
        OGuard aGuard( Application::GetSolarMutex() );

        if(m_pGlobalShapeList == NULL)
            m_pGlobalShapeList = new SvxShapeList();

        if(m_pGlobalShapeList->GetPos(this) == LIST_ENTRY_NOTFOUND)
            m_pGlobalShapeList->Insert(this);
    }

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
            SID_ATTR_3D_START, SID_ATTR_3D_END,
            0);

        Reference< beans::XPropertySet > xShape( (OWeakObject*)this, UNO_QUERY );
        aPropSet.ObtainSettingsFromPropertySet(rPropSet, aSet, xShape);
        pObj->SetAttributes( aSet, sal_False );
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
        const sal_Int32 nOwnTypes = 6;      // !DANGER! Keep this updated!

        maTypeSequence.realloc( nBaseTypes  + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);

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

                OGuard aGuard( Application::GetSolarMutex() );
                if(m_pGlobalShapeList != NULL)
                    m_pGlobalShapeList->Remove(this);
            }
        }
        else if( pSdrHint->GetKind() == HINT_MODELCLEARED )
        {
            pModel = NULL;
        }
        else if( pSdrHint->GetKind() == HINT_OBJLISTCLEARED )
        {
            pObj = NULL;

            OGuard aGuard( Application::GetSolarMutex() );
            if(m_pGlobalShapeList != NULL)
                m_pGlobalShapeList->Remove(this);

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

    if( pObj && pModel)
    {
        Rectangle aRect( getLogicRectHack(pObj) );
        Point aLocalPos( Position.X, Position.Y );
        ForceMetricToItemPoolMetric(aLocalPos);

        // Position ist absolut, relativ zum Anker stellen
        aLocalPos -= pObj->GetAnchorPos();

        long nDX = aLocalPos.X() - aRect.Left();
        long nDY = aLocalPos.Y() - aRect.Top();

        pObj->Move( Size( nDX, nDY ) );
        pModel->SetChanged();
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
                delete pPage->NbcRemoveObject(nNum);
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
    SfxItemPropertySetInfo aSet( ImplGetSvxUnoOutlinerTextCursorPropertyMap() );

    Reference< beans::XPropertySetInfo > aRet (
        new SfxExtItemPropertySetInfo(
                aPropSet.getPropertyMap(),
                aSet.getProperties() ) );
    return aRet;
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addPropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::addVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removeVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName )
{
    SfxItemSet aSet( pModel->GetItemPool(), nWID, nWID );
    if( !SetFillAttribute( nWID, rName, aSet ) )
        return sal_False;

    pObj->SetAttributes( aSet, sal_False );

    return sal_True;
}

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet )
{
    const SfxItemPool* pPool = rSet.GetPool();

    const String aSearchName( rName );
    const USHORT nCount = pPool->GetItemCount(nWID);
    const NameOrIndex *pItem;

    for( USHORT nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)pPool->GetItem(nWID, nSurrogate);
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
        if(pMap && pMap->nWID)
        {
            // check for readonly
            if( ( pMap->nFlags & ::com::sun::star::beans::PropertyAttribute::READONLY ) != 0 )
                throw beans::UnknownPropertyException();

            switch( pMap->nWID )
            {
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
                                // create storage and inplace object
                                String aEmptyStr;
                                SvStorageRef aStor = new SvStorage( aEmptyStr, STREAM_STD_READWRITE );
                                SvInPlaceObjectRef aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit( aClassName, aStor);
                                ((SdrOle2Obj*)pObj)->SetObjRef(aIPObj);

                                Rectangle aRect( ( (SdrOle2Obj*) pObj)->GetLogicRect() );
                                aIPObj->SetVisAreaSize( aRect.GetSize() );

                                return;
                            }
                        }
                    }
                }
                break;
            }
            case OWN_ATTR_EDGE_START_OBJ:
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
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_FILLFLOATTRANSPARENCE:
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            // case XATTR_LINEDASH:
            {
                if( pMap->nMemberId == MID_NAME )
                {
                    OUString aStr;
                    if( rVal >>= aStr )
                        if( SetFillAttribute( pMap->nWID, aStr ) )
                            return;
                    break;
                }

                // warning, this fall-through is intended
            }
            default:
            {
                SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
                pObj->TakeAttributes( aSet, sal_False, sal_False );

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
                    if(pMap->nWID >= SID_ATTR_3D_START && pMap->nWID <= SID_ATTR_3D_END)
                    {
                        // 3D-Attribut, eigenen Default
                        // Diese sollten IMMER gesetzt sein, da TakeAttributes an
                        // 3D-Objekten alle Items erzeugt und eintraegt
                        DBG_ERROR("AW: Got NO default item from group SID_ATTR_3D_");
                    }
                    else
                    {
                        // Default aus ItemPool holen
                        if(pModel->GetItemPool().IsWhich(pMap->nWID))
                            aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));
                    }
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
                        pObj->SetAttributes( aSet, sal_False );
                    }
                }
                return;
            }
            }

            throw lang::IllegalArgumentException();
        }
        else
        {
            Reference< beans::XPropertySet > xPropertySet;
            if(!xTextAgg.is() ) { Reference< ::com::sun::star::text::XText > xText( (OWeakObject*)this, UNO_QUERY ); }
            if(!xTextAgg.is() )
            {
                uno::Any aAny(
                    xTextAgg->queryInterface( ::getCppuType((const Reference< beans::XPropertySet >*)0)));
                aAny >>= xPropertySet;
            }

            if( xPropertySet.is() )
                xPropertySet->setPropertyValue( rPropertyName, rVal );
        }

        pModel->SetChanged();
    }
    else
    {
        // since we have no actual sdr object right now
        // remember all properties in a list. These
        // properties will be set when the sdr object is
        // created

//      if(pMap && pMap->nWID)
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
        if(pMap && pMap->nWID)
        {
            switch( pMap->nWID )
            {
            case OWN_ATTR_BITMAP:
            {
                aAny = GetBitmap();
                if(!aAny.hasValue())
                    throw uno::RuntimeException();

                break;
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
                if( pObj && pObj->ISA(SdrOle2Obj))
                {
                    const SvInPlaceObjectRef& rIPRef = ((SdrOle2Obj*)pObj)->GetObjRef();
                    if (rIPRef.Is() )
                    {
                        const SvGlobalName &rClassName = rIPRef->GetClassName();
                        aCLSID = rClassName.GetHexName();
                    }
                }
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
                            aAny <<= pEdgeObj->getGluePointIndex( pMap->nWID == OWN_ATTR_GLUEID_TAIL );
                            break;
                        }
                    }
                }
                break;
            }
            default:
            {
                SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
                pObj->TakeAttributes( aSet, sal_False, sal_False );

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
                    if(pMap->nWID >= SID_ATTR_3D_START && pMap->nWID <= SID_ATTR_3D_END)
                    {
                        // 3D-Attribut, eigenen Default
                        // Diese sollten IMMER gesetzt sein, da TakeAttributes an
                        // 3D-Objekten alle Items erzeugt und eintraegt
                        DBG_ERROR("AW: Got NO default item from group SID_ATTR_3D_");
                    }
                    else
                    {
                        // Default aus ItemPool holen
                        if(pModel->GetItemPool().IsWhich(pMap->nWID))
                            aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));
                    }



                }

                if(aSet.Count())
                    aAny = GetAnyForItem( aSet, pMap );
            }
            }
        }
        else
        {
            GET_TEXT_INTERFACE( beans::XPropertySet, xPropertySet );
            if( xPropertySet.is() )
                aAny = xPropertySet->getPropertyValue( PropertyName );
        }
    }
    else
    {

// Fixme: we should return default values for OWN_ATTR !

//      if(pMap && pMap->nWID)
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
                if( aAny.getValueType() == ::getCppuType((const sal_Int32 *)0))
                    aAny <<= (sal_Int32)(TWIPS_TO_MM(*(sal_Int32*)aAny.getValue()));
                else if( aAny.getValueType() == ::getCppuType((const sal_uInt32*)0))
                    aAny <<= (sal_uInt32)(TWIPS_TO_MM(*(sal_uInt32*)aAny.getValue()));
                else if( aAny.getValueType() == ::getCppuType((const sal_uInt16*)0))
                    aAny <<= (sal_uInt16)(TWIPS_TO_MM(*(sal_uInt16*)aAny.getValue()));
                else
                    DBG_ERROR("AW: Missing unit translation to 100th mm!");
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

    if( pObj == NULL )
        throw uno::RuntimeException();

    if(pMap && pMap->nWID)
    {
        if(pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END)
        {
            return beans::PropertyState_DIRECT_VALUE;
        }
        else
        {
            SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
            pObj->TakeAttributes( aSet, sal_False, sal_True );

            if(!aSet.Count())
            {
                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // Not-Persistant Attribute, hole diese extra
                    pObj->TakeNotPersistAttr(aSet, sal_False);
                }
            }

            switch( aSet.GetItemState( pMap->nWID, sal_False ) )
            {
            case SFX_ITEM_DONTCARE:
            case SFX_ITEM_DISABLED:
                return beans::PropertyState_AMBIGUOUS_VALUE;
            case SFX_ITEM_READONLY:
            case SFX_ITEM_SET:
                return beans::PropertyState_DIRECT_VALUE;
            case SFX_ITEM_DEFAULT:
                return beans::PropertyState_DEFAULT_VALUE;
            case SFX_ITEM_UNKNOWN:
            default:
                throw beans::UnknownPropertyException();
            }
        }
    }
    else
    {
        GET_TEXT_INTERFACE( beans::XPropertyState, xPropertyState );
        if( xPropertyState.is() )
            return xPropertyState->getPropertyState( PropertyName );
        else
            return beans::PropertyState_AMBIGUOUS_VALUE;
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

    if( pObj == NULL || pModel == NULL )
        throw uno::RuntimeException();

    if(pMap && pMap->nWID)
    {
        if(pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END)
        {
            return;
        }
        else
        {
            SfxItemSet aSet( pModel->GetItemPool(), pMap->nWID, pMap->nWID);
            pObj->TakeAttributes( aSet, sal_False, sal_True );

            if(!aSet.Count())
            {
                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // Not-Persistant Attribute, hole diese extra
                    pObj->TakeNotPersistAttr(aSet, sal_False);
                }
            }

            aSet.ClearItem( pMap->nWID );

            if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
            {
                // Not-Persist Attribute extra setzen
                pObj->ApplyNotPersistAttr( aSet );
            }
            else
            {
                pObj->SetAttributes( aSet, sal_False );
            }
        }
    }
    else
    {
        GET_TEXT_INTERFACE( beans::XPropertyState, xPropertyState );

        if( xPropertyState.is() )
            xPropertyState->setPropertyToDefault( PropertyName );
    }

    pModel->SetChanged();
}

uno::Any SAL_CALL SvxShape::getPropertyDefault( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pObj == NULL )
        throw uno::RuntimeException();

    if(pMap && pMap->nWID)
    {
        if(pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END)
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
    else
    {
        GET_TEXT_INTERFACE( beans::XPropertyState, xPropertyState );

        if( xPropertyState.is() )
            return xPropertyState->getPropertyDefault( aPropertyName );
        else
            return uno::Any();
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

uno::Sequence< OUString > SAL_CALL SvxShape::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq;

    GET_TEXT_INTERFACE( XServiceInfo, xInfo );

    if( xInfo.is() )
        aSeq = xInfo->getSupportedServiceNames();

    addToSequence( aSeq, 2, "com.sun.star.drawing.ShapeDescriptor",
                            "com.sun.star.drawing.Shape");


    if( pObj && pObj->GetObjInventor() == SdrInventor)
    {
        switch(pObj->GetObjIdentifier())
        {
        case OBJ_TEXT:
            addToSequence( aSeq, 1, "com.sun.star.drawing.TextShape" );
            break;
        case OBJ_OLE2:
            addToSequence( aSeq, 1, "com.sun.star.drawing.OLE2Shape" );
            break;
        case OBJ_PAGE:
            addToSequence( aSeq, 1, "com.sun.star.drawing.PageShape" );
            break;
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

/***********************************************************************
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
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    addToSequence( aSeq, 7, "com.sun.star.drawing.AreaShapeDescriptor",
                            "com.sun.star.drawing.AreaShape",
                            "com.sun.star.drawing.LineShape",
                            "com.sun.star.drawing.RectangleShape",
                            "com.sun.star.drawing.Text",
                            "com.sun.star.drawing.RotationDescriptor",
                            "com.sun.star.drawing.ShadowDescriptor");
    return aSeq;
}

/** returns a StarOffice API wrapper for the given SdrObject */
uno::Reference< drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ()
{
    uno::Reference< drawing::XShape > xShape( SvxShape::GetShapeForSdrObj( pObj ) );
    return xShape;
}

/** returns the SdrObject from the given StarOffice API wrapper */
SdrObject* GetSdrObjectFromXShape( uno::Reference< drawing::XShape > xShape ) throw()
{
    SvxShape* pShape = SvxShape::getImplementation( xShape );
    return pShape ? pShape->GetSdrObject() : NULL;
}


