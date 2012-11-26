/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define _SVX_USE_UNOGLOBALS_
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/awt/TextAlign.hpp>  //added by BerryJia for fixing Bug102407 2002-11-4
#include <com/sun/star/style/ParagraphAdjust.hpp>   //added by BerryJia for fixing Bug102407 2002-11-4
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <svtools/fltcall.hxx>
#include <svtools/filter.hxx>
#include <boost/scoped_ptr.hpp>
#include <svx/svdpool.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <tools/urlobj.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/unoshape.hxx>
#include <svx/unopage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdouno.hxx>
#include "shapeimpl.hxx"
#include "svx/unoshprp.hxx"
#include <svx/svdoashp.hxx>
#include "unopolyhelper.hxx"
#include <svx/svdview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svdopath.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <vcl/cvtgrf.hxx>
#include <svx/svdograf.hxx>
#ifndef SVX_LIGHT
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>
#endif
#include "toolkit/unohlp.hxx"
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>
#include <svx/globaldrawitempool.hxx>

using ::rtl::OUString;
using namespace ::osl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

#define INTERFACE_TYPE( xint ) \
    ::getCppuType((const Reference< xint >*)0)

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const Reference< xint >*)0) ) \
        aAny <<= Reference< xint >(this)

class GDIMetaFile;
class SvStream;
sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pFilterConfigItem = NULL, sal_Bool bPlaceable = sal_True );

/***********************************************************************
* class SvxShapeGroup                                                  *
***********************************************************************/

SvxShapeGroup::SvxShapeGroup( SdrObject* pObj, SvxDrawPage* pDrawPage  )  throw() :
    SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_GROUP), aSvxMapProvider.GetPropertySet(SVXMAP_GROUP, GetGlobalDrawObjectItemPool()) ),
    mxPage( pDrawPage )
{
}

//----------------------------------------------------------------------
SvxShapeGroup::~SvxShapeGroup() throw()
{
}

//----------------------------------------------------------------------
void SvxShapeGroup::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShape::Create( pNewObj, pNewPage );
    mxPage = pNewPage;
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeGroup::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return SvxShape::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeGroup::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XShapeGroup );
    else QUERYINT( drawing::XShapes );
    else QUERYINT( container::XIndexAccess );
    else QUERYINT( container::XElementAccess );
    else
        return SvxShape::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeGroup::acquire() throw ( )
{
    SvxShape::acquire();
}

void SAL_CALL SvxShapeGroup::release() throw ( )
{
    SvxShape::release();
}

uno::Sequence< uno::Type > SAL_CALL SvxShapeGroup::getTypes()
    throw (uno::RuntimeException)
{
    return SvxShape::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeGroup::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeGroup::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShape::getShapeType();
}

//------------------------------------------------------------------1----
awt::Point SAL_CALL SvxShapeGroup::getPosition() throw(uno::RuntimeException)
{
    return SvxShape::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShape::setPosition(Position);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxShapeGroup::getSize() throw(uno::RuntimeException)
{
    return SvxShape::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShape::setSize( rSize );
}

// drawing::XShapeGroup

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::enterGroup(  ) throw(uno::RuntimeException)
{
    // Todo
//  pDrView->EnterMarkedGroup();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::leaveGroup(  ) throw(uno::RuntimeException)
{
    // Todo
//  pDrView->LeaveOneGroup();
}

//----------------------------------------------------------------------

// XShapes
void SAL_CALL SvxShapeGroup::add( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( mpObj.is()&& mxPage.is() && pShape )
    {
        SdrObject* pSdrShape = pShape->GetSdrObject();
        if( pSdrShape == NULL )
            pSdrShape = mxPage->_CreateSdrObject( xShape );

        if( pSdrShape->IsObjectInserted() )
            pSdrShape->getParentOfSdrObject()->RemoveObjectFromSdrObjList( pSdrShape->GetNavigationPosition() );

        mpObj->getChildrenOfSdrObject()->InsertObjectToSdrObjList(*pSdrShape);

        // #85922# It makes no sense to set the layer asked
        // from the group object since these is an iteration
        // over the contained objects. In consequence, this
        // statement erases all layer information from the draw
        // objects. Layers need to be set at draw objects directly
        // and have nothing to do with grouping at all.
        // pSdrShape->SetLayer(pObject->GetLayer());

        // Establish connection between new SdrObject and its wrapper before
        // inserting the new shape into the group.  There a new wrapper
        // would be created when this connection would not already exist.
        if(pShape)
            pShape->Create( pSdrShape, mxPage.get() );

        if( mpModel )
            mpModel->SetChanged();
    }
    else
    {
        DBG_ERROR("could not add XShape to group shape!");
    }
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::remove( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrObject* pSdrShape = NULL;
    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( pShape )
        pSdrShape = pShape->GetSdrObject();

    if( !mpObj.is() || pSdrShape == NULL || pSdrShape->getParentOfSdrObject()->getSdrObjectFromSdrObjList() != mpObj.get() )
        throw uno::RuntimeException();

    SdrObjList& rList = *pSdrShape->getParentOfSdrObject();

    const sal_uInt32 nObjCount = rList.GetObjCount();
    sal_uInt32 nObjNum = 0;
    while( nObjNum < nObjCount )
    {
        if(rList.GetObj( nObjNum ) == pSdrShape )
            break;
        nObjNum++;
    }

    if( nObjNum < nObjCount )
    {
        // #i29181#
        // If the SdrObject which is about to be deleted is in any selection,
        // deselect it first.
        const ::std::set< SdrView* > aAllSdrViews(pSdrShape->getSdrModelFromSdrObject().getSdrViews());

        for(::std::set< SdrView* >::const_iterator aLoopViews(aAllSdrViews.begin());
            aLoopViews != aAllSdrViews.end(); aLoopViews++)
            {
            (*aLoopViews)->MarkObj(*pSdrShape, true);
        }

        SdrObject* pObject = rList.RemoveObjectFromSdrObjList( nObjNum );
        deleteSdrObjectSafeAndClearPointer( pObject );
    }
    else
    {
        DBG_ASSERT( 0, "Fatality! SdrObject is not belonging to its SdrObjList! [CL]" );
    }

    if( mpModel )
        mpModel->SetChanged();
}

// XIndexAccess

//----------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShapeGroup::getCount() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nRetval = 0;

    if(mpObj.is() && mpObj->getChildrenOfSdrObject())
        nRetval = mpObj->getChildrenOfSdrObject()->GetObjCount();
    else
        throw uno::RuntimeException();

    return nRetval;
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeGroup::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( !mpObj.is() || mpObj->getChildrenOfSdrObject() == NULL )
        throw uno::RuntimeException();

    if( mpObj->getChildrenOfSdrObject()->GetObjCount() <= (sal_uInt32)Index )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pDestObj = mpObj->getChildrenOfSdrObject()->GetObj( Index );

    if(pDestObj == NULL)
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape( pDestObj->getUnoShape(), uno::UNO_QUERY );
    return uno::makeAny( xShape );
}

// ::com::sun::star::container::XElementAccess

//----------------------------------------------------------------------
uno::Type SAL_CALL SvxShapeGroup::getElementType() throw( uno::RuntimeException )
{
    return ::getCppuType(( const Reference< drawing::XShape >*)0);
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxShapeGroup::hasElements() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    return mpObj.is() && mpObj->getChildrenOfSdrObject() && (mpObj->getChildrenOfSdrObject()->GetObjCount() > 0);
}

//----------------------------------------------------------------------
// ::com::sun::star::lang::XServiceInfo

uno::Sequence< OUString > SAL_CALL SvxShapeGroup::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
*                                                                      *
***********************************************************************/

SvxShapeConnector::SvxShapeConnector( SdrObject* pObj )  throw() :
    SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_CONNECTOR), aSvxMapProvider.GetPropertySet(SVXMAP_CONNECTOR, GetGlobalDrawObjectItemPool()) )
{
}

//----------------------------------------------------------------------
SvxShapeConnector::~SvxShapeConnector() throw()
{
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShapeConnector::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeConnector::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XConnectorShape );
    else
        return SvxShapeText::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeConnector::acquire() throw ( )
{
    SvxShapeText::acquire();
}

void SAL_CALL SvxShapeConnector::release() throw ( )
{
    SvxShapeText::release();
}
// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeConnector::getTypes()
    throw (uno::RuntimeException)
{
    return SvxShape::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeConnector::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeConnector::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShapeText::getShapeType();
}

//------------------------------------------------------------------1----
awt::Point SAL_CALL SvxShapeConnector::getPosition() throw(uno::RuntimeException)
{
    return SvxShapeText::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShapeText::setPosition(Position);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxShapeConnector::getSize() throw(uno::RuntimeException)
{
    return SvxShapeText::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShapeText::setSize( rSize );
}

//----------------------------------------------------------------------

// XConnectorShape

void SAL_CALL SvxShapeConnector::connectStart( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType ) throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( mpObj.is() && pShape )
    {
        SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(mpObj.get());
        OSL_ENSURE(pEdge, "OOps, SvxShapeConnector without SdrEdgeObj (!)");
        pEdge->ConnectToNode( sal_True, pShape->mpObj.get() );
    }

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::connectEnd( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType  )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( mpObj.is() && pShape )
    {
        SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(mpObj.get());
        OSL_ENSURE(pEdge, "OOps, SvxShapeConnector without SdrEdgeObj (!)");
        pEdge->ConnectToNode( sal_False, pShape->mpObj.get() );
    }

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::disconnectBegin( const uno::Reference< drawing::XConnectableShape >&  )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is())
    {
        SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(mpObj.get());
        OSL_ENSURE(pEdge, "OOps, SvxShapeConnector without SdrEdgeObj (!)");
        pEdge->DisconnectFromNode( sal_True );
    }

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::disconnectEnd( const uno::Reference< drawing::XConnectableShape >& )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is())
    {
        SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(mpObj.get());
        OSL_ENSURE(pEdge, "OOps, SvxShapeConnector without SdrEdgeObj (!)");
        pEdge->DisconnectFromNode( sal_False );
    }

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
// ::com::sun::star::lang::XServiceInfo
//----------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL SvxShapeConnector::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

/***********************************************************************
* class SvxShapeControl                                                *
***********************************************************************/
SvxShapeControl::SvxShapeControl( SdrObject* pObj )  throw() :
    SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_CONTROL), aSvxMapProvider.GetPropertySet(SVXMAP_CONTROL, GetGlobalDrawObjectItemPool()) )
{
    setSvxShapeKind(SvxShapeKind_Control);
}

//----------------------------------------------------------------------
SvxShapeControl::~SvxShapeControl() throw()
{
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeControl::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XControlShape );
    else
        return SvxShapeText::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeControl::acquire() throw ( )
{
    SvxShapeText::acquire();
}

void SAL_CALL SvxShapeControl::release() throw ( )
{
    SvxShapeText::release();
}
// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeControl::getTypes()
    throw (uno::RuntimeException)
{
    return SvxShape::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeControl::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeControl::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShapeText::getShapeType();
}

//------------------------------------------------------------------1----
awt::Point SAL_CALL SvxShapeControl::getPosition() throw(uno::RuntimeException)
{
    return SvxShapeText::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShapeText::setPosition(Position);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxShapeControl::getSize() throw(uno::RuntimeException)
{
    return SvxShapeText::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShapeText::setSize( rSize );
}

//----------------------------------------------------------------------
// XControlShape

Reference< awt::XControlModel > SAL_CALL SvxShapeControl::getControl()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< awt::XControlModel > xModel;

    SdrUnoObj* pUnoObj = dynamic_cast< SdrUnoObj * >(mpObj.get());
    if( pUnoObj )
        xModel = pUnoObj->GetUnoControlModel();

    return xModel;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setControl( const Reference< awt::XControlModel >& xControl )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrUnoObj* pUnoObj = dynamic_cast< SdrUnoObj * >(mpObj.get());
    if( pUnoObj )
        pUnoObj->SetUnoControlModel( xControl );

    if( mpModel )
        mpModel->SetChanged();
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapeControl::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

static struct
{
    const sal_Char* mpAPIName;
    sal_uInt16 mnAPINameLen;

    const sal_Char* mpFormName;
    sal_uInt16 mnFormNameLen;
}
SvxShapeControlPropertyMapping[] =
{
    // Warning: The first entry must be FontSlant because the any needs to be converted
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE), MAP_CHAR_LEN("FontSlant")  }, //  const sal_Int16 => ::com::sun::star::awt::FontSlant
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME), MAP_CHAR_LEN("FontName") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME), MAP_CHAR_LEN("FontStyleName") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY), MAP_CHAR_LEN("FontFamily") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET), MAP_CHAR_LEN("FontCharset") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT), MAP_CHAR_LEN("FontHeight") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH), MAP_CHAR_LEN("FontPitch" ) },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT), MAP_CHAR_LEN("FontWeight" ) },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_UNDERLINE), MAP_CHAR_LEN("FontUnderline") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_STRIKEOUT), MAP_CHAR_LEN("FontStrikeout") },
    { MAP_CHAR_LEN("CharKerning"), MAP_CHAR_LEN("FontKerning") },
    { MAP_CHAR_LEN("CharWordMode"), MAP_CHAR_LEN("FontWordLineMode" ) },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_COLOR),   MAP_CHAR_LEN("TextColor") },
    { MAP_CHAR_LEN("CharRelief"),   MAP_CHAR_LEN("FontRelief") },
    { MAP_CHAR_LEN("CharUnderlineColor"),   MAP_CHAR_LEN("TextLineColor") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_ADJUST), MAP_CHAR_LEN("Align") },
    { MAP_CHAR_LEN("TextVerticalAdjust"), MAP_CHAR_LEN("VerticalAlign") },
    { MAP_CHAR_LEN("ControlBackground"), MAP_CHAR_LEN("BackgroundColor") },
    { MAP_CHAR_LEN("ControlSymbolColor"), MAP_CHAR_LEN("SymbolColor") },
    { MAP_CHAR_LEN("ControlBorder"), MAP_CHAR_LEN("Border") },
    { MAP_CHAR_LEN("ControlBorderColor"), MAP_CHAR_LEN("BorderColor") },
    { MAP_CHAR_LEN("ControlTextEmphasis"),  MAP_CHAR_LEN("FontEmphasisMark") },
    { MAP_CHAR_LEN("ImageScaleMode"),  MAP_CHAR_LEN("ScaleMode") },
    { MAP_CHAR_LEN("ControlWritingMode"), MAP_CHAR_LEN("WritingMode") },
    //added for exporting OCX control
    { MAP_CHAR_LEN("ControlTypeinMSO"), MAP_CHAR_LEN("ControlTypeinMSO") },
    { MAP_CHAR_LEN("ObjIDinMSO"), MAP_CHAR_LEN("ObjIDinMSO") },
    { NULL,0, NULL, 0 }
};

namespace
{
    static bool lcl_convertPropertyName( const OUString& rApiName, OUString& rInternalName )
    {
        sal_uInt16 i = 0;
        while( SvxShapeControlPropertyMapping[i].mpAPIName )
        {
            if( rApiName.reverseCompareToAsciiL( SvxShapeControlPropertyMapping[i].mpAPIName, SvxShapeControlPropertyMapping[i].mnAPINameLen ) == 0 )
            {
                rInternalName = OUString( SvxShapeControlPropertyMapping[i].mpFormName, SvxShapeControlPropertyMapping[i].mnFormNameLen, RTL_TEXTENCODING_ASCII_US );
            }
            ++i;
        }
        return rInternalName.getLength() > 0;
    }

    struct EnumConversionMap
    {
        sal_Int16   nAPIValue;
        sal_Int16   nFormValue;
    };

    EnumConversionMap aMapAdjustToAlign[] =
    {
        // note that order matters:
        // lcl_convertTextAlignmentToParaAdjustment and lcl_convertParaAdjustmentToTextAlignment search this map from the _beginning_
        // and use the first matching entry
        {style::ParagraphAdjust_LEFT,           (sal_Int16)awt::TextAlign::LEFT},
        {style::ParagraphAdjust_CENTER,         (sal_Int16)awt::TextAlign::CENTER},
        {style::ParagraphAdjust_RIGHT,          (sal_Int16)awt::TextAlign::RIGHT},
        {style::ParagraphAdjust_BLOCK,          (sal_Int16)awt::TextAlign::RIGHT},
        {style::ParagraphAdjust_STRETCH,        (sal_Int16)awt::TextAlign::LEFT},
        {-1,-1}
    };

    static void lcl_mapFormToAPIValue( Any& _rValue, const EnumConversionMap* _pMap )
    {
        sal_Int16 nValue = sal_Int16();
        OSL_VERIFY( _rValue >>= nValue );

        const EnumConversionMap* pEntry = _pMap;
        while ( pEntry && ( pEntry->nFormValue != -1 ) )
        {
            if ( nValue == pEntry->nFormValue )
            {
                _rValue <<= pEntry->nAPIValue;
                return;
            }
            ++pEntry;
        }
    }

    static void lcl_mapAPIToFormValue( Any& _rValue, const EnumConversionMap* _pMap )
    {
        sal_Int32 nValue = 0;
        OSL_VERIFY( _rValue >>= nValue );

        const EnumConversionMap* pEntry = _pMap;
        while ( pEntry && ( pEntry->nAPIValue != -1 ) )
        {
            if ( nValue == pEntry->nAPIValue )
            {
                _rValue <<= pEntry->nFormValue;
                return;
            }
            ++pEntry;
        }
    }

    static void lcl_convertTextAlignmentToParaAdjustment( Any& rValue )
    {
        lcl_mapFormToAPIValue( rValue, aMapAdjustToAlign );
    }

    static void lcl_convertParaAdjustmentToTextAlignment( Any& rValue )
    {
        lcl_mapAPIToFormValue( rValue, aMapAdjustToAlign );
    }

    void convertVerticalAdjustToVerticalAlign( Any& _rValue ) SAL_THROW( ( lang::IllegalArgumentException ) )
    {
        if ( !_rValue.hasValue() )
            return;

        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        style::VerticalAlignment    eAlign  = style::VerticalAlignment_TOP;
        if ( !( _rValue >>= eAdjust ) )
            throw lang::IllegalArgumentException();
        switch ( eAdjust )
        {
        case drawing::TextVerticalAdjust_TOP:    eAlign = style::VerticalAlignment_TOP; break;
        case drawing::TextVerticalAdjust_BOTTOM: eAlign = style::VerticalAlignment_BOTTOM; break;
        default:                                 eAlign = style::VerticalAlignment_MIDDLE; break;
        }
        _rValue <<= eAlign;
    }

    void convertVerticalAlignToVerticalAdjust( Any& _rValue )
    {
        if ( !_rValue.hasValue() )
            return;
        style::VerticalAlignment    eAlign  = style::VerticalAlignment_TOP;
        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        OSL_VERIFY( _rValue >>= eAlign );
        switch ( eAlign )
        {
        case style::VerticalAlignment_TOP:    eAdjust = drawing::TextVerticalAdjust_TOP; break;
        case style::VerticalAlignment_BOTTOM: eAdjust = drawing::TextVerticalAdjust_BOTTOM; break;
        default:                              eAdjust = drawing::TextVerticalAdjust_CENTER; break;
        }
        _rValue <<= eAdjust;
    }
}

void SAL_CALL SvxShapeControl::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException)
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( aPropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertySet > xControl( getControl(), uno::UNO_QUERY );
        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xControl->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                uno::Any aConvertedValue( aValue );
                if ( aFormsName.equalsAscii( "FontSlant" ) )
                {
                    awt::FontSlant nSlant;
                    if( !(aValue >>= nSlant ) )
                        throw lang::IllegalArgumentException();
                    aConvertedValue <<= (sal_Int16)nSlant;
                }
                else if ( aFormsName.equalsAscii( "Align" ) )
                {
                    lcl_convertParaAdjustmentToTextAlignment( aConvertedValue );
                }
                else if ( aFormsName.equalsAscii( "VerticalAlign" ) )
                {
                    convertVerticalAdjustToVerticalAlign( aConvertedValue );
                }

                xControl->setPropertyValue( aFormsName, aConvertedValue );
            }
        }
    }
    else
    {
        SvxShape::setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL SvxShapeControl::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( aPropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertySet > xControl( getControl(), uno::UNO_QUERY );

        uno::Any aValue;
        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xControl->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                aValue = xControl->getPropertyValue( aFormsName );
                if ( aFormsName.equalsAscii( "FontSlant" ) )
                {
                    awt::FontSlant eSlant = awt::FontSlant_NONE;
                    sal_Int16 nSlant = sal_Int16();
                    if ( aValue >>= nSlant )
                    {
                        eSlant = (awt::FontSlant)nSlant;
                    }
                    else
                    {
                        OSL_VERIFY( aValue >>= eSlant );
                    }
                    aValue <<= eSlant;
                }
                else if ( aFormsName.equalsAscii( "Align" ) )
                {
                    lcl_convertTextAlignmentToParaAdjustment( aValue );
                }
                else if ( aFormsName.equalsAscii( "VerticalAlign" ) )
                {
                    convertVerticalAlignToVerticalAdjust( aValue );
                }
            }
        }

        return aValue;
    }
    else
    {
        return SvxShape::getPropertyValue( aPropertyName );
    }

}

// XPropertyState
beans::PropertyState SAL_CALL SvxShapeControl::getPropertyState( const ::rtl::OUString& PropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( PropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xPropSet( getControl(), uno::UNO_QUERY );

        if( xControl.is() && xPropSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                return xControl->getPropertyState( aFormsName );
            }
        }

        return beans::PropertyState_DEFAULT_VALUE;
    }
    else
    {
        return SvxShape::getPropertyState( PropertyName );
    }
}

void SAL_CALL SvxShapeControl::setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( PropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xPropSet( getControl(), uno::UNO_QUERY );

        if( xControl.is() && xPropSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                xControl->setPropertyToDefault( aFormsName );
            }
        }
    }
    else
    {
        SvxShape::setPropertyToDefault( PropertyName );
    }
}

uno::Any SAL_CALL SvxShapeControl::getPropertyDefault( const ::rtl::OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OUString aFormsName;
    if ( lcl_convertPropertyName( aPropertyName, aFormsName ) )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );

        if( xControl.is() )
        {
            Any aDefault( xControl->getPropertyDefault( aFormsName ) );
            if ( aFormsName.equalsAscii( "FontSlant" ) )
            {
                sal_Int16 nSlant( 0 );
                aDefault >>= nSlant;
                aDefault <<= (awt::FontSlant)nSlant;
            }
            else if ( aFormsName.equalsAscii( "Align" ) )
            {
                lcl_convertTextAlignmentToParaAdjustment( aDefault );
            }
            else if ( aFormsName.equalsAscii( "VerticalAlign" ) )
            {
                convertVerticalAlignToVerticalAdjust( aDefault );
            }
            return aDefault;
        }

        throw beans::UnknownPropertyException();
    }
    else
    {
        return SvxShape::getPropertyDefault( aPropertyName );
    }
}


/***********************************************************************
* class SvxShapeDimensioning                                           *
***********************************************************************/

//----------------------------------------------------------------------
SvxShapeDimensioning::SvxShapeDimensioning( SdrObject* pObj ) throw()
:   SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_DIMENSIONING), aSvxMapProvider.GetPropertySet(SVXMAP_DIMENSIONING, GetGlobalDrawObjectItemPool()) )
{
}

//----------------------------------------------------------------------
SvxShapeDimensioning::~SvxShapeDimensioning() throw()
{
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapeDimensioning::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
SvxShapeCircle::SvxShapeCircle( SdrObject* pObj ) throw()
:   SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_CIRCLE), aSvxMapProvider.GetPropertySet(SVXMAP_CIRCLE, GetGlobalDrawObjectItemPool()) )
{
}

//----------------------------------------------------------------------
SvxShapeCircle::~SvxShapeCircle() throw()
{
}

// ::com::sun::star::lang::XServiceInfo
// XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapeCircle::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
bool SvxShapePolyPolygon::isBezierBased() const
{
    if(mpObj.is())
    {
        const SdrPathObjType aSdrPathObjType(((SdrPathObj*)mpObj.get())->getSdrPathObjType());

        if(PathType_OpenBezier == aSdrPathObjType || PathType_ClosedBezier == aSdrPathObjType)
        {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------
SvxShapePolyPolygon::SvxShapePolyPolygon( SdrObject* pObj )
 throw( com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException)
: SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_POLYPOLYGON), aSvxMapProvider.GetPropertySet(SVXMAP_POLYPOLYGON, GetGlobalDrawObjectItemPool()) )
{
}

//----------------------------------------------------------------------
SvxShapePolyPolygon::~SvxShapePolyPolygon() throw()
{
}

//----------------------------------------------------------------------
// depends on polygon type, need to solve this dynamically

const SfxItemPropertyMapEntry* SvxShapePolyPolygon::getPropertyMapEntries() const
{
    // the PropertyMap of this object depends on it's content and thus can change during it's
    // lifetime. It depends on if it's using beziers or not. Thus, return the corresponding type
    // dynamically
    if(isBezierBased())
    {
        // return bezier-based PropertyMap
        return aSvxMapProvider.GetMap(SVXMAP_POLYPOLYGONBEZIER);
    }
    else
    {
        // call parent, use SVXMAP_POLYPOLYGON as used in the constructor
        return SvxShapeText::getPropertyMapEntries();
    }
}

//----------------------------------------------------------------------

bool SvxShapePolyPolygon::setPropertyValueImpl( const ::rtl::OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // the type of xShape depends on the content of this object, thus allow all
    // possible geometry set methods; e.g. someone may construct a bezier-based
    // xShape, but as long as no data is set, the type would be non-bezier and the
    // interface suppotred would be PolyPolygonDescriptor. To not conflict and to
    // allow setting bezier-based data at that newly created object, allow all
    // data types from PolyPolygonDescriptor and PolyPolygonBezierDescriptor.
    // Especially 'Geometry' is double in these definitions, thus decide based
    // on the data type, but allow both possible ones
    switch( pProperty->nWID )
    {
        case OWN_ATTR_VALUE_POLYPOLYGONBEZIER:
        {
            if( rValue.getValue() && (rValue.getValueType() == ::getCppuType(( const drawing::PolyPolygonBezierCoords*)0) ) )
            {
                if( mpObj.is() )
                {
                    // get polygpon data
                    basegfx::B2DPolyPolygon aNewPolyPolygon(
                        basegfx::tools::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
                            *(const drawing::PolyPolygonBezierCoords*)rValue.getValue()));

                    if(aNewPolyPolygon.count())
                    {
                        // migrate to pool metric
                        ForceMetricToItemPoolMetric(aNewPolyPolygon);

                        // position relative to anchor
                        if(isWriterAnchorUsed())
                        {
                            aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(mpObj->GetAnchorPos()));
                        }
                    }

                    // set at object
                    SetPolygon(aNewPolyPolygon);
                }
                return true;
            }
            break;
        }
        case OWN_ATTR_VALUE_POLYPOLYGON:
        {
            if( rValue.getValue() && (rValue.getValueType() == ::getCppuType(( const drawing::PointSequenceSequence*)0) ) )
            {
                if( mpObj.is() )
                {
                    // get polygpon data
                    basegfx::B2DPolyPolygon aNewPolyPolygon(
                        basegfx::tools::UnoPointSequenceSequenceToB2DPolyPolygon(
                            *(const drawing::PointSequenceSequence*)rValue.getValue()));

                    if(aNewPolyPolygon.count())
                    {
                        // migrate to pool metric
                        ForceMetricToItemPoolMetric(aNewPolyPolygon);

                        // position relative to anchor
                        if(isWriterAnchorUsed())
                        {
                            aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(mpObj->GetAnchorPos()));
                        }
                    }

                    // set at object
                    SetPolygon(aNewPolyPolygon);
                }
                return true;
            }
            break;
        }
        case OWN_ATTR_BASE_GEOMETRY:
        {
            if( rValue.getValue() )
            {
                if( rValue.getValueType() == ::getCppuType(( const drawing::PointSequenceSequence*)0))
                {
                    if( mpObj.is() )
                    {
                        // get polygpon data
                        basegfx::B2DPolyPolygon aNewPolyPolygon(
                            basegfx::tools::UnoPointSequenceSequenceToB2DPolyPolygon(
                                *(const drawing::PointSequenceSequence*)rValue.getValue()));

                        if(aNewPolyPolygon.count())
                        {
                            // migrate to pool metric
                            ForceMetricToItemPoolMetric(aNewPolyPolygon);

                            // BaseGeometry means the polygon is just scaled, but has no position, mirroring, shear
                            // or rotation. Apply these current values from the object
                            const basegfx::B2DHomMatrix aNoScaleTrans(
                                basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                    basegfx::B2DVector(mpObj->isMirroredX() ? -1.0 : 1.0, mpObj->isMirroredY() ? -1.0 : 1.0),
                                    mpObj->getSdrObjectShearX(),
                                    mpObj->getSdrObjectRotate(),
                                    mpObj->getSdrObjectTranslate()));

                            aNewPolyPolygon.transform(aNoScaleTrans);

                            // position relative to anchor
                            if(isWriterAnchorUsed())
                            {
                                aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(mpObj->GetAnchorPos()));
                            }
                        }

                        // set at object
                        SetPolygon(aNewPolyPolygon);
                    }
                    return true;
                }
                else if( rValue.getValueType() == ::getCppuType(( const drawing::PolyPolygonBezierCoords*)0))
                {
                    if( mpObj.is() )
                    {
                        // get polygpon data
                        basegfx::B2DPolyPolygon aNewPolyPolygon(
                            basegfx::tools::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
                                *(const drawing::PolyPolygonBezierCoords*)rValue.getValue()));

                        if(aNewPolyPolygon.count())
                        {
                            // migrate to pool metric
                            ForceMetricToItemPoolMetric(aNewPolyPolygon);

                            // BaseGeometry means the polygon is just scaled, but has no position, shear
                            // or rotation. Apply these current values from the object
                            const basegfx::B2DHomMatrix aNoScaleTrans(
                                basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
                                    mpObj->getSdrObjectShearX(),
                                    mpObj->getSdrObjectRotate(),
                                    mpObj->getSdrObjectTranslate()));

                            aNewPolyPolygon.transform(aNoScaleTrans);

                            // position relative to anchor
                            if(isWriterAnchorUsed())
                            {
                                aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(mpObj->GetAnchorPos()));
                            }
                        }

                        // set at object
                        SetPolygon(aNewPolyPolygon);
                    }
                    return true;
                }
            }
            break;
        }
        case OWN_ATTR_VALUE_POLYGON:
        {
            if( rValue.getValue() && (rValue.getValueType() == ::getCppuType(( const drawing::PointSequence*)0) ))
            {
                if( mpObj.is() )
                {
                    // get polygpon data
                    basegfx::B2DPolyPolygon aNewPolyPolygon(
                        basegfx::tools::UnoPointSequenceToB2DPolygon(
                            *(const drawing::PointSequence*)rValue.getValue()));

                    if(aNewPolyPolygon.count())
                    {
                        // migrate to pool metric
                        ForceMetricToItemPoolMetric(aNewPolyPolygon);

                        // position relative to anchor
                        if(isWriterAnchorUsed())
                        {
                            aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(mpObj->GetAnchorPos()));
                        }
                    }

                    // set at object
                    SetPolygon(aNewPolyPolygon);
                }
                return true;
            }
            break;
        }
        default:
        {
            return SvxShapeText::setPropertyValueImpl( rName, pProperty, rValue );
        }
    }

    throw lang::IllegalArgumentException();
}

//----------------------------------------------------------------------

bool SvxShapePolyPolygon::getPropertyValueImpl( const ::rtl::OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // the data reader should be based on getting the data type first, thus be more strict
    // with what is allowed or not based on being a bezier type or not
    switch( pProperty->nWID )
    {
        case OWN_ATTR_VALUE_POLYPOLYGONBEZIER:
        {
            if(isBezierBased())
            {
                drawing::PolyPolygonBezierCoords aRetval;
                basegfx::B2DPolyPolygon aPolyPolygon(GetPolygon());

                if(aPolyPolygon.count())
                {
                    // make pos relative to anchor
                    if(isWriterAnchorUsed())
                    {
                        aPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(-mpObj->GetAnchorPos()));
                    }

                    // migrtate to 1/100th mm
                    ForceMetricTo100th_mm(aPolyPolygon);

                    // convert Polygon to needed data representation
                    basegfx::tools::B2DPolyPolygonToUnoPolyPolygonBezierCoords(aPolyPolygon, aRetval);
                }

                rValue <<= aRetval;
            }
            else
            {
                // not allowed to get a PolyPolygon with bezier when bezier is not used
                throw lang::IllegalArgumentException();
            }
            break;
        }
        case OWN_ATTR_VALUE_POLYPOLYGON:
        {
            if(isBezierBased())
            {
                // not allowed to get a PolyPolygon when bezier is used
                throw lang::IllegalArgumentException();
            }
            else
            {
                drawing::PointSequenceSequence aRetval;
                basegfx::B2DPolyPolygon aPolyPolygon(GetPolygon());

                if(aPolyPolygon.count())
                {
                    // make pos relative to anchor
                    if(isWriterAnchorUsed())
                    {
                        aPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(-mpObj->GetAnchorPos()));
                    }

                    // migrtate to 1/100th mm
                    ForceMetricTo100th_mm(aPolyPolygon);

                    // convert Polygon to needed data representation
                    basegfx::tools::B2DPolyPolygonToUnoPointSequenceSequence(aPolyPolygon, aRetval);
                }

                rValue <<= aRetval;
            }
            break;
        }
        case OWN_ATTR_BASE_GEOMETRY:
        {
            basegfx::B2DPolyPolygon aPolyPolygon(GetPolygon());

            if(aPolyPolygon.count())
            {
                // make pos relative to anchor
                if(isWriterAnchorUsed())
                {
                    aPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(-mpObj->GetAnchorPos()));
                }

                // migrtate to 1/100th mm
                ForceMetricTo100th_mm(aPolyPolygon);

                // BaseGeometry means to get only the scaled and unmirrored polygon, so transform
                // the polygon to only contain object scale
                // get object transform
                basegfx::B2DHomMatrix aOnlyScaleTransform(mpObj->getSdrObjectTransformation());

                // extract the scale
                const basegfx::B2DVector aScale(basegfx::absolute(mpObj->getSdrObjectScale()));

                // get transformation to unit coordinates
                aOnlyScaleTransform.invert();

                // add scale again
                aOnlyScaleTransform.scale(aScale);

                // transform the polygon
                aPolyPolygon.transform(aOnlyScaleTransform);

                if(aPolyPolygon.areControlPointsUsed())
                {
                    // convert Polygon to needed data representation
                    drawing::PolyPolygonBezierCoords aRetval;

                    basegfx::tools::B2DPolyPolygonToUnoPolyPolygonBezierCoords(aPolyPolygon, aRetval);
                    rValue <<= aRetval;
                }
                else
                {
                    // convert Polygon to needed data representation
                    drawing::PointSequenceSequence aRetval;

                    basegfx::tools::B2DPolyPolygonToUnoPointSequenceSequence(aPolyPolygon, aRetval);
                    rValue <<= aRetval;
                }
            }
            else
            {
                // empty PolyPolygon
                drawing::PolyPolygonBezierCoords aRetval;

                rValue <<= aRetval;
            }

            break;
        }
        case OWN_ATTR_VALUE_POLYGON:
        {
            if(isBezierBased())
            {
                // not allowed to get a PolyPolygon when bezier is used
                throw lang::IllegalArgumentException();
            }
            else
            {
                drawing::PointSequence aRetval;
                basegfx::B2DPolyPolygon aPolyPolygon(GetPolygon());

                if(aPolyPolygon.count())
                {
                    // make pos relative to anchor
                    if(isWriterAnchorUsed())
                    {
                        aPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(-mpObj->GetAnchorPos()));
                    }

                    // migrtate to 1/100th mm
                    ForceMetricTo100th_mm(aPolyPolygon);

                    // convert Polygon to needed data representation
                    basegfx::tools::B2DPolygonToUnoPointSequence(aPolyPolygon.getB2DPolygon(0), aRetval);
                }

                rValue <<= aRetval;
            }
            break;
        }
        case OWN_ATTR_VALUE_POLYGONKIND:
        {
            rValue <<= GetPolygonKind();
            break;
        }
        default:
        {
            return SvxShapeText::getPropertyValueImpl( rName, pProperty, rValue );
        }
    }

    return true;
}

//----------------------------------------------------------------------
drawing::PolygonKind SvxShapePolyPolygon::GetPolygonKind() const throw()
{
    OGuard aGuard( Application::GetSolarMutex() );
    drawing::PolygonKind aRetval(drawing::PolygonKind_LINE);

    if(mpObj.is())
    {
        const SdrPathObjType aSdrPathObjType(((SdrPathObj*)mpObj.get())->getSdrPathObjType());

        switch(aSdrPathObjType)
        {
            case PathType_Line:
            {
                aRetval = drawing::PolygonKind_LINE;
                break;
            }
            case PathType_OpenPolygon:
            {
                aRetval = drawing::PolygonKind_PLIN;
                break;
            }
            case PathType_ClosedPolygon:
            {
                aRetval = drawing::PolygonKind_POLY;
                break;
            }
            case PathType_OpenBezier:
            {
                aRetval = drawing::PolygonKind_PATHLINE;
                break;
            }
            case PathType_ClosedBezier:
            {
                aRetval = drawing::PolygonKind_PATHFILL;
                break;
            }
        }
    }

    return aRetval;
}

//----------------------------------------------------------------------
void SvxShapePolyPolygon::SetPolygon(const basegfx::B2DPolyPolygon& rNew) throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is())
        ((SdrPathObj*)mpObj.get())->setB2DPolyPolygonInObjectCoordinates(rNew);
}

//----------------------------------------------------------------------
basegfx::B2DPolyPolygon SvxShapePolyPolygon::GetPolygon() const throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is())
    {
        return ((SdrPathObj*)mpObj.get())->getB2DPolyPolygonInObjectCoordinates();
    }
    else
    {
        return basegfx::B2DPolyPolygon();
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapePolyPolygon::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

/***********************************************************************
* class SvxGraphicObject                                               *
***********************************************************************/

//----------------------------------------------------------------------
SvxGraphicObject::SvxGraphicObject( SdrObject* pObj ) throw()
:   SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_GRAPHICOBJECT), aSvxMapProvider.GetPropertySet(SVXMAP_GRAPHICOBJECT, GetGlobalDrawObjectItemPool()) )
{
}

//----------------------------------------------------------------------
SvxGraphicObject::~SvxGraphicObject() throw()
{
}

//----------------------------------------------------------------------

bool SvxGraphicObject::setPropertyValueImpl( const ::rtl::OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    bool bOk = false;
    switch( pProperty->nWID )
    {
        case OWN_ATTR_VALUE_FILLBITMAP:
        {
            if( rValue.getValue() )
            {
                if( rValue.getValueType() == ::getCppuType(( const uno::Sequence< sal_Int8 >*)0) )
                {
                    uno::Sequence<sal_Int8>* pSeq( (uno::Sequence<sal_Int8>*)rValue.getValue() );
                    SvMemoryStream  aMemStm;
                    Graphic         aGraphic;

                    aMemStm.SetBuffer( (char*)pSeq->getConstArray(), pSeq->getLength(), sal_False, pSeq->getLength() );

                    if( GraphicConverter::Import( aMemStm, aGraphic ) == ERRCODE_NONE )
                    {
                        static_cast<SdrGrafObj*>(mpObj.get())->SetGraphic(aGraphic);
                        bOk = true;
                    }
                }
            }
            else if( (rValue.getValueType() == awt::XBitmap::static_type()) || (rValue.getValueType() == graphic::XGraphic::static_type()))
            {
                Reference< graphic::XGraphic> xGraphic( rValue, UNO_QUERY );
                if( xGraphic.is() )
                {
                    ((SdrGrafObj*)mpObj.get())->SetGraphic(Graphic(xGraphic));
                    bOk = true;
                }
                else
                {
                    // Bitmap in das Objekt packen
                    Reference< awt::XBitmap > xBmp( rValue, UNO_QUERY );
                    if( xBmp.is() )
                    {
                        // Bitmap einsetzen
                        Graphic aGraphic(VCLUnoHelper::GetBitmap( xBmp ));
                        ((SdrGrafObj*)mpObj.get())->SetGraphic(aGraphic);
                        bOk = true;
                    }
                }
            }
            break;
        }
        case OWN_ATTR_GRAFURL:
        {
            OUString aURL;
            if( rValue >>= aURL )
            {
                if( aURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) == 0 )
                {
                    // graphic manager url
                    aURL = aURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 );
                    String aTmpStr(aURL);
                    ByteString aUniqueID( aTmpStr, RTL_TEXTENCODING_UTF8 );
                    GraphicObject aGrafObj( aUniqueID );

                    // #101808# since loading a graphic can cause a reschedule of the office
                    //          it is possible that our shape is removed while where in this
                    //          method.
                    if( mpObj.is() )
                    {
                        static_cast<SdrGrafObj*>(mpObj.get())->ReleaseGraphicLink();
                        static_cast<SdrGrafObj*>(mpObj.get())->SetGraphicObject( aGrafObj );
                    }
                }
                else if( aURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) ) != 0 )
                {
                    // normal link
                    String              aFilterName;
                    const SfxFilter*    pSfxFilter = NULL;
                    SfxMedium           aSfxMedium( aURL, STREAM_READ | STREAM_SHARE_DENYNONE, sal_False );

                    SFX_APP()->GetFilterMatcher().GuessFilter( aSfxMedium, &pSfxFilter, SFX_FILTER_IMPORT, SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );

                    if( !pSfxFilter )
                    {
                        INetURLObject aURLObj( aURL );

                        if( aURLObj.GetProtocol() == INET_PROT_NOT_VALID )
                        {
                            String aValidURL;

                            if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aURL, aValidURL ) )
                                aURLObj = INetURLObject( aValidURL );
                        }

                        if( aURLObj.GetProtocol() != INET_PROT_NOT_VALID )
                        {
                            GraphicFilter* pGrfFilter = GraphicFilter::GetGraphicFilter();
                            aFilterName = pGrfFilter->GetImportFormatName( pGrfFilter->GetImportFormatNumberForShortName( aURLObj.getExtension() ) );
                        }
                    }
                    else
                        aFilterName = pSfxFilter->GetFilterName();

                    // #101808# since loading a graphic can cause a reschedule of the office
                    //          it is possible that our shape is removed while where in this
                    //          method.
                    if( mpObj.is() )
                        static_cast<SdrGrafObj*>(mpObj.get())->SetGraphicLink( aURL, aFilterName );

                }
                bOk = true;
            }
            break;
        }

        case OWN_ATTR_GRAFSTREAMURL:
        {
            OUString aStreamURL;

            if( rValue >>= aStreamURL )
            {
                if( aStreamURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) ) != 0 )
                    aStreamURL = OUString();

                if( mpObj.is() )
                {
                    static_cast<SdrGrafObj*>(mpObj.get())->SetGrafStreamURL( aStreamURL );
                    static_cast<SdrGrafObj*>(mpObj.get())->ForceSwapOut();
                }
                bOk = true;
            }
            break;
        }

        case OWN_ATTR_VALUE_GRAPHIC:
        {
            Reference< graphic::XGraphic > xGraphic( rValue, uno::UNO_QUERY );
            if( xGraphic.is() )
            {
                static_cast< SdrGrafObj*>( mpObj.get() )->SetGraphic( xGraphic );
                bOk = true;
            }
            break;
        }
        default:
        {
            return SvxShapeText::setPropertyValueImpl( rName, pProperty, rValue );
        }
    }

    if( !bOk )
        throw lang::IllegalArgumentException();

    if( mpModel )
        mpModel->SetChanged();

    return true;
}

//----------------------------------------------------------------------

bool SvxGraphicObject::getPropertyValueImpl( const ::rtl::OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
        case OWN_ATTR_VALUE_FILLBITMAP:
        {
            sal_Bool bSwapped = static_cast< SdrGrafObj* >( mpObj.get() )->IsSwappedOut();
            const Graphic& rGraphic = static_cast< SdrGrafObj*>( mpObj.get() )->GetGraphic();

            if(rGraphic.GetType() != GRAPHIC_GDIMETAFILE)
            {
                // Objekt in eine Bitmap packen
                Reference< ::com::sun::star::awt::XBitmap >  xBitmap( VCLUnoHelper::CreateBitmap(static_cast< SdrGrafObj*>( mpObj.get() )->GetGraphic().GetBitmapEx()) );
                rValue <<= xBitmap;
            }
            else
            {
                SvMemoryStream aDestStrm( 65535, 65535 );

                ConvertGDIMetaFileToWMF( rGraphic.GetGDIMetaFile(), aDestStrm, NULL, sal_False );
                const uno::Sequence<sal_Int8> aSeq(
                    static_cast< const sal_Int8* >(aDestStrm.GetData()),
                    aDestStrm.GetEndOfData());
                rValue <<= aSeq;
            }
            if ( bSwapped )
                static_cast< SdrGrafObj* >( mpObj.get() )->ForceSwapOut();
            break;
        }

        case OWN_ATTR_GRAFURL:
        {
            if( static_cast< SdrGrafObj*>( mpObj.get() )->IsLinkedGraphic() )
            {
                rValue <<= OUString( static_cast< SdrGrafObj*>( mpObj.get() )->GetFileName() );
            }
            else
            {
                sal_Bool bSwapped = static_cast< SdrGrafObj* >( mpObj.get() )->IsSwappedOut();
                const GraphicObject& rGrafObj = static_cast< SdrGrafObj*>( mpObj.get() )->GetGraphicObject(true);
                OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
                aURL += OUString::createFromAscii( rGrafObj.GetUniqueID().GetBuffer() );
                rValue <<= aURL;
                if ( bSwapped )
                    static_cast< SdrGrafObj* >( mpObj.get() )->ForceSwapOut();
            }
            break;
        }

        case OWN_ATTR_REPLACEMENTGRAFURL:
        {
            const GraphicObject* pGrafObj = static_cast< SdrGrafObj* >(mpObj.get())->GetReplacementGraphicObject();

            if(pGrafObj)
            {
                OUString aURL(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
                aURL += OUString::createFromAscii(pGrafObj->GetUniqueID().GetBuffer());
                rValue <<= aURL;
            }

            break;
        }

        case OWN_ATTR_GRAFSTREAMURL:
        {
            const OUString  aStreamURL( ( (SdrGrafObj*) mpObj.get() )->GetGrafStreamURL() );
            if( aStreamURL.getLength() )
                rValue <<= aStreamURL;
            break;
        }

        case OWN_ATTR_VALUE_GRAPHIC:
        {
            sal_Bool bSwapped = static_cast< SdrGrafObj* >( mpObj.get() )->IsSwappedOut();
            Reference< graphic::XGraphic > xGraphic( static_cast< SdrGrafObj* >( mpObj.get() )->GetGraphic().GetXGraphic() );
            rValue <<= xGraphic;
            if ( bSwapped )
                static_cast< SdrGrafObj* >( mpObj.get() )->ForceSwapOut();
            break;
        }

        case OWN_ATTR_GRAPHIC_STREAM:
        {
            rValue <<= static_cast< SdrGrafObj* >( mpObj.get() )->getInputStream();
            break;
        }
        default:
        {
            return SvxShapeText::getPropertyValueImpl(rName, pProperty,rValue);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////

SvxShapeCaption::SvxShapeCaption( SdrObject* pObj ) throw()
: SvxShapeText( pObj, aSvxMapProvider.GetMap(SVXMAP_CAPTION), aSvxMapProvider.GetPropertySet(SVXMAP_CAPTION, GetGlobalDrawObjectItemPool()) )
{
}

SvxShapeCaption::~SvxShapeCaption() throw()
{
}

/***********************************************************************
* class SvxCustomShape                                                   *
***********************************************************************/

SvxCustomShape::SvxCustomShape( SdrObject* pObj )  throw() :
    SvxShapeText( pObj, aSvxMapProvider.GetMap( SVXMAP_CUSTOMSHAPE ), aSvxMapProvider.GetPropertySet(SVXMAP_CUSTOMSHAPE, GetGlobalDrawObjectItemPool()) )
{
}

//----------------------------------------------------------------------
SvxCustomShape::~SvxCustomShape() throw()
{
}

//----------------------------------------------------------------------

void SvxCustomShape::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShapeText::Create( pNewObj, pNewPage );
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxCustomShape::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxCustomShape::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = SvxShapeText::queryAggregation( rType );
    if ( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface(rType, static_cast<drawing::XEnhancedCustomShapeDefaulter*>(this) );
    return aReturn;
}

void SAL_CALL SvxCustomShape::acquire() throw ( )
{
    SvxShapeText::acquire();
}

void SAL_CALL SvxCustomShape::release() throw ( )
{
    SvxShapeText::release();
}

//----------------------------------------------------------------------

uno::Sequence< uno::Type > SAL_CALL SvxCustomShape::getTypes()
    throw (uno::RuntimeException)
{
    return SvxShapeText::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxCustomShape::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxCustomShape::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShape::getShapeType();
}

//------------------------------------------------------------------1----

// TTTT: Not needed
//Polygon Rect2Poly(const Rectangle& rRect, long aOldRotation, long aOldShear) // TTTT needed?
//{
//  Polygon aPol(5);
//  aPol[0]=rRect.TopLeft();
//  aPol[1]=rRect.TopRight();
//  aPol[2]=rRect.BottomRight();
//  aPol[3]=rRect.BottomLeft();
//  aPol[4]=rRect.TopLeft();
//
//    if (aOldShear)
//        ShearPoly(aPol,rRect.TopLeft(),tan(aOldShear*nPi180));
//
//    if (aOldRotation)
//        RotatePoly(aPol,rRect.TopLeft(),sin(aOldRotation*nPi180), cos(aOldRotation*nPi180));
//
//  return aPol;
//}
//
//void Poly2Rect(const Polygon& rPol, Rectangle& rRect, long& rRotation, long& rShear) // TTTT needed?
//{
//  rRotation=GetAngle(rPol[1]-rPol[0]);
//  rShear=NormAngle360(rRotation);
//  // Drehung ist damit im Kasten
//
//  Point aPt1(rPol[1]-rPol[0]);
//  if (rRotation) RotatePoint(aPt1,Point(0,0),-sin(rRotation*nPi180), cos(rRotation*nPi180)); // -Sin fuer Rueckdrehung
//  long nWdt=aPt1.X();
//
//  Point aPt0(rPol[0]);
//  Point aPt3(rPol[3]-rPol[0]);
//  if (rRotation) RotatePoint(aPt3,Point(0,0),-sin(rRotation*nPi180), cos(rRotation*nPi180)); // -Sin fuer Rueckdrehung
//  long nHgt=aPt3.Y();
//
//  if(aPt3.X())
//  {
//      // #i74358# the axes are not orthogonal, so for getting the correct height,
//      // calculate the length of aPt3
//
//      // #i74358# this change was wrong, in the field of the old geometry stuff
//      // it is not an error. The new height always is the same as before; shear
//      // does not change object height at all. This is different from the interactions,
//      // but obviously wanted in the old versions.
//      //
//      // nHgt = static_cast< long >(sqrt(static_cast< double >(aPt3.X() * aPt3.X() + aPt3.Y() * aPt3.Y())));
//  }
//
//    long nShW=GetAngle(aPt3);
//  nShW-=27000; // ShearWink wird zur Senkrechten gemessen
//  nShW=-nShW;  // Negieren, denn '+' ist Rechtskursivierung
//
//  bool bMirr=aPt3.Y()<0;
//  if (bMirr) { // "Punktetausch" bei Spiegelung
//      nHgt=-nHgt;
//      nShW+=18000;
//      aPt0=rPol[3];
//  }
//  nShW=NormAngle180(nShW);
//  if (nShW<-9000 || nShW>9000) {
//      nShW=NormAngle180(nShW+18000);
//  }
//  const long SDRMAXSHEAR(8900);
//  if (nShW<-SDRMAXSHEAR) nShW=-SDRMAXSHEAR; // ShearWinkel begrenzen auf +/- 89.00 deg
//  if (nShW>SDRMAXSHEAR)  nShW=SDRMAXSHEAR;
//  rShear=nShW;
//  Point aRU(aPt0);
//  aRU.X()+=nWdt;
//  aRU.Y()+=nHgt;
//  rRect=Rectangle(aPt0,aRU);
//}

awt::Point SAL_CALL SvxCustomShape::getPosition() throw(uno::RuntimeException)
{
    // TTTT: All exceptions because of mirroring should be obsolete
    return SvxShape::getPosition();

    //OGuard aGuard( Application::GetSolarMutex() );
    //if ( mpModel && mpObj.is() )
    //{
    //  sal_Bool bMirroredX = sal_False;
    //  sal_Bool bMirroredY = sal_False;
    //
    //  if ( mpObj.is() )
    //  {
    //        const SdrObjCustomShape* pSdrObjCustomShape = dynamic_cast< const SdrObjCustomShape* >(mpObj.get());
    //
    //        if(pSdrObjCustomShape)
    //        {
    //            bMirroredX = isMirroredX();
    //            bMirroredY = isMirroredY();
    //        }
    //        // TTTT:
    //      //bMirroredX = ( ((SdrObjCustomShape*)mpObj.get())->IsMirroredX() );
    //      //bMirroredY = ( ((SdrObjCustomShape*)mpObj.get())->IsMirroredY() );
    //  }
    //  // get aRect, this is the unrotated snaprect
    //  Rectangle aRect(sdr::legacy::GetLogicRect(*((SdrObjCustomShape*)mpObj.get())));
    //  Rectangle aRectangle( aRect );
    //
    //  if ( bMirroredX || bMirroredY )
    //  {
    //      // we have to retrieve the unmirrored rect
    //      long aRotation(sdr::legacy::GetRotateAngle(*mpObj.get()));
    //      long aShear(sdr::legacy::GetShearAngleX(*mpObj.get()));
    //
    //      if ( bMirroredX )
    //      {
    //          Polygon aPol( Rect2Poly( aRect, aRotation, aShear ) );
    //          Rectangle aBoundRect( aPol.GetBoundRect() );
    //
    //          Point aRef1( ( aBoundRect.Left() + aBoundRect.Right() ) >> 1, aBoundRect.Top() );
    //          Point aRef2( aRef1.X(), aRef1.Y() + 1000 );
    //          sal_uInt16 i;
    //          sal_uInt16 nPntAnz=aPol.GetSize();
    //          for (i=0; i<nPntAnz; i++)
    //          {
    //              MirrorPoint(aPol[i],aRef1,aRef2);
    //          }
    //          // Polygon wenden und etwas schieben
    //          Polygon aPol0(aPol);
    //          aPol[0]=aPol0[1];
    //          aPol[1]=aPol0[0];
    //          aPol[2]=aPol0[3];
    //          aPol[3]=aPol0[2];
    //          aPol[4]=aPol0[1];
    //          Poly2Rect(aPol,aRectangle,aRotation, aShear);
    //      }
    //      if ( bMirroredY )
    //      {
    //          Polygon aPol( Rect2Poly( aRectangle, aRotation, aShear ) );
    //          Rectangle aBoundRect( aPol.GetBoundRect() );
    //
    //          Point aRef1( aBoundRect.Left(), ( aBoundRect.Top() + aBoundRect.Bottom() ) >> 1 );
    //          Point aRef2( aRef1.X() + 1000, aRef1.Y() );
    //          sal_uInt16 i;
    //          sal_uInt16 nPntAnz=aPol.GetSize();
    //          for (i=0; i<nPntAnz; i++)
    //          {
    //              MirrorPoint(aPol[i],aRef1,aRef2);
    //          }
    //          // Polygon wenden und etwas schieben
    //          Polygon aPol0(aPol);
    //          aPol[0]=aPol0[1];
    //          aPol[1]=aPol0[0];
    //          aPol[2]=aPol0[3];
    //          aPol[3]=aPol0[2];
    //          aPol[4]=aPol0[1];
    //          Poly2Rect( aPol, aRectangle, aRotation, aShear );
    //      }
    //  }
    //
    //  basegfx::B2DPoint aPt( aRectangle.Left(), aRectangle.Top() );
    //
    //    if(isWriterAnchorUsed())
    //    {
    //      aPt -= mpObj->GetAnchorPos();
    //    }
    //
    //  ForceMetricTo100th_mm(aPt);
    //
    //  return ::com::sun::star::awt::Point( basegfx::fround(aPt.getX()), basegfx::fround(aPt.getY()) );
    //}
    //else
    //  return SvxShape::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxCustomShape::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShapeText::setPosition(Position);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxCustomShape::getSize() throw(uno::RuntimeException)
{
    return SvxShapeText::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxCustomShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShapeText::setSize( rSize );
}

//----------------------------------------------------------------------

//----------------------------------------------------------------------
void SAL_CALL SvxCustomShape::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException)
{
    // TTTT: All exceptions because of mirroring should be obsolete
    SvxShape::setPropertyValue( aPropertyName, aValue );

    //OGuard aGuard( Application::GetSolarMutex() );
    //SdrObject* pObject = mpObj.get();
    //
    //const bool bCustomShapeGeometry(pObject && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CustomShapeGeometry" ) ));
    //bool bMirroredX = false;
    //bool bMirroredY = false;
    //
    //if ( bCustomShapeGeometry )
    //{
    //  bMirroredX = ( ((SdrObjCustomShape*)pObject)->IsMirroredX() );
    //  bMirroredY = ( ((SdrObjCustomShape*)pObject)->IsMirroredY() );
    //}
    //
    //SvxShape::setPropertyValue( aPropertyName, aValue );
    //
    //if ( bCustomShapeGeometry )
    //{
    //  ((SdrObjCustomShape*)pObject)->MergeDefaultAttributes(0);
    //  const Rectangle aRect( sdr::legacy::GetSnapRect(*pObject) );
    //
    //  // #i38892#
    //  const bool bNeedsMirrorX(((SdrObjCustomShape*)pObject)->IsMirroredX() != bMirroredX);
    //  const bool bNeedsMirrorY(((SdrObjCustomShape*)pObject)->IsMirroredY() != bMirroredY);
    //  boost::scoped_ptr< SdrGluePointList > pListCopy;
    //
    //  if( bNeedsMirrorX || bNeedsMirrorY )
    //  {
    //      const SdrGluePointList* pList = pObject->GetGluePointList();
    //      if( pList )
    //          pListCopy.reset( new SdrGluePointList(*pList) );
    //  }
    //
    //    if ( bNeedsMirrorX )
    //  {
    //      Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
    //      Point aBottom( aTop.X(), aTop.Y() + 1000 );
    //      sdr::legacy::MirrorSdrObject(*pObject, aTop, aBottom );
    //      // NbcMirroring is flipping the current mirror state,
    //      // so we have to set the correct state again
    //      ((SdrObjCustomShape*)pObject)->SetMirroredX( bMirroredX ? sal_False : sal_True );
    //  }
    //  if ( bNeedsMirrorY )
    //  {
    //      Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
    //      Point aRight( aLeft.X() + 1000, aLeft.Y() );
    //      sdr::legacy::MirrorSdrObject(*pObject, aLeft, aRight );
    //      // NbcMirroring is flipping the current mirror state,
    //      // so we have to set the correct state again
    //      ((SdrObjCustomShape*)pObject)->SetMirroredY( bMirroredY ? sal_False : sal_True );
    //  }
    //
    //  if( pListCopy )
    //  {
    //      SdrGluePointList* pNewList = const_cast< SdrGluePointList* >( pObject->GetGluePointList() );
    //      if(pNewList)
    //          *pNewList = *pListCopy;
    //  }
    //}
}

// TTTT:
//bool SvxCustomShape::getPropertyValueImpl( const ::rtl::OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
//{
//  switch( pProperty->nWID )
//  {
//      case SDRATTR_ROTATEANGLE:
//      {
//          double fAngle = static_cast<SdrObjCustomShape*>(mpObj.get())->GetObjectRotation();
//          fAngle *= 100;
//          rValue <<= (sal_Int32)fAngle;
//          return true;
//      }
//      default:
//        {
//            return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
//      }
//    }
//}
//----------------------------------------------------------------------

void SvxCustomShape::createCustomShapeDefaults( const rtl::OUString& rValueType ) throw (::com::sun::star::uno::RuntimeException)
{
    ((SdrObjCustomShape*)mpObj.get())->MergeDefaultAttributes( &rValueType );
}

// EOF
