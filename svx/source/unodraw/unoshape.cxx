/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoshape.cxx,v $
 *
 *  $Revision: 1.169 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:13:05 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define _SVX_USE_UNOGLOBALS_

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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
#include <svx/unotext.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <comphelper/extract.hxx>

#include <toolkit/unohlp.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <vcl/gfxlink.hxx>
#include <vcl/virdev.hxx>

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SVDOPAGE_HXX
#include "svx/svdopage.hxx"
#endif
#ifndef _SVX_XFLBSTIT_HXX
#include "svx/xflbstit.hxx"
#endif
#ifndef _SVX_XFLBMTIT_HXX
#include "svx/xflbmtit.hxx"
#endif
#ifndef _SVX_XLNSTIT_HXX
#include "svx/xlnstit.hxx"
#endif
#ifndef _SVX_XLNEDIT_HXX
#include "svx/xlnedit.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include "svx/svdogrp.hxx"
#endif
#ifndef _E3D_SCENE3D_HXX
#include "svx/scene3d.hxx"
#endif
#include "svx/svdmodel.hxx"
#include "globl3d.hxx"
#include "svx/fmglob.hxx"
#include "svx/unopage.hxx"
#include "svx/view3d.hxx"
#include "svx/unoshape.hxx"
#include "svx/svxids.hrc"
#include "svx/unoshtxt.hxx"
#include "svx/svdpage.hxx"
#include "svx/unoshprp.hxx"
#include "svx/sxciaitm.hxx" // todo: remove
#include "svx/svdograf.hxx"
#include "unoapi.hxx"
#include "svx/svdomeas.hxx"
#include "svx/svdpagv.hxx"

#include <tools/shl.hxx>    //
#include "svx/dialmgr.hxx"      // not nice, we need our own resources some day
#include "svx/dialogs.hrc"      //

#ifndef _SVDCAPT_HXX
#include "svx/svdocapt.hxx"
#endif

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#ifndef _SVX_XFLFTRIT_HXX
#include "svx/xflftrit.hxx"
#endif

#ifndef _XTABLE_HXX
#include "svx/xtable.hxx"
#endif

#ifndef _SVX_XBTMPIT_HXX
#include "svx/xbtmpit.hxx"
#endif

#ifndef _SVX_XFLGRIT_HXX
#include "svx/xflgrit.hxx"
#endif

#ifndef _SVX_XFLHTIT_HXX
#include "svx/xflhtit.hxx"
#endif

#ifndef _SVX_XLNEDIT_HXX
#include "svx/xlnedit.hxx"
#endif

#ifndef _SVX_XLNSTIT_HXX
#include "svx/xlnstit.hxx"
#endif

#ifndef _SVX_XLNDSIT_HXX
#include "svx/xlndsit.hxx"
#endif

#ifndef _SVDGLOB_HXX
#include "svdglob.hxx"
#endif
#include "svdstr.hrc"
#include "unomaster.hxx"

#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#include <vector>

// #i68523#
#ifndef _E3D_LATHE3D_HXX
#include "svx/lathe3d.hxx"
#endif
#ifndef _E3D_EXTRUD3D_HXX
#include "svx/extrud3d.hxx"
#endif

#include <comphelper/scopeguard.hxx>
#include <boost/bind.hpp>

using ::rtl::OUString;
using namespace ::osl;
using namespace ::vos;
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
        {MAP_CHAR_LEN("TextUserDefinedAttributes"),         EE_CHAR_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0,0}
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
        {MAP_CHAR_LEN("TextField"),                     EE_FEATURE_FIELD,   &::getCppuType((const uno::Reference< text::XTextField >*)0),   beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextPortionType"),               WID_PORTIONTYPE,    &::getCppuType((const ::rtl::OUString*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextUserDefinedAttributes"),         EE_CHAR_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0,0}
    };

    return aSvxTextPortionPropertyMap;
}

class GDIMetaFile;
class SvStream;
sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pFilterConfigItem = NULL, sal_Bool bPlaceable = sal_True );

uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject );

/***********************************************************************
* class SvxShape                                                       *
***********************************************************************/

struct SvxShapeImpl
{
    SfxItemSet*     mpItemSet;
    sal_uInt32      mnObjId;
    SvxShapeMaster* mpMaster;
    bool            mbHasSdrObjectOwnership;

    /** CL, OD 2005-07-19 #i52126# - this is initially 0 and set when
     *  a SvxShape::Create() call is executed. It is then set to the created
     *  SdrObject so a multiple call to SvxShape::Create() with same SdrObject
     *  is prohibited.
     */
    SdrObject*      mpCreatedObj;
};

DBG_NAME(SvxShape)

SvxShape::SvxShape( SdrObject* pObject ) throw()
:   maSize(100,100)
,   mpImpl(NULL)
,   mbIsMultiPropertyCall(false)
,   maPropSet(aSvxMapProvider.GetMap(SVXMAP_SHAPE))
,   maDisposeListeners( maMutex )
,   mbDisposing( false )
,   mpObj(pObject)
,   mpModel(NULL)
,   mnLockCount(0)
{
    DBG_CTOR(SvxShape,NULL);
    Init();
}

//----------------------------------------------------------------------
SvxShape::SvxShape( SdrObject* pObject, const SfxItemPropertyMap* pPropertyMap ) throw()
:   maSize(100,100)
,   mpImpl(NULL)
,   mbIsMultiPropertyCall(false)
,   maPropSet(pPropertyMap)
,   maDisposeListeners( maMutex )
,   mbDisposing( false )
,   mpObj(pObject)
,   mpModel(NULL)
,   mnLockCount(0)
{
    DBG_CTOR(SvxShape,NULL);
    Init();
}

//----------------------------------------------------------------------
SvxShape::SvxShape() throw()
:   maSize(100,100)
,   mpImpl(NULL)
,   mbIsMultiPropertyCall(false)
,   maPropSet(aSvxMapProvider.GetMap(SVXMAP_SHAPE))
,   maDisposeListeners( maMutex )
,   mbDisposing( false )
,   mpObj(NULL)
,   mpModel(NULL)
,   mnLockCount(0)
{
    DBG_CTOR(SvxShape,NULL);
    Init();
}

//----------------------------------------------------------------------
SvxShape::~SvxShape() throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT( mnLockCount == 0, "Locked shape was disposed!" );

    if( mpModel )
        EndListening( *mpModel );

    if(mpImpl && mpImpl->mpMaster)
        mpImpl->mpMaster->dispose();

    if( HasSdrObjectOwnership() && mpObj.is() )
    {
        mpImpl->mbHasSdrObjectOwnership = false;
        SdrObject* pObject = mpObj.get();
        SdrObject::Free( pObject );
    }

    delete mpImpl, mpImpl = NULL;

    DBG_DTOR(SvxShape,NULL);
}

//----------------------------------------------------------------------

void SvxShape::TakeSdrObjectOwnership()
{
    if ( mpImpl )
        mpImpl->mbHasSdrObjectOwnership = true;
}

//----------------------------------------------------------------------

bool SvxShape::HasSdrObjectOwnership() const
{
    OSL_PRECOND( mpImpl, "SvxShape::HasSdrObjectOwnership: no impl!?" );
    if ( !mpImpl )
        return false;

    if ( !mpImpl->mbHasSdrObjectOwnership )
        return false;

    OSL_ENSURE( mpObj.is(), "SvxShape::HasSdrObjectOwnership: have the ownership of an object which I don't know!" );
    return mpObj.is();
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
    return mpImpl ? mpImpl->mnObjId : (sal_uInt32)OBJ_NONE;
}

//----------------------------------------------------------------------

void SvxShape::setMaster( SvxShapeMaster* pMaster )
{
    mpImpl->mpMaster = pMaster;
}

SvxShapeMaster* SvxShape::getMaster()
{
    return mpImpl ? mpImpl->mpMaster : NULL;
}

const SvxShapeMaster* SvxShape::getMaster() const
{
    return mpImpl ? mpImpl->mpMaster : NULL;
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShape::queryAggregation( const uno::Type& rType ) throw (uno::RuntimeException)
{
    if( mpImpl->mpMaster )
    {
        uno::Any aAny;
        if( mpImpl->mpMaster->queryAggregation( rType, aAny ) )
            return aAny;
    }

    return SvxShape_UnoImplHelper::queryAggregation(rType);
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
        return reinterpret_cast<SvxShape*>(sal::static_int_cast<sal_uIntPtr>(xUT->getSomething( SvxShape::getUnoTunnelId())));
    else
        return NULL;
}

//----------------------------------------------------------------------
sal_Int64 SAL_CALL SvxShape::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    else
    {
        return 0;
    }
}

//----------------------------------------------------------------------
SvxShape* SvxShape::GetShapeForSdrObj( SdrObject* pObj ) throw()
{
    return getImplementation( pObj->getUnoShape() );
}

void SvxShape::Init() throw()
{
    if( NULL == mpImpl )
    {
        mpImpl = new SvxShapeImpl;
        mpImpl->mpItemSet = NULL;
        mpImpl->mpMaster = NULL;
        mpImpl->mnObjId = 0;
        mpImpl->mbHasSdrObjectOwnership= false;
        // --> CL, OD 2005-07-19 #i52126#
        mpImpl->mpCreatedObj = NULL;
        // <--
    }

    mbIsMultiPropertyCall = sal_False;

    // only init if we already have an object
    // if we get an object later Init() will
    // be called again
    DBG_TESTSOLARMUTEX();
    if(!mpObj.is())
        return;

    osl_incrementInterlockedCount( &m_refCount );
    {
        mpObj->setUnoShape( *this, SdrObject::GrantXShapeAccess() );
    }
    osl_decrementInterlockedCount( &m_refCount );

    mpModel = mpObj->GetModel();

    // #i40944#
    // Do not simply return when no model but do the type corrections
    // following below.
    if(mpModel)
    {
        StartListening( *mpModel );
    }

    const sal_uInt32 nInventor = mpObj->GetObjInventor();

    // is it one of ours (svx) ?
    if( nInventor == SdrInventor || nInventor == E3dInventor || nInventor == FmFormInventor )
    {
        if(nInventor == FmFormInventor)
        {
            mpImpl->mnObjId = OBJ_UNO;
        }
        else
        {
            mpImpl->mnObjId = mpObj->GetObjIdentifier();
            if( nInventor == E3dInventor )
                mpImpl->mnObjId |= E3D_INVENTOR_FLAG;
        }

        switch(mpImpl->mnObjId)
        {
        case OBJ_CCUT:          // Kreisabschnitt
        case OBJ_CARC:          // Kreisbogen
        case OBJ_SECT:          // Kreissektor
            mpImpl->mnObjId = OBJ_CIRC;
            break;

        case E3D_SCENE_ID | E3D_INVENTOR_FLAG:
            mpImpl->mnObjId = E3D_POLYSCENE_ID | E3D_INVENTOR_FLAG;
            break;
        }
    }
}

//----------------------------------------------------------------------
void SvxShape::Create( SdrObject* pNewObj, SvxDrawPage* /*pNewPage*/ ) throw()
{
    DBG_ASSERT( mpImpl, "svx::SvxShape::Create(), no mpImpl!" );

    DBG_TESTSOLARMUTEX();

    // --> CL, OD 2005-07-19 #i52126# - correct condition
    if ( pNewObj && (mpImpl && (mpImpl->mpCreatedObj != pNewObj) ) )
    // <--
    {
        DBG_ASSERT( pNewObj->GetModel(), "no model for SdrObject?" );
        // --> CL, OD 2005-07-19 #i52126#
        mpImpl->mpCreatedObj = pNewObj;
        // <--

        if( mpObj.is() && mpObj->GetModel() )
        {
            EndListening( *mpObj->GetModel() );
        }

        mpObj.reset( pNewObj );

        Init();

        ObtainSettingsFromPropertySet( maPropSet );

        // save user call
        SdrObjUserCall* pUser = mpObj->GetUserCall();
        mpObj->SetUserCall(NULL);

        setPosition( maPosition );
        setSize( maSize );

        // restore user call after we set the initial size
        mpObj->SetUserCall( pUser );

        // if this shape was already named, use this name
        if( maShapeName.getLength() )
        {
            mpObj->SetName( maShapeName );
            maShapeName = OUString();
        }
    }
}

//----------------------------------------------------------------------

void SvxShape::ChangeModel( SdrModel* pNewModel )
{
    DBG_TESTSOLARMUTEX();
    if( mpObj.is() && mpObj->GetModel() )
    {
        if( mpObj->GetModel() != pNewModel )
        {
            EndListening( *mpObj->GetModel() );
        }
    }

    // --> CL, OD 2005-07-19 #i52126# - always listen to new model
    if( pNewModel )
    {
        StartListening( *pNewModel );
    }
    // <--

    // HACK #i53696# ChangeModel should be virtual, but it isn't. can't change that for 2.0.1
    SvxShapeText* pShapeText = dynamic_cast< SvxShapeText* >( this );
    if( pShapeText )
    {
        SvxTextEditSource* pTextEditSource = dynamic_cast< SvxTextEditSource* >( pShapeText->GetEditSource() );
        if( pTextEditSource )
            pTextEditSource->ChangeModel( pNewModel );
    }

    mpModel = pNewModel;

    if( mpImpl->mpMaster )
        mpImpl->mpMaster->modelChanged( pNewModel );
}

//----------------------------------------------------------------------

void SvxShape::ForceMetricToItemPoolMetric(Pair& rPoint) const throw()
{
    DBG_TESTSOLARMUTEX();
    if(mpModel)
    {
        SfxMapUnit eMapUnit = mpModel->GetItemPool().GetMetric(0);
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
    DBG_TESTSOLARMUTEX();
    SfxMapUnit eMapUnit = SFX_MAPUNIT_100TH_MM;
    if(mpModel)
    {
        eMapUnit = mpModel->GetItemPool().GetMetric(0);
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
    DBG_TESTSOLARMUTEX();
    if(mpObj.is() && rPropSet.AreThereOwnUsrAnys() && mpModel)
    {
        SfxItemSet aSet( mpModel->GetItemPool(), SDRATTR_START, SDRATTR_END, 0);
        Reference< beans::XPropertySet > xShape( (OWeakObject*)this, UNO_QUERY );
        maPropSet.ObtainSettingsFromPropertySet(rPropSet, aSet, xShape);

        mpObj->SetMergedItemSetAndBroadcast(aSet);

        mpObj->ApplyNotPersistAttr( aSet );
    }
}

//----------------------------------------------------------------------

uno::Any SvxShape::GetBitmap( sal_Bool bMetaFile /* = sal_False */ ) const throw()
{
    DBG_TESTSOLARMUTEX();
    uno::Any aAny;

    if( !mpObj.is() || mpModel == NULL || !mpObj->IsInserted() || NULL == mpObj->GetPage() )
        return aAny;

    VirtualDevice aVDev;
    aVDev.SetMapMode(MapMode(MAP_100TH_MM));

    SdrModel* pModel = mpObj->GetModel();
    SdrPage* pPage = mpObj->GetPage();

    E3dView* pView = new E3dView( pModel, &aVDev );
    pView->hideMarkHandles();
    SdrPageView* pPageView = pView->ShowSdrPage(pPage);

    SdrObject *pTempObj = mpObj.get();
    pView->MarkObj(pTempObj,pPageView);

    Rectangle aRect(pTempObj->GetCurrentBoundRect());
    aRect.Justify();
    Size aSize(aRect.GetSize());

    GDIMetaFile aMtf( pView->GetAllMarkedMetaFile() );
    if( bMetaFile )
    {
        SvMemoryStream aDestStrm( 65535, 65535 );
        ConvertGDIMetaFileToWMF( aMtf, aDestStrm, NULL, sal_False );
        uno::Sequence<sal_Int8> aSeq((sal_Int8*)aDestStrm.GetData(), aDestStrm.GetSize());
        aAny.setValue( &aSeq, ::getCppuType((const uno::Sequence< sal_Int8 >*)0) );
    }
    else
    {
        Graphic aGraph(aMtf);
        aGraph.SetPrefSize(aSize);
        aGraph.SetPrefMapMode(MAP_100TH_MM);

        Reference< awt::XBitmap > xBmp( aGraph.GetXGraphic(), UNO_QUERY );
        aAny <<= xBmp;
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
    case OBJ_MEDIA:
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
//                  *pTypes++ = ::getCppuType((const uno::Reference< beans::XTolerantMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = beans::XMultiPropertyStates::static_type();
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
                    aTypeSequence.realloc( 14 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
//                  *pTypes++ = ::getCppuType((const uno::Reference< beans::XTolerantMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = beans::XMultiPropertyStates::static_type();
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
                    aTypeSequence.realloc( 16 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
//                  *pTypes++ = ::getCppuType((const uno::Reference< beans::XTolerantMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = beans::XMultiPropertyStates::static_type();
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
                    aTypeSequence.realloc( 13 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
//                  *pTypes++ = ::getCppuType((const uno::Reference< beans::XTolerantMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = beans::XMultiPropertyStates::static_type();
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
                    aTypeSequence.realloc( 13 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
//                  *pTypes++ = ::getCppuType((const uno::Reference< beans::XTolerantMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = beans::XMultiPropertyStates::static_type();
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
    case OBJ_TABLE:
    case OBJ_CUSTOMSHAPE:
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
                    aTypeSequence.realloc( 15 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShape >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< lang::XComponent >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0);
//                  *pTypes++ = ::getCppuType((const uno::Reference< beans::XTolerantMultiPropertySet >*)0);
                    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState >*)0);
                    *pTypes++ = beans::XMultiPropertyStates::static_type();
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

//----------------------------------------------------------------------

Reference< uno::XInterface > SvxShape_NewInstance()
{
    return uno::Reference< uno::XInterface >(static_cast< OWeakObject* >( new SvxShape() ) );
}

//----------------------------------------------------------------------

/** called from SdrObject::SendUserCall
    Currently only called for SDRUSERCALL_CHILD_CHGATTR
*/
void SvxShape::onUserCall(SdrUserCallType eUserCall, const Rectangle& )
{
    switch( eUserCall )
    {
    case SDRUSERCALL_CHILD_CHGATTR:
        {
            beans::PropertyChangeEvent aEvent;
            aEvent.Further = sal_False;
            aEvent.PropertyHandle = 0;
            aEvent.Source = static_cast< ::cppu::OWeakObject* >( this );
            maDisposeListeners.notifyEach( &beans::XPropertyChangeListener::propertyChange, aEvent );
        }
        break;
    default:
        break;
    }
}

//----------------------------------------------------------------------
// SfxListener
//----------------------------------------------------------------------

void SvxShape::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    DBG_TESTSOLARMUTEX();
    if( !mpObj.is() )
        return;

    // #i55919# HINT_OBJCHG is only interesting if it's for this object

    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
    if (!pSdrHint || ( /* (pSdrHint->GetKind() != HINT_OBJREMOVED)  && */
        (pSdrHint->GetKind() != HINT_MODELCLEARED) &&
        // #110094#-9 (pSdrHint->GetKind() != HINT_OBJLISTCLEAR) &&
        ((pSdrHint->GetKind() != HINT_OBJCHG || pSdrHint->GetObject() != mpObj.get() ))))
        return;

    uno::Reference< uno::XInterface > xSelf( mpObj->getWeakUnoShape() );
    if( !xSelf.is() )
    {
        mpObj.reset( NULL );
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
        case HINT_MODELCLEARED:
        {
            bClearMe = sal_True;
            mpModel = NULL;
            break;
        }
        default:
            break;
    };

    if( bClearMe )
    {
        mpObj.reset( NULL );
        if(!mbDisposing)
            dispose();
    }
}

// XShape

//----------------------------------------------------------------------
// The "*LogicRectHack" functions also existed in sch, and those
// duplicate symbols cause Bad Things To Happen (TM)  #i9462#.
// Prefixing with 'svx' and marking static to make sure name collisions
// do not occur.

static sal_Bool svx_needLogicRectHack( SdrObject* pObj )
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

static Rectangle svx_getLogicRectHack( SdrObject* pObj )
{
    if(svx_needLogicRectHack(pObj))
    {
        return pObj->GetSnapRect();
    }
    else
    {
        return pObj->GetLogicRect();
    }
}

//----------------------------------------------------------------------

static void svx_setLogicRectHack( SdrObject* pObj, const Rectangle& rRect )
{
    if(svx_needLogicRectHack(pObj))
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

    if( mpObj.is() && mpModel)
    {
        Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
        Point aPt( aRect.Left(), aRect.Top() );

        // Position is relativ to anchor, so recalc to absolut position
        if( mpModel->IsWriter() )
            aPt -= mpObj->GetAnchorPos();

        ForceMetricTo100th_mm(aPt);
        return ::com::sun::star::awt::Point( aPt.X(), aPt.Y() );
    }
    else
    {
        return maPosition;
    }
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpObj.is() && mpModel )
    {
        // do NOT move 3D objects, this would change the homogen
        // transformation matrix
        if(!mpObj->ISA(E3dCompoundObject))
        {
            Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
            Point aLocalPos( Position.X, Position.Y );
            ForceMetricToItemPoolMetric(aLocalPos);

            // Position ist absolut, relativ zum Anker stellen
            if( mpModel->IsWriter() )
                aLocalPos += mpObj->GetAnchorPos();

            long nDX = aLocalPos.X() - aRect.Left();
            long nDY = aLocalPos.Y() - aRect.Top();

            mpObj->Move( Size( nDX, nDY ) );
            mpModel->SetChanged();
        }
    }

    maPosition = Position;
}

//----------------------------------------------------------------------
awt::Size SAL_CALL SvxShape::getSize() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpObj.is() && mpModel)
    {
        Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
        Size aObjSize( aRect.getWidth(), aRect.getHeight() );
        ForceMetricTo100th_mm(aObjSize);
        return ::com::sun::star::awt::Size( aObjSize.getWidth(), aObjSize.getHeight() );
    }
    else
        return maSize;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpObj.is() && mpModel)
    {
        Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
        Size aLocalSize( rSize.Width, rSize.Height );
        ForceMetricToItemPoolMetric(aLocalSize);

        if(mpObj->GetObjInventor() == SdrInventor && mpObj->GetObjIdentifier() == OBJ_MEASURE )
        {
            Fraction aWdt(aLocalSize.Width(),aRect.Right()-aRect.Left());
            Fraction aHgt(aLocalSize.Height(),aRect.Bottom()-aRect.Top());
            Point aPt = mpObj->GetSnapRect().TopLeft();
            mpObj->Resize(aPt,aWdt,aHgt);
        }
        else
        {
            aRect.SetSize(aLocalSize);
            svx_setLogicRectHack( mpObj.get(), aRect );
        }

        mpModel->SetChanged();
    }
    maSize = rSize;
}

//----------------------------------------------------------------------

// XNamed
OUString SAL_CALL SvxShape::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    if( mpObj.is() )
    {
        return mpObj->GetName();
    }
    else
    {
        return maShapeName;
    }
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    if( mpObj.is() )
    {
        mpObj->SetName( aName );
    }
    else
    {
        maShapeName = aName;
    }
}

// XShapeDescriptor

//----------------------------------------------------------------------
OUString SAL_CALL SvxShape::getShapeType() throw(uno::RuntimeException)
{
    if( 0 == maShapeType.getLength() )
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

    return maShapeType;
}

// XComponent

//----------------------------------------------------------------------
void SAL_CALL SvxShape::dispose() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mbDisposing )
        return; // caught a recursion

    mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *(OWeakAggObject*) this;
    maDisposeListeners.disposeAndClear(aEvt);

    if(mpObj.is() && mpObj->IsInserted() && mpObj->GetPage() )
    {
        SdrPage* pPage = mpObj->GetPage();
        // SdrObject aus der Page loeschen
        sal_uInt32 nCount = pPage->GetObjCount();
        for( sal_uInt32 nNum = 0; nNum < nCount; nNum++ )
        {
            if(pPage->GetObj(nNum) == mpObj.get())
            {
                OSL_VERIFY( pPage->RemoveObject(nNum) == mpObj.get() );
                // in case we have the ownership of the SdrObject, a Free
                // would do nothing. So ensure the ownership is reset.
                mpImpl->mbHasSdrObjectOwnership = false;
                SdrObject* pObject = mpObj.get();
                SdrObject::Free( pObject );
                break;
            }
        }
    }

    if( mpModel )
    {
        EndListening( *mpModel );
        mpModel = NULL;
    }
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addEventListener( const Reference< lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    maDisposeListeners.addInterface(xListener);
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::removeEventListener( const Reference< lang::XEventListener >& aListener ) throw(uno::RuntimeException)
{
   maDisposeListeners.removeInterface(aListener);
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
    return maPropSet.getPropertySetInfo();
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addPropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >& xListener  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    maDisposeListeners.addInterface(xListener);
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& , const Reference< beans::XPropertyChangeListener >& xListener  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    maDisposeListeners.removeInterface(xListener);
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::addVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxShape::removeVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

//----------------------------------------------------------------------

sal_Bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName )
{
    SfxItemSet aSet( mpModel->GetItemPool(),    (USHORT)nWID, (USHORT)nWID );

    if( SetFillAttribute( nWID, rName, aSet, mpModel ) )
    {
        //mpObj->SetItemSetAndBroadcast(aSet);
        mpObj->SetMergedItemSetAndBroadcast(aSet);

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

            XBitmapEntry* pEntry = pBitmapList->GetBitmap( nPos );
            XFillBitmapItem aBmpItem;
            aBmpItem.SetWhich( XATTR_FILLBITMAP );
            aBmpItem.SetName( rName );
            aBmpItem.SetBitmapValue( pEntry->GetXBitmap() );
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

            XGradientEntry* pEntry = pGradientList->GetGradient( nPos );
            XFillGradientItem aGrdItem;
            aGrdItem.SetWhich( XATTR_FILLGRADIENT );
            aGrdItem.SetName( rName );
            aGrdItem.SetGradientValue( pEntry->GetGradient() );
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

            XHatchEntry* pEntry = pHatchList->GetHatch( nPos );
            XFillHatchItem aHatchItem;
            aHatchItem.SetWhich( XATTR_FILLHATCH );
            aHatchItem.SetName( rName );
            aHatchItem.SetHatchValue( pEntry->GetHatch() );
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

            XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );
            if( XATTR_LINEEND == nWID )
            {
                XLineEndItem aLEItem;
                aLEItem.SetWhich( XATTR_LINEEND );
                aLEItem.SetName( rName );
                aLEItem.SetLineEndValue( pEntry->GetLineEnd() );
                rSet.Put( aLEItem );
            }
            else
            {
                XLineStartItem aLSItem;
                aLSItem.SetWhich( XATTR_LINESTART );
                aLSItem.SetName( rName );
                aLSItem.SetLineStartValue( pEntry->GetLineEnd() );
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

            XDashEntry* pEntry = pDashList->GetDash( nPos );
            XLineDashItem aDashItem;
            aDashItem.SetWhich( XATTR_LINEDASH );
            aDashItem.SetName( rName );
            aDashItem.SetDashValue( pEntry->GetDash() );
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
                const basegfx::B2DPolyPolygon aEmptyPoly;
                if( nWID == XATTR_LINEEND )
                    rSet.Put( XLineEndItem( aEmpty, aEmptyPoly ) );
                else
                    rSet.Put( XLineStartItem( aEmpty, aEmptyPoly ) );

                return sal_True;
            }
        case XATTR_FILLFLOATTRANSPARENCE:
            {
                // #85953# Set a disabled XFillFloatTransparenceItem
                rSet.Put(XFillFloatTransparenceItem());

                return sal_True;
            }
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

// static
uno::Any SAL_CALL SvxShape::GetFillAttributeByName(
    const ::rtl::OUString& rPropertyName, const ::rtl::OUString& rName, SdrModel* pModel )
{
    uno::Any aResult;
    DBG_ASSERT( pModel, "Invalid Model in GetFillAttributeByName()" );
    if( ! pModel )
        return aResult;

    sal_Int16 nWhich = SvxUnoGetWhichIdForNamedProperty( rPropertyName );

    // search pool for item
    const SfxItemPool& rPool = pModel->GetItemPool();

    const String aSearchName( rName );
    const USHORT nCount = rPool.GetItemCount((USHORT)nWhich);
    const NameOrIndex *pItem = 0;
    bool bFound = false;

    for( USHORT nSurrogate = 0; ! bFound && nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)rPool.GetItem((USHORT)nWhich, nSurrogate);
        if( pItem && ( pItem->GetName() == aSearchName ) )
        {
            bFound = true;
        }
    }

    // check the property lists that are loaded for the model for items that
    // support such.
    String aStrName;
    SvxUnogetInternalNameForItem( nWhich, rName, aStrName );

    switch( nWhich )
    {
        case XATTR_FILLBITMAP:
        {
            XFillBitmapItem aBmpItem;
            if( ! bFound )
            {
                XBitmapList* pBitmapList = pModel->GetBitmapList();

                if( !pBitmapList )
                    break;

                long nPos = ((XPropertyList*)pBitmapList)->Get(aStrName);
                if( nPos == -1 )
                    break;

                XBitmapEntry* pEntry = pBitmapList->GetBitmap( nPos );
                aBmpItem.SetWhich( XATTR_FILLBITMAP );
                aBmpItem.SetName( rName );
                aBmpItem.SetBitmapValue( pEntry->GetXBitmap() );
                pItem = & aBmpItem;
            }
            DBG_ASSERT( pItem, "Invalid Item" );
            if( pItem )
                pItem->QueryValue( aResult ); // default: XBitmap. MID_GRAFURL instead?
        }
        break;

        case XATTR_FILLGRADIENT:
        {
            XFillGradientItem aGrdItem;
            if( ! bFound )
            {
                XGradientList* pGradientList = pModel->GetGradientList();

                if( !pGradientList )
                    break;

                long nPos = ((XPropertyList*)pGradientList)->Get(aStrName);
                if( nPos == -1 )
                    break;

                XGradientEntry* pEntry = pGradientList->GetGradient( nPos );
                aGrdItem.SetWhich( XATTR_FILLGRADIENT );
                aGrdItem.SetName( rName );
                aGrdItem.SetGradientValue( pEntry->GetGradient() );
                pItem = & aGrdItem;
            }
            DBG_ASSERT( pItem, "Invalid Item" );
            if( pItem )
                pItem->QueryValue( aResult, MID_FILLGRADIENT );
        }
        break;

        case XATTR_FILLHATCH:
        {
            XFillHatchItem aHatchItem;
            if( ! bFound )
            {
                XHatchList* pHatchList = pModel->GetHatchList();

                if( !pHatchList )
                    break;

                long nPos = ((XPropertyList*)pHatchList)->Get(aStrName);
                if( nPos == -1 )
                    break;

                XHatchEntry* pEntry = pHatchList->GetHatch( nPos );
                aHatchItem.SetWhich( XATTR_FILLHATCH );
                aHatchItem.SetName( rName );
                aHatchItem.SetHatchValue( pEntry->GetHatch() );
                pItem = & aHatchItem;
            }
            DBG_ASSERT( pItem, "Invalid Item" );
            if( pItem )
                pItem->QueryValue( aResult, MID_FILLHATCH );
        }
        break;

        case XATTR_LINEEND:
        case XATTR_LINESTART:
        {
            if( ! bFound )
            {
                XLineEndList* pLineEndList = pModel->GetLineEndList();

                if( !pLineEndList )
                    break;

                long nPos = ((XPropertyList*)pLineEndList)->Get(aStrName);
                if( nPos == -1 )
                    break;

                XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );
                if( nWhich == XATTR_LINEEND )
                {
                    XLineEndItem aLEItem;
                    aLEItem.SetWhich( XATTR_LINEEND );
                    aLEItem.SetName( rName );
                    aLEItem.SetLineEndValue( pEntry->GetLineEnd() );
                    aLEItem.QueryValue( aResult );
                }
                else
                {
                    XLineStartItem aLSItem;
                    aLSItem.SetWhich( XATTR_LINESTART );
                    aLSItem.SetName( rName );
                    aLSItem.SetLineStartValue( pEntry->GetLineEnd() );
                    aLSItem.QueryValue( aResult );
                }
            }
            else
            {
                DBG_ASSERT( pItem, "Invalid Item" );
                if( pItem )
                    pItem->QueryValue( aResult );
            }
        }
        break;

        case XATTR_LINEDASH:
        {
            XLineDashItem aDashItem;
            if( ! bFound )
            {
                XDashList* pDashList = pModel->GetDashList();

                if( !pDashList )
                    break;

                long nPos = ((XPropertyList*)pDashList)->Get(aStrName);
                if( nPos == -1 )
                    break;

                XDashEntry* pEntry = pDashList->GetDash( nPos );
                aDashItem.SetWhich( XATTR_LINEDASH );
                aDashItem.SetName( rName );
                aDashItem.SetDashValue( pEntry->GetDash() );
                pItem = & aDashItem;
            }
            DBG_ASSERT( pItem, "Invalid Item" );
            if( pItem )
                pItem->QueryValue( aResult, MID_LINEDASH );
        }
        break;
    }

    return aResult;
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

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(rPropertyName);

    if( mpObj.is() && mpModel )
    {
        if( pMap == NULL )
            throw beans::UnknownPropertyException();

        if( (pMap->nFlags & beans::PropertyAttribute::READONLY ) != 0 )
            throw beans::PropertyVetoException();

        mpModel->SetChanged();

        if(!setPropertyValueImpl( pMap, rVal ) )
        {
            DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST, "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            sal_Bool bIsNotPersist = pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST && pMap->nWID != SDRATTR_TEXTDIRECTION;

            if( pMap->nWID == SDRATTR_ECKENRADIUS )
            {
                sal_Int32 nCornerRadius = 0;
                if( !(rVal >>= nCornerRadius) || (nCornerRadius < 0) || (nCornerRadius > 5000000))
                    throw IllegalArgumentException();
            }

            SfxItemSet* pSet;
            if( mbIsMultiPropertyCall && !bIsNotPersist )
            {
                if( mpImpl->mpItemSet == NULL )
                {
                    pSet = mpImpl->mpItemSet = mpObj->GetMergedItemSet().Clone();
                }
                else
                {
                    pSet = mpImpl->mpItemSet;
                }
            }
            else
            {
                pSet = new SfxItemSet( mpModel->GetItemPool(),  pMap->nWID, pMap->nWID);
            }

            if( pSet->GetItemState( pMap->nWID ) != SFX_ITEM_SET )
                pSet->Put(mpObj->GetMergedItem(pMap->nWID));

            if( !SvxUnoTextRangeBase::SetPropertyValueHelper( *pSet, pMap, rVal, *pSet ))
            {
                if( pSet->GetItemState( pMap->nWID ) != SFX_ITEM_SET )
                {
                    if(bIsNotPersist)
                    {
                        // Not-Persistant Attribute, hole diese extra
                        mpObj->TakeNotPersistAttr(*pSet, sal_False);
                    }
                }

                if( pSet->GetItemState( pMap->nWID ) != SFX_ITEM_SET )
                {
                    // Default aus ItemPool holen
                    if(mpModel->GetItemPool().IsWhich(pMap->nWID))
                        pSet->Put(mpModel->GetItemPool().GetDefaultItem(pMap->nWID));
                }

                if( pSet->GetItemState( pMap->nWID ) == SFX_ITEM_SET )
                {
                    maPropSet.setPropertyValue( pMap, rVal, *pSet );

                }
            }

            if(bIsNotPersist)
            {
                // Not-Persist Attribute extra setzen
                mpObj->ApplyNotPersistAttr( *pSet );
                delete pSet;
            }
            else
            {
                // if we have a XMultiProperty call then the item set
                // will be set in setPropertyValues later
                if( !mbIsMultiPropertyCall )
                {
                    mpObj->SetMergedItemSetAndBroadcast( *pSet );

                    delete pSet;
                }
            }
            return;
        }
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
            maPropSet.setPropertyValue( pMap, rVal );
    }
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

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    uno::Any aAny;
    if( mpObj.is() && mpModel )
    {
        if(pMap == NULL )
            throw beans::UnknownPropertyException();

        if( !getPropertyValueImpl( pMap, aAny ) )
        {
            DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || (pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST), "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            SfxItemSet aSet( mpModel->GetItemPool(),    pMap->nWID, pMap->nWID);
            aSet.Put(mpObj->GetMergedItem(pMap->nWID));

            if(SvxUnoTextRangeBase::GetPropertyValueHelper(  aSet, pMap, aAny ))
                return aAny;

            if(!aSet.Count())
            {
                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // Not-Persistant Attribute, hole diese extra
                    mpObj->TakeNotPersistAttr(aSet, sal_False);
                }
            }

            if(!aSet.Count())
            {
                // Default aus ItemPool holen
                if(mpModel->GetItemPool().IsWhich(pMap->nWID))
                    aSet.Put(mpModel->GetItemPool().GetDefaultItem(pMap->nWID));
            }

            if(aSet.Count())
                aAny = GetAnyForItem( aSet, pMap );
        }
    }
    else
    {

// Fixme: we should return default values for OWN_ATTR !

        if(pMap && pMap->nWID)
//      FixMe: see setPropertyValue
            aAny = maPropSet.getPropertyValue( pMap );

    }
    return aAny;
}

//----------------------------------------------------------------------

// XMultiPropertySet
void SAL_CALL SvxShape::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    OGuard aSolarGuard( Application::GetSolarMutex() );

    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pNames = aPropertyNames.getConstArray();

    const uno::Any* pValues = aValues.getConstArray();

    // make sure mbIsMultiPropertyCall and mpImpl->mpItemSet are
    // reseted even when an execption is thrown
    const ::comphelper::ScopeGuard aGuard( boost::bind( &SvxShape::endSetPropertyValues, this ) );

    mbIsMultiPropertyCall = sal_True;

    if( mpImpl->mpMaster )
    {
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pNames++, pValues++ )
        {
            try
            {
                setPropertyValue( *pNames, *pValues );
            }
            catch( beans::UnknownPropertyException& e )
            {
                (void)e;
            }
            catch( uno::Exception& ex )
            {
                (void)ex;
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xSet;
        queryInterface( ::getCppuType( (const uno::Reference< beans::XPropertySet >*) 0) ) >>= xSet;

        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pNames++, pValues++ )
        {
            try
            {
                xSet->setPropertyValue( *pNames, *pValues );
            }
            catch( beans::UnknownPropertyException& e )
            {
                (void)e;
            }
            catch( uno::Exception& ex )
            {
                (void)ex;
            }
        }
    }

    if( mpImpl->mpItemSet && mpObj.is() )
        mpObj->SetMergedItemSetAndBroadcast( *mpImpl->mpItemSet );
}

//----------------------------------------------------------------------

void SvxShape::endSetPropertyValues()
{
    mbIsMultiPropertyCall = sal_False;
    if( mpImpl->mpItemSet )
    {
        delete mpImpl->mpItemSet;
        mpImpl->mpItemSet = 0;
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
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++, pNames++ )
        {
            try
            {
                *pValue = getPropertyValue( *pNames );
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

        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++, pNames++ )
        {
            try
            {
                *pValue = xSet->getPropertyValue( *pNames );
            }
            catch( uno::Exception& )
            {
                DBG_ERROR( "SvxShape::getPropertyValues, unknown property asked" );
            }
        }
    }

    return aRet;
}

void SAL_CALL SvxShape::addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >&  ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxShape::removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >&  ) throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SvxShape::firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >&  ) throw (::com::sun::star::uno::RuntimeException)
{
}

//----------------------------------------------------------------------

uno::Any SvxShape::GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertyMap* pMap ) const
{
    DBG_TESTSOLARMUTEX();
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
        if( mpObj->GetObjInventor() == SdrInventor)
        {
            drawing::CircleKind eKind;
            switch(mpObj->GetObjIdentifier())
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
        aAny = maPropSet.getPropertyValue( pMap, aSet );

        if( *pMap->pType != aAny.getValueType() )
        {
            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
            if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                sal_Int32 nValue = 0;
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

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    if( !mpObj.is() || pMap == NULL )
        throw beans::UnknownPropertyException();

    beans::PropertyState eState;
    if( !getPropertyStateImpl( pMap, eState ) )
    {
        const SfxItemSet& rSet = mpObj->GetMergedItemSet();

        switch( rSet.GetItemState( pMap->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            eState = beans::PropertyState_DIRECT_VALUE;
            break;
        case SFX_ITEM_DEFAULT:
            eState = beans::PropertyState_DEFAULT_VALUE;
            break;
//      case SFX_ITEM_UNKNOWN:
//      case SFX_ITEM_DONTCARE:
//      case SFX_ITEM_DISABLED:
        default:
            eState = beans::PropertyState_AMBIGUOUS_VALUE;
            break;
        }

        // if a item is set, this doesn't mean we want it :)
        if( ( beans::PropertyState_DIRECT_VALUE == eState ) )
        {
            switch( pMap->nWID )
            {
            // the following items are disabled by changing the
            // fill style or the line style. so there is no need
            // to export items without names which should be empty
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_LINEDASH:
                {
                    NameOrIndex* pItem = (NameOrIndex*)rSet.GetItem((USHORT)pMap->nWID);
                    if( ( pItem == NULL ) || ( pItem->GetName().Len() == 0) )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
                break;

            // #i36115#
            // If e.g. the LineStart is on NONE and thus the string has length 0, it still
            // may be a hard attribute covering the set LineStart of the parent (Style).
            // #i37644#
            // same is for fill float transparency
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_FILLFLOATTRANSPARENCE:
                {
                    NameOrIndex* pItem = (NameOrIndex*)rSet.GetItem((USHORT)pMap->nWID);
                    if( ( pItem == NULL ) )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
                break;
            }
        }
    }
    return eState;
}

//----------------------------------------------------------------------

bool SvxShape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CAPTION_POINT:
    {
        awt::Point aPnt;
        if( rValue >>= aPnt )
        {
            Point aVclPoint( aPnt.X, aPnt.Y );

            // #90763# position is relative to top left, make it absolute
            basegfx::B2DPolyPolygon aNewPolyPolygon;
            basegfx::B2DHomMatrix aNewHomogenMatrix;
            mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aVclPoint.X() += FRound(aNewHomogenMatrix.get(0, 2));
            aVclPoint.Y() += FRound(aNewHomogenMatrix.get(1, 2));

            // #88657# metric of pool maybe twips (writer)
            ForceMetricToItemPoolMetric(aVclPoint);

            // #88491# position relative to anchor
            if( mpModel->IsWriter() )
            {
                aVclPoint += mpObj->GetAnchorPos();
            }

            ((SdrCaptionObj*)mpObj.get())->SetTailPos(aVclPoint);

            return true;
        }
        break;
    }
    case OWN_ATTR_TRANSFORMATION:
    {
        drawing::HomogenMatrix3 aMatrix;
        if(rValue >>= aMatrix)
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon;
            basegfx::B2DHomMatrix aNewHomogenMatrix;

            mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aNewHomogenMatrix.set(0, 0, aMatrix.Line1.Column1);
            aNewHomogenMatrix.set(0, 1, aMatrix.Line1.Column2);
            aNewHomogenMatrix.set(0, 2, aMatrix.Line1.Column3);
            aNewHomogenMatrix.set(1, 0, aMatrix.Line2.Column1);
            aNewHomogenMatrix.set(1, 1, aMatrix.Line2.Column2);
            aNewHomogenMatrix.set(1, 2, aMatrix.Line2.Column3);
            aNewHomogenMatrix.set(2, 0, aMatrix.Line3.Column1);
            aNewHomogenMatrix.set(2, 1, aMatrix.Line3.Column2);
            aNewHomogenMatrix.set(2, 2, aMatrix.Line3.Column3);

            mpObj->TRSetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
            return true;
        }
        break;
    }

    case OWN_ATTR_ZORDER:
    {
        sal_Int32 nNewOrdNum = 0;
        if(rValue >>= nNewOrdNum)
        {
            SdrObjList* pObjList = mpObj->GetObjList();
            if( pObjList )
            {
#ifdef DBG_UTIL
                SdrObject* pCheck =
#endif
                            pObjList->SetObjectOrdNum( mpObj->GetOrdNum(), (ULONG)nNewOrdNum );
                DBG_ASSERT( pCheck == mpObj.get(), "GetOrdNum() failed!" );
            }
            return true;
        }
        break;
    }
    case OWN_ATTR_FRAMERECT:
    {
        awt::Rectangle aUnoRect;
        if(rValue >>= aUnoRect)
        {
            Point aTopLeft( aUnoRect.X, aUnoRect.Y );
            Size aObjSize( aUnoRect.Width, aUnoRect.Height );
            ForceMetricToItemPoolMetric(aTopLeft);
            ForceMetricToItemPoolMetric(aObjSize);
            Rectangle aRect;
            aRect.SetPos(aTopLeft);
            aRect.SetSize(aObjSize);
            mpObj->SetSnapRect(aRect);
            return true;
        }
        break;
    }
    case OWN_ATTR_MIRRORED:
    {
        sal_Bool bMirror = sal_Bool();
        if(rValue >>= bMirror )
        {
            SdrGrafObj* pObj = dynamic_cast< SdrGrafObj* >( mpObj.get() );
            if( pObj )
                pObj->SetMirrored(bMirror);
            return true;
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
        SdrEdgeObj* pEdgeObj = dynamic_cast< SdrEdgeObj* >(mpObj.get());
        if(pEdgeObj)
        {
            switch(pProperty->nWID)
            {
            case OWN_ATTR_EDGE_START_OBJ:
            case OWN_ATTR_EDGE_END_OBJ:
                {
                    Reference< drawing::XShape > xShape;
                    if( rValue >>= xShape )
                    {
                        SdrObject* pNode = GetSdrObjectFromXShape( xShape );
                        if( pNode )
                        {
                            pEdgeObj->ConnectToNode( pProperty->nWID == OWN_ATTR_EDGE_START_OBJ, pNode );
                            pEdgeObj->setGluePointIndex( pProperty->nWID == OWN_ATTR_EDGE_START_OBJ, -1 );
                            return true;
                        }
                    }
                    break;
                }

            case OWN_ATTR_EDGE_START_POS:
            case OWN_ATTR_EDGE_END_POS:
                {
                    awt::Point aUnoPoint;
                    if( rValue >>= aUnoPoint )
                    {
                        Point aPoint( aUnoPoint.X, aUnoPoint.Y );
                        if( mpModel->IsWriter() )
                            aPoint += mpObj->GetAnchorPos();

                        ForceMetricToItemPoolMetric( aPoint );
                        pEdgeObj->SetTailPoint( pProperty->nWID == OWN_ATTR_EDGE_START_POS, aPoint );
                        return true;
                    }
                    break;
                }

            case OWN_ATTR_GLUEID_HEAD:
            case OWN_ATTR_GLUEID_TAIL:
                {
                    sal_Int32 nId = 0;
                    if( rValue >>= nId )
                    {
                        pEdgeObj->setGluePointIndex( pProperty->nWID == OWN_ATTR_GLUEID_HEAD, nId );
                        return true;
                    }
                }
            }
        }
        break;
    }
    case OWN_ATTR_MEASURE_START_POS:
    case OWN_ATTR_MEASURE_END_POS:
    {
        SdrMeasureObj* pMeasureObj = dynamic_cast< SdrMeasureObj* >(mpObj.get());
        awt::Point aUnoPoint;
        if(pMeasureObj && ( rValue >>= aUnoPoint ) )
        {
            Point aPoint( aUnoPoint.X, aUnoPoint.Y );

            if( mpModel->IsWriter() )
                aPoint += mpObj->GetAnchorPos();

            pMeasureObj->NbcSetPoint( aPoint, pProperty->nWID == OWN_ATTR_MEASURE_START_POS ? 0L : 1L );
            pMeasureObj->SetChanged();
            pMeasureObj->BroadcastObjectChange();
            return true;
        }
        break;
    }
    case OWN_ATTR_FILLBMP_MODE:
        {
            drawing::BitmapMode eMode;
            if(!(rValue >>= eMode) )
            {
                sal_Int32 nMode = 0;
                if(!(rValue >>= nMode))
                    break;

                eMode = (drawing::BitmapMode)nMode;
            }
            mpObj->SetMergedItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
            mpObj->SetMergedItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
            return true;
        }

    case SDRATTR_LAYERID:
    {
        sal_Int16 nLayerId = sal_Int16();
        if( rValue >>= nLayerId )
        {
            SdrLayer* pLayer = mpModel->GetLayerAdmin().GetLayerPerID((unsigned char)nLayerId);
            if( pLayer )
            {
                mpObj->SetLayer((unsigned char)nLayerId);
                return true;
            }
        }
        break;
    }

    case SDRATTR_LAYERNAME:
    {
        OUString aLayerName;
        if( rValue >>= aLayerName )
        {
            const SdrLayer* pLayer=mpModel->GetLayerAdmin().GetLayer(aLayerName, TRUE);
            if( pLayer != NULL )
            {
                mpObj->SetLayer( pLayer->GetID() );
                return true;
            }
        }
        break;
    }
    case SDRATTR_ROTATEANGLE:
    {
        sal_Int32 nAngle = 0;
        if( rValue >>= nAngle )
        {
            Point aRef1(mpObj->GetSnapRect().Center());
            nAngle -= mpObj->GetRotateAngle();
            if (nAngle!=0)
            {
                double nSin=sin(nAngle*nPi180);
                double nCos=cos(nAngle*nPi180);
                mpObj->Rotate(aRef1,nAngle,nSin,nCos);
            }
            return true;
        }

        break;
    }

    case SDRATTR_SHEARANGLE:
    {
        sal_Int32 nShear = 0;
        if( rValue >>= nShear )
        {
            nShear -= mpObj->GetShearAngle();
            if(nShear != 0 )
            {
                Point aRef1(mpObj->GetSnapRect().Center());
                double nTan=tan(nShear*nPi180);
                mpObj->Shear(aRef1,nShear,nTan,FALSE);
                return true;
            }
        }

        break;
    }

    case SDRATTR_OBJMOVEPROTECT:
    {
        sal_Bool bMoveProtect = sal_Bool();
        if( rValue >>= bMoveProtect )
        {
            mpObj->SetMoveProtect(bMoveProtect);
            return true;
        }
        break;
    }
    case SDRATTR_OBJECTNAME:
    {
        OUString aName;
        if( rValue >>= aName )
        {
            mpObj->SetName( aName );
            return true;
        }
        break;
    }

    // #i68101#
    case OWN_ATTR_MISC_OBJ_TITLE:
    {
        OUString aTitle;
        if( rValue >>= aTitle )
        {
            mpObj->SetTitle( aTitle );
            return true;
        }
        break;
    }
    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription;
        if( rValue >>= aDescription )
        {
            mpObj->SetDescription( aDescription );
            return true;
        }
        break;
    }

    case SDRATTR_OBJPRINTABLE:
    {
        sal_Bool bPrintable = sal_Bool();
        if( rValue >>= bPrintable )
        {
            mpObj->SetPrintable(bPrintable);
            return true;
        }
        break;
    }
    case SDRATTR_OBJSIZEPROTECT:
    {
        sal_Bool bResizeProtect = sal_Bool();
        if( rValue >>= bResizeProtect )
        {
            mpObj->SetResizeProtect(bResizeProtect);
            return true;
        }
        break;
    }
    case OWN_ATTR_PAGE_NUMBER:
    {
        sal_Int32 nPageNum = 0;
        if( (rValue >>= nPageNum) && ( nPageNum >= 0 ) && ( nPageNum <= 0xffff ) )
        {
            SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(mpObj.get());
            if( pPageObj )
            {
                SdrModel* pModel = pPageObj->GetModel();
                SdrPage* pNewPage = 0L;
                const sal_uInt16 nDestinationPageNum((sal_uInt16)((nPageNum << 1L) - 1L));

                if(pModel)
                {
                    if(nDestinationPageNum < pModel->GetPageCount())
                    {
                        pNewPage = pModel->GetPage(nDestinationPageNum);
                    }
                }

                pPageObj->SetReferencedPage(pNewPage);
            }

            return true;
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
        if( pProperty->nMemberId == MID_NAME )
        {
            OUString aApiName;
            if( rValue >>= aApiName )
            {
                if( SetFillAttribute( pProperty->nWID, aApiName ) )
                    return true;
            }
            break;
        }
        else
        {
            return false;
        }
    }
    default:
    {
        return false;
    }
    }
    throw lang::IllegalArgumentException();
}

//----------------------------------------------------------------------

bool SvxShape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_HASLEVELS:
    {
        rValue <<= SvxTextEditSource::hasLevels( mpObj.get() );
        break;
    }

    case OWN_ATTR_CAPTION_POINT:
    {
        Point aVclPoint = ((SdrCaptionObj*)mpObj.get())->GetTailPos();

        // #88491# make pos relative to anchor
        if( mpModel->IsWriter() )
        {
            aVclPoint -= mpObj->GetAnchorPos();
        }

        // #88657# metric of pool maybe twips (writer)
        ForceMetricTo100th_mm(aVclPoint);

        // #90763# pos is absolute, make it relative to top left
        basegfx::B2DPolyPolygon aNewPolyPolygon;
        basegfx::B2DHomMatrix aNewHomogenMatrix;
        mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

        aVclPoint.X() -= FRound(aNewHomogenMatrix.get(0, 2));
        aVclPoint.Y() -= FRound(aNewHomogenMatrix.get(1, 2));

        awt::Point aPnt( aVclPoint.X(), aVclPoint.Y() );
        rValue <<= aPnt;
        break;
    }

    case OWN_ATTR_TRANSFORMATION:
    {
        basegfx::B2DPolyPolygon aNewPolyPolygon;
        basegfx::B2DHomMatrix aNewHomogenMatrix;
        mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
        drawing::HomogenMatrix3 aMatrix;

        aMatrix.Line1.Column1 = aNewHomogenMatrix.get(0, 0);
        aMatrix.Line1.Column2 = aNewHomogenMatrix.get(0, 1);
        aMatrix.Line1.Column3 = aNewHomogenMatrix.get(0, 2);
        aMatrix.Line2.Column1 = aNewHomogenMatrix.get(1, 0);
        aMatrix.Line2.Column2 = aNewHomogenMatrix.get(1, 1);
        aMatrix.Line2.Column3 = aNewHomogenMatrix.get(1, 2);
        aMatrix.Line3.Column1 = aNewHomogenMatrix.get(2, 0);
        aMatrix.Line3.Column2 = aNewHomogenMatrix.get(2, 1);
        aMatrix.Line3.Column3 = aNewHomogenMatrix.get(2, 2);

        rValue <<= aMatrix;

        break;
    }

    case OWN_ATTR_ZORDER:
    {
        rValue <<= (sal_Int32)mpObj->GetOrdNum();
        break;
    }

    case OWN_ATTR_BITMAP:
    {
        rValue = GetBitmap();
        if(!rValue.hasValue())
            throw uno::RuntimeException();

        break;
    }

    case OWN_ATTR_ISFONTWORK:
    {
        rValue <<= (sal_Bool)(mpObj->ISA(SdrTextObj) && ((SdrTextObj*)mpObj.get())->IsFontwork());
        break;
    }

    case OWN_ATTR_FRAMERECT:
    {
        Rectangle aRect( mpObj->GetSnapRect() );
        Point aTopLeft( aRect.TopLeft() );
        Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
        ForceMetricTo100th_mm(aTopLeft);
        ForceMetricTo100th_mm(aObjSize);
        ::com::sun::star::awt::Rectangle aUnoRect(
            aTopLeft.X(), aTopLeft.Y(),
            aObjSize.getWidth(), aObjSize.getHeight() );
        rValue <<= aUnoRect;
        break;
    }

    case OWN_ATTR_BOUNDRECT:
    {
        Rectangle aRect( mpObj->GetCurrentBoundRect() );
        Point aTopLeft( aRect.TopLeft() );
        Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
        ForceMetricTo100th_mm(aTopLeft);
        ForceMetricTo100th_mm(aObjSize);
        ::com::sun::star::awt::Rectangle aUnoRect(
            aTopLeft.X(), aTopLeft.Y(),
            aObjSize.getWidth(), aObjSize.getHeight() );
        rValue <<= aUnoRect;
        break;
    }

    case OWN_ATTR_LDNAME:
    {
        OUString aName( mpObj->GetName() );
        rValue <<= aName;
        break;
    }

    case OWN_ATTR_LDBITMAP:
    {
        sal_uInt16 nId;
        if( mpObj->GetObjInventor() == SdrInventor && mpObj->GetObjIdentifier() == OBJ_OLE2 )
        {
            nId = RID_UNODRAW_OLE2;
        }
        else if( mpObj->GetObjInventor() == SdrInventor && mpObj->GetObjIdentifier() == OBJ_GRAF )
        {
            nId = RID_UNODRAW_GRAPHICS;
        }
        else
        {
            nId = RID_UNODRAW_OBJECTS;
        }

        BitmapEx aBmp( SVX_RES(nId) );
        Reference< awt::XBitmap > xBmp( VCLUnoHelper::CreateBitmap( aBmp ) );

        rValue <<= xBmp;
        break;
    }

    case OWN_ATTR_MIRRORED:
    {
        sal_Bool bMirror = sal_False;
        if( mpObj.is() && mpObj->ISA(SdrGrafObj) )
            bMirror = ((SdrGrafObj*)mpObj.get())->IsMirrored();

        rValue <<= bMirror;
    }

    case OWN_ATTR_EDGE_START_OBJ:
    case OWN_ATTR_EDGE_START_POS:
    case OWN_ATTR_EDGE_END_POS:
    case OWN_ATTR_EDGE_END_OBJ:
    case OWN_ATTR_GLUEID_HEAD:
    case OWN_ATTR_GLUEID_TAIL:
    {
        SdrEdgeObj* pEdgeObj = dynamic_cast<SdrEdgeObj*>(mpObj.get());
        if(pEdgeObj)
        {
            switch(pProperty->nWID)
            {
            case OWN_ATTR_EDGE_START_OBJ:
            case OWN_ATTR_EDGE_END_OBJ:
                {
                    SdrObject* pNode = pEdgeObj->GetConnectedNode(pProperty->nWID == OWN_ATTR_EDGE_START_OBJ);
                    if(pNode)
                    {
                        Reference< drawing::XShape > xShape( GetXShapeForSdrObject( pNode ) );
                        if(xShape.is())
                            rValue <<= xShape;

                    }
                    break;
                }

            case OWN_ATTR_EDGE_START_POS:
            case OWN_ATTR_EDGE_END_POS:
                {
                    Point aPoint( pEdgeObj->GetTailPoint( pProperty->nWID == OWN_ATTR_EDGE_START_POS ) );
                    if( mpModel->IsWriter() )
                        aPoint -= mpObj->GetAnchorPos();

                    ForceMetricTo100th_mm( aPoint );
                    awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );

                    rValue <<= aUnoPoint;
                    break;
                }
            case OWN_ATTR_GLUEID_HEAD:
            case OWN_ATTR_GLUEID_TAIL:
                {
                    rValue <<= pEdgeObj->getGluePointIndex( pProperty->nWID == OWN_ATTR_GLUEID_HEAD );
                    break;
                }
            }
        }
        break;
    }

    case OWN_ATTR_MEASURE_START_POS:
    case OWN_ATTR_MEASURE_END_POS:
    {
        SdrMeasureObj* pMeasureObj = dynamic_cast<SdrMeasureObj*>(mpObj.get());
        if(pMeasureObj)
        {
            Point aPoint( pMeasureObj->GetPoint( pProperty->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 ) );
            if( mpModel->IsWriter() )
                aPoint -= mpObj->GetAnchorPos();

            awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );

            rValue <<= aUnoPoint;
            break;
        }
        break;
    }

    case OWN_ATTR_FILLBMP_MODE:
    {
        const SfxItemSet& rObjItemSet = mpObj->GetMergedItemSet();

        XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)&rObjItemSet.Get(XATTR_FILLBMP_STRETCH);
        XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)&rObjItemSet.Get(XATTR_FILLBMP_TILE);

        if( pTileItem && pTileItem->GetValue() )
        {
            rValue <<= drawing::BitmapMode_REPEAT;
        }
        else if( pStretchItem && pStretchItem->GetValue() )
        {
            rValue <<= drawing::BitmapMode_STRETCH;
        }
        else
        {
            rValue <<= drawing::BitmapMode_NO_REPEAT;
        }
        break;
    }
    case SDRATTR_LAYERID:
        rValue <<= (sal_Int16)mpObj->GetLayer();
        break;

    case SDRATTR_LAYERNAME:
    {
        SdrLayer* pLayer = mpModel->GetLayerAdmin().GetLayerPerID(mpObj->GetLayer());
        if( pLayer )
        {
            OUString aName( pLayer->GetName() );
            rValue <<= aName;
        }
        break;
    }

    case SDRATTR_ROTATEANGLE:
        rValue <<= mpObj->GetRotateAngle();
        break;

    case SDRATTR_SHEARANGLE:
        rValue <<= mpObj->GetShearAngle();
        break;

    case SDRATTR_OBJMOVEPROTECT:
        rValue = uno::makeAny( (sal_Bool) mpObj->IsMoveProtect() );
        break;

    case SDRATTR_OBJECTNAME:
    {
        OUString aName( mpObj->GetName() );
        rValue <<= aName;
        break;
    }

    // #i68101#
    case OWN_ATTR_MISC_OBJ_TITLE:
    {
        OUString aTitle( mpObj->GetTitle() );
        rValue <<= aTitle;
        break;
    }

    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription( mpObj->GetDescription() );
        rValue <<= aDescription;
        break;
    }

    case SDRATTR_OBJPRINTABLE:
        rValue = uno::makeAny( (sal_Bool) mpObj->IsPrintable() );
        break;

    case SDRATTR_OBJSIZEPROTECT:
        rValue = uno::makeAny( (sal_Bool)mpObj->IsResizeProtect() );
        break;

    case OWN_ATTR_PAGE_NUMBER:
    {
        SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(mpObj.get());
        if(pPageObj)
        {
            SdrPage* pPage = pPageObj->GetReferencedPage();
            sal_Int32 nPageNumber = (pPage) ? pPage->GetPageNum() : 0L;
            nPageNumber++;
            nPageNumber >>= 1;
            rValue <<= nPageNumber;
        }
        break;
    }

    case OWN_ATTR_UINAME_SINGULAR:
    {
        String aTmp;
        mpObj->TakeObjNameSingul( aTmp );
        rValue <<= OUString( aTmp );
        break;
    }

    case OWN_ATTR_UINAME_PLURAL:
    {
        String aTmp;
        mpObj->TakeObjNamePlural( aTmp );
        rValue <<= OUString( aTmp );
        break;
    }

    default:
        return false;
    }
    return true;
}

//----------------------------------------------------------------------

bool SvxShape::getPropertyStateImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::beans::PropertyState& rState ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    if( pProperty->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = mpObj->GetMergedItemSet();

        if( rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
            rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
        {
            rState = beans::PropertyState_DIRECT_VALUE;
        }
        else
        {
            rState = beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else if((( pProperty->nWID >= OWN_ATTR_VALUE_START && pProperty->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pProperty->nWID >= SDRATTR_NOTPERSIST_FIRST && pProperty->nWID <= SDRATTR_NOTPERSIST_LAST )) && ( pProperty->nWID != SDRATTR_TEXTDIRECTION ) )
    {
        rState = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------

bool SvxShape::setPropertyToDefaultImpl( const SfxItemPropertyMap* pProperty ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    if( pProperty->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        mpObj->ClearMergedItem( XATTR_FILLBMP_STRETCH );
        mpObj->ClearMergedItem( XATTR_FILLBMP_TILE );
        return true;
    }
    else if((pProperty->nWID >= OWN_ATTR_VALUE_START && pProperty->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pProperty->nWID >= SDRATTR_NOTPERSIST_FIRST && pProperty->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return true;
    }
    else
    {
        return false;
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

    const SfxItemPropertyMap* pProperty = maPropSet.getPropertyMapEntry(PropertyName);

    if( !mpObj.is() || mpModel == NULL || pProperty == NULL )
        throw beans::UnknownPropertyException();

    if( !setPropertyToDefaultImpl( pProperty ) )
    {
        mpObj->ClearMergedItem( pProperty->nWID );
    }

    mpModel->SetChanged();
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

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    if( !mpObj.is() || pMap == NULL || mpModel == NULL )
        throw beans::UnknownPropertyException();

    if(( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return getPropertyValue( aPropertyName );
    }
    else
    {
        // Default aus ItemPool holen
        if(!mpModel->GetItemPool().IsWhich(pMap->nWID))
            throw beans::UnknownPropertyException();

        SfxItemSet aSet( mpModel->GetItemPool(),    pMap->nWID, pMap->nWID);
        aSet.Put(mpModel->GetItemPool().GetDefaultItem(pMap->nWID));

        return GetAnyForItem( aSet, pMap );
    }
}

// XMultiPropertyStates
void SvxShape::setAllPropertiesToDefault() throw (uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( !mpObj.is() )
        throw lang::DisposedException();
    mpObj->ClearMergedItem(); // nWhich == 0 => all

    if(mpObj->ISA(SdrGrafObj))
    {
        // defaults for graphic objects have changed:
        mpObj->SetMergedItem( XFillStyleItem( XFILL_NONE ) );
        mpObj->SetMergedItem( XLineStyleItem( XLINE_NONE ) );
    }

    // #i68523# special handling for Svx3DCharacterModeItem, this is not saved
    // but needs to be TRUE in svx, pool default (false) in sch. Since sch
    // does not load lathe or extrude objects, it is possible to set the items
    // here.
    // For other solution possibilities, see task description.
    if(mpObj->ISA(E3dLatheObj) || mpObj->ISA(E3dExtrudeObj))
    {
        mpObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }

    mpModel->SetChanged();
}

void SvxShape::setPropertiesToDefault(
    const uno::Sequence<OUString>& aPropertyNames )
    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    for ( sal_Int32 pos = 0; pos < aPropertyNames.getLength(); ++pos )
        setPropertyToDefault( aPropertyNames[pos] );
}

uno::Sequence<uno::Any> SvxShape::getPropertyDefaults(
    const uno::Sequence<OUString>& aPropertyNames )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException)
{
    ::std::vector<uno::Any> ret;
    for ( sal_Int32 pos = 0; pos < aPropertyNames.getLength(); ++pos )
        ret.push_back( getPropertyDefault( aPropertyNames[pos] ) );
    return uno::Sequence<uno::Any>( &ret[0], ret.size() );
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

const char* sUNO_service_style_ParagraphProperties          = STAR_NAMESPACE "style.ParagraphProperties";
const char* sUNO_service_style_ParagraphPropertiesComplex   = STAR_NAMESPACE "style.ParagraphPropertiesComplex";
const char* sUNO_service_style_ParagraphPropertiesAsian     = STAR_NAMESPACE "style.ParagraphPropertiesAsian";
const char* sUNO_service_style_CharacterProperties          = STAR_NAMESPACE "style.CharacterProperties";
const char* sUNO_service_style_CharacterPropertiesComplex   = STAR_NAMESPACE "style.CharacterPropertiesComplex";
const char* sUNO_service_style_CharacterPropertiesAsian     = STAR_NAMESPACE "style.CharacterPropertiesAsian";

const char* sUNO_service_drawing_FillProperties             = STAR_NAMESPACE "drawing.FillProperties";
const char* sUNO_service_drawing_TextProperties             = STAR_NAMESPACE "drawing.TextProperties";
const char* sUNO_service_drawing_LineProperties             = STAR_NAMESPACE "drawing.LineProperties";
const char* sUNO_service_drawing_ConnectorProperties        = STAR_NAMESPACE "drawing.ConnectorProperties";
const char* sUNO_service_drawing_MeasureProperties          = STAR_NAMESPACE "drawing.MeasureProperties";
const char* sUNO_service_drawing_ShadowProperties           = STAR_NAMESPACE "drawing.ShadowProperties";

const char* sUNO_service_drawing_RotationDescriptor         = STAR_NAMESPACE "drawing.RotationDescriptor";

const char* sUNO_service_drawing_Text                       = STAR_NAMESPACE "drawing.Text";
const char* sUNO_service_drawing_GroupShape                 = STAR_NAMESPACE "drawing.GroupShape";

const char* sUNO_service_drawing_CustomShapeProperties      = STAR_NAMESPACE "drawing.CustomShapeProperties";
const char* sUNO_service_drawing_CustomShape                    = STAR_NAMESPACE "drawing.CustomShape";

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
const char* sUNO_service_drawing_MediaShape                 = STAR_NAMESPACE "drawing.MediaShape";


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
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpObj.is() && mpObj->GetObjInventor() == SdrInventor)
    {
        const UINT16 nIdent = mpObj->GetObjIdentifier();

        switch(nIdent)
        {
        case OBJ_GRUP:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
        case OBJ_CUSTOMSHAPE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_CustomShapeServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_CustomShapeServices, 13,
                            sUNO_service_drawing_CustomShape,
                              sUNO_service_drawing_Shape,
                            sUNO_service_drawing_CustomShapeProperties,
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
                        pSeq = &SvxShape_CustomShapeServices;
                    }
                }
                return *pSeq;
            }
        case OBJ_LINE:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard _aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
        case OBJ_MEDIA:
            {
                static uno::Sequence< OUString > *pSeq = 0;
                if( 0 == pSeq )
                {
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
                    {
                        static uno::Sequence< OUString > SvxShape_MediaServices;

                        SvxServiceInfoHelper::addToSequence( SvxShape_MediaServices, 2,
                            sUNO_service_drawing_MediaShape,
                            sUNO_service_drawing_Shape);

                        pSeq = &SvxShape_MediaServices;
                    }
                }
                return *pSeq;
            }
        }
    }
    else if( mpObj.is() && mpObj->GetObjInventor() == FmFormInventor)
    {
#if OSL_DEBUG_LEVEL > 0
        const UINT16 nIdent = mpObj->GetObjIdentifier();
        OSL_ENSURE( nIdent == OBJ_UNO, "SvxShape::_getSupportedServiceNames: FmFormInventor, but no UNO object?" );
#endif
        static uno::Sequence< OUString > *pSeq = 0;
        if( 0 == pSeq )
        {
//                  OGuard aGuard( Application::GetSolarMutex() );
//                  if( 0 == pSeq )
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
    OSL_ENSURE( false, "SvxShape::_getSupportedServiceNames: could not determine object type!" );
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
    OGuard aGuard( Application::GetSolarMutex() );
    uno::Reference< container::XIndexContainer > xGluePoints( mxGluePoints );

    if( mpObj.is() && !xGluePoints.is() )
    {
        uno::Reference< container::XIndexContainer > xNew( SvxUnoGluePointAccess_createInstance( mpObj.get() ), uno::UNO_QUERY );
        mxGluePoints = xGluePoints = xNew;
    }

    return xGluePoints;
}

//----------------------------------------------------------------------

// XChild
uno::Reference< uno::XInterface > SAL_CALL SvxShape::getParent(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( mpObj.is() && mpObj->GetObjList() )
    {
        SdrObjList* pObjList = mpObj->GetObjList();

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
        default:
            DBG_ERROR( "SvxShape::getParent(  ): unexpected SdrObjListKind" );
            break;
        }
    }

    uno::Reference< uno::XInterface > xParent;
    return xParent;
}

//----------------------------------------------------------------------

void SAL_CALL SvxShape::setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& )
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
            const sal_uInt32 nId = mpObj->GetObjIdentifier();

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
SvxShapeText::SvxShapeText() throw ()
: SvxShape(NULL, aSvxMapProvider.GetMap(SVXMAP_TEXT) ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorPropertyMap() )
{
}

//----------------------------------------------------------------------
SvxShapeText::SvxShapeText( SdrObject* pObject ) throw ()
: SvxShape( pObject, aSvxMapProvider.GetMap(SVXMAP_TEXT) ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorPropertyMap() )
{
    if( pObject && pObject->GetModel() )
        SetEditSource( new SvxTextEditSource( pObject, 0, static_cast< uno::XWeak * >( this ) ) );
}

//----------------------------------------------------------------------
SvxShapeText::SvxShapeText( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ()
: SvxShape( pObject, pPropertySet ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorPropertyMap() )
{
    if( pObject && pObject->GetModel() )
        SetEditSource( new SvxTextEditSource( pObject, 0, static_cast< uno::XWeak * >( this ) ) );
}

//----------------------------------------------------------------------
SvxShapeText::~SvxShapeText() throw ()
{
    // check if only this instance is registered at the ranges
    DBG_ASSERT( (NULL == GetEditSource()) || (GetEditSource()->getRanges().size()==1),
        "svx::SvxShapeText::~SvxShapeText(), text shape with living text ranges destroyed!");
}

void SvxShapeText::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw ()
{
    if( pNewObj && (NULL == GetEditSource()))
        SetEditSource( new SvxTextEditSource( pNewObj, 0, static_cast< uno::XWeak* >(this) ) );

    SvxShape::Create( pNewObj, pNewPage );
}

// XInterface
//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeText::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    return SvxShape::queryInterface( rType );
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShapeText::queryAggregation( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aAny( SvxShape::queryAggregation( rType ) );
    if( aAny.hasValue() )
        return aAny;

    return SvxUnoTextBase::queryAggregation( rType );
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

// ::com::sun::star::text::XTextRange
uno::Reference< text::XTextRange > SAL_CALL SvxShapeText::getStart() throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getStart();

}

uno::Reference< text::XTextRange > SAL_CALL SvxShapeText::getEnd() throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getEnd();
}

OUString SAL_CALL SvxShapeText::getString() throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getString();
}


void SAL_CALL SvxShapeText::setString( const OUString& aString ) throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    SvxUnoTextBase::setString( aString );
}

// overide these for special property handling in subcasses. Return true if property is handled
bool SvxShapeText::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // HACK-fix #99090#
    // since SdrTextObj::SetVerticalWriting exchanges
    // SDRATTR_TEXT_AUTOGROWWIDTH and SDRATTR_TEXT_AUTOGROWHEIGHT,
    // we have to set the textdirection here

    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObj.get() );
        if( pTextObj )
        {
            com::sun::star::text::WritingMode eMode;
            if( rValue >>= eMode )
            {
                pTextObj->SetVerticalWriting( eMode == com::sun::star::text::WritingMode_TB_RL );
            }
        }
        return true;
    }
    return SvxShape::setPropertyValueImpl( pProperty, rValue );
}

bool SvxShapeText::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    return SvxShape::getPropertyValueImpl( pProperty, rValue );
}

bool SvxShapeText::getPropertyStateImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::beans::PropertyState& rState ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    return SvxShape::getPropertyStateImpl( pProperty, rState );
}

bool SvxShapeText::setPropertyToDefaultImpl( const SfxItemPropertyMap* pProperty ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    return SvxShape::setPropertyToDefaultImpl( pProperty );
}

/***********************************************************************
* class SvxShapeRect                                                   *
***********************************************************************/
DBG_NAME(SvxShapeRect)
SvxShapeRect::SvxShapeRect( SdrObject* pObj ) throw()
: SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_SHAPE) )

{
    DBG_CTOR(SvxShapeRect,NULL);
}

SvxShapeRect::~SvxShapeRect() throw()
{
    DBG_DTOR(SvxShapeRect,NULL);
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
    return pShape ? pShape->GetSdrObject() : 0;
}

//----------------------------------------------------------------------

SdrObject* SdrObject::getSdrObjectFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInt )
{
    SvxShape* pSvxShape = SvxShape::getImplementation( xInt );
    return pSvxShape ? pSvxShape->GetSdrObject() : 0;
}
