/*************************************************************************
 *
 *  $RCSfile: unolayer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-19 09:52:58 $
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

#pragma hdrstop


#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdobj.hxx>

// folgende fuer InsertSdPage()
#ifndef _SVDLAYER_HXX //autogen
#include <svx/svdlayer.hxx>
#endif

#ifndef SVX_LIGHT
#ifndef _SD_DOCSHELL_HXX
#include <docshell.hxx>
#endif
#endif

#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _UNOMODEL_HXX
#include <unomodel.hxx>
#endif
#ifndef _UNOLAYER_HXX
#include <unolayer.hxx>
#endif

#ifndef _SD_UNOPRNMS_HXX
#include "unoprnms.hxx"
#endif

#include "unohelp.hxx"
#include "frmview.hxx"
#include "drviewsh.hxx"
#include "sdview.hxx"

#include "viewshel.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "glob.hrc"

#include "unokywds.hxx"

using namespace ::rtl;
using namespace ::vos;
using namespace ::com::sun::star;

//=============================================================================
// class SdLayer
//=============================================================================

#define WID_LAYER_LOCKED    1
#define WID_LAYER_PRINTABLE 2
#define WID_LAYER_VISIBLE   3
#define WID_LAYER_NAME      4

const SfxItemPropertyMap* ImplGetSdLayerPropertyMap()
{
    static const SfxItemPropertyMap aSdLayerPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_LAYER_LOCKED),      WID_LAYER_LOCKED,   &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_LAYER_PRINTABLE),   WID_LAYER_PRINTABLE,&::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_LAYER_VISIBLE),     WID_LAYER_VISIBLE,  &::getBooleanCppuType(),            0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_LAYER_NAME),        WID_LAYER_NAME,     &::getCppuType((const OUString*)0), 0, 0 },
        { 0,0,0,0,0}
    };

    return aSdLayerPropertyMap_Impl;
}

String SdLayer::convertToInternalName( const OUString& rName )
{
    if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_LayerName_background) ) )
    {
        return String( SdResId( STR_LAYER_BCKGRND ) );
    }
    else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_LayerName_background_objects) ) )
    {
        return  String( SdResId( STR_LAYER_BCKGRNDOBJ ) );
    }
    else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_LayerName_layout) ) )
    {
        return  String( SdResId( STR_LAYER_LAYOUT ) );
    }
    else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_LayerName_controls) ) )
    {
        return  String( SdResId( STR_LAYER_CONTROLS ) );
    }
    else if( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_LayerName_measurelines) ) )
    {
        return  String( SdResId( STR_LAYER_MEASURELINES ) );
    }
    else
    {
        return String( rName );
    }
}

OUString SdLayer::convertToExternalName( const String& rName )
{
    const String aCompare( rName );
    if( rName == String( SdResId( STR_LAYER_BCKGRND ) ) )
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_LayerName_background) );
    }
    else if( rName == String( SdResId( STR_LAYER_BCKGRNDOBJ ) ) )
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_LayerName_background_objects) );
    }
    else if( rName == String( SdResId( STR_LAYER_LAYOUT ) ) )
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_LayerName_layout) );
    }
    else if( rName == String( SdResId( STR_LAYER_CONTROLS ) ) )
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_LayerName_controls) );
    }
    else if( rName == String( SdResId( STR_LAYER_MEASURELINES ) ) )
    {
        return OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_LayerName_measurelines) );
    }
    else
    {
        return OUString( rName );
    }
}

/** */
SdLayer::SdLayer( SdLayerManager* pLayerManager_, SdrLayer* pSdrLayer_ ) throw()
: aPropSet(ImplGetSdLayerPropertyMap()),
  pLayerManager(pLayerManager_), pLayer(pSdrLayer_),
  mxLayerManager(pLayerManager_)
{
}

/** */
SdLayer::~SdLayer() throw()
{
}

// uno helper
UNO3_GETIMPLEMENTATION_IMPL( SdLayer );

// XServiceInfo
OUString SAL_CALL SdLayer::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( OUString::createFromAscii(sUNO_SdLayer) );
}

sal_Bool SAL_CALL SdLayer::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdLayer::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    OUString aServiceName( OUString::createFromAscii(sUNO_Service_DrawingLayer) );
    uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// beans::XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdLayer::getPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return aPropSet.getPropertySetInfo();
}

void SAL_CALL SdLayer::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pLayer == NULL || pLayerManager == NULL)
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    switch( pMap ? pMap->nWID : -1 )
    {
    case WID_LAYER_LOCKED:
    {
        sal_Bool bValue;
        if(!sd::any2bool( aValue, bValue ))
            throw lang::IllegalArgumentException();
        set( LOCKED, bValue );
        break;
    }
    case WID_LAYER_PRINTABLE:
    {
        sal_Bool bValue;
        if(!sd::any2bool( aValue, bValue ))
            throw lang::IllegalArgumentException();
        set( PRINTABLE, bValue );
        break;
    }
    case WID_LAYER_VISIBLE:
    {
        sal_Bool bValue;
        if(!sd::any2bool( aValue, bValue ))
            throw lang::IllegalArgumentException();
        set( VISIBLE, bValue );
        break;
    }
    case WID_LAYER_NAME:
    {
        OUString aName;
        if(!(aValue >>= aName))
            throw lang::IllegalArgumentException();

        pLayer->SetName(SdLayer::convertToInternalName( aName ) );
        pLayerManager->UpdateLayerView();
        break;
    }
    default:
        throw beans::UnknownPropertyException();
        break;
    }

#ifndef SVX_LIGHT
    if( pLayerManager->GetDocShell() )
        pLayerManager->GetDocShell()->SetModified();
#endif
}

uno::Any SAL_CALL SdLayer::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pLayer == NULL || pLayerManager == NULL)
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    uno::Any aValue;

    switch( pMap ? pMap->nWID : -1 )
    {
    case WID_LAYER_LOCKED:
        sd::bool2any( get( LOCKED ), aValue );
        break;
    case WID_LAYER_PRINTABLE:
        sd::bool2any( get( PRINTABLE ), aValue );
        break;
    case WID_LAYER_VISIBLE:
        sd::bool2any( get( VISIBLE ), aValue );
        break;
    case WID_LAYER_NAME:
    {
        OUString aRet( SdLayer::convertToExternalName( pLayer->GetName() ) );
        aValue <<= aRet;
        break;
    }
    default:
        throw beans::UnknownPropertyException();
        break;
    }

    return aValue;
}

void SAL_CALL SdLayer::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdLayer::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdLayer::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdLayer::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

/** */
sal_Bool SdLayer::get( LayerAttribute what ) throw()
{
    if(pLayer&&pLayerManager)
    {
        // Versuch 1. ist eine beliebige Seite geoeffnet?
        SdView *pView = pLayerManager->GetView();
        SdrPageView* pSdrPageView = NULL;
        if(pView)
            pSdrPageView = pView->GetPageViewPvNum(0);

        if(pSdrPageView)
        {
            String aLayerName = pLayer->GetName();
            switch(what)
            {
            case VISIBLE:   return pSdrPageView->IsLayerVisible(aLayerName);
            case PRINTABLE: return pSdrPageView->IsLayerPrintable(aLayerName);
            case LOCKED:    return pSdrPageView->IsLayerLocked(aLayerName);
            }
        }

#ifndef SVX_LIGHT
        // Versuch 2. Info von der FrameView besorgen
        if(pLayerManager->GetDocShell())
        {
            FrameView *pFrameView = pLayerManager->GetDocShell()->GetFrameView();
            if(pFrameView)
                switch(what)
                {
                case VISIBLE:   return pFrameView->GetVisibleLayers().IsSet(pLayer->GetID());
                case PRINTABLE: return pFrameView->GetPrintableLayers().IsSet(pLayer->GetID());
                case LOCKED:    return pFrameView->GetLockedLayers().IsSet(pLayer->GetID());
                }
        }
#endif
    }
    return sal_False; //TODO: uno::Exception?
}

void SdLayer::set( LayerAttribute what, sal_Bool flag ) throw()
{
    if(pLayer&&pLayerManager)
    {
        // Versuch 1. ist eine beliebige Seite geoeffnet?
        SdView *pView = pLayerManager->GetView();
        SdrPageView* pSdrPageView = NULL;
        if(pView)
            pSdrPageView = pView->GetPageViewPvNum(0);

        if(pSdrPageView)
        {
            String aLayerName(pLayer->GetName());
            switch(what)
            {
            case VISIBLE:   pSdrPageView->SetLayerVisible(aLayerName,flag);
                            break;
            case PRINTABLE: pSdrPageView->SetLayerPrintable(aLayerName,flag);
                            break;
            case LOCKED:    pSdrPageView->SetLayerLocked(aLayerName,flag);
                            break;
            }
        }

#ifndef SVX_LIGHT
        // Versuch 2. Info von der FrameView besorgen
        if(pLayerManager->GetDocShell())
        {
            FrameView *pFrameView = pLayerManager->GetDocShell()->GetFrameView();

            if(pFrameView)
            {
                SetOfByte aNewLayers;
                switch(what)
                {
                case VISIBLE:   aNewLayers = pFrameView->GetVisibleLayers();
                                break;
                case PRINTABLE: aNewLayers = pFrameView->GetPrintableLayers();
                                break;
                case LOCKED:    aNewLayers = pFrameView->GetLockedLayers();
                                break;
                }

                aNewLayers.Set(pLayer->GetID(),flag);

                switch(what)
                {
                case VISIBLE:   pFrameView->SetVisibleLayers(aNewLayers);
                                break;
                case PRINTABLE: pFrameView->SetPrintableLayers(aNewLayers);
                                break;
                case LOCKED:    pFrameView->SetLockedLayers(aNewLayers);
                                break;
                }
                return;
            }
        }
#endif
    }
    //TODO: uno::Exception?
}

//=============================================================================
// class SdLayerManager
//=============================================================================

/** */
SdLayerManager::SdLayerManager( SdXImpressDocument& rMyModel ) throw()
:rModel(rMyModel)
{
}

/** */
SdLayerManager::~SdLayerManager() throw()
{
}

// uno helper
UNO3_GETIMPLEMENTATION_IMPL( SdLayerManager );

// XServiceInfo
OUString SAL_CALL SdLayerManager::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( OUString::createFromAscii(sUNO_SdLayerManager) );
}

sal_Bool SAL_CALL SdLayerManager::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdLayerManager::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    OUString aServiceName( OUString::createFromAscii(sUNO_Service_DrawingLayerManager) );
    uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XLayerManager
uno::Reference< drawing::XLayer > SAL_CALL SdLayerManager::insertNewByIndex( sal_Int32 nIndex )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Reference< drawing::XLayer > xLayer;

    if( rModel.pDoc )
    {
        SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();
        sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();
        sal_uInt16 nLayer = nLayerCnt - 2 + 1;
        String aLayerName;

        // Ueberpruefung auf schon vorhandene Namen
        while( aLayerName.Len()==0 || rLayerAdmin.GetLayer( aLayerName, sal_False) )
        {
            aLayerName = String(SdResId(STR_LAYER));
            aLayerName += String::CreateFromInt32( (sal_Int32)nLayer );
            nLayer++;
        }

        SdrLayerAdmin& rLA=rModel.pDoc->GetLayerAdmin();
        const sal_Int32 nMax=rLA.GetLayerCount();
        if (nIndex>nMax) nIndex=nMax;
        xLayer = new SdLayer( this, rLA.NewLayer(aLayerName,(USHORT)nIndex) );
        rModel.SetModified();
    }
    return xLayer;
}

void SAL_CALL SdLayerManager::remove( const uno::Reference< drawing::XLayer >& xLayer )
    throw(container::NoSuchElementException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdLayer* pSdLayer = SdLayer::getImplementation(xLayer);

    if(pSdLayer && GetView())
    {
        const SdrLayer* pSdrLayer = pSdLayer->GetSdrLayer();
        GetView()->DeleteLayer( pSdrLayer->GetName() );

        UpdateLayerView();
    }

    rModel.SetModified();
}

void SAL_CALL SdLayerManager::attachShapeToLayer( const uno::Reference< drawing::XShape >& xShape, const uno::Reference< drawing::XLayer >& xLayer )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(rModel.pDoc==NULL)
        return;

    SdLayer* pSdLayer = SdLayer::getImplementation(xLayer);
    SdrLayer* pSdrLayer = pSdLayer?pSdLayer->GetSdrLayer():NULL;
    if(pSdrLayer==NULL)
        return;

    SvxShape* pShape = SvxShape::getImplementation( xShape );
    SdrObject* pSdrObject = pShape?pShape->GetSdrObject():NULL;

    if(pSdrObject && pSdrLayer )
        pSdrObject->SetLayer(pSdrLayer->GetID());

    rModel.SetModified();
}

uno::Reference< drawing::XLayer > SAL_CALL SdLayerManager::getLayerForShape( const uno::Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Reference< drawing::XLayer >  xLayer;

    if(rModel.pDoc)
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        SdrObject* pObj = pShape?pShape->GetSdrObject():NULL;
        if(pObj)
        {
            SdrLayerID aId = pObj->GetLayer();
            SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();
            xLayer = new SdLayer(this,rLayerAdmin.GetLayerPerID(aId));
        }
    }
    return xLayer;
}

// XIndexAccess
sal_Int32 SAL_CALL SdLayerManager::getCount()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( rModel.pDoc )
    {
        SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();
        return rLayerAdmin.GetLayerCount();
    }

    return 0;
}

uno::Any SAL_CALL SdLayerManager::getByIndex( sal_Int32 nLayer )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( nLayer >= getCount() || nLayer < 0 )
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;

    if( rModel.pDoc )
    {
        SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();
        uno::Reference< drawing::XLayer > xLM( new SdLayer( (SdLayerManager*)this, rLayerAdmin.GetLayer((sal_uInt16)nLayer) ) );
        aAny <<= xLM;
    }
    return aAny;
}


// XNameAccess
uno::Any SAL_CALL SdLayerManager::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Reference< drawing::XLayer >  xLayer;

    uno::Any aAny;

    if( rModel.pDoc )
    {
        SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();
        SdrLayer* pLayer = rLayerAdmin.GetLayer( SdLayer::convertToInternalName( aName ), FALSE );
        if( pLayer == NULL )
            throw container::NoSuchElementException();

        xLayer = new SdLayer( (SdLayerManager*)this, pLayer );
        aAny <<= xLayer;
        return aAny;
    }

    throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdLayerManager::getElementNames()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();
    const sal_uInt16 nLayerCount = rLayerAdmin.GetLayerCount();

    uno::Sequence< OUString > aSeq( nLayerCount );

    OUString* pStrings = aSeq.getArray();
    SdrLayer* pLayer;

    for( sal_uInt16 nLayer = 0; nLayer < nLayerCount; nLayer++ )
    {
        pLayer = rLayerAdmin.GetLayer( nLayer );
        if( pLayer )
            *pStrings++ = SdLayer::convertToExternalName( pLayer->GetName() );
    }

    return aSeq;
}

sal_Bool SAL_CALL SdLayerManager::hasByName( const OUString& aName ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrLayerAdmin& rLayerAdmin = rModel.pDoc->GetLayerAdmin();

    return NULL != rLayerAdmin.GetLayer( SdLayer::convertToInternalName( aName ), FALSE );
}

// XElementAccess
uno::Type SAL_CALL SdLayerManager::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( drawing::XLayer );
}

sal_Bool SAL_CALL SdLayerManager::hasElements() throw(uno::RuntimeException)
{
    return getCount() > 0;
}

/** Falls an den Layern was geaendert wurde sorgt diese Methode dafuer, das
    die Aenderungen auch in der sdbcx::View sichtbar gemacht werden */
void SdLayerManager::UpdateLayerView( sal_Bool modify ) const throw()
{
#ifndef SVX_LIGHT
    if(rModel.pDocShell)
    {
        SdDrawViewShell* pDrViewSh =
            PTR_CAST(SdDrawViewShell, rModel.pDocShell->GetViewShell());

        if(pDrViewSh)
        {
            sal_Bool bLayerMode = pDrViewSh->GetLayerMode();
            pDrViewSh->ChangeEditMode(pDrViewSh->GetEditMode(), !bLayerMode);
            pDrViewSh->ChangeEditMode(pDrViewSh->GetEditMode(), bLayerMode);
        }

        if(modify)
            rModel.pDoc->SetChanged(sal_True);
    }
#endif
}

/** */
SdView* SdLayerManager::GetView() const throw()
{
#ifndef SVX_LIGHT
    if( rModel.pDocShell )
    {
        SdViewShell* pViewSh = rModel.pDocShell->GetViewShell();
        if(pViewSh)
            return pViewSh->GetView();
    }
#endif
    return NULL;
}



