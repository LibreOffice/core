/*************************************************************************
 *
 *  $RCSfile: shapeuno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-27 15:27:37 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <comphelper/uno3.hxx>
#include <svtools/unoevent.hxx>
#include <svtools/unoimap.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoshape.hxx>

#include <com/sun/star/drawing/XShape.hpp>

#include "shapeuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "unonames.hxx"
#include "unoguard.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetShapeMap()
{
    static SfxItemPropertyMap aShapeMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_IMAGEMAP), 0, &getCppuType((uno::Reference<container::XIndexContainer>*)0), 0, 0 },
        {0,0,0,0}
    };
    return aShapeMap_Impl;
}

const SvEventDescription* lcl_GetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { 0, NULL }
    };
    return aMacroDescriptionsImpl;
}

//------------------------------------------------------------------------

ScShapeObj::ScShapeObj( uno::Reference<drawing::XShape>& xShape )
{
    comphelper::increment( m_refCount );

    mxShapeAgg = uno::Reference<uno::XAggregation>( xShape, uno::UNO_QUERY );
    if (mxShapeAgg.is())
    {
        xShape = NULL;      // during setDelegator, mxShapeAgg must be the only ref

        mxShapeAgg->setDelegator( (cppu::OWeakObject*)this );

        xShape = uno::Reference<drawing::XShape>( mxShapeAgg, uno::UNO_QUERY );
    }

    comphelper::decrement( m_refCount );
}

ScShapeObj::~ScShapeObj()
{
//  if (mxShapeAgg.is())
//      mxShapeAgg->setDelegator(uno::Reference<uno::XInterface>());
}

// XInterface

uno::Any SAL_CALL ScShapeObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( beans::XPropertyState )
    SC_QUERYINTERFACE( text::XTextContent )
    SC_QUERYINTERFACE( lang::XComponent )
    SC_QUERYINTERFACE( lang::XTypeProvider )

    uno::Any aRet = OWeakObject::queryInterface( rType );
    if ( !aRet.hasValue() && mxShapeAgg.is() )
        aRet = mxShapeAgg->queryAggregation( rType );

    return aRet;
}

void SAL_CALL ScShapeObj::acquire() throw(uno::RuntimeException)
{
    OWeakObject::acquire();
}

void SAL_CALL ScShapeObj::release() throw(uno::RuntimeException)
{
    OWeakObject::release();
}

uno::Reference<beans::XPropertySet> lcl_GetPropertySet( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<beans::XPropertySet> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<beans::XPropertySet>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<beans::XPropertyState> lcl_GetPropertyState( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<beans::XPropertyState> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<beans::XPropertyState>*) 0) ) >>= xRet;
    return xRet;
}

uno::Reference<lang::XComponent> lcl_GetComponent( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<lang::XComponent> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<lang::XComponent>*) 0) ) >>= xRet;
    return xRet;
}

//  XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScShapeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  mix own and aggregated properties:
    uno::Reference<beans::XPropertySetInfo> xRet;
    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
    {
        uno::Reference<beans::XPropertySetInfo> xAggInfo = xAggProp->getPropertySetInfo();
        const uno::Sequence<beans::Property> aPropSeq = xAggInfo->getProperties();
        xRet = new SfxExtItemPropertySetInfo( lcl_GetShapeMap(), aPropSeq );
    }
    return xRet;
}

ScDocument* lcl_GetDocument( SdrObject* pObj )
{
    if( pObj )
    {
        ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
        if ( pModel )
            return pModel->GetDocument();
    }
    return NULL;
}

void SAL_CALL ScShapeObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ImageMap aImageMap;
            uno::Reference< uno::XInterface > xImageMapInt;
            aValue >>= xImageMapInt;

            if( !xImageMapInt.is() || !SvUnoImageMap_fillImageMap( xImageMapInt, aImageMap ) )
                throw lang::IllegalArgumentException();

            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(pObj);
            if( pIMapInfo )
            {
                // replace existing image map
                pIMapInfo->SetImageMap( aImageMap );
            }
            else
            {
                // insert new user data with image map
                pObj->InsertUserData(new ScIMapInfo(aImageMap) );
            }
        }
    }
    else
    {
        uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
        if ( xAggProp.is() )
            xAggProp->setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL ScShapeObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        uno::Reference< uno::XInterface > xImageMap;
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(GetSdrObject());
            if( pIMapInfo )
            {
                const ImageMap& rIMap = pIMapInfo->GetImageMap();
                xImageMap = SvUnoImageMap_createInstance( rIMap, lcl_GetSupportedMacroItems() );
            }
            else
                xImageMap = SvUnoImageMap_createInstance( lcl_GetSupportedMacroItems() );
        }
        aAny <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
    }
    else
    {
        uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
        if ( xAggProp.is() )
            aAny = xAggProp->getPropertyValue( aPropertyName );
    }

    return aAny;
}

void SAL_CALL ScShapeObj::addPropertyChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->addPropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::removePropertyChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::addVetoableChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->addVetoableChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::removeVetoableChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<beans::XPropertySet> xAggProp = lcl_GetPropertySet(mxShapeAgg);
    if ( xAggProp.is() )
        xAggProp->removeVetoableChangeListener( aPropertyName, aListener );
}

//  XPropertyState

beans::PropertyState SAL_CALL ScShapeObj::getPropertyState( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! mix own and aggregated properties

    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    uno::Reference<beans::XPropertyState> xAggState = lcl_GetPropertyState(mxShapeAgg);
    if ( xAggState.is() )
        eRet = xAggState->getPropertyState( aPropertyName );

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScShapeObj::getPropertyStates(
                                const uno::Sequence<rtl::OUString>& aPropertyNames )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //  simple loop to get own and aggregated states

    const rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScShapeObj::setPropertyToDefault( const rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! mix own and aggregated properties

    uno::Reference<beans::XPropertyState> xAggState = lcl_GetPropertyState(mxShapeAgg);
    if ( xAggState.is() )
        xAggState->setPropertyToDefault( aPropertyName );
}

uno::Any SAL_CALL ScShapeObj::getPropertyDefault( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! mix own and aggregated properties

    uno::Any aAny;
    uno::Reference<beans::XPropertyState> xAggState = lcl_GetPropertyState(mxShapeAgg);
    if ( xAggState.is() )
        aAny = xAggState->getPropertyDefault( aPropertyName );

    return aAny;
}

// XTextContent

void SAL_CALL ScShapeObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    throw lang::IllegalArgumentException();     // anchor cannot be changed
}

BOOL lcl_GetPageNum( SdrPage* pPage, SdrModel& rModel, USHORT& rNum )
{
    USHORT nCount = rModel.GetPageCount();
    for (USHORT i=0; i<nCount; i++)
        if ( rModel.GetPage(i) == pPage )
        {
            rNum = i;
            return TRUE;
        }

    return FALSE;
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getAnchor() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<text::XTextRange> xRet;

    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
        SdrPage* pPage = pObj->GetPage();
        if ( pModel )
        {
            ScDocument* pDoc = pModel->GetDocument();
            if ( pDoc )
            {
                SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                if ( pObjSh && pObjSh->ISA(ScDocShell) )
                {
                    ScDocShell* pDocSh = (ScDocShell*)pObjSh;

                    USHORT nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        Point aPos = pObj->GetBoundRect().TopLeft();
                        ScRange aRange = pDoc->GetRange( nTab, Rectangle( aPos, aPos ) );

                        //  anchor is always the cell

                        xRet = new ScCellObj( pDocSh, aRange.aStart );
                    }
                }
            }
        }
    }

    return xRet;
}

// XComponent

void SAL_CALL ScShapeObj::dispose() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp = lcl_GetComponent(mxShapeAgg);
    if ( xAggComp.is() )
        xAggComp->dispose();
}

void SAL_CALL ScShapeObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp = lcl_GetComponent(mxShapeAgg);
    if ( xAggComp.is() )
        xAggComp->addEventListener(xListener);
}

void SAL_CALL ScShapeObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp = lcl_GetComponent(mxShapeAgg);
    if ( xAggComp.is() )
        xAggComp->removeEventListener(xListener);
}

// XTypeProvider

uno::Sequence<uno::Type> SAL_CALL ScShapeObj::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypeSequence;

    uno::Reference<lang::XTypeProvider> xBaseProvider;
    if ( mxShapeAgg.is() )
        mxShapeAgg->queryAggregation( getCppuType((uno::Reference<lang::XTypeProvider>*) 0) ) >>= xBaseProvider;

    DBG_ASSERT( xBaseProvider.is(), "ScShapeObj: No XTypeProvider from aggregated shape!" );
    if( xBaseProvider.is() )
    {
        aTypeSequence = xBaseProvider->getTypes();
        long nBaseLen = aTypeSequence.getLength();

        aTypeSequence.realloc( nBaseLen + 1 );
        uno::Type* pPtr = aTypeSequence.getArray();
        pPtr[nBaseLen + 0] = getCppuType((const uno::Reference<text::XTextContent>*)0);
    }
    return aTypeSequence;
}

uno::Sequence<sal_Int8> SAL_CALL ScShapeObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    //! an id must be generated for each unique implementation id of the aggregated object

    return uno::Sequence<sal_Int8>(0);      // no id available
}

SdrObject* ScShapeObj::GetSdrObject() const throw()
{
    if(mxShapeAgg.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( mxShapeAgg );
        if(pShape)
            return pShape->GetSdrObject();
    }

    return NULL;
}

