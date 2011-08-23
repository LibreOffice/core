/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <com/sun/star/awt/TextAlign.hpp>  //added by BerryJia for fixing Bug102407 2002-11-4
#include <com/sun/star/style/ParagraphAdjust.hpp>   //added by BerryJia for fixing Bug102407 2002-11-4
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <bf_goodies/matrix3d.hxx>
#include <unotools/localfilehelper.hxx>
#include <vcl/svapp.hxx>
#include <bf_svtools/fltcall.hxx>
#include "impgrf.hxx"

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "unopage.hxx"
#include "svdpage.hxx"
#include "svdmodel.hxx"
#include "svdouno.hxx"
#include "shapeimpl.hxx"

class GDIMetaFile;
class SvStream;

namespace binfilter {

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

using rtl::OUString;

#define INTERFACE_TYPE( xint ) \
    ::getCppuType((const Reference< xint >*)0)

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const Reference< xint >*)0) ) \
        aAny <<= Reference< xint >(this)

sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream, FilterConfigItem* pConfigItem = NULL, sal_Bool bPlaceable=sal_True);

/***********************************************************************
* class SvxShapeGroup                                                  *
***********************************************************************/

SvxShapeGroup::SvxShapeGroup( SdrObject* pSdrObj, SvxDrawPage* pDrawPage  )  throw() :
    SvxShape( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_GROUP) ),
    pPage( pDrawPage )
{
    if( pPage )
        pPage->acquire();
}

//----------------------------------------------------------------------
SvxShapeGroup::~SvxShapeGroup() throw()
{
    if( pPage )
        pPage->release();
}

//----------------------------------------------------------------------
void SvxShapeGroup::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw()
{
    if( pPage )
    {
        pPage->release();
        pPage = NULL;
    }

    SvxShape::Create( pNewObj, pNewPage );

    pPage = pNewPage;
    if( pPage )
        pPage->acquire();

}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeGroup::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return SvxShape::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeGroup::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XShapeGroup );
    else QUERYINT( drawing::XShapes );
    else QUERYINT( container::XIndexAccess );
    else QUERYINT( container::XElementAccess );
    else
        SvxShape::queryAggregation( rType, aAny );

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
//	pDrView->EnterMarkedGroup();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::leaveGroup(  ) throw(uno::RuntimeException)
{
    // Todo
//	pDrView->LeaveOneGroup();
}

//----------------------------------------------------------------------

// XShapes
void SAL_CALL SvxShapeGroup::add( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( pObj != NULL && pPage != NULL && pShape != NULL )
    {
        SdrObject* pSdrShape = pShape->GetSdrObject();
        if( pSdrShape == NULL )
            pSdrShape = pPage->_CreateSdrObject( xShape );

        if( pSdrShape->IsInserted() )
            pSdrShape->GetObjList()->RemoveObject( pSdrShape->GetOrdNum() );

        pObj->GetSubList()->InsertObject( pSdrShape );
        
        // #85922# It makes no sense to set the layer asked
        // from the group object since these is an iteration
        // over the contained objects. In consequence, this
        // statement erases all layer information from the draw
        // objects. Layers need to be set at draw objects directly
        // and have nothing to do with grouping at all.
        // pSdrShape->SetLayer(pObj->GetLayer());

        if(pShape)
            pShape->Create( pSdrShape, pPage );

        if( pModel )
            pModel->SetChanged();
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
    ::SolarMutexGuard aGuard;

    SdrObject* pSdrShape = NULL;
    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( pShape )
        pSdrShape = pShape->GetSdrObject();

    if(pObj == NULL || pSdrShape == NULL || pSdrShape->GetObjList()->GetOwnerObj() != pObj )
        throw uno::RuntimeException();

    SdrObjList& rList = *pSdrShape->GetObjList();

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
        delete rList.NbcRemoveObject( nObjNum );
        pShape->InvalidateSdrObject();
    }
    else
    {
        DBG_ASSERT( 0, "Fatality! SdrObject is not belonging to its SdrObjList! [CL]" );
    }

    if( pModel )
        pModel->SetChanged();
}

// XIndexAccess

//----------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShapeGroup::getCount() throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    sal_Int32 nRetval = 0;

    if(pObj && pObj->GetSubList())
        nRetval = pObj->GetSubList()->GetObjCount();
    else
        throw uno::RuntimeException();

    return nRetval;
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeGroup::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    if( pObj == NULL || pObj->GetSubList() == NULL )
        throw uno::RuntimeException();


    if( pObj->GetSubList()->GetObjCount() <= (sal_uInt32)Index )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pDestObj = pObj->GetSubList()->GetObj( Index );

    if(pDestObj == NULL)
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape( pDestObj->getUnoShape(), uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xShape;
    return aAny;
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
    ::SolarMutexGuard aGuard;

    return pObj && pObj->GetSubList() && (pObj->GetSubList()->GetObjCount() > 0);
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

SvxShapeConnector::SvxShapeConnector( SdrObject* pSdrObj )  throw() :
    SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_CONNECTOR) )
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

void SAL_CALL SvxShapeConnector::connectStart( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType nPos ) throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( pShape )
        pObj->ConnectToNode( sal_True, pShape->pObj );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::connectEnd( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType nPos )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( pObj && pShape )
        pObj->ConnectToNode( sal_False, pShape->pObj );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::disconnectBegin( const uno::Reference< drawing::XConnectableShape >& xShape )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    if(pObj)
        pObj->DisconnectFromNode( sal_True );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::disconnectEnd( const uno::Reference< drawing::XConnectableShape >& xShape )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    if(pObj)
        pObj->DisconnectFromNode( sal_False );

    if( pModel )
        pModel->SetChanged();
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


SvxShapeControl::SvxShapeControl( SdrObject* pSdrObj )  throw() :
    SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_CONTROL) )
{
    setShapeKind( OBJ_UNO );
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

uno::Any SAL_CALL SvxShapeControl::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
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
    ::SolarMutexGuard aGuard;

    Reference< awt::XControlModel > xModel;

    SdrUnoObj* pUnoObj = PTR_CAST(SdrUnoObj, pObj);
    if( pUnoObj )
        xModel = pUnoObj->GetUnoControlModel();

    return xModel;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setControl( const Reference< awt::XControlModel >& xControl )
    throw( uno::RuntimeException )
{
    ::SolarMutexGuard aGuard;

    SdrUnoObj* pUnoObj = PTR_CAST(SdrUnoObj, pObj);
    if( pUnoObj )
        pUnoObj->SetUnoControlModel( xControl );

    if( pModel )
        pModel->SetChanged();
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
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_COLOR),	MAP_CHAR_LEN("TextColor") },
    { MAP_CHAR_LEN("CharRelief"),	MAP_CHAR_LEN("FontRelief") },
    { MAP_CHAR_LEN("CharUnderlineColor"),	MAP_CHAR_LEN("TextLineColor") },
    { MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_ADJUST), MAP_CHAR_LEN("Align") },
    { MAP_CHAR_LEN("ControlBackground"), MAP_CHAR_LEN("BackgroundColor") },
    { MAP_CHAR_LEN("ControlBorder"), MAP_CHAR_LEN("Border") },
    { MAP_CHAR_LEN("ControlTextEmphasis"),	MAP_CHAR_LEN("FontEmphasisMark") },
    { NULL,0, NULL, 0 }
};

void SvxShapeControl::convertPropertyName( const OUString& rApiName, OUString& rInternalName, sal_Bool& rNeedsConversion )
{
    sal_uInt16 i = 0;
    while( SvxShapeControlPropertyMapping[i].mpAPIName )
    {
        if( rApiName.reverseCompareToAsciiL( SvxShapeControlPropertyMapping[i].mpAPIName, SvxShapeControlPropertyMapping[i].mnAPINameLen ) == 0 )
        {
            rInternalName = OUString( SvxShapeControlPropertyMapping[i].mpFormName, SvxShapeControlPropertyMapping[i].mnFormNameLen, RTL_TEXTENCODING_ASCII_US ); 
            rNeedsConversion = i == 0;
        }
        i++;
    }
}

//added by BerryJia for fixing Bug102407 2002-11-04
static struct
{
    sal_Int16 nAPIValue;
    sal_Int16 nFormValue;
}
SvxShapeControlPropertyValueMapping[] = 
{
    // note that order matters:
    // valueAlignToParaAdjust and valueParaAdjustToAlign search this map from the _beginning_
    // and use the first matching entry
    {style::ParagraphAdjust_LEFT,			(sal_Int16)awt::TextAlign::LEFT},
    {style::ParagraphAdjust_CENTER,			(sal_Int16)awt::TextAlign::CENTER},
    {style::ParagraphAdjust_RIGHT,			(sal_Int16)awt::TextAlign::RIGHT},
    {style::ParagraphAdjust_BLOCK,			(sal_Int16)awt::TextAlign::RIGHT},
    {style::ParagraphAdjust_STRETCH,		(sal_Int16)awt::TextAlign::LEFT},
    {style::ParagraphAdjust_MAKE_FIXED_SIZE,(sal_Int16)awt::TextAlign::LEFT},
    {-1,-1}
};

void SvxShapeControl::valueAlignToParaAdjust(Any& rValue)  
{
    sal_Int16 nValue;
    rValue >>= nValue;
    sal_uInt16 i = 0;
    while (-1 != SvxShapeControlPropertyValueMapping[i].nFormValue)
    {
        if (nValue == SvxShapeControlPropertyValueMapping[i].nFormValue)
        {
            rValue <<= (SvxShapeControlPropertyValueMapping[i].nAPIValue);
            return;
        }
        i++;
    }
}

void SvxShapeControl::valueParaAdjustToAlign(Any& rValue)  
{
    sal_Int32 nValue;
    rValue >>= nValue;
    sal_uInt16 i = 0;
    while (-1 != SvxShapeControlPropertyValueMapping[i].nAPIValue)
    {
        if ( nValue == SvxShapeControlPropertyValueMapping[i].nAPIValue)
        {
            rValue <<= (SvxShapeControlPropertyValueMapping[i].nFormValue);
            return;
        }
        i++;
    }
}

void SAL_CALL SvxShapeControl::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException)
{
    OUString aFormsName;
    sal_Bool bNeedConversion;

    convertPropertyName( aPropertyName, aFormsName, bNeedConversion );
    if( aFormsName.getLength() )
    {
        uno::Reference< beans::XPropertySet > xControl( getControl(), uno::UNO_QUERY );
        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xControl->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                if( bNeedConversion )
                {
                    awt::FontSlant nSlant;
                    if( !(aValue >>= nSlant ) )
                        throw lang::IllegalArgumentException();

                    xControl->setPropertyValue( aFormsName, uno::makeAny( (sal_Int16)nSlant ) );
                }
                else
                {
                    //modified by BerryJia for fixing Bug102407 2002-11-4
                    Any rValue;
                    rValue = aValue;
                    if (::rtl::OUString::createFromAscii("Align") == aFormsName)
                        valueParaAdjustToAlign(rValue);
                    xControl->setPropertyValue( aFormsName, rValue );
                }
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
    sal_Bool bNeedConversion;

    convertPropertyName( aPropertyName, aFormsName, bNeedConversion );
    if( aFormsName.getLength() )
    {
        uno::Reference< beans::XPropertySet > xControl( getControl(), uno::UNO_QUERY );

        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xControl->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aFormsName ) )
            {
                if( bNeedConversion )
                {
                    sal_Int16 nSlant;
                    xControl->getPropertyValue( aFormsName ) >>= nSlant;
                    return uno::makeAny( (awt::FontSlant)nSlant );
                }
                else
                {
                    //modified by BerryJia for fixing Bug102407 2002-11-4
                    Any rValue;
                    rValue = xControl->getPropertyValue( aFormsName );
                    if (::rtl::OUString::createFromAscii("Align") == aFormsName)
                        valueAlignToParaAdjust(rValue);
                    return rValue;
                }
            }
        }

        uno::Any aAny;
        return aAny;
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
    sal_Bool bNeedConversion;

    convertPropertyName( PropertyName, aFormsName, bNeedConversion );
    if( aFormsName.getLength() )
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
    sal_Bool bNeedConversion;

    convertPropertyName( PropertyName, aFormsName, bNeedConversion );
    if( aFormsName.getLength() )
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
    sal_Bool bNeedConversion;

    convertPropertyName( aPropertyName, aFormsName, bNeedConversion );
    if( aFormsName.getLength() )
    {
        uno::Reference< beans::XPropertyState > xControl( getControl(), uno::UNO_QUERY );

        if( xControl.is() )
        {
            if( bNeedConversion )
            {
                sal_Int16 nSlant;
                xControl->getPropertyDefault( aFormsName ) >>= nSlant;

                return uno::makeAny( (awt::FontSlant)nSlant );
            }
            else
            {
                return xControl->getPropertyDefault( aFormsName );
            }
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
SvxShapeDimensioning::SvxShapeDimensioning( SdrObject* pSdrObj ) throw()
:	SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_DIMENSIONING) )
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
SvxShapeCircle::SvxShapeCircle( SdrObject* pSdrObj ) throw()
:	SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_CIRCLE) )
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
}//namespace binfilter
#include "svdopath.hxx"
namespace binfilter {//STRIP009
//----------------------------------------------------------------------
SvxShapePolyPolygon::SvxShapePolyPolygon( SdrObject* pSdrObj , drawing::PolygonKind eNew )
 throw( ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException)
: SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_POLYPOLYGON) ),
    ePolygonKind( eNew )
{
}

//----------------------------------------------------------------------
SvxShapePolyPolygon::~SvxShapePolyPolygon() throw()
{
}

void SAL_CALL ImplSvxPolyPolygonToPointSequenceSequence( const drawing::PointSequenceSequence* pOuterSequence, XPolyPolygon& rNewPolyPolygon ) throw()
{
    // Koordinaten in das PolyPolygon packen
    // Zeiger auf innere sequences holen
    const drawing::PointSequence* pInnerSequence = pOuterSequence->getConstArray();
    const drawing::PointSequence* pInnerSeqEnd   = pInnerSequence + pOuterSequence->getLength();

    // #85920# Clear the given polygon, since the new one shall be SET, not ADDED
    rNewPolyPolygon.Clear();

    for(;pInnerSequence != pInnerSeqEnd;++pInnerSequence)
    {
        sal_Int32 nInnerSequenceCount = pInnerSequence->getLength();

        // Neues XPolygon vorbereiten
        XPolygon aNewPolygon((USHORT)nInnerSequenceCount);

        // Zeiger auf Arrays holen
        const awt::Point* pArray	= pInnerSequence->getConstArray();
        const awt::Point* pArrayEnd = pArray + nInnerSequenceCount;

        for(USHORT b=0;pArray != pArrayEnd;++b,++pArray)
        {
            Point& rPoint = aNewPolygon[b];
            rPoint.X() = pArray->X;
            rPoint.Y() = pArray->Y;
        }

        // Neues Teilpolygon einfuegen
        rNewPolyPolygon.Insert(aNewPolygon);
    }
}

//----------------------------------------------------------------------

void SAL_CALL SvxShapePolyPolygon::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException)
{
    ::SolarMutexGuard aGuard;

    if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGON)))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PointSequenceSequence*)0) )
            throw lang::IllegalArgumentException();

        XPolyPolygon aNewPolyPolygon;

        ImplSvxPolyPolygonToPointSequenceSequence( (drawing::PointSequenceSequence*)aValue.getValue(), aNewPolyPolygon );

        SetPolygon(aNewPolyPolygon);
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PointSequenceSequence*)0) )
            throw lang::IllegalArgumentException();

        if( pObj )
        {
            XPolyPolygon aNewPolyPolygon;
            Matrix3D aMatrix3D;
        
            pObj->TRGetBaseGeometry(aMatrix3D, aNewPolyPolygon);

            ImplSvxPolyPolygonToPointSequenceSequence( (drawing::PointSequenceSequence*)aValue.getValue(), aNewPolyPolygon );

            pObj->TRSetBaseGeometry(aMatrix3D, aNewPolyPolygon);
        }
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGON)))
    {
        drawing::PointSequence* pSequence = (drawing::PointSequence*)aValue.getValue();

        if( !pSequence || aValue.getValueType() != ::getCppuType((const drawing::PointSequence*)0 ) )
            throw lang::IllegalArgumentException();

        // Koordinaten in das PolyPolygon packen
        const sal_Int32 nSequenceCount = pSequence->getLength();

        // Neues PolyPolygon vorbereiten
        XPolyPolygon aNewPolyPolygon;

        // Neues XPolygon vorbereiten
        XPolygon aNewPolygon((USHORT)nSequenceCount);

        // Zeiger auf Arrays holen
        awt::Point* pArray = pSequence->getArray();

        for(sal_Int32 b=0;b<nSequenceCount;b++)
        {
            aNewPolygon[(USHORT)b] = Point( pArray->X,  pArray->Y );
            pArray++;
        }

        // Neues Teilpolygon einfuegen
        aNewPolyPolygon.Insert(aNewPolygon);

        // Polygon setzen
        SetPolygon(aNewPolyPolygon);
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }

    if( pModel )
        pModel->SetChanged();
}

void SAL_CALL ImplSvxPointSequenceSequenceToPolyPolygon( const XPolyPolygon& rPolyPoly, drawing::PointSequenceSequence& rRetval )
{
    if( rRetval.getLength() != rPolyPoly.Count() )
        rRetval.realloc( rPolyPoly.Count() );

    // Zeiger auf aeussere Arrays holen
    drawing::PointSequence* pOuterSequence = rRetval.getArray();

    for(sal_uInt16 a=0;a<rPolyPoly.Count();a++)
    {
        // Einzelpolygon holen
        const XPolygon& rPoly = rPolyPoly[a];

        // Platz in Arrays schaffen
        pOuterSequence->realloc((sal_Int32)rPoly.GetPointCount());

        // Pointer auf arrays holen
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        for(sal_uInt16 b=0;b<rPoly.GetPointCount();b++)
        {
            *pInnerSequence = awt::Point( rPoly[b].X(), rPoly[b].Y() );
            pInnerSequence++;
        }

        pOuterSequence++;
    }
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShapePolyPolygon::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGON)))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        const XPolyPolygon& rPolyPoly = GetPolygon();
        drawing::PointSequenceSequence aRetval( rPolyPoly.Count() );

        ImplSvxPointSequenceSequenceToPolyPolygon( rPolyPoly, aRetval );

        return uno::Any( &aRetval, ::getCppuType((const drawing::PointSequenceSequence*)0) );
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        XPolyPolygon aPolyPoly;
        Matrix3D aMatrix3D;
        if( pObj )
            pObj->TRGetBaseGeometry( aMatrix3D, aPolyPoly );

        drawing::PointSequenceSequence aRetval( aPolyPoly.Count() );

        ImplSvxPointSequenceSequenceToPolyPolygon( aPolyPoly, aRetval );

        return uno::Any( &aRetval, ::getCppuType((const drawing::PointSequenceSequence*)0) );
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGON)))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        const XPolyPolygon& rPolyPoly = GetPolygon();

        sal_Int32 nCount = 0;
        if( rPolyPoly.Count() > 0 )
            nCount = rPolyPoly[0].GetPointCount();

        drawing::PointSequence aRetval( nCount );

        if( nCount > 0 )
        {
            // Einzelpolygon holen
            const XPolygon& rPoly = rPolyPoly[0];

            // Pointer auf arrays holen
            awt::Point* pSequence = aRetval.getArray();

            for(sal_Int32 b=0;b<nCount;b++)
                *pSequence++ = awt::Point( rPoly[(USHORT)b].X(), rPoly[(USHORT)b].Y() );
        }

        return uno::Any( &aRetval, ::getCppuType((const drawing::PointSequence*)0) );
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGONKIND)))
    {
        drawing::PolygonKind ePT = GetPolygonKind();
        return Any( &ePT, ::getCppuType((const drawing::PolygonKind*)0) );
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

//----------------------------------------------------------------------
drawing::PolygonKind SvxShapePolyPolygon::GetPolygonKind() const throw()
{
    return ePolygonKind;
}

//----------------------------------------------------------------------
void SvxShapePolyPolygon::SetPolygon(const XPolyPolygon& rNew) throw()
{
    ::SolarMutexGuard aGuard;

    if(pObj)
        ((SdrPathObj*)pObj)->SetPathPoly(rNew);
}

//----------------------------------------------------------------------
const XPolyPolygon& SvxShapePolyPolygon::GetPolygon() const throw()
{
    ::SolarMutexGuard aGuard;

    if(pObj)
        return ((SdrPathObj*)pObj)->GetPathPoly();
    return aEmptyPoly;
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapePolyPolygon::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

/***********************************************************************
* class SvxShapePolyPolygonBezier                                      *
***********************************************************************/
}
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
namespace binfilter {//STRIP009
//----------------------------------------------------------------------
SvxShapePolyPolygonBezier::SvxShapePolyPolygonBezier( SdrObject* pSdrObj , drawing::PolygonKind eNew ) throw()
:	SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_POLYPOLYGONBEZIER) ),
    ePolygonKind( eNew )
{
}

//----------------------------------------------------------------------
SvxShapePolyPolygonBezier::~SvxShapePolyPolygonBezier() throw()
{
}

void SAL_CALL ImplSvxPolyPolygonBezierCoordsToPolyPolygon( drawing::PolyPolygonBezierCoords* pSourcePolyPolygon, XPolyPolygon& rNewPolyPolygon )
    throw( IllegalArgumentException )
{
    sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();
    if(pSourcePolyPolygon->Flags.getLength() != nOuterSequenceCount)
        throw IllegalArgumentException();

    // Zeiger auf innere sequences holen
    const drawing::PointSequence* pInnerSequence = pSourcePolyPolygon->Coordinates.getConstArray();
    const drawing::FlagSequence* pInnerSequenceFlags = pSourcePolyPolygon->Flags.getConstArray();
    sal_Bool bIsCurve(FALSE);
    sal_Bool bCurveValid(TRUE);
    sal_Bool bCurveTestActive(FALSE);

    for(sal_Int32 a(0); bCurveValid && a < nOuterSequenceCount; a++)
    {
        sal_Int32 nInnerSequenceCount = pInnerSequence->getLength();

        if(pInnerSequenceFlags->getLength() != nInnerSequenceCount)
            throw IllegalArgumentException();

        // Neues XPolygon vorbereiten
        XPolygon aNewPolygon((USHORT)nInnerSequenceCount);

        // Zeiger auf Arrays holen
        const awt::Point* pArray = pInnerSequence->getConstArray();
        const drawing::PolygonFlags* pArrayFlags = pInnerSequenceFlags->getConstArray();

        for(sal_Int32 b(0); bCurveValid && b < nInnerSequenceCount; b++)
        {
            // coordinate data
            aNewPolygon[(USHORT)b] = Point( pArray->X, pArray->Y );
            pArray++;

            // flag data
            XPolyFlags ePolyFlag = (XPolyFlags)((sal_uInt16)*pArrayFlags++);

            // set curve flag
            if(!bIsCurve && ePolyFlag == XPOLY_CONTROL)
                bIsCurve = TRUE;

            // curve testing
            if(bIsCurve && bCurveValid && (bCurveTestActive || ePolyFlag == XPOLY_CONTROL))
            {
                if(!bCurveTestActive)
                {
                    // first control point found, test it
                    if(b == 0)
                    {
                        // no curve startpoint possible
                        bCurveValid = FALSE;
                    }
                    else
                    {
                        // test type of prev point
                        XPolyFlags ePrevPolyFlag = (XPolyFlags)((sal_uInt16)*(pArrayFlags - 2));
                        if(ePrevPolyFlag == XPOLY_CONTROL)
                        {
                            // curve startpoint is a control point,
                            // this is not allowed (three control points in a row)
                            bCurveValid = FALSE;
                        }
                    }

                    // next curve test state
                    bCurveTestActive = TRUE;
                }
                else
                {
                    // prev was a valid curve start, this should be the second 
                    // curve control point, test it
                    if(ePolyFlag != XPOLY_CONTROL)
                    {
                        // no second curve control point
                        bCurveValid = FALSE;
                    }
                    else if(b == nInnerSequenceCount-1)
                    {
                        // no curve endpoint possible
                        bCurveValid = FALSE;
                    }
                    else
                    {
                        // test type of next point
                        XPolyFlags eNextPolyFlag = (XPolyFlags)((sal_uInt16)*pArrayFlags);
                        if(eNextPolyFlag == XPOLY_CONTROL)
                        {
                            // curve endpoint is the next control point,
                            // this is not allowed (three control points in a row)
                            bCurveValid = FALSE;
                        }
                    }

                    // end curve test for this segment
                    bCurveTestActive = FALSE;
                }
            }

            aNewPolygon.SetFlags((USHORT)b, ePolyFlag);
        }

        pInnerSequence++;
        pInnerSequenceFlags++;

        // Neues Teilpolygon einfuegen
        rNewPolyPolygon.Insert(aNewPolygon);
    }

    // throw exception if polygon data is an invalid curve definition
    if(bIsCurve && !bCurveValid)
        throw IllegalArgumentException();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapePolyPolygonBezier::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException )
{
    ::SolarMutexGuard aGuard;

    if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGONBEZIER)))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
            throw IllegalArgumentException();

        XPolyPolygon aNewPolyPolygon;
        ImplSvxPolyPolygonBezierCoordsToPolyPolygon( (drawing::PolyPolygonBezierCoords*)aValue.getValue(), aNewPolyPolygon );
        SetPolygon(aNewPolyPolygon);
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
            throw IllegalArgumentException();

        if( pObj )
        {
            Matrix3D aMatrix3D;
            XPolyPolygon aNewPolyPolygon;

            pObj->TRGetBaseGeometry(aMatrix3D, aNewPolyPolygon);
            ImplSvxPolyPolygonBezierCoordsToPolyPolygon( (drawing::PolyPolygonBezierCoords*)aValue.getValue(), aNewPolyPolygon );
            pObj->TRSetBaseGeometry(aMatrix3D, aNewPolyPolygon);
        }
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }

    if( pModel )
        pModel->SetChanged();
}

void SAL_CALL ImplSvxPolyPolygonToPolyPolygonBezierCoords( const XPolyPolygon& rPolyPoly, drawing::PolyPolygonBezierCoords& rRetval )
{
    // Polygone innerhalb vrobereiten
    rRetval.Coordinates.realloc((sal_Int32)rPolyPoly.Count());
    rRetval.Flags.realloc((sal_Int32)rPolyPoly.Count());

    // Zeiger auf aeussere Arrays holen
    drawing::PointSequence* pOuterSequence = rRetval.Coordinates.getArray();
    drawing::FlagSequence*  pOuterFlags = rRetval.Flags.getArray();

    for(sal_uInt16 a=0;a<rPolyPoly.Count();a++)
    {
        // Einzelpolygon holen
        const XPolygon& rPoly = rPolyPoly[a];

        // Platz in Arrays schaffen
        pOuterSequence->realloc((sal_Int32)rPoly.GetPointCount());
        pOuterFlags->realloc((sal_Int32)rPoly.GetPointCount());

        // Pointer auf arrays holen
        awt::Point* pInnerSequence = pOuterSequence->getArray();
        drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

        for(sal_uInt16 b=0;b<rPoly.GetPointCount();b++)
        {
            *pInnerSequence++ = awt::Point( rPoly[b].X(), rPoly[b].Y() );
            *pInnerFlags++ = (drawing::PolygonFlags)((sal_uInt16)rPoly.GetFlags(b));
        }

        pOuterSequence++;
        pOuterFlags++;
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapePolyPolygonBezier::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;
    uno::Any aAny;

    if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGONBEZIER)))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        const XPolyPolygon& rPolyPoly = GetPolygon();
        drawing::PolyPolygonBezierCoords aRetval;
        ImplSvxPolyPolygonToPolyPolygonBezierCoords(rPolyPoly, aRetval );

        aAny <<= aRetval;
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        if( pObj )
        {
            // PolyPolygon in eine struct PolyPolygon packen
            XPolyPolygon aPolyPoly;
            Matrix3D aMatrix3D;
            pObj->TRGetBaseGeometry( aMatrix3D, aPolyPoly );

            drawing::PolyPolygonBezierCoords aRetval;
            ImplSvxPolyPolygonToPolyPolygonBezierCoords(aPolyPoly, aRetval );

            aAny <<= aRetval;
        }
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGONKIND)))
    {
        aAny <<= (drawing::PolygonKind)GetPolygonKind();
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }

    return aAny;
}

//----------------------------------------------------------------------
drawing::PolygonKind SvxShapePolyPolygonBezier::GetPolygonKind() const throw()
{
    return ePolygonKind;
}

//----------------------------------------------------------------------
void SvxShapePolyPolygonBezier::SetPolygon(const XPolyPolygon& rNew) throw()
{
    ::SolarMutexGuard aGuard;

    if(pObj)
        ((SdrPathObj*)pObj)->SetPathPoly(rNew);
}

//----------------------------------------------------------------------
const XPolyPolygon& SvxShapePolyPolygonBezier::GetPolygon() const throw()
{
    ::SolarMutexGuard aGuard;

    if(pObj)
        return ((SdrPathObj*)pObj)->GetPathPoly();
    return aEmptyPoly;
}


// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapePolyPolygonBezier::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}

/***********************************************************************
* class SvxGraphicObject                                               *
***********************************************************************/
}
#include <vcl/cvtgrf.hxx>
#include <svdograf.hxx>
#ifndef SVX_LIGHT
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_sfx2/fcontnr.hxx>
#endif

#include "toolkit/unohlp.hxx"
namespace binfilter {//STRIP009
//----------------------------------------------------------------------
SvxGraphicObject::SvxGraphicObject( SdrObject* pSdrObj ) throw()
:	SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_GRAPHICOBJECT) )
{
}

//----------------------------------------------------------------------
SvxGraphicObject::~SvxGraphicObject() throw()
{
}

//----------------------------------------------------------------------
void SAL_CALL SvxGraphicObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException)
{
    ::SolarMutexGuard aGuard;

    if(pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_FILLBITMAP)))
    {
        if( aValue.getValueType() == ::getCppuType(( const uno::Sequence< sal_Int8 >*)0) )
        {
            uno::Sequence<sal_Int8>* pSeq = (uno::Sequence<sal_Int8>*)aValue.getValue();
            SvMemoryStream	aMemStm;
            Graphic			aGraphic;

            aMemStm.SetBuffer( (char*)pSeq->getConstArray(), pSeq->getLength(), sal_False, pSeq->getLength() );

            if( GraphicConverter::Import( aMemStm, aGraphic ) == ERRCODE_NONE )
            {
                ((SdrGrafObj*)pObj)->SetGraphic(aGraphic);
            }
        }
        else if( aValue.getValueType() == INTERFACE_TYPE( awt::XBitmap))
        {
            // Bitmap in das Objekt packen
            Reference< awt::XBitmap > xBmp;
            if( aValue >>= xBmp )
            {
                // Bitmap einsetzen
                Graphic aGraphic(VCLUnoHelper::GetBitmap( xBmp ));
                ((SdrGrafObj*)pObj)->SetGraphic(aGraphic);
            }
        }
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFURL)))
    {
        OUString aURL;
        if(!(aValue >>= aURL))
            throw lang::IllegalArgumentException();

        if( aURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) == 0 )
        {
            // graphic manager url
            aURL = aURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 );
            String aTmpStr(aURL);
            ByteString aUniqueID( aTmpStr, RTL_TEXTENCODING_UTF8 );
            BfGraphicObject aGrafObj( aUniqueID );

            // #101808# since loading a graphic can cause a reschedule of the office
            //			it is possible that our shape is removed while where in this
            //			method.
            if( pObj )
            {
                ((SdrGrafObj*)pObj)->ReleaseGraphicLink();
                ((SdrGrafObj*)pObj)->SetGraphicObject( aGrafObj );
            }
        }
        else if( aURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) ) != 0 )
        {
            // normal link
            String				aFilterName;
            const SfxFilter*	pSfxFilter = NULL;
            SfxMedium			aSfxMedium( aURL, STREAM_READ | STREAM_SHARE_DENYNONE, FALSE );
            
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
                    GraphicFilter* pGrfFilter = GetGrfFilter();
                    aFilterName = pGrfFilter->GetImportFormatName( pGrfFilter->GetImportFormatNumberForShortName( aURLObj.getExtension() ) );
                }
            }
            else
                aFilterName = pSfxFilter->GetFilterName();

            // #101808# since loading a graphic can cause a reschedule of the office
            //			it is possible that our shape is removed while where in this
            //			method.
            if( pObj )
                ((SdrGrafObj*)pObj)->SetGraphicLink( aURL, aFilterName );
        }
        else
        {
        }
        
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFSTREAMURL)))
    {
        OUString aStreamURL;

        if( !( aValue >>= aStreamURL ) )
            throw lang::IllegalArgumentException();


        if( aStreamURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) ) != 0 )
        {
            aStreamURL = OUString();
        }

        ((SdrGrafObj*)pObj)->SetGrafStreamURL( aStreamURL );
        ((SdrGrafObj*)pObj)->ForceSwapOut();
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxGraphicObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if(pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_FILLBITMAP)))
    {
        const Graphic& rGraphic = ((SdrGrafObj*)pObj)->GetGraphic();

        if(rGraphic.GetType() != GRAPHIC_GDIMETAFILE)
        {
            // Objekt in eine Bitmap packen
            Reference< ::com::sun::star::awt::XBitmap >  xBitmap( VCLUnoHelper::CreateBitmap(((SdrGrafObj*)pObj)->GetGraphic().GetBitmapEx()) );
            return uno::Any( &xBitmap, ::getCppuType((const Reference<awt::XBitmap>*)0) );
        }
        else
        {
            SvMemoryStream aDestStrm( 65535, 65535 );

            ConvertGDIMetaFileToWMF( rGraphic.GetGDIMetaFile(), aDestStrm, NULL, sal_False );
            const uno::Sequence<sal_Int8> aSeq(
                static_cast< const sal_Int8* >(aDestStrm.GetData()),
                aDestStrm.GetEndOfData());
            return uno::Any( &aSeq, ::getCppuType(( uno::Sequence< sal_Int8 >*)0) );
        }
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFURL)) )
    {
        uno::Any aAny;
        if( ((SdrGrafObj*)pObj)->IsLinkedGraphic() )
        {
            aAny <<= OUString( ((SdrGrafObj*)pObj)->GetFileName() );
        }
        else
        {
            const BfGraphicObject& rGrafObj = ((SdrGrafObj*)pObj)->GetGraphicObject();
            OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));			
            aURL += OUString::createFromAscii( rGrafObj.GetUniqueID().GetBuffer() );
            aAny <<= aURL;
        }
        return aAny;
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFSTREAMURL)) )
    {
        const OUString	aStreamURL( ( (SdrGrafObj*) pObj )->GetGrafStreamURL() );
        uno::Any		aAny;

        if( aStreamURL.getLength() )
            aAny <<= aStreamURL;

        return aAny;
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxGraphicObject::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return SvxShapeText::getSupportedServiceNames();
}


void SvxConvertPolyPolygonBezierToXPolygon( const drawing::PolyPolygonBezierCoords* pSourcePolyPolygon, XPolygon& rNewPolygon )
    throw( lang::IllegalArgumentException )
{
    sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();
    if( nOuterSequenceCount != 1 || pSourcePolyPolygon->Flags.getLength() != nOuterSequenceCount)
        throw lang::IllegalArgumentException();

    // Zeiger auf innere sequences holen
    const drawing::PointSequence* pInnerSequence = pSourcePolyPolygon->Coordinates.getConstArray();
    const drawing::FlagSequence* pInnerSequenceFlags = pSourcePolyPolygon->Flags.getConstArray();

    sal_Int32 nInnerSequenceCount = pInnerSequence->getLength();

    if(pInnerSequenceFlags->getLength() != nInnerSequenceCount)
        throw lang::IllegalArgumentException();

    // Zeiger auf Arrays holen
    const awt::Point* pArray = pInnerSequence->getConstArray();
    const drawing::PolygonFlags* pArrayFlags = pInnerSequenceFlags->getConstArray();

    for(sal_Int32 b=0;b<nInnerSequenceCount;b++)
    {
        rNewPolygon[(USHORT)b] = Point( pArray->X, pArray->Y );
        pArray++;
        rNewPolygon.SetFlags((USHORT)b, (XPolyFlags)((sal_uInt16)*pArrayFlags++));
    }
}

void SvxConvertXPolygonToPolyPolygonBezier( const XPolygon& rPolygon, drawing::PolyPolygonBezierCoords& rRetval ) throw()
{
    // Polygone innerhalb vrobereiten
    rRetval.Coordinates.realloc(1);
    rRetval.Flags.realloc(1);

    // Zeiger auf aeussere Arrays holen
    drawing::PointSequence* pOuterSequence = rRetval.Coordinates.getArray();
    drawing::FlagSequence*  pOuterFlags = rRetval.Flags.getArray();

    // Platz in Arrays schaffen
    pOuterSequence->realloc((sal_Int32)rPolygon.GetPointCount());
    pOuterFlags->realloc((sal_Int32)rPolygon.GetPointCount());

    // Pointer auf arrays holen
    awt::Point* pInnerSequence = pOuterSequence->getArray();
    drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

    for(sal_uInt16 b=0;b<rPolygon.GetPointCount();b++)
    {
        *pInnerSequence++ = awt::Point( rPolygon[b].X(), rPolygon[b].Y() );
        *pInnerFlags++ = (drawing::PolygonFlags)((sal_uInt16)rPolygon.GetFlags(b));
    }
}

///////////////////////////////////////////////////////////////////////

SvxShapeCaption::SvxShapeCaption( SdrObject* pSdrObj ) throw()
: SvxShapeText( pSdrObj, aSvxMapProvider.GetMap(SVXMAP_CAPTION) )
{
}

SvxShapeCaption::~SvxShapeCaption() throw()
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
