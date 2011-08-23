/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define _SVX_USE_UNOGLOBALS_

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _B2D_MATRIX3D_HXX
#include <bf_goodies/matrix3d.hxx>
#endif

#ifndef _FLTCALL_HXX
#include <bf_svtools/fltcall.hxx>
#endif

#include <toolkit/unohlp.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <bf_so3/outplace.hxx>

#ifndef _SVDOPAGE_HXX
#include "svdopage.hxx"
#endif
#ifndef _SVX_XFLBSTIT_HXX
#include "xflbstit.hxx"
#endif
#ifndef _SVX_XFLBMTIT_HXX
#include "xflbmtit.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif
#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif
#include "globl3d.hxx"
#include "fmglob.hxx"
#include "unopage.hxx"
#include "view3d.hxx"
#include "unoshape.hxx"
#include "svxids.hrc"
#include "unoshtxt.hxx"
#include "unoshprp.hxx"
#include "svdoole2.hxx"
#include "sxciaitm.hxx" // todo: remove
#include "svdograf.hxx"
#include "unoapi.hxx"
#include "svdomeas.hxx"

#include "dialogs.hrc"		//

#ifndef _SVDCAPT_HXX
#include "svdocapt.hxx"
#endif


#ifndef _SVX_XFLFTRIT_HXX
#include "xflftrit.hxx"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_XBTMPIT_HXX
#include "xbtmpit.hxx"
#endif


#ifndef _SVX_XFLHTIT_HXX
#include "xflhtit.hxx"
#endif

#ifndef _SVX_XLNEDIT_HXX
#include "xlnedit.hxx"
#endif

#ifndef _SVX_XLNSTIT_HXX
#include "xlnstit.hxx"
#endif

#ifndef _SVX_XLNDSIT_HXX
#include "xlndsit.hxx"
#endif

#include "svdstr.hrc"
#include "unomaster.hxx"

#ifndef _OUTLOBJ_HXX //autogen
#include <outlobj.hxx>
#endif

class GDIMetaFile;
class SvStream;

namespace binfilter {

using namespace ::osl;
using namespace ::vos;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

const SfxItemPropertyMap* ImplGetSvxUnoOutlinerTextCursorPropertyMap()
{
    // Propertymap fuer einen Outliner Text
    static const SfxItemPropertyMap aSvxUnoOutlinerTextCursorPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {MAP_CHAR_LEN("TextUserDefinedAttributes"),			EE_CHAR_XMLATTRIBS,		&::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  , 		0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),			EE_PARA_XMLATTRIBS,		&::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  , 		0,     0},
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
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {MAP_CHAR_LEN("TextField"),						EE_FEATURE_FIELD,	&::getCppuType((const uno::Reference< text::XTextField >*)0),	beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextPortionType"),				WID_PORTIONTYPE,	&::getCppuType((const ::rtl::OUString*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextUserDefinedAttributes"),			EE_CHAR_XMLATTRIBS,		&::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  , 		0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),			EE_PARA_XMLATTRIBS,		&::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  , 		0,     0},
        {0,0}
    };

    return aSvxTextPortionPropertyMap;
}

sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pConfigItem = NULL, sal_Bool bPlaceable = sal_True );
uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject );

DECLARE_LIST( SvxShapeList, SvxShape * )//STRIP008 ;

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

struct SvxShapeImpl
{
    SfxItemSet*		mpItemSet;
    sal_uInt32		mnObjId;
    SvxShapeMaster*	mpMaster;
};

//UNO3_GETIMPLEMENTATION_IMPL( SvxShape );

SvxShape::SvxShape( SdrObject* pObject ) throw()
:	aPropSet(aSvxMapProvider.GetMap(SVXMAP_SHAPE)),
    pObj	(pObject),
    bDisposing( sal_False ),
    pModel(NULL),
    aSize(100,100),
    mnLockCount(0),
    aDisposeListeners( maMutex ),
    mpImpl(NULL)
{
    Init();
}

//----------------------------------------------------------------------
SvxShape::SvxShape( SdrObject* pObject, const SfxItemPropertyMap* pPropertyMap ) throw()
:	aPropSet(pPropertyMap),
    bDisposing( sal_False ),
    pObj	(pObject),
    pModel(NULL),
    aSize(100,100),
    mnLockCount(0),
    aDisposeListeners( maMutex ),
    mpImpl(NULL)

{
    Init();
}

//----------------------------------------------------------------------
SvxShape::~SvxShape() throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT( mnLockCount == 0, "Locked shape was disposed!" );

    if( pModel )
        EndListening( *pModel );

    if( mpImpl )
    {
        if(mpImpl->mpMaster)
            mpImpl->mpMaster->dispose();

        delete mpImpl;
    }
}

//----------------------------------------------------------------------

void SvxShape::setShapeKind( sal_uInt32 nKind )
{
    if( mpImpl )
        mpImpl->mnObjId = nKind;
}

//----------------------------------------------------------------------

sal_uInt32 SvxShape::getShapeKind() const
{
    return mpImpl ? mpImpl->mnObjId : OBJ_NONE;
}

//----------------------------------------------------------------------

void SvxShape::setMaster( SvxShapeMaster* pMaster )
{
    mpImpl->mpMaster = pMaster;
}

//----------------------------------------------------------------------
sal_Bool SvxShape::queryAggregation( const ::com::sun::star::uno::Type & rType, ::com::sun::star::uno::Any& aAny )
{
    if( mpImpl->mpMaster )
    {
        if( mpImpl->mpMaster->queryAggregation( rType, aAny ) )
            return sal_True;
    }

    if( rType == ::getCppuType((const uno::Reference< beans::XPropertyState >*)0) )
        aAny <<= uno::Reference< beans::XPropertyState >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    else if( rType == ::getCppuType((const uno::Reference< drawing::XShape >*)0) )
        aAny <<= uno::Reference< drawing::XShape >(this);
    else if( rType == ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0) )
        aAny <<= uno::Reference< lang::XTypeProvider >(this);
    else if( rType == ::getCppuType((const uno::Reference< uno::XAggregation >*)0) )
        aAny <<= uno::Reference< uno::XAggregation >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XMultiPropertySet >(this);
    else if( rType == ::getCppuType((const uno::Reference< drawing::XShapeDescriptor >*)0) )
        aAny <<= uno::Reference< drawing::XShapeDescriptor >(this);
    else if( rType == ::getCppuType((const uno::Reference< document::XActionLockable >*)0) )
        aAny <<= uno::Reference< document::XActionLockable >(this);
    else if( rType == ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0) )
        aAny <<= uno::Reference< lang::XUnoTunnel >(this);
    else if( rType == ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0) )
        aAny <<= uno::Reference< drawing::XGluePointsSupplier >(this);
    else if( rType == ::getCppuType((const uno::Reference< container::XNamed >*)0) )
        aAny <<= uno::Reference< container::XNamed >(this);
    else if( rType == ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0) )
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    else if( rType == ::getCppuType((const uno::Reference< container::XChild >*)0) )
        aAny <<= uno::Reference< container::XChild >(this);
    else if( rType == ::getCppuType((const uno::Reference< lang::XComponent >*)0) )
        aAny <<= uno::Reference< lang::XComponent >(this);
    else if( rType == ::getCppuType((const uno::Reference< uno::XInterface >*)0) )
        aAny <<= uno::Reference< uno::XInterface >(static_cast< XInterface * >(static_cast< OWeakObject * >(this)));
    else if( rType == ::getCppuType((const uno::Reference< uno::XWeak >*)0) )
        aAny <<= uno::Reference< uno::XWeak >(this);
    else
        return sal_False;

    return sal_True;
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
SvxShape* SvxShape::getImplementation( const uno::Reference< uno::XInterface >& xInt )
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
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    else
    {
        return NULL;
    }
}

//----------------------------------------------------------------------
void SvxShape::Init() throw()
{
    if( NULL == mpImpl )
    {
        mpImpl = new SvxShapeImpl;
        mpImpl->mpItemSet = NULL;
        mpImpl->mpMaster = NULL;
        mpImpl->mnObjId = 0;
    }

    mbIsMultiPropertyCall = sal_False;

    // only init if we already have an object
    // if we get an object later Init() will
    // be called again
    if(pObj == NULL)
        return;

    osl_incrementInterlockedCount( &m_refCount );
    {
        pObj->setUnoShape( *this, SdrObject::GrantXShapeAccess() );
    }
    osl_decrementInterlockedCount( &m_refCount );

    // no model? this should not be
    if(!pObj->GetModel())
        return;

    pModel = pObj->GetModel();

    StartListening( *pModel );

    const sal_uInt32 nInventor = pObj->GetObjInventor();

    // is it one of ours (svx) ?
    if( nInventor == SdrInventor || nInventor == E3dInventor || nInventor == FmFormInventor )
    {
        if(nInventor == FmFormInventor)
        {
            mpImpl->mnObjId = OBJ_UNO;
        }
        else
        {
            mpImpl->mnObjId = pObj->GetObjIdentifier();
            if( nInventor == E3dInventor )
                mpImpl->mnObjId |= E3D_INVENTOR_FLAG;
        }

        switch(mpImpl->mnObjId)
        {
        case OBJ_CCUT:			// Kreisabschnitt
        case OBJ_CARC:			// Kreisbogen
        case OBJ_SECT:			// Kreissektor
            mpImpl->mnObjId = OBJ_CIRC;
            break;

        case E3D_SCENE_ID | E3D_INVENTOR_FLAG:
            mpImpl->mnObjId = E3D_POLYSCENE_ID | E3D_INVENTOR_FLAG;
            break;
        }
    }
}

//----------------------------------------------------------------------
void SvxShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw()
{
    if( pNewObj && ( (pObj != pNewObj) || (pModel == NULL) ) )
    {
        DBG_ASSERT( pNewObj->GetModel(), "no model for SdrObject?" );

        pObj = pNewObj;

        Init();

        ObtainSettingsFromPropertySet( aPropSet );

        // save user call
        SdrObjUserCall* pUser = pObj->GetUserCall();
        pObj->SetUserCall(NULL);

        setPosition( aPosition );
        setSize( aSize );

        // restore user call after we set the initial size
        pObj->SetUserCall( pUser );

        // if this shape was already named, use this name
        if( aShapeName.getLength() )
        {
            pObj->SetName( aShapeName );
            aShapeName = OUString();
        }
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
    if(pObj && rPropSet.AreThereOwnUsrAnys() && pModel)
    {
        SfxItemSet aSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END, 0);
        Reference< beans::XPropertySet > xShape( (OWeakObject*)this, UNO_QUERY );
        aPropSet.ObtainSettingsFromPropertySet(rPropSet, aSet, xShape);
        pObj->SetItemSetAndBroadcast(aSet);
        pObj->ApplyNotPersistAttr( aSet );
    }
}

//----------------------------------------------------------------------

uno::Any SvxShape::GetBitmap( sal_Bool bMetaFile /* = sal_False */ ) const throw()
{
    uno::Any aAny;

    if( pObj == NULL || pModel == NULL || !pObj->IsInserted() || NULL == pObj->GetPage() )
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
        ConvertGDIMetaFileToWMF( aMtf, aDestStrm, NULL, sal_False );
        const uno::Sequence<sal_Int8> aSeq(
            static_cast< const sal_Int8* >(aDestStrm.GetData()),
            aDestStrm.GetEndOfData());
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

//----------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL SvxShape::getTypes()
    throw (uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getTypes();
    }
    else
    {
        return _getTypes();
    }
}

//----------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL SvxShape::_getTypes()
    throw(uno::RuntimeException)
{
    switch( mpImpl->mnObjId )
    {
    // shapes without text
    case OBJ_OLE2:
    case OBJ_PAGE:
    case OBJ_FRAME:
    case OBJ_OLE2_PLUGIN:
    case OBJ_OLE2_APPLET:
    case E3D_CUBEOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_SPHEREOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_LATHEOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_EXTRUDEOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_POLYGONOBJ_ID|E3D_INVENTOR_FLAG:
        {
            static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 11 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);
                }
            }
            return aTypeSequence;
        }
    // group shape
    case OBJ_GRUP:
        {
            static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 13 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShapes>*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShapeGroup>*)0);
                }
            }
            return aTypeSequence;
        }
    // connector shape
    case OBJ_EDGE:
        {
            static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 15 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XConnectorShape>*)0);
                    // from SvxUnoTextBase::getTypes()
                    *pTypes++ = ::getCppuType(( const uno::Reference< text::XText >*)0);
                    *pTypes++ = ::getCppuType(( const uno::Reference< container::XEnumerationAccess >*)0);
                    *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRangeMover >*)0);
                }
            }
            return aTypeSequence;
        }
    // control shape
    case OBJ_UNO:
        {
            static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 12 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XControlShape>*)0);
                }
            }
            return aTypeSequence;
        }
    // 3d scene shape
    case E3D_POLYSCENE_ID|E3D_INVENTOR_FLAG:
        {
            static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 12 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShapes>*)0);
                }
            }
            return aTypeSequence;
        }
    // shapes with text
    case OBJ_RECT:
    case OBJ_CIRC:
    case OBJ_MEASURE:
    case OBJ_LINE:
    case OBJ_POLY:
    case OBJ_PLIN:
    case OBJ_PATHLINE:
    case OBJ_PATHFILL:
    case OBJ_FREELINE:
    case OBJ_FREEFILL:
    case OBJ_PATHPOLY:
    case OBJ_PATHPLIN:
    case OBJ_GRAF:
    case OBJ_TEXT:
    case OBJ_CAPTION:
    default:
        {
            static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 14 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XGluePointsSupplier >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XChild >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XUnoTunnel >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< container::XNamed >*)0);
                    // from SvxUnoTextBase::getTypes()
                    *pTypes++ = ::getCppuType(( const uno::Reference< text::XText >*)0);
                    *pTypes++ = ::getCppuType(( const uno::Reference< container::XEnumerationAccess >*)0);
                    *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRangeMover >*)0);
                }
            }
            return aTypeSequence;
        }
    }
}

//----------------------------------------------------------------------

uno::Sequence< sal_Int8 > SAL_CALL SvxShape::getImplementationId()
    throw (uno::RuntimeException)
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static ::cppu::OImplementationId aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

// SfxListener

//----------------------------------------------------------------------
void SvxShape::Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw()
{
    if( pObj == NULL )
        return;

    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
    if (!pSdrHint || ( /* (pSdrHint->GetKind() != HINT_OBJREMOVED)  && */
        (pSdrHint->GetKind() != HINT_MODELCLEARED) &&
        (pSdrHint->GetKind() != HINT_OBJLISTCLEAR) &&
        (pSdrHint->GetKind() != HINT_OBJCHG)))
        return;

    uno::Reference< uno::XInterface > xSelf( pObj->getWeakUnoShape() );
    if( !xSelf.is() )
    {
        pObj = NULL;
        return;
    }

    sal_Bool bClearMe = sal_False;

    switch( pSdrHint->GetKind() )
    {
        case HINT_OBJCHG:
        {
            updateShapeKind();
            break;
        }
/*
        case HINT_OBJREMOVED:
        {
            if( pObj == pSdrHint->GetObject() )
                bClearMe = sal_True;
            break;
        }
*/
        case HINT_MODELCLEARED:
        {
            bClearMe = sal_True;
            pModel = NULL;
            break;
        }
        case HINT_OBJLISTCLEAR:
        {
            SdrObjList* pObjList = pObj ? pObj->GetObjList() : NULL;
            while( pObjList )
            {
                if( pSdrHint->GetObjList() == pObjList )
                {
                    bClearMe = sal_True;
                    break;
                }

                pObjList = pObjList->GetUpList();
            }
            break;
        }
    };

    if( bClearMe )
    {
        pObj = NULL;
        if(!bDisposing)
            dispose();
    }
}

// XShape

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

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

        // Position is relativ to anchor, so recalc to absolut position
        if( pModel->IsWriter() )
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
            if( pModel->IsWriter() )
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

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

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
    if( 0 == aShapeType.getLength() )
    {
        UHashMapEntry* pMap = pSdrShapeIdentifierMap;
        while ( ( pMap->nId != mpImpl->mnObjId ) && pMap->aIdentifier.getLength() )
            ++pMap;

        if ( pMap->aIdentifier.getLength() )
        {
            return pMap->aIdentifier;
        }
        else
        {
            DBG_ERROR("[CL] unknown SdrObjekt identifier");
        }
    }

    return aShapeType;
}

// XComponent

//----------------------------------------------------------------------
void SAL_CALL SvxShape::dispose() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( bDisposing )
        return;	// catched a recursion

    bDisposing = sal_True;

    lang::EventObject aEvt;
    aEvt.Source = *(OWeakAggObject*) this;
    aDisposeListeners.disposeAndClear(aEvt);

    SdrObject* pObj = GetSdrObject();
    if(pObj && pObj->IsInserted() && pObj->GetPage() )
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

    if( pModel )
    {
        EndListening( *pModel );
        pModel = NULL;
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
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getPropertySetInfo();
    }
    else
    {
        return _getPropertySetInfo();
    }
}

Reference< beans::XPropertySetInfo > SAL_CALL
    SvxShape::_getPropertySetInfo() throw(uno::RuntimeException)
{
    return aPropSet.getPropertySetInfo();
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addPropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& aPropertyName, const Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::addVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removeVetoableChangeListener( const OUString& PropertyName, const Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

//----------------------------------------------------------------------

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName )
{
    SfxItemSet aSet( pModel->GetItemPool(),	(USHORT)nWID, (USHORT)nWID );

    if( SetFillAttribute( nWID, rName, aSet, pModel ) )
    {
        pObj->SetItemSetAndBroadcast(aSet);
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}

//----------------------------------------------------------------------

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName, SfxItemSet& rSet, SdrModel* pModel )
{
    // check if an item with the given name and which id is inside the models
    // pool or the stylesheet pool, if found its puttet in the itemse
    if( !SetFillAttribute( nWID, rName, rSet ) )
    {
        // we did not find such item in one of the pools, so we check
        // the property lists that are loaded for the model for items
        // that support such.
        String aStrName;
        SvxUnogetInternalNameForItem( (sal_Int16)nWID, rName, aStrName );

        switch( nWID )
        {
        case XATTR_FILLBITMAP:
        { 
            XBitmapList* pBitmapList = pModel->GetBitmapList();

            if( !pBitmapList )
                return sal_False;

            long nPos = ((XPropertyList*)pBitmapList)->Get(aStrName);
            if( nPos == -1 )
                return sal_False;

            XBitmapEntry* pEntry = pBitmapList->Get( nPos );
            XFillBitmapItem aBmpItem;
            aBmpItem.SetWhich( XATTR_FILLBITMAP );
            aBmpItem.SetName( rName );
            aBmpItem.SetValue( pEntry->GetXBitmap() );
            rSet.Put( aBmpItem );
            break;
        }
        case XATTR_FILLGRADIENT:
        {
            XGradientList* pGradientList = pModel->GetGradientList();

            if( !pGradientList )
                return sal_False;

            long nPos = ((XPropertyList*)pGradientList)->Get(aStrName);
            if( nPos == -1 )
                return sal_False;

            XGradientEntry* pEntry = pGradientList->Get( nPos );
            XFillGradientItem aGrdItem;
            aGrdItem.SetWhich( XATTR_FILLGRADIENT );
            aGrdItem.SetName( rName );
            aGrdItem.SetValue( pEntry->GetGradient() );
            rSet.Put( aGrdItem );
            break;
        }
        case XATTR_FILLHATCH:
        {
            XHatchList* pHatchList = pModel->GetHatchList();

            if( !pHatchList )
                return sal_False;

            long nPos = ((XPropertyList*)pHatchList)->Get(aStrName);
            if( nPos == -1 )
                return sal_False;

            XHatchEntry* pEntry = pHatchList->Get( nPos );
            XFillHatchItem aHatchItem;
            aHatchItem.SetWhich( XATTR_FILLHATCH );
            aHatchItem.SetName( rName );
            aHatchItem.SetValue( pEntry->GetHatch() );
            rSet.Put( aHatchItem );
            break;
        }
        case XATTR_LINEEND:
        case XATTR_LINESTART:
        {
            XLineEndList* pLineEndList = pModel->GetLineEndList();

            if( !pLineEndList )
                return sal_False;

            long nPos = ((XPropertyList*)pLineEndList)->Get(aStrName);
            if( nPos == -1 )
                return sal_False;

            XLineEndEntry* pEntry = pLineEndList->Get( nPos );
            if( XATTR_LINEEND == nWID )
            {
                XLineEndItem aLEItem;
                aLEItem.SetWhich( XATTR_LINEEND );
                aLEItem.SetName( rName );
                aLEItem.SetValue( pEntry->GetLineEnd() );
                rSet.Put( aLEItem );
            }
            else
            {
                XLineStartItem aLSItem;
                aLSItem.SetWhich( XATTR_LINESTART );
                aLSItem.SetName( rName );
                aLSItem.SetValue( pEntry->GetLineEnd() );
                rSet.Put( aLSItem );
            }

            break;
        }
        case XATTR_LINEDASH:
        { 
            XDashList* pDashList = pModel->GetDashList();

            if( !pDashList )
                return sal_False;

            long nPos = ((XPropertyList*)pDashList)->Get(aStrName);
            if( nPos == -1 )
                return sal_False;

            XDashEntry* pEntry = pDashList->Get( nPos );
            XLineDashItem aDashItem;
            aDashItem.SetWhich( XATTR_LINEDASH );
            aDashItem.SetName( rName );
            aDashItem.SetValue( pEntry->GetDash() );
            rSet.Put( aDashItem );
            break;
        }
        default:
            return sal_False;
        }
    }

    return sal_True;
}

//----------------------------------------------------------------------

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
        case XATTR_FILLFLOATTRANSPARENCE:
            {
                // #85953# Set a disabled XFillFloatTransparenceItem
                rSet.Put(XFillFloatTransparenceItem());

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
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        mpImpl->mpMaster->setPropertyValue( rPropertyName, rVal );
    }
    else
    {
        _setPropertyValue( rPropertyName, rVal );
    }
}

void SAL_CALL SvxShape::_setPropertyValue( const OUString& rPropertyName, const uno::Any& rVal )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(rPropertyName);

    if( pObj && pModel )
    {
        if( pMap == NULL )
            throw beans::UnknownPropertyException();

        if( (pMap->nFlags & beans::PropertyAttribute::READONLY ) != 0 )
            throw beans::PropertyVetoException();

        pModel->SetChanged();

        switch( pMap->nWID )
        {
        case OWN_ATTR_CAPTION_POINT:
        {
            awt::Point aPnt;
            if( rVal >>= aPnt )
            {
                Point aVclPoint( aPnt.X, aPnt.Y );

                // #90763# position is relative to top left, make it absolute
                XPolyPolygon aEmptyPolygon;
                Matrix3D aMatrix3D;
                pObj->TRGetBaseGeometry(aMatrix3D, aEmptyPolygon);
                aVclPoint.X() += FRound(aMatrix3D[0][2]);
                aVclPoint.Y() += FRound(aMatrix3D[1][2]);

                // #88657# metric of pool maybe twips (writer)
                ForceMetricToItemPoolMetric(aVclPoint);

                // #88491# position relative to anchor
                if( pModel->IsWriter() )
                {
                    aVclPoint += pObj->GetAnchorPos();
                }

                ((SdrCaptionObj*)pObj)->SetTailPos(aVclPoint);

                return;
            }
            break;
        }
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
                SdrObjList* pObjList = pObj->GetObjList();
                if( pObjList )
                {
                    SdrObject* pCheck =
                        pObjList->SetObjectOrdNum( pObj->GetOrdNum(), (ULONG)nNewOrdNum );
                    DBG_ASSERT( pCheck == pObj, "GetOrdNum() failed!" );
                    return;
                }
            }
            break;
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
        case OWN_ATTR_MIRRORED:
        {
            sal_Bool bMirror;
            if(rVal >>= bMirror )
            {
                if( pObj && pObj->ISA(SdrGrafObj) )
                {
                    ((SdrGrafObj*)pObj)->SetMirrored(bMirror);
                    return;
                }
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
                            SvGlobalName aClassName;

                            if( aClassName.MakeId( aCLSID ) )
                            {
                                // create storage and inplace object
                                String              aEmptyStr;
                                SvStorageRef        aStor( new SvStorage( aEmptyStr, STREAM_STD_READWRITE ) );
                                SvInPlaceObjectRef  aIPObj( &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit( aClassName, aStor) );
                                SvPersist*          pPersist = pModel->GetPersist();
                                String              aPersistName;
                                OUString            aTmpStr;
                                Any                 aAny( getPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ) ) );
                                sal_Bool            bOk = sal_False;

                                if( aAny >>= aTmpStr )
                                    aPersistName = aTmpStr;

                                // if we already have a shape name check if its a unique storage name
                                if( aPersistName.Len() && !pPersist->Find( aPersistName ) )
                                {
                                    SvInfoObjectRef xSub = new SvEmbeddedInfoObject( aIPObj, aPersistName );
                                    bOk = pPersist->Move( xSub, aPersistName );
                                }
                                else
                                {
                                    // generate a unique name
                                    String aStr( aPersistName = String( RTL_CONSTASCII_USTRINGPARAM("Object ") ) );

                                    // for-Schleife wegen Storage Bug 46033
                                    for( sal_Int32 i = 1, n = 0; n < 100; n++ )
                                    {
                                        do
                                        {
                                            aStr = aPersistName;
                                            aStr += String::CreateFromInt32( i++ );
                                        }
                                        while ( pPersist->Find( aStr ) );

                                        SvInfoObjectRef xSub( new SvEmbeddedInfoObject( aIPObj, aStr ) );

                                        if( pPersist->Move( xSub, aStr ) ) // Eigentuemer Uebergang
                                        {
                                            bOk = sal_True;
                                            aPersistName = aStr;
                                            break;
                                        }
                                    }
                                }

                                DBG_ASSERT( bOk, "could not create move ole stream!" );

                                if( bOk )
                                {
                                    aAny <<= ( aTmpStr = aPersistName );
                                    setPropertyValue( OUString::createFromAscii( UNO_NAME_OLE2_PERSISTNAME ), aAny );
                                }

                                static_cast< SdrOle2Obj* >( pObj )->SetObjRef( aIPObj );
                                aIPObj->SetVisAreaSize( static_cast< SdrOle2Obj* >( pObj )->GetLogicRect().GetSize() );

                                return;
                            }
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
                            if( pModel->IsWriter() )
                                aPoint += pObj->GetAnchorPos();

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

                if( pModel->IsWriter() )
                    aPoint += pObj->GetAnchorPos();

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
        case OWN_ATTR_PAGE_NUMBER:
        {
            sal_Int32 nPageNum;
            if( (rVal >>= nPageNum) && ( nPageNum >= 0 ) && ( nPageNum <= 0xffff ) )
            {
                nPageNum <<= 1;
                nPageNum -= 1;

                SdrPageObj* pPageObj = PTR_CAST(SdrPageObj,pObj);
                if( pPageObj )
                    pPageObj->SetPageNum( (sal_uInt16)nPageNum );
                return;
            }
            break;
        }
        case OWN_ATTR_OLE_VISAREA:
        {
            awt::Rectangle aVisArea;
            if( (rVal >>= aVisArea) && pObj->ISA(SdrOle2Obj))
            {
                Rectangle aTmpArea( aVisArea.X, aVisArea.Y, aVisArea.X + aVisArea.Width, aVisArea.Y + aVisArea.Height );
                ((SdrOle2Obj*)pObj)->SetVisibleArea( aTmpArea );
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
            DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST, "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            sal_Bool bIsNotPersist = pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST && pMap->nWID != SDRATTR_TEXTDIRECTION;

            if( pMap->nWID == SDRATTR_ECKENRADIUS )
            {
                sal_Int32 nCornerRadius;
                if( !(rVal >>= nCornerRadius) || (nCornerRadius < 0) || (nCornerRadius > 5000000))
                    throw IllegalArgumentException();
            }

            // HACK-fix #99090#
            // since SdrTextObj::SetVerticalWriting exchanges
            // SDRATTR_TEXT_AUTOGROWWIDTH and SDRATTR_TEXT_AUTOGROWHEIGHT,
            // we have to set the textdirection here

            if( pMap->nWID == SDRATTR_TEXTDIRECTION && pObj->ISA(SdrTextObj))
            {
                ::com::sun::star::text::WritingMode eMode;
                rVal >>= eMode;
                bool bVertical = eMode == ::com::sun::star::text::WritingMode_TB_RL;
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if( bVertical || pOPO )
                {
                    if( NULL == pOPO )
                    {
                        ((SdrTextObj*)pObj)->ForceOutlinerParaObject();
                        pOPO = pObj->GetOutlinerParaObject();
                    }

                    pOPO->SetVertical(bVertical);
                }
            }

            SfxItemSet* pSet;
            if( mbIsMultiPropertyCall && !bIsNotPersist )
            {
                if( mpImpl->mpItemSet == NULL )
                {
                    pSet = mpImpl->mpItemSet = pObj->GetItemSet().Clone();
                }
                else
                {
                    pSet = mpImpl->mpItemSet;
                }
            }
            else
            {
                pSet = new SfxItemSet( pModel->GetItemPool(),	pMap->nWID, pMap->nWID);
            }

            if( pSet->GetItemState( pMap->nWID ) != SFX_ITEM_SET )
                pSet->Put(pObj->GetItem(pMap->nWID));

            if( !SvxUnoTextRangeBase::SetPropertyValueHelper( *pSet, pMap, rVal, *pSet ))
            {
                if( pSet->GetItemState( pMap->nWID ) != SFX_ITEM_SET )
                {
                    if(bIsNotPersist)
                    {
                        // Not-Persistant Attribute, hole diese extra
                        pObj->TakeNotPersistAttr(*pSet, sal_False);
                    }
                }

                if( pSet->GetItemState( pMap->nWID ) != SFX_ITEM_SET )
                {
                    // Default aus ItemPool holen
                    if(pModel->GetItemPool().IsWhich(pMap->nWID))
                        pSet->Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));
                }

                if( pSet->GetItemState( pMap->nWID ) == SFX_ITEM_SET )
                {
                    aPropSet.setPropertyValue( pMap, rVal, *pSet );

                }
            }

            if(bIsNotPersist)
            {
                // Not-Persist Attribute extra setzen
                pObj->ApplyNotPersistAttr( *pSet );
                delete pSet;
            }
            else
            {
                // if we have a XMultiProperty call then the item set
                // will be set in setPropertyValues later
                if( !mbIsMultiPropertyCall )
                {
                    pObj->SetItemSetAndBroadcast( *pSet );
                    delete pSet;
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
//		  But since this class is aggregated from classes
//		  that support additional properties that we don't
//		  know here we silently store *all* properties, even
//		  if they may be not supported after creation
            aPropSet.setPropertyValue( pMap, rVal );
    }
}

//----------------------------------------------------------------------

const SvGlobalName SvxShape::GetClassName_Impl(::rtl::OUString& rHexCLSID)
{
    SvGlobalName aClassName;
    if( pObj && pObj->ISA(SdrOle2Obj))
    {
        rHexCLSID = ::rtl::OUString();

        if( static_cast< SdrOle2Obj* >( pObj )->IsEmpty() )
        {
            SvPersist* pPersist = pModel->GetPersist();

            if( pPersist )
            {
                SvInfoObject * pEle = pPersist->Find( static_cast< SdrOle2Obj* >( pObj )->GetPersistName() );

                if( pEle )
                {
                    aClassName = pEle->GetClassName();
                    rHexCLSID = aClassName.GetHexName();
                }
            }
        }

        if (!rHexCLSID.getLength())
        {
            const SvInPlaceObjectRef& rIPRef = ((SdrOle2Obj*)pObj)->GetObjRef();

            if (rIPRef.Is() )
            {
                aClassName = rIPRef->GetClassName();
                rHexCLSID = aClassName.GetHexName();
            }
        }
    }

    return aClassName;
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShape::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpImpl && mpImpl->mpMaster )
        return mpImpl->mpMaster->getPropertyValue( PropertyName );
    else
        return _getPropertyValue( PropertyName );
}

//----------------------------------------------------------------------

uno::Any SvxShape::_getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
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
            case OWN_ATTR_HASLEVELS:
            {
                aAny <<= SvxTextEditSource::hasLevels( pObj );
                break;
            }
            case OWN_ATTR_CAPTION_POINT:
            {
                Point aVclPoint = ((SdrCaptionObj*)pObj)->GetTailPos();

                // #88491# make pos relative to anchor
                if( pModel->IsWriter() )
                {
                    aVclPoint -= pObj->GetAnchorPos();
                }

                // #88657# metric of pool maybe twips (writer)
                ForceMetricTo100th_mm(aVclPoint);

                // #90763# pos is absolute, make it relative to top left
                Matrix3D aMatrix3D;
                XPolyPolygon aPolyPolygon;
                pObj->TRGetBaseGeometry( aMatrix3D, aPolyPolygon );
                aVclPoint.X() -= FRound(aMatrix3D[0][2]);
                aVclPoint.Y() -= FRound(aMatrix3D[1][2]);

                awt::Point aPnt( aVclPoint.X(), aVclPoint.Y() );
                aAny <<= aPnt;
                break;
            }
            case OWN_ATTR_INTERNAL_OLE:
            {
                ::rtl::OUString sCLSID;
                sal_Bool bInternal = SvFactory::IsIntern( GetClassName_Impl(sCLSID), 0 );
                aAny <<= bInternal;
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
            case OWN_ATTR_ISFONTWORK:
            {
                aAny <<= (sal_Bool)(pObj->ISA(SdrTextObj) && ((SdrTextObj*)pObj)->IsFontwork());
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
            { DBG_BF_ASSERT(0, "STRIP"); // Discussed with DL, this is not needed. Thus the bitmaps may be removed again, too.
//STRIP003				sal_uInt16 nId;
//STRIP003				if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
//STRIP003				{
//STRIP003					nId = RID_UNODRAW_OLE2;
//STRIP003				}
//STRIP003				else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
//STRIP003				{
//STRIP003					nId = RID_UNODRAW_GRAPHICS;
//STRIP003				}
//STRIP003				else
//STRIP003				{
//STRIP003					nId = RID_UNODRAW_OBJECTS;
//STRIP003				}
//STRIP003
//STRIP003				BitmapEx aBmp( SVX_RES(nId) );
//STRIP003				Reference< awt::XBitmap > xBmp( VCLUnoHelper::CreateBitmap( aBmp ) );
//STRIP003
//STRIP003				aAny <<= xBmp;
                break;
            }
            case OWN_ATTR_OLE_VISAREA:
            {
                awt::Rectangle aVisArea;
                if( pObj->ISA(SdrOle2Obj))
                {
                    SdrOle2Obj& aObj = *(SdrOle2Obj*)pObj;
                    const SvInPlaceObjectRef& xInplace = aObj.GetObjRef();
                    if( xInplace.Is() )
                    {
                        Rectangle aTmpArea( xInplace->GetVisArea() );
                        aVisArea = awt::Rectangle( aTmpArea.Left(), aTmpArea.Top(), aTmpArea.GetWidth(), aTmpArea.GetHeight() );
                    }
                }
                aAny <<= aVisArea;
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
                if( pObj->ISA(SdrOle2Obj))
                {
                    SdrOle2Obj& aObj = *(SdrOle2Obj*)pObj;
                    SvOutPlaceObjectRef xOut( aObj.GetObjRef() );
                    if ( xOut.Is() )
                        aAny <<= xOut->GetUnoComponent();
                    else
                        aAny <<= ((SdrOle2Obj*)pObj)->getXModel();
                }

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
                SvGlobalName aClassName = GetClassName_Impl(aCLSID);
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

                        ConvertGDIMetaFileToWMF( *aObj.GetGDIMetaFile(), aDestStrm, NULL, sal_False );
                        const uno::Sequence<sal_Int8> aSeq(
                            static_cast< const sal_Int8* >(aDestStrm.GetData()),
                            aDestStrm.GetEndOfData());
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
                            if( pModel->IsWriter() )
                                aPoint -= pObj->GetAnchorPos();

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
                    Point aPoint( pMeasureObj->GetPoint( pMap->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 ) );
                    if( pModel->IsWriter() )
                        aPoint -= pObj->GetAnchorPos();

                    awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );

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
                aAny = uno::makeAny( (sal_Bool) pObj->IsMoveProtect() );
                break;
            case SDRATTR_OBJECTNAME:
            {
                OUString aName( pObj->GetName() );
                aAny <<= aName;
                break;
            }
            case SDRATTR_OBJPRINTABLE:
                aAny = uno::makeAny( (sal_Bool) pObj->IsPrintable() );
                break;
            case SDRATTR_OBJSIZEPROTECT:
                aAny = uno::makeAny( (sal_Bool)pObj->IsResizeProtect() );
                break;
            case OWN_ATTR_PAGE_NUMBER:
            {
                SdrPageObj* pPageObj = PTR_CAST(SdrPageObj,pObj);
                if(pPageObj)
                {
                    sal_Int32 nPageNumber = pPageObj->GetPageNum();
                    nPageNumber++;
                    nPageNumber >>= 1;
                    aAny <<= nPageNumber;
                }
                break;
            }
            default:
            {
                DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || (pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST), "Not persist item not handled!" );
                DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

                SfxItemSet aSet( pModel->GetItemPool(),	pMap->nWID, pMap->nWID);
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
                    // Default aus ItemPool holen
                    if(pModel->GetItemPool().IsWhich(pMap->nWID))
                        aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));
                }

                if(aSet.Count())
                    aAny = GetAnyForItem( aSet, pMap );
            }
        }
    }
    else
    {

// Fixme: we should	return default values for OWN_ATTR !

        if(pMap && pMap->nWID)
//		FixMe: see setPropertyValue
            aAny = aPropSet.getPropertyValue( pMap );

    }
    return aAny;
}

//----------------------------------------------------------------------

// XMultiPropertySet
void SAL_CALL SvxShape::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pNames = aPropertyNames.getConstArray();

    const uno::Any* pValues = aValues.getConstArray();


    try
    {
        mbIsMultiPropertyCall = sal_True;

        if( mpImpl->mpMaster )
        {
            for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
                setPropertyValue( *pNames++, *pValues++ );
        }
        else
        {
            uno::Reference< beans::XPropertySet > xSet;
            queryInterface( ::getCppuType( (const uno::Reference< beans::XPropertySet >*) 0) ) >>= xSet;

            for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
                xSet->setPropertyValue( *pNames++, *pValues++ );
        }

        mbIsMultiPropertyCall = sal_False;

        if( mpImpl->mpItemSet )
        {
            pObj->SetItemSetAndBroadcast( *mpImpl->mpItemSet );
            delete mpImpl->mpItemSet;
            mpImpl->mpItemSet = NULL;
        }
    }
    catch( beans::UnknownPropertyException& e )
    {
        mbIsMultiPropertyCall = sal_False;

        if( mpImpl->mpItemSet )
        {
            delete mpImpl->mpItemSet;
            mpImpl->mpItemSet = NULL;
        }

        const OUString aMsg( RTL_CONSTASCII_USTRINGPARAM("UnknownPropertyException"));
        const uno::Reference< uno::XInterface > xContext;

        throw lang::WrappedTargetException( aMsg, xContext, uno::makeAny(e) );
    }
}

//----------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL SvxShape::getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException)
{
    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pNames = aPropertyNames.getConstArray();

    uno::Sequence< uno::Any > aRet( nCount );
    uno::Any* pValue = aRet.getArray();;

    if( mpImpl->mpMaster )
    {
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++ )
        {
            try
            {
                *pValue = getPropertyValue( *pNames++ );
            }
            catch( uno::Exception& )
            {
                DBG_ERROR( "SvxShape::getPropertyValues, unknown property asked" );
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xSet;
        queryInterface( ::getCppuType( (const uno::Reference< beans::XPropertySet >*) 0) ) >>= xSet;

        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++ )
        {
            try
            {
                *pValue = xSet->getPropertyValue( *pNames++ );
            }
            catch( uno::Exception& )
            {
                DBG_ERROR( "SvxShape::getPropertyValues, unknown property asked" );
            }
        }
    }

    return aRet;
}

void SAL_CALL SvxShape::addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxShape::removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxShape::firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
}

//----------------------------------------------------------------------

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
            case OBJ_CIRC:			// Kreis, Ellipse
                eKind = drawing::CircleKind_FULL;
                break;
            case OBJ_CCUT:			// Kreisabschnitt
                eKind = drawing::CircleKind_CUT;
                break;
            case OBJ_CARC:			// Kreisbogen
                eKind = drawing::CircleKind_ARC;
                break;
            case OBJ_SECT:			// Kreissektor
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

        if( *pMap->pType != aAny.getValueType() )
        {
            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
            if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                sal_Int32 nValue;
                aAny >>= nValue;
                aAny <<= (sal_Int16)nValue;
            }
            else
            {
                DBG_ERROR("SvxShape::GetAnyForItem() Returnvalue has wrong Type!" );
            }
        }

    }
    }

    return aAny;
}

//----------------------------------------------------------------------

// XPropertyState
beans::PropertyState SAL_CALL SvxShape::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getPropertyState( PropertyName );
    }
    else
    {
        return _getPropertyState( PropertyName );
    }
}

beans::PropertyState SAL_CALL SvxShape::_getPropertyState( const OUString& PropertyName )
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
    else if((( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST )) && ( pMap->nWID != SDRATTR_TEXTDIRECTION ) )
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        const SfxItemSet& rSet = pObj->GetItemSet();

        beans::PropertyState eState;

        switch( rSet.GetItemState( pMap->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            eState = beans::PropertyState_DIRECT_VALUE;
            break;
        case SFX_ITEM_DEFAULT:
            eState = beans::PropertyState_DEFAULT_VALUE;
            break;
//		case SFX_ITEM_UNKNOWN:
//		case SFX_ITEM_DONTCARE:
//		case SFX_ITEM_DISABLED:
        default:
            eState = beans::PropertyState_AMBIGUOUS_VALUE;
            break;
        }

        // if a item is set, this doesn't mean we want it :)
        if( ( beans::PropertyState_DIRECT_VALUE == eState ) )
        {
            switch( pMap->nWID )
            {
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_FILLFLOATTRANSPARENCE:
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_LINEDASH:
                {
                    NameOrIndex* pItem = (NameOrIndex*)rSet.GetItem((USHORT)pMap->nWID);
                    if( ( pItem == NULL ) || ( pItem->GetName().Len() == 0) )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
            }
        }
        return eState;
    }
}

//----------------------------------------------------------------------

uno::Sequence< beans::PropertyState > SAL_CALL SvxShape::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    const sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aRet( nCount );
    beans::PropertyState* pState = aRet.getArray();

    if( mpImpl->mpMaster )
    {
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
            pState[nIdx] = getPropertyState( pNames[nIdx] );

    }
    else
    {
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
            pState[nIdx] = getPropertyState( pNames[nIdx] );
    }

    return aRet;
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        mpImpl->mpMaster->setPropertyToDefault( PropertyName );
    }
    else
    {
        _setPropertyToDefault( PropertyName );
    }
}

void SAL_CALL SvxShape::_setPropertyToDefault( const OUString& PropertyName )
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

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShape::getPropertyDefault( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getPropertyDefault( aPropertyName );
    }
    else
    {
        return _getPropertyDefault( aPropertyName );
    }
}

uno::Any SAL_CALL SvxShape::_getPropertyDefault( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    if( pObj == NULL || pMap == NULL || pModel == NULL )
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

        SfxItemSet aSet( pModel->GetItemPool(),	pMap->nWID, pMap->nWID);
        aSet.Put(pModel->GetItemPool().GetDefaultItem(pMap->nWID));

        return GetAnyForItem( aSet, pMap );
    }
}

//----------------------------------------------------------------------

//----------------------------------------------------------------------
// XServiceInfo
//----------------------------------------------------------------------
OUString SAL_CALL SvxShape::getImplementationName()
    throw(uno::RuntimeException)
{
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM("SvxShape") );
    return aServiceName;
}

#define STAR_NAMESPACE "com.sun.star."

const char* sUNO_service_style_ParagraphProperties			= STAR_NAMESPACE "style.ParagraphProperties";
const char* sUNO_service_style_ParagraphPropertiesComplex	= STAR_NAMESPACE "style.ParagraphPropertiesComplex";
const char* sUNO_service_style_ParagraphPropertiesAsian		= STAR_NAMESPACE "style.ParagraphPropertiesAsian";
const char* sUNO_service_style_CharacterProperties			= STAR_NAMESPACE "style.CharacterProperties";
const char* sUNO_service_style_CharacterPropertiesComplex	= STAR_NAMESPACE "style.CharacterPropertiesComplex";
const char* sUNO_service_style_CharacterPropertiesAsian		= STAR_NAMESPACE "style.CharacterPropertiesAsian";

const char* sUNO_service_drawing_FillProperties				= STAR_NAMESPACE "drawing.FillProperties";
const char* sUNO_service_drawing_TextProperties				= STAR_NAMESPACE "drawing.TextProperties";
const char* sUNO_service_drawing_LineProperties				= STAR_NAMESPACE "drawing.LineProperties";
const char* sUNO_service_drawing_ConnectorProperties		= STAR_NAMESPACE "drawing.ConnectorProperties";
const char* sUNO_service_drawing_MeasureProperties			= STAR_NAMESPACE "drawing.MeasureProperties";
const char* sUNO_service_drawing_ShadowProperties			= STAR_NAMESPACE "drawing.ShadowProperties";

const char* sUNO_service_drawing_RotationDescriptor			= STAR_NAMESPACE "drawing.RotationDescriptor";

const char* sUNO_service_drawing_Text						= STAR_NAMESPACE "drawing.Text";
const char* sUNO_service_drawing_GroupShape					= STAR_NAMESPACE "drawing.GroupShape";

const char* sUNO_service_drawing_PolyPolygonDescriptor		= STAR_NAMESPACE "drawing.PolyPolygonDescriptor";
const char* sUNO_service_drawing_PolyPolygonBezierDescriptor= STAR_NAMESPACE "drawing.PolyPolygonBezierDescriptor";

const char* sUNO_service_drawing_LineShape					= STAR_NAMESPACE "drawing.LineShape";
const char* sUNO_service_drawing_Shape						= STAR_NAMESPACE "drawing.Shape";
const char* sUNO_service_drawing_RectangleShape				= STAR_NAMESPACE "drawing.RectangleShape";
const char* sUNO_service_drawing_EllipseShape				= STAR_NAMESPACE "drawing.EllipseShape";
const char* sUNO_service_drawing_PolyPolygonShape			= STAR_NAMESPACE "drawing.PolyPolygonShape";
const char* sUNO_service_drawing_PolyLineShape				= STAR_NAMESPACE "drawing.PolyLineShape";
const char* sUNO_service_drawing_OpenBezierShape			= STAR_NAMESPACE "drawing.OpenBezierShape";
const char* sUNO_service_drawing_ClosedBezierShape			= STAR_NAMESPACE "drawing.ClosedBezierShape";
const char* sUNO_service_drawing_TextShape					= STAR_NAMESPACE "drawing.TextShape";
const char* sUNO_service_drawing_GraphicObjectShape			= STAR_NAMESPACE "drawing.GraphicObjectShape";
const char* sUNO_service_drawing_OLE2Shape					= STAR_NAMESPACE "drawing.OLE2Shape";
const char* sUNO_service_drawing_PageShape					= STAR_NAMESPACE "drawing.PageShape";
const char* sUNO_service_drawing_CaptionShape				= STAR_NAMESPACE "drawing.CaptionShape";
const char* sUNO_service_drawing_MeasureShape				= STAR_NAMESPACE "drawing.MeasureShape";
const char* sUNO_service_drawing_FrameShape					= STAR_NAMESPACE "drawing.FrameShape";
const char* sUNO_service_drawing_ControlShape				= STAR_NAMESPACE "drawing.ControlShape";
const char* sUNO_service_drawing_ConnectorShape				= STAR_NAMESPACE "drawing.ConnectorShape";


uno::Sequence< OUString > SAL_CALL SvxShape::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getSupportedServiceNames();
    }
    else
    {
        return _getSupportedServiceNames();
    }
}

uno::Sequence< OUString > SAL_CALL SvxShape::_getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    if( pObj && pObj->GetObjInventor() == SdrInventor)
    {
        const UINT16 nIdent = pObj->GetObjIdentifier();

        switch(nIdent)
        {
        case OBJ_GRUP:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_GroupServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_GroupServices, 2,
                            sUNO_service_drawing_GroupShape,
                              sUNO_service_drawing_Shape );

                        pSeq = &SvxShape_GroupServices;
                    }
                }

                return *pSeq;
            }
        case OBJ_LINE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_LineServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_LineServices,14,
                            sUNO_service_drawing_LineShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_PolyPolygonDescriptor,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_LineServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_RECT:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_RectServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_RectServices,14,
                            sUNO_service_drawing_RectangleShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);
                        pSeq = &SvxShape_RectServices;
                    }

                }
                return *pSeq;
            }

        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_CircServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_CircServices,14,
                            sUNO_service_drawing_EllipseShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_CircServices;
                    }
                }

                return *pSeq;
            }

        case OBJ_PATHPLIN:
        case OBJ_PLIN:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_PathServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_PathServices,14,
                            sUNO_service_drawing_PolyLineShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_PolyPolygonDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);
                        pSeq = &SvxShape_PathServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_PATHPOLY:
        case OBJ_POLY:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_PolyServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_PolyServices,15,
                            sUNO_service_drawing_PolyPolygonShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_FillProperties,

                            sUNO_service_drawing_PolyPolygonDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_PolyServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_FREELINE:
        case OBJ_PATHLINE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_FreeLineServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_FreeLineServices,15,
                            sUNO_service_drawing_OpenBezierShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_FillProperties,

                            sUNO_service_drawing_PolyPolygonBezierDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_FreeLineServices;
                    }
                }

                return *pSeq;
            }

        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_FreeFillServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_FreeFillServices,15,
                            sUNO_service_drawing_ClosedBezierShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_FillProperties,

                            sUNO_service_drawing_PolyPolygonBezierDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_FreeFillServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_OUTLINETEXT:
        case OBJ_TITLETEXT:
        case OBJ_TEXT:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_TextServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_TextServices,14,
                            sUNO_service_drawing_TextShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_TextServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_GRAF:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_GrafServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_GrafServices, 12,
                            sUNO_service_drawing_GraphicObjectShape,

                            sUNO_service_drawing_Shape,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_GrafServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_OLE2:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_Ole2Services;

                        SvxServiceInfoHelper::addToSequence( SvxShape_Ole2Services, 2,
                            sUNO_service_drawing_OLE2Shape,
                            sUNO_service_drawing_Shape);

                        pSeq = &SvxShape_Ole2Services;
                    }
                }
                return *pSeq;
            }

        case OBJ_CAPTION:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_CaptionServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_CaptionServices,14,
                            sUNO_service_drawing_CaptionShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_CaptionServices;
                    }
                }

                return *pSeq;
            }

        case OBJ_PAGE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_PageServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_PageServices, 2,
                            sUNO_service_drawing_PageShape,
                            sUNO_service_drawing_Shape );

                        pSeq = &SvxShape_PageServices;
                    }
                }

                return *pSeq;
            }

        case OBJ_MEASURE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_MeasureServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_MeasureServices,15,
                            sUNO_service_drawing_MeasureShape,

                            sUNO_service_drawing_MeasureProperties,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_PolyPolygonDescriptor,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_MeasureServices;
                    }
                }

                return *pSeq;
            }

        case OBJ_FRAME:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_FrameServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_FrameServices, 2,
                            sUNO_service_drawing_FrameShape,
                            sUNO_service_drawing_Shape );

                        pSeq = &SvxShape_FrameServices;
                    }
                }

                return *pSeq;
            }

        case OBJ_UNO:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_UnoServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_UnoServices, 2,
                            sUNO_service_drawing_ControlShape,
                            sUNO_service_drawing_Shape );

                        pSeq = &SvxShape_UnoServices;
                    }
                }
                return *pSeq;
            }

        case OBJ_EDGE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_EdgeServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_EdgeServices,15,
                            sUNO_service_drawing_ConnectorShape,
                            sUNO_service_drawing_ConnectorProperties,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_PolyPolygonDescriptor,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor);

                        pSeq = &SvxShape_EdgeServices;
                    }
                }
                return *pSeq;
            }
        }
    }
    else if( pObj && pObj->GetObjInventor() == FmFormInventor)
    {
        const UINT16 nIdent = pObj->GetObjIdentifier();

        switch(nIdent)
        {
        case OBJ_FM_CONTROL:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
                    OGuard aGuard( Application::GetSolarMutex() );
                    if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_UnoServices;
                        SvxServiceInfoHelper::addToSequence( SvxShape_UnoServices, 2,
                            sUNO_service_drawing_ControlShape,
                            sUNO_service_drawing_Shape );

                        pSeq = &SvxShape_UnoServices;
                    }
                }
                return *pSeq;
            }
        }
    }
    uno::Sequence< OUString > aSeq;
    return aSeq;
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxShape::supportsService( const OUString& ServiceName ) throw ( uno::RuntimeException )
{
    Sequence< OUString > SupportedServices( getSupportedServiceNames() );
    const ::rtl::OUString * pArray = SupportedServices.getConstArray();
    const sal_Int32 nCount = SupportedServices.getLength();
    sal_Int32 i;
    for( i = 0; i < nCount; i++ )
        if( *pArray++ == ServiceName )
            return sal_True;
    return sal_False;
}

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

void SAL_CALL SvxShape::setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent )
    throw(lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}

//----------------------------------------------------------------------

/** called from the XActionLockable interface methods on initial locking */
void SvxShape::lock()
{
}

//----------------------------------------------------------------------

/** called from the XActionLockable interface methods on final unlock */
void SvxShape::unlock()
{
}

//----------------------------------------------------------------------

// XActionLockable
sal_Bool SAL_CALL SvxShape::isActionLocked(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    return mnLockCount != 0;
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addActionLock(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT( mnLockCount < 0xffff, "lock overflow in SvxShape!" );
    mnLockCount++;

    if( mnLockCount == 1 )
        lock();
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::removeActionLock(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT( mnLockCount > 0, "lock underflow in SvxShape!" );
    mnLockCount--;

    if( mnLockCount == 0 )
        unlock();
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::setActionLocks( sal_Int16 nLock ) throw (::com::sun::star::uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( (mnLockCount == 0) && (nLock != 0) )
        unlock();

    if( (mnLockCount != 0) && (nLock == 0) )
        lock();

    mnLockCount = (sal_uInt16)nLock;
}

//----------------------------------------------------------------------

sal_Int16 SAL_CALL SvxShape::resetActionLocks(  ) throw (::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mnLockCount != 0 )
        unlock();

    sal_Int16 nOldLocks = (sal_Int16)mnLockCount;
    mnLockCount = 0;

    return nOldLocks;
}

//----------------------------------------------------------------------

/** since polygon shapes can change theire kind during editing, we have
    to recheck it here.
    Circle shapes also change theire kind, but theire all treated equal
    so no update is necessary.
*/
void SvxShape::updateShapeKind()
{
    switch( mpImpl->mnObjId )
    {
        case OBJ_LINE:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
        {
            const sal_uInt32 nId = pObj->GetObjIdentifier();

            if( nId != mpImpl->mnObjId )
            {
                mpImpl->mnObjId = nId;

            }
            break;
        }
    };
}

/***********************************************************************
* class SvxShapeText                                                   *
***********************************************************************/
SvxShapeText::SvxShapeText( SdrObject* pObject ) throw ()
: SvxShape( pObject, aSvxMapProvider.GetMap(SVXMAP_TEXT) ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorPropertyMap() )
{
    if( pObject && pObject->GetModel() )
        SetEditSource( new SvxTextEditSource( pObject ) );
}

//----------------------------------------------------------------------
SvxShapeText::SvxShapeText( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ()
: SvxShape( pObject, pPropertySet ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorPropertyMap() )
{
    if( pObject && pObject->GetModel() )
        SetEditSource( new SvxTextEditSource( pObject ) );
}

//----------------------------------------------------------------------
SvxShapeText::~SvxShapeText() throw ()
{
}

void SvxShapeText::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw ()
{
    if( pNewObj && (NULL == GetEditSource()))
        SetEditSource( new SvxTextEditSource( pNewObj ) );

    SvxShape::Create( pNewObj, pNewPage );
}

// XInterface
//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeText::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    return SvxShape::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeText::queryAggregation( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aAny;

    if( !SvxShape::queryAggregation( rType, aAny ) )
        SvxUnoTextBase::queryAggregation( rType, aAny );

    return aAny;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeText::acquire() throw()
{
    SvxShape::acquire();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeText::release() throw()
{
    SvxShape::release();
}

// XServiceInfo
//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeText::getImplementationName() throw( uno::RuntimeException )
{
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM("SvxShapeText") );
    return aServiceName;
}

//----------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL SvxShapeText::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxShapeText::supportsService( const OUString& ServiceName ) throw ( uno::RuntimeException )
{
    return SvxShape::supportsService(ServiceName);
}

    // XTypeProvider
//----------------------------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL SvxShapeText::getTypes()
    throw( uno::RuntimeException )
{
    return SvxShape::getTypes();
}

sal_Int64 SAL_CALL SvxShapeText::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{
    const sal_Int64 nReturn = SvxShape::getSomething( rId );
    if( nReturn )
        return nReturn;

    return SvxUnoTextBase::getSomething( rId );
}

//----------------------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL SvxShapeText::getImplementationId()
    throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static ::cppu::OImplementationId aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//----------------------------------------------------------------------

/** called from the XActionLockable interface methods on initial locking */
void SvxShapeText::lock()
{
    SvxTextEditSource* pEditSource = (SvxTextEditSource*)GetEditSource();
    if( pEditSource )
        pEditSource->lock();
}

//----------------------------------------------------------------------

/** called from the XActionLockable interface methods on final unlock */
void SvxShapeText::unlock()
{
    SvxTextEditSource* pEditSource = (SvxTextEditSource*)GetEditSource();
    if( pEditSource )
        pEditSource->unlock();
}


/***********************************************************************
* class SvxShapeRect                                                   *
***********************************************************************/
SvxShapeRect::SvxShapeRect( SdrObject* pObj ) throw()
: SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_SHAPE) )

{

}

SvxShapeRect::~SvxShapeRect() throw()
{
}

uno::Any SAL_CALL SvxShapeRect::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeRect::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return SvxShapeText::queryAggregation( rType );
}

void SAL_CALL SvxShapeRect::acquire() throw()
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShapeRect::release() throw()
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
}
