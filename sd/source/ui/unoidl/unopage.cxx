/*************************************************************************
 *
 *  $RCSfile: unopage.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-26 16:02:55 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_PRESENTATIONRANGE_HPP_
#include <com/sun/star/presentation/PresentationRange.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif
#ifndef _TOOLKIT_UNOIFACE_HXX
#include <toolkit/unohlp.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOMODEL_HXX
#include <unomodel.hxx>
#endif
#ifndef _SD_UNOPAGE_HXX
#include <unopage.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SDRESID_HXX
#include <sdresid.hxx>
#endif
#include <glob.hrc>
#ifndef _SD_PAGE_HXX //autogen
#include <sdpage.hxx>
#endif
#ifndef _SD_UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SDATTR_HXX
#include <sdattr.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX //autogen
#include <svx/unoshape.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <comphelper/extract.hxx>

#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _WMF_HXX
#include <svtools/wmf.hxx>
#endif

#include <svx/svdview.hxx>
#include "misc.hxx"
#include "sdview.hxx"
#ifndef SVX_LIGHT
#include "docshell.hxx"
#endif
#include "viewshel.hxx"
#include "drviewsh.hxx"
#include "unoobj.hxx"
#include "res_bmp.hrc"
#include "unokywds.hxx"
#include "unopback.hxx"
#include "unohelp.hxx"

using namespace ::vos;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;

#define WID_PAGE_LEFT   0
#define WID_PAGE_RIGHT  1
#define WID_PAGE_TOP    2
#define WID_PAGE_BOTTOM 3
#define WID_PAGE_WIDTH  4
#define WID_PAGE_HEIGHT 5
#define WID_PAGE_EFFECT 6
#define WID_PAGE_CHANGE 7
#define WID_PAGE_SPEED  8
#define WID_PAGE_NUMBER 9
#define WID_PAGE_ORIENT 10
#define WID_PAGE_LAYOUT 11
#define WID_PAGE_DURATION 12
#define WID_PAGE_LDNAME 13
#define WID_PAGE_LDBITMAP 14
#define WID_PAGE_BACK 15
#define WID_PAGE_PREVIEW 16
#define WID_PAGE_VISIBLE 17
#define WID_PAGE_SOUNDFILE 18
#define WID_PAGE_BACKFULL 19

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

static sal_Char __FAR_DATA sEmptyPageName[sizeof("page")] = "page";

const SfxItemPropertyMap* ImplGetDrawPagePropertyMap( sal_Bool bImpress )
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static const SfxItemPropertyMap aDrawPagePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE( beans::XPropertySet ),                  beans::PropertyAttribute::MAYBEVOID,0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_CHANGE),           WID_PAGE_CHANGE,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_DURATION),         WID_PAGE_DURATION,  &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_EFFECT),           WID_PAGE_EFFECT,    &::getCppuType((const presentation::FadeEffect*)0),     0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LAYOUT),           WID_PAGE_LAYOUT,    &::getCppuType((const sal_Int16*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE( awt::XBitmap),                          beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_SPEED),            WID_PAGE_SPEED,     &::getCppuType((const presentation::AnimationSpeed*)0), 0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_PREVIEW),          WID_PAGE_PREVIEW,   SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_VISIBLE),          WID_PAGE_VISIBLE,   &::getBooleanCppuType(),                        0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_SOUNDFILE),         WID_PAGE_SOUNDFILE, &::getCppuType((const OUString*)0),             0, 0},
        {0,0,0,0,0}
    };

    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static const SfxItemPropertyMap aGraphicPagePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE( beans::XPropertySet),                   beans::PropertyAttribute::MAYBEVOID,0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE(awt::XBitmap),                           beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_PREVIEW),          WID_PAGE_PREVIEW,   SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    if( bImpress )
        return aDrawPagePropertyMap_Impl;
    else
        return aGraphicPagePropertyMap_Impl;
}

const SfxItemPropertyMap* ImplGetMasterPagePropertyMap()
{
    static const SfxItemPropertyMap aMasterPagePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BACKGROUND),       WID_PAGE_BACK,      &ITYPE(beans::XPropertySet),                    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_BOTTOM),           WID_PAGE_BOTTOM,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_LEFT),             WID_PAGE_LEFT,      &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_RIGHT),            WID_PAGE_RIGHT,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_TOP),              WID_PAGE_TOP,       &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_HEIGHT),           WID_PAGE_HEIGHT,    &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     WID_PAGE_LDBITMAP,  &ITYPE(awt::XBitmap),                           beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       WID_PAGE_LDNAME,    &::getCppuType((const OUString*)0),             beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_NUMBER),           WID_PAGE_NUMBER,    &::getCppuType((const sal_Int16*)0),            beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_ORIENTATION),      WID_PAGE_ORIENT,    &::getCppuType((const view::PaperOrientation*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_PAGE_WIDTH),            WID_PAGE_WIDTH,     &::getCppuType((const sal_Int32*)0),            0,  0},
        { MAP_CHAR_LEN("BackgroundFullSize"),           WID_PAGE_BACKFULL,  &::getBooleanCppuType(),                        0, 0},
        {0,0,0,0,0}
    };
    return aMasterPagePropertyMap_Impl;
}

SfxItemPropertyMap aEmptyPropertyMap_Impl[] =
{
    {0,0,0,0,0}
};

UNO3_GETIMPLEMENTATION2_IMPL( SdGenericDrawPage, SvxFmDrawPage );

/***********************************************************************
*                                                                      *
***********************************************************************/
SdGenericDrawPage::SdGenericDrawPage( SdXImpressDocument* _pModel, SdPage* pInPage, const SfxItemPropertyMap* pMap ) throw()
:       SvxFmDrawPage( (SdrPage*) pInPage ),
        mpModel     ( _pModel ),
        maPropSet   ( (pInPage&& (pInPage->GetPageKind() != PK_STANDARD))?&pMap[1]:pMap ),
        SdUnoSearchReplaceShape(this),
        mbHasBackgroundObject(sal_False),
        mrBHelper( maMutex )
{
    mxModel = (::cppu::OWeakObject*)(SvxDrawPage*)mpModel;
}

SdGenericDrawPage::~SdGenericDrawPage() throw()
{
}

// this is called whenever a SdrObject must be created for a empty api shape wrapper
SdrObject * SdGenericDrawPage::_CreateSdrObject( const uno::Reference< drawing::XShape >& xShape ) throw()
{
    if( NULL == pPage || !xShape.is() )
        return NULL;

    String aType( xShape->getShapeType() );
    const String aPrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.") );
    if(aType.CompareTo( aPrefix, aPrefix.Len() ) != 0)
        return SvxFmDrawPage::_CreateSdrObject( xShape );

    aType = aType.Copy( aPrefix.Len() );

    PresObjKind eObjKind = PRESOBJ_NONE;

    if( aType.EqualsAscii( "TitleTextShape" ) )
    {
        eObjKind = PRESOBJ_TITLE;
    }
    else if( aType.EqualsAscii( "OutlinerShape" ) )
    {
        eObjKind = PRESOBJ_OUTLINE;
    }
    else if( aType.EqualsAscii( "SubtitleShape" ) )
    {
        eObjKind = PRESOBJ_TEXT;
    }
    else if( aType.EqualsAscii( "OLE2Shape" ) )
    {
        eObjKind = PRESOBJ_OBJECT;
    }
    else if( aType.EqualsAscii( "ChartShape" ) )
    {
        eObjKind = PRESOBJ_CHART;
    }
    else if( aType.EqualsAscii( "TableShape" ) )
    {
        eObjKind = PRESOBJ_TABLE;
    }
    else if( aType.EqualsAscii( "GraphicObjectShape" ) )
    {
#ifdef STARIMAGE_AVAILABLE
        eObjKind = PRESOBJ_IMAGE;
#else
        eObjKind = PRESOBJ_GRAPHIC;
#endif
    }
    else if( aType.EqualsAscii( "OrgChartShape" ) )
    {
        eObjKind = PRESOBJ_ORGCHART;
    }
    else if( aType.EqualsAscii( "PageShape" ) )
    {
        if( GetPage()->GetPageKind() == PK_NOTES && GetPage()->IsMasterPage() )
            eObjKind = PRESOBJ_TITLE;
        else
            eObjKind = PRESOBJ_PAGE;
    }
    else if( aType.EqualsAscii( "NotesShape" ) )
    {
        eObjKind = PRESOBJ_NOTES;
    }
    else if( aType.EqualsAscii( "HandoutShape" ) )
    {
        eObjKind = PRESOBJ_HANDOUT;
    }

    Rectangle aRect( eObjKind == PRESOBJ_TITLE ? GetPage()->GetTitleRect() : GetPage()->GetLayoutRect()  );

    const awt::Point aPos( aRect.Left(), aRect.Top() );
    xShape->setPosition( aPos );

    const awt::Size aSize( aRect.GetWidth(), aRect.GetHeight() );
    xShape->setSize( aSize );

    SdrObject *pPresObj = GetPage()->CreatePresObj( eObjKind, FALSE, aRect, sal_True );

    if( pPresObj )
        pPresObj->SetUserCall( GetPage() );

    return pPresObj;
}

// XInterface
uno::Any SAL_CALL SdGenericDrawPage::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( beans::XPropertySet );
    else QUERYINT( container::XNamed );
    else QUERYINT( util::XReplaceable );
    else QUERYINT( util::XSearchable );
    else QUERYINT( document::XLinkTargetSupplier );
    else QUERYINT( drawing::XShapeCombiner );
    else QUERYINT( drawing::XShapeBinder );
    else QUERYINT( lang::XComponent );
    else
        return SvxDrawPage::queryInterface( rType );

    return aAny;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdGenericDrawPage::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL SdGenericDrawPage::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( (GetPage() == NULL) || (mpModel == NULL) )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    switch( pMap ? pMap->nWID : -1 )
    {
        case WID_PAGE_LEFT:
        case WID_PAGE_RIGHT:
        case WID_PAGE_TOP:
        case WID_PAGE_BOTTOM:
        case WID_PAGE_LAYOUT:
        case WID_PAGE_DURATION:
        case WID_PAGE_CHANGE:
        {
            sal_Int32 nValue;
            if(!(aValue >>= nValue))
                throw lang::IllegalArgumentException();

            switch( pMap->nWID )
            {
            case WID_PAGE_LEFT:
                SetLftBorder(nValue);
                break;
            case WID_PAGE_RIGHT:
                SetRgtBorder( nValue );
                break;
            case WID_PAGE_TOP:
                SetUppBorder( nValue );
                break;
            case WID_PAGE_BOTTOM:
                SetLwrBorder( nValue );
                break;
            case WID_PAGE_CHANGE:
                GetPage()->SetPresChange( (PresChange)nValue );
                break;
            case WID_PAGE_LAYOUT:
                GetPage()->SetAutoLayout( (AutoLayout)nValue, sal_True );
                break;
            case WID_PAGE_DURATION:
                GetPage()->SetTime((sal_uInt32)nValue);
                break;
            }
            break;
        }
        case WID_PAGE_WIDTH:
        {
            sal_Int32 nWidth;
            if(!(aValue >>= nWidth))
                throw lang::IllegalArgumentException();

            SetWidth( nWidth );
            break;
        }
        case WID_PAGE_HEIGHT:
        {
            sal_Int32 nHeight;
            if(!(aValue >>= nHeight))
                throw lang::IllegalArgumentException();

            SetHeight( nHeight );
            break;
        }
        case WID_PAGE_ORIENT:
        {
            sal_Int32 nEnum;
            if(!::cppu::enum2int( nEnum, aValue ))
                throw lang::IllegalArgumentException();

            Orientation eOri = (((view::PaperOrientation)nEnum) == view::PaperOrientation_PORTRAIT)?ORIENTATION_PORTRAIT:ORIENTATION_LANDSCAPE;

            if( eOri != GetPage()->GetOrientation() )
            {
                SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
                const PageKind ePageKind = GetPage()->GetPageKind();

                USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
                    pPage->SetOrientation( eOri );
                }

                nPageCnt = pDoc->GetSdPageCount(ePageKind);

                for (i = 0; i < nPageCnt; i++)
                {
                    SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
                    pPage->SetOrientation( eOri );
                }
            }
            break;
        }
        case WID_PAGE_EFFECT:
        {
            sal_Int32 nEnum;
            if(!::cppu::enum2int( nEnum, aValue ))
                throw lang::IllegalArgumentException();

            GetPage()->SetFadeEffect( (presentation::FadeEffect)nEnum );
            break;
        }
        case WID_PAGE_BACK:
            setBackground( aValue );
            break;
        case WID_PAGE_SPEED:
        {
            sal_Int32 nEnum;
            if(!::cppu::enum2int( nEnum, aValue ))
                throw lang::IllegalArgumentException();

            GetPage()->SetFadeSpeed( (FadeSpeed) nEnum );
            break;
        }
        case WID_PAGE_VISIBLE :
        {
            sal_Bool    bVisible;
            if( ! ( aValue >>= bVisible ) )
                throw lang::IllegalArgumentException();
            GetPage()->SetExcluded( bVisible == FALSE );
            break;
        }
        case WID_PAGE_SOUNDFILE :
        {
            OUString aURL;
            if( ! ( aValue >>= aURL ) )
                throw lang::IllegalArgumentException();

            GetPage()->SetSoundFile( aURL );
            GetPage()->SetSound( sal_True );
            break;
        }
        case WID_PAGE_BACKFULL:
        {
            sal_Bool    bFullSize;
            if( ! ( aValue >>= bFullSize ) )
                throw lang::IllegalArgumentException();
            GetPage()->SetBackgroundFullSize( bFullSize );
            break;
        }
        default:
            throw beans::UnknownPropertyException();
            break;
    }

    mpModel->SetModified();
}

/***********************************************************************
*                                                                      *
***********************************************************************/
uno::Any SAL_CALL SdGenericDrawPage::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;
    if( (GetPage() == NULL) || (mpModel == NULL) )
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    switch( pMap ? pMap->nWID : -1 )
    {
    case WID_PAGE_LEFT:
        aAny <<= (sal_Int32)( GetPage()->GetLftBorder() );
        break;
    case WID_PAGE_RIGHT:
        aAny <<= (sal_Int32)( GetPage()->GetRgtBorder() );
        break;
    case WID_PAGE_TOP:
        aAny <<= (sal_Int32)( GetPage()->GetUppBorder() );
        break;
    case WID_PAGE_BOTTOM:
        aAny <<= (sal_Int32)( GetPage()->GetLwrBorder() );
        break;
    case WID_PAGE_WIDTH:
        aAny <<= (sal_Int32)( GetPage()->GetSize().getWidth() );
        break;
    case WID_PAGE_HEIGHT:
        aAny <<= (sal_Int32)( GetPage()->GetSize().getHeight() );
        break;
    case WID_PAGE_ORIENT:
        aAny = ::cppu::int2enum( (sal_Int32)((GetPage()->GetOrientation() == ORIENTATION_PORTRAIT)? view::PaperOrientation_PORTRAIT: view::PaperOrientation_LANDSCAPE), ::getCppuType((const view::PaperOrientation*)0) );
        break;
    case WID_PAGE_EFFECT:
        aAny = ::cppu::int2enum( (sal_Int32)GetPage()->GetFadeEffect(), ::getCppuType((const presentation::FadeEffect*)0) );
        break;
    case WID_PAGE_CHANGE:
        aAny <<= (sal_Int32)( GetPage()->GetPresChange() );
        break;
    case WID_PAGE_SPEED:
        aAny = ::cppu::int2enum( (sal_Int32)GetPage()->GetFadeSpeed(), ::getCppuType((const presentation::AnimationSpeed*)0) );
        break;
    case WID_PAGE_LAYOUT:
        aAny <<= (sal_Int16)( GetPage()->GetAutoLayout() );
        break;
    case WID_PAGE_NUMBER:
        aAny <<= (sal_Int16)((sal_uInt16)((GetPage()->GetPageNum()-1)>>1) + 1);
        break;
    case WID_PAGE_DURATION:
        aAny <<= (sal_Int32)(GetPage()->GetTime());
        break;
    case WID_PAGE_LDNAME:
    {
        const OUString aName( GetPage()->GetName() );
        aAny <<= aName;
        break;
    }
    case WID_PAGE_LDBITMAP:
        {
            uno::Reference< awt::XBitmap > xBitmap( VCLUnoHelper::CreateBitmap( BitmapEx( SdResId( BMP_PAGE ) ) ) );
            aAny <<= xBitmap;
        }
        break;
    case WID_PAGE_BACK:
        getBackground( aAny );
        break;
    case WID_PAGE_PREVIEW :
        {
#ifndef SVX_LIGHT
            SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
            if ( pDoc )
            {
                SdDrawDocShell* pDocShell = pDoc->GetDocSh();
                if ( pDocShell )
                {
                    sal_uInt16 nPgNum = 0;
                    sal_uInt16 nPageCount = pDoc->GetSdPageCount( PK_STANDARD );
                    sal_uInt16 nPageNumber = ( (sal_uInt16)( ( GetPage()->GetPageNum() - 1 ) >> 1 ) + 1 );
                    while( nPgNum < nPageCount )
                    {
                        pDoc->SetSelected( pDoc->GetSdPage( nPgNum, PK_STANDARD ), nPgNum == nPageNumber );
                        nPgNum++;
                    }
                    GDIMetaFile* pMetaFile = pDocShell->GetPreviewMetaFile();
                    if ( pMetaFile )
                    {
                        Point   aPoint;
                        Size    aSize( GetPage()->GetSize() );
                        pMetaFile->AddAction( (MetaAction*) new MetaFillColorAction( COL_WHITE, TRUE ), 0 );
                        pMetaFile->AddAction( (MetaAction*) new MetaRectAction( Rectangle( aPoint, aSize ) ), 1 );
                        pMetaFile->SetPrefMapMode( MAP_100TH_MM );
                        pMetaFile->SetPrefSize( aSize );

                        SvMemoryStream aDestStrm( 65535, 65535 );
                        ConvertGDIMetaFileToWMF( *pMetaFile, aDestStrm, NULL, NULL, sal_False );
                        uno::Sequence<sal_Int8> aSeq( (sal_Int8*)aDestStrm.GetData(), aDestStrm.Tell() );
                        aAny <<= aSeq;
                        delete pMetaFile;
                    }
                }
            }
#endif
        }
        break;

    case WID_PAGE_VISIBLE :
    {
        sal_Bool bVisible = GetPage()->IsExcluded() == FALSE;
        aAny <<= uno::Any( &bVisible, ::getBooleanCppuType() );
        break;
    }

    case WID_PAGE_SOUNDFILE :
    {
        OUString aURL;
        if( GetPage()->IsSoundOn() )
            aURL = GetPage()->GetSoundFile();
        aAny <<= aURL;
        break;
    }
    case WID_PAGE_BACKFULL:
    {
        sal_Bool bFullSize = GetPage()->GetBackgroundFullSize();
        aAny = uno::Any( &bFullSize, ::getBooleanCppuType() );
        break;
    }

    default:
        throw beans::UnknownPropertyException();
        break;
    }
    return aAny;
}

void SAL_CALL SdGenericDrawPage::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdGenericDrawPage::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdGenericDrawPage::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdGenericDrawPage::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

uno::Reference< drawing::XShape >  SdGenericDrawPage::_CreateShape( SdrObject *pObj ) const
{
    PresObjKind eKind = GetPage()->GetPresObjKind(pObj);

    SvxShape* pShape = NULL;

    if(pObj->GetObjInventor() == SdrInventor)
    {
        sal_uInt32 nInventor = pObj->GetObjIdentifier();
        switch( nInventor )
        {
        case OBJ_TITLETEXT:
            pShape = new SvxShape( pObj );
            if( GetPage()->GetPageKind() == PK_NOTES && GetPage()->IsMasterPage() )
            {
                // fake a empty PageShape if its a title shape on the master page
                pShape->SetShapeType(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PageShape")));
            }
            else
            {
                pShape->SetShapeType(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TitleTextShape")));
            }
            eKind = PRESOBJ_NONE;
            break;
        case OBJ_OUTLINETEXT:
            pShape = new SvxShape( pObj );
            pShape->SetShapeType(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.OutlinerShape")));
            eKind = PRESOBJ_NONE;
            break;
        }
    }

    uno::Reference< drawing::XShape >  xShape( pShape );

    if(!xShape.is())
        xShape = SvxFmDrawPage::_CreateShape( pObj );


    if( eKind != PRESOBJ_NONE )
    {
        String aShapeType( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation."));

        switch( eKind )
        {
        case PRESOBJ_TITLE:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("TitleTextShape") );
            break;
        case PRESOBJ_OUTLINE:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("OutlinerShape") );
            break;
        case PRESOBJ_TEXT:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("SubtitleShape") );
            break;
        case PRESOBJ_GRAPHIC:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("GraphicObjectShape") );
            break;
        case PRESOBJ_OBJECT:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("OLE2Shape") );
            break;
        case PRESOBJ_CHART:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("ChartShape") );
            break;
        case PRESOBJ_ORGCHART:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("OrgChartShape") );
            break;
        case PRESOBJ_TABLE:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("TableShape") );
            break;
        case PRESOBJ_BACKGROUND:
            DBG_ASSERT( sal_False, "Danger! Someone got hold of the horrible background shape!" );
            break;
        case PRESOBJ_PAGE:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("PageShape") );
            break;
        case PRESOBJ_HANDOUT:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("HandoutShape") );
            break;
        case PRESOBJ_NOTES:
            aShapeType += String( RTL_CONSTASCII_USTRINGPARAM("NotesShape") );
            break;
        }

        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if( pShape )
            pShape->SetShapeType( aShapeType );
    }

    // SdXShape aggregiert SvxShape
    new SdXShape( xShape, mpModel );
    return xShape;
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL SdGenericDrawPage::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxFmDrawPage::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 3, "com.sun.star.drawing.GenericDrawPage",
                                                  "com.sun.star.document.LinkTarget",
                                                  "com.sun.star.document.LinkTargetSupplier");
    return aSeq;
}

// XLinkTargetSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdGenericDrawPage::getLinks(  )
    throw(uno::RuntimeException)
{
    return new SdPageLinkTargets( (SdGenericDrawPage*)this );
}

void SdGenericDrawPage::setBackground( const uno::Any& rValue ) throw()
{
    DBG_ERROR( "Don't call me, I'm useless!" );
}

void SdGenericDrawPage::getBackground( uno::Any& rValue ) throw()
{
    DBG_ERROR( "Don't call me, I'm useless!" );
}

//----------------------------------------------------------------------
uno::Reference< drawing::XShape > SAL_CALL SdGenericDrawPage::combine( const uno::Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(pPage,"SdrPage ist NULL! [CL]");
    DBG_ASSERT(pView, "SdrView ist NULL! [CL]");

    uno::Reference< drawing::XShape > xShape;
    if(GetPage()==NULL||pView==NULL||!xShapes.is()||mpModel==NULL)
        return xShape;

    SdrPageView* pPageView = pView->ShowPage( GetPage(), Point() );

    _SelectObjectsInView( xShapes, pPageView );

    pView->CombineMarkedObjects( sal_False );

    pView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = pView->GetMarkList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        if( pObj )
            xShape = uno::Reference< drawing::XShape >::query( pObj->getUnoShape() );
    }

    pView->HidePage(pPageView);

    mpModel->SetModified();

    return xShape;
}

//----------------------------------------------------------------------
void SAL_CALL SdGenericDrawPage::split( const uno::Reference< drawing::XShape >& xGroup )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(GetPage()==NULL||pView==NULL||!xGroup.is()||mpModel==NULL)
        return;

    SdrPageView* pPageView = pView->ShowPage( GetPage(), Point() );
    _SelectObjectInView( xGroup, pPageView );
    pView->DismantleMarkedObjects( sal_False );
    pView->HidePage(pPageView);

    mpModel->SetModified();
}

//----------------------------------------------------------------------
uno::Reference< drawing::XShape > SAL_CALL SdGenericDrawPage::bind( const uno::Reference< drawing::XShapes >& xShapes )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Reference< drawing::XShape > xShape;
    if(GetPage()==NULL||pView==NULL||!xShapes.is()||mpModel==NULL)
        return xShape;

    SdrPageView* pPageView = pView->ShowPage( GetPage(), Point() );

    _SelectObjectsInView( xShapes, pPageView );

    pView->CombineMarkedObjects( sal_True );

    pView->AdjustMarkHdl();
    const SdrMarkList& rMarkList = pView->GetMarkList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        if( pObj )
            xShape = uno::Reference< drawing::XShape >::query( pObj->getUnoShape() );
    }

    pView->HidePage(pPageView);

    mpModel->SetModified();

    return xShape;
}

//----------------------------------------------------------------------
void SAL_CALL SdGenericDrawPage::unbind( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(GetPage()==NULL||pView==NULL||!xShape.is()||mpModel==NULL)
        return;

    SdrPageView* pPageView = pView->ShowPage( GetPage(), Point() );
    _SelectObjectInView( xShape, pPageView );
    pView->DismantleMarkedObjects( sal_True );
    pView->HidePage(pPageView);

    mpModel->SetModified();
}

void SdGenericDrawPage::SetLftBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetLftBorder() )
    {
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
        const PageKind ePageKind = GetPage()->GetPageKind();

        USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetLftBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetLftBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetRgtBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetRgtBorder() )
    {
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
        const PageKind ePageKind = GetPage()->GetPageKind();

        USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetRgtBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetRgtBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetUppBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetUppBorder() )
    {
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
        const PageKind ePageKind = GetPage()->GetPageKind();

        USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetUppBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetUppBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetLwrBorder( sal_Int32 nValue )
{
    if( nValue != GetPage()->GetLwrBorder() )
    {
        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
        const PageKind ePageKind = GetPage()->GetPageKind();

        USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetLwrBorder( nValue );
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetLwrBorder( nValue );
        }
    }
}

void SdGenericDrawPage::SetWidth( sal_Int32 nWidth )
{
    Size aSize( GetPage()->GetSize() );
    if( aSize.getWidth() != nWidth )
    {
        aSize.setWidth( nWidth );

        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
        const PageKind ePageKind = GetPage()->GetPageKind();

        USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }
    }
}

void SdGenericDrawPage::SetHeight( sal_Int32 nHeight )
{
    Size aSize( GetPage()->GetSize() );
    if( aSize.getHeight() != nHeight )
    {
        aSize.setHeight( nHeight );

        SdDrawDocument* pDoc = (SdDrawDocument*)GetPage()->GetModel();
        const PageKind ePageKind = GetPage()->GetPageKind();

        USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetMasterSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }

        nPageCnt = pDoc->GetSdPageCount(ePageKind);

        for (i = 0; i < nPageCnt; i++)
        {
            SdPage* pPage = pDoc->GetSdPage(i, ePageKind);
            pPage->SetSize(aSize);
        }
    }
}

// XInterface
void SdGenericDrawPage::release() throw()
{
    uno::Reference< uno::XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_decrementInterlockedCount( &m_refCount ) == 0)
        {
            if (! mrBHelper.bDisposed)
            {
                uno::Reference< uno::XInterface > xHoldAlive( (uno::XWeak*)this );
                // First dispose
                try
                {
                    dispose();
                }
                catch(::com::sun::star::uno::Exception&)
                {
                    // release should not throw exceptions
                }

                // only the alive ref holds the object
                OSL_ASSERT( m_refCount == 1 );
                // destroy the object if xHoldAlive decrement the refcount to 0
                return;
            }
        }
        // restore the reference count
        osl_incrementInterlockedCount( &m_refCount );
    }
    OWeakAggObject::release();
}

// XComponent
void SdGenericDrawPage::disposing() throw()
{
    Invalidate();
}

// XComponent
void SdGenericDrawPage::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it rubust, hold a self Reference.
    uno::Reference< lang::XComponent > xSelf( this );

    // Guard dispose against multible threading
    // Remark: It is an error to call dispose more than once
    sal_Bool bDoDispose = sal_False;
    {
    osl::MutexGuard aGuard( mrBHelper.rMutex );
    if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
    {
        // only one call go into this section
        mrBHelper.bInDispose = sal_True;
        bDoDispose = sal_True;
    }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            uno::Reference< uno::XInterface > xSource( uno::Reference< uno::XInterface >::query( (lang::XComponent *)this ) );
            document::EventObject aEvt;
            aEvt.Source = xSource;
            // inform all listeners to release this object
            // The listener container are automaticly cleared
            mrBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(::com::sun::star::uno::Exception& e)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
            throw e;
        }

        // the values bDispose and bInDisposing must set in this order.
        // No multithread call overcome the "!rBHelper.bDisposed && !rBHelper.bInDispose" guard.
        mrBHelper.bDisposed = sal_True;
        mrBHelper.bInDispose = sal_False;
    }
    else
    {
        // in a multithreaded environment, it can't be avoided, that dispose is called twice.
        // However this condition is traced, because it MAY indicate an error.
        OSL_TRACE( "OComponentHelper::dispose() - dispose called twice" );
    }
}

// XComponent
void SAL_CALL SdGenericDrawPage::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    mrBHelper.addListener( ::getCppuType( &aListener ) , aListener );
}

// XComponent
void SAL_CALL SdGenericDrawPage::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    mrBHelper.removeListener( ::getCppuType( &aListener ) , aListener );
}

//========================================================================
// SdPageLinkTargets
//========================================================================

SdPageLinkTargets::SdPageLinkTargets( SdGenericDrawPage* pUnoPage ) throw()
{
    mxPage = pUnoPage;
    mpUnoPage = pUnoPage;
}

SdPageLinkTargets::~SdPageLinkTargets() throw()
{
}

    // XElementAccess
uno::Type SAL_CALL SdPageLinkTargets::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE(beans::XPropertySet);
}

sal_Bool SAL_CALL SdPageLinkTargets::hasElements()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != NULL )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            String aStr( pObj->GetName() );
            if( aStr.Len() )
                return sal_True;
        }
    }

    return sal_False;
}

// container::XNameAccess

// XNameAccess
uno::Any SAL_CALL SdPageLinkTargets::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != NULL )
    {
        SdrObject* pObj = FindObject( aName );
        if( pObj )
        {
            uno::Reference< beans::XPropertySet > aRef( pObj->getUnoShape(), uno::UNO_QUERY );
            aAny <<= aRef;
        }
    }

    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdPageLinkTargets::getElementNames()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_uInt32 nObjCount = 0;

    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage != NULL )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            String aStr( pObj->GetName() );
            if( aStr.Len() )
                nObjCount++;
        }
    }

    uno::Sequence< OUString > aSeq( nObjCount );
    if( nObjCount > 0 )
    {
        OUString* pStr = aSeq.getArray();

        SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            String aStr( pObj->GetName() );
            if( aStr.Len() )
                *pStr++ = aStr;
        }
    }

    return aSeq;
}

sal_Bool SAL_CALL SdPageLinkTargets::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    return FindObject( aName ) != NULL;
}

/***********************************************************************
*                                                                      *
***********************************************************************/
SdrObject* SdPageLinkTargets::FindObject( const String& rName ) const throw()
{
    SdPage* pPage = mpUnoPage->GetPage();
    if( pPage == NULL )
        return NULL;

    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        String aStr( pObj->GetName() );
        if( aStr.Len() && (aStr == rName) )
            return pObj;
    }

    return NULL;
}

// XServiceInfo
OUString SAL_CALL SdPageLinkTargets::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdPageLinkTargets") );
}

sal_Bool SAL_CALL SdPageLinkTargets::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdPageLinkTargets::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    const OUString aSN( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.LinkTargets") );
    uno::Sequence< OUString > aSeq( &aSN, 1);
    return aSeq;
}

//========================================================================
// SdDrawPage
//========================================================================

SdDrawPage::SdDrawPage(  SdXImpressDocument* mpModel, SdPage* pPage ) throw()
: SdGenericDrawPage( mpModel, pPage, ImplGetDrawPagePropertyMap( mpModel->IsImpressDocument() ) )
{
}

SdDrawPage::~SdDrawPage() throw()
{
}

// XInterface
uno::Any SAL_CALL SdDrawPage::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;
    if( rType == ITYPE( drawing::XMasterPageTarget ) )
        aAny <<= uno::Reference< drawing::XMasterPageTarget >( this );
    else if( mpModel && mpModel->IsImpressDocument() &&
             GetPage()  && GetPage()->GetPageKind() != PK_HANDOUT &&
             rType == ITYPE( presentation::XPresentationPage ) )
        aAny <<= uno::Reference< presentation::XPresentationPage >( this );
    else
        return SdGenericDrawPage::queryInterface( rType );

    return aAny;
}

void SAL_CALL SdDrawPage::acquire() throw(uno::RuntimeException)
{
    SvxDrawPage::acquire();
}

void SAL_CALL SdDrawPage::release() throw(uno::RuntimeException)
{
    SvxDrawPage::release();
}

UNO3_GETIMPLEMENTATION2_IMPL( SdDrawPage, SdGenericDrawPage );

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SdDrawPage::getTypes() throw(uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        sal_Bool bPresPage = mpModel && mpModel->IsImpressDocument() && GetPage() && GetPage()->GetPageKind() != PK_HANDOUT;

        const uno::Sequence< uno::Type > aBaseTypes( SdGenericDrawPage::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        const sal_Int32 nOwnTypes = bPresPage ? 11 : 10;        // !DANGER! Keep this updated!

        maTypeSequence.realloc(  nBaseTypes + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ITYPE(drawing::XDrawPage);
        *pTypes++ = ITYPE(beans::XPropertySet);
        *pTypes++ = ITYPE(container::XNamed);
        *pTypes++ = ITYPE(drawing::XMasterPageTarget);
        *pTypes++ = ITYPE(lang::XServiceInfo);
        *pTypes++ = ITYPE(util::XReplaceable);
        *pTypes++ = ITYPE(document::XLinkTargetSupplier);
        *pTypes++ = ITYPE( drawing::XShapeCombiner );
        *pTypes++ = ITYPE( drawing::XShapeBinder );
        *pTypes++ = ITYPE( lang::XComponent );

        if( bPresPage )
            *pTypes++ = ITYPE(presentation::XPresentationPage);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SdDrawPage::getImplementationId() throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

OUString SdDrawPage::getPageApiName( SdPage* pPage )
{
    OUString aPageName;

    if(pPage)
    {
        aPageName = pPage->GetRealName();

        if( aPageName.getLength() == 0 )
        {
            OUStringBuffer sBuffer;
            sBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( sEmptyPageName ) );
            const sal_Int32 nPageNum = ( ( pPage->GetPageNum() - 1 ) >> 1 ) + 1;
            sBuffer.append( nPageNum );
            aPageName = sBuffer.makeStringAndClear();
        }
    }

    return aPageName;
}

OUString SdDrawPage::getPageApiNameFromUiName( const String& rUIName )
{
    OUString aApiName;

    String aDefPageName(SdResId(STR_PAGE));
    aDefPageName += sal_Unicode( ' ' );

    if( rUIName.Equals( aDefPageName, 0, aDefPageName.Len() ) )
    {
        aApiName = OUString( RTL_CONSTASCII_USTRINGPARAM( sEmptyPageName ) );
        aApiName += rUIName.Copy( aDefPageName.Len() );
    }
    else
    {
        aApiName = rUIName;
    }

    return aApiName;
}

String SdDrawPage::getUiNameFromPageApiName( const ::rtl::OUString& rApiName )
{
    const String aDefPageName(RTL_CONSTASCII_USTRINGPARAM( sEmptyPageName ));
    if( rApiName.compareTo( aDefPageName, aDefPageName.Len() ) == 0 )
    {
        OUStringBuffer sBuffer;
        sBuffer.append( String(SdResId(STR_PAGE)) );
        sBuffer.append( sal_Unicode( ' ' ) );
        sBuffer.append( rApiName.copy( aDefPageName.Len() ) );
        return sBuffer.makeStringAndClear();
    }

    return rApiName;
}

// XServiceInfo
OUString SAL_CALL SdDrawPage::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdDrawPage") );
}

uno::Sequence< OUString > SAL_CALL SdDrawPage::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SdGenericDrawPage::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.drawing.DrawPage" );

    if( mpModel && mpModel->IsImpressDocument() )
        SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.DrawPage" );

    return aSeq;
}

sal_Bool SAL_CALL SdDrawPage::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::supportsService( ServiceName );
}

// XNamed
void SAL_CALL SdDrawPage::setName( const OUString& rName )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT( GetPage() && !GetPage()->IsMasterPage(), "Don't call base implementation for masterpages!" );

    if(GetPage() && GetPage()->GetPageKind() != PK_NOTES)
    {
        OUString aName( rName );

        if(aName.compareToAscii( sEmptyPageName, sizeof( sEmptyPageName ) - 1 ) == 0)
        {
            sal_Int32 nPageNumber = aName.copy( sizeof( sEmptyPageName ) - 1 ).toInt32();
            if( nPageNumber == ( ( GetPage()->GetPageNum() - 1 ) >> 1 ) + 1 )
                aName = OUString();
        }

        GetPage()->SetName( aName );

        SdPage* pNotesPage = mpModel->GetDoc()->GetSdPage( (GetPage()->GetPageNum()-1)>>1, PK_NOTES );
        pNotesPage->SetName(aName);

#ifndef SVX_LIGHT
        // fake a mode change to repaint the page tab bar
        SdDrawDocShell* pDocSh = mpModel->GetDocShell();
        SdViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : NULL;
        if( pViewSh && pViewSh->ISA( SdDrawViewShell ) )
        {
            SdDrawViewShell* pDrawViewSh = (SdDrawViewShell*)pViewSh;

            EditMode eMode = pDrawViewSh->GetEditMode();
            if( eMode == EM_PAGE )
            {
                BOOL bLayer = pDrawViewSh->GetLayerMode();

                pDrawViewSh->ChangeEditMode( eMode, !bLayer );
                pDrawViewSh->ChangeEditMode( eMode, bLayer );
            }
        }

        mpModel->SetModified();
#endif
    }
}

OUString SAL_CALL SdDrawPage::getName()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    return getPageApiName( GetPage() );
}

// XMasterPageTarget
uno::Reference< drawing::XDrawPage > SAL_CALL SdDrawPage::getMasterPage(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(mpModel,"SdDrawPage hat kein Model??");
    if(mpModel && GetPage())
    {
        uno::Reference< drawing::XDrawPages > xPages( mpModel->getMasterPages() );
        uno::Reference< drawing::XDrawPage > xPage( (pPage->GetMasterPage(0))->getUnoPage(), uno::UNO_QUERY );
        return xPage;
    }
    return NULL;
}

void SAL_CALL SdDrawPage::setMasterPage( const uno::Reference< drawing::XDrawPage >& xMasterPage )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(mpModel,"SdDrawPage hat kein Model??");
    if(mpModel && pPage)
    {
        SdMasterPage* pMasterPage = SdMasterPage::getImplementation( xMasterPage );
        if( pMasterPage && pMasterPage->isValid() )
        {
            pPage->RemoveMasterPage(0);

            SdPage* pSdPage = (SdPage*) pMasterPage->GetSdrPage();
            sal_uInt16 nPos = pSdPage->GetPageNum();
            pPage->InsertMasterPage(nPos);

            pPage->SetBorder(pSdPage->GetLftBorder(),pSdPage->GetUppBorder(),
                              pSdPage->GetRgtBorder(),pSdPage->GetLwrBorder() );

            pPage->SetSize( pSdPage->GetSize() );
            pPage->SetOrientation( pSdPage->GetOrientation() );

            mpModel->SetModified();
        }

    }
}

// XPresentationPage
uno::Reference< drawing::XDrawPage > SAL_CALL SdDrawPage::getNotesPage()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pPage && mpModel && mpModel->GetDoc() )
    {
        SdPage* pNotesPage = mpModel->GetDoc()->GetSdPage( (pPage->GetPageNum()-1)>>1, PK_NOTES );
        if( pNotesPage )
        {
            uno::Reference< drawing::XDrawPage > xPage( pNotesPage->getUnoPage(), uno::UNO_QUERY );
            return xPage;
        }
    }
    return NULL;
}


// XIndexAccess
sal_Int32 SAL_CALL SdDrawPage::getCount()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::getCount();
}

uno::Any SAL_CALL SdDrawPage::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    return SdGenericDrawPage::getByIndex( Index );
}

// XElementAccess
uno::Type SAL_CALL SdDrawPage::getElementType()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::getElementType();
}

sal_Bool SAL_CALL SdDrawPage::hasElements()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::hasElements();
}

// XShapes
void SAL_CALL SdDrawPage::add( const uno::Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    SdGenericDrawPage::add( xShape );
}

void SAL_CALL SdDrawPage::remove( const uno::Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    SvxShape* pShape = SvxShape::getImplementation( xShape );
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj )
        {
            GetPage()->GetPresObjList()->Remove((void*) pObj);
            pObj->SetUserCall(NULL);
        }
    }

    SdGenericDrawPage::remove( xShape );
}

void SdDrawPage::setBackground( const uno::Any& rValue )
    throw( lang::IllegalArgumentException )
{
    uno::Reference< beans::XPropertySet > xSet;

    if( !(rValue >>= xSet) && !rValue.hasValue() )
        throw lang::IllegalArgumentException();

    if( !xSet.is() )
    {
        // the easy case, clear the background obj
        GetPage()->SetBackgroundObj( NULL );
        return;
    }

    // prepare background object
    SdrObject* pObj = GetPage()->GetBackgroundObj();
    if( NULL == pObj )
    {
        pObj = new SdrRectObj();
        GetPage()->SetBackgroundObj( pObj );
    }

    const sal_Int32 nLeft = GetPage()->GetLftBorder();
    const sal_Int32 nRight = GetPage()->GetRgtBorder();
    const sal_Int32 nUpper = GetPage()->GetUppBorder();
    const sal_Int32 nLower = GetPage()->GetLwrBorder();

    Point aPos ( nLeft, nRight );
    Size aSize( GetPage()->GetSize() );
    aSize.Width()  -= nLeft  + nRight - 1;
    aSize.Height() -= nUpper + nLower - 1;
    Rectangle aRect( aPos, aSize );
    pObj->SetLogicRect( aRect );

    // is it our own implementation?
    SdUnoPageBackground* pBack = SdUnoPageBackground::getImplementation( xSet );

    SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), XATTR_FILL_FIRST, XATTR_FILLRESERVED_LAST );

    if( pBack )
    {
        pBack->fillItemSet( (SdDrawDocument*)GetPage()->GetModel(), aSet );
    }
    else
    {
        SdUnoPageBackground* pBackground = new SdUnoPageBackground();

        uno::Reference< beans::XPropertySetInfo >  xSetInfo( xSet->getPropertySetInfo() );
        uno::Reference< beans::XPropertySet >  xDestSet( (beans::XPropertySet*)pBackground );
        uno::Reference< beans::XPropertySetInfo >  xDestSetInfo( xDestSet->getPropertySetInfo() );

        uno::Sequence< beans::Property > aProperties( xDestSetInfo->getProperties() );
        sal_Int32 nCount = aProperties.getLength();
        beans::Property* pProp = aProperties.getArray();

        while( nCount-- )
        {
            const OUString aPropName( pProp->Name );
            if( xSetInfo->hasPropertyByName( aPropName ) )
                xDestSet->setPropertyValue( aPropName,
                        xSet->getPropertyValue( aPropName ) );

            pProp++;
        }

        pBackground->fillItemSet( (SdDrawDocument*)GetPage()->GetModel(), aSet );
    }

//-/    pObj->NbcSetAttributes( aSet, sal_False );
    if( aSet.Count() == 0 )
    {
        GetPage()->SetBackgroundObj( NULL );
    }
    else
    {
        pObj->SetItemSet(aSet);
    }

    pPage->SendRepaintBroadcast();
}

void SdDrawPage::getBackground( uno::Any& rValue ) throw()
{
    SdrObject* pObj = GetPage()->GetBackgroundObj();
    if( NULL == pObj )
    {
        rValue.clear();
    }
    else
    {
        uno::Reference< beans::XPropertySet > xSet( new SdUnoPageBackground( GetModel()->GetDoc(), pObj ) );
        rValue <<= xSet;
    }
}

//========================================================================
// class SdMasterPage
//========================================================================

SdMasterPage::SdMasterPage( SdXImpressDocument* mpModel, SdPage* pPage ) throw()
: SdGenericDrawPage( mpModel, pPage, ImplGetMasterPagePropertyMap() ),
  mpBackgroundObj(NULL)
{
    if( pPage && GetPage()->GetPageKind() == PK_STANDARD )
    {
        sal_uInt32 nMasterIndex = 0;
        sal_uInt32 nMasterCount = GetPage()->GetPresObjList()->Count();

        for (nMasterIndex = 0; nMasterIndex < nMasterCount; nMasterIndex++)
        {
            // loop over all presentation objects in the masterpage
            SdrObject* pMasterObj = (SdrObject*) GetPage()->GetPresObjList()->GetObject(nMasterIndex);

            if (pMasterObj && pMasterObj->GetObjInventor() == SdrInventor)
            {
                sal_uInt16 nId = pMasterObj->GetObjIdentifier();

                if (nId == OBJ_RECT && pMasterObj->IsEmptyPresObj() )
                {
                    mpBackgroundObj = pMasterObj;
                    break;
                }
            }
        }

        mbHasBackgroundObject = sal_True;
    }
}

SdMasterPage::~SdMasterPage() throw()
{
}

// XInterface
uno::Any SAL_CALL SdMasterPage::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    if( rType == ITYPE( container::XIndexAccess ) )
        aAny <<= uno::Reference< container::XIndexAccess >((presentation::XPresentationPage*)(this));
    else if( rType == ITYPE( container::XElementAccess ) )
        aAny <<=  uno::Reference< container::XElementAccess >((presentation::XPresentationPage*)(this));
    else if( rType == ITYPE( container::XNamed ) )
        aAny <<=  uno::Reference< container::XNamed >(this);
    else if( rType == ITYPE( presentation::XPresentationPage ) &&
             ( mpModel && mpModel->IsImpressDocument() &&
               pPage  && GetPage()->GetPageKind() != PK_HANDOUT) )
        aAny <<= uno::Reference< presentation::XPresentationPage >( this );
    else
        return SdGenericDrawPage::queryInterface( rType );

    return aAny;
}

void SAL_CALL SdMasterPage::acquire() throw(uno::RuntimeException)
{
    SvxDrawPage::acquire();
}

void SAL_CALL SdMasterPage::release() throw(uno::RuntimeException)
{
    SvxDrawPage::release();
}

UNO3_GETIMPLEMENTATION2_IMPL( SdMasterPage, SdGenericDrawPage );

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SdMasterPage::getTypes() throw(uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        sal_Bool bPresPage = mpModel && mpModel->IsImpressDocument() && pPage && GetPage()->GetPageKind() != PK_HANDOUT;

        const uno::Sequence< uno::Type > aBaseTypes( SdGenericDrawPage::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        const sal_Int32 nOwnTypes = bPresPage ? 9 : 8;      // !DANGER! Keep this updated!

        maTypeSequence.realloc(  nBaseTypes + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ITYPE(drawing::XDrawPage);
        *pTypes++ = ITYPE(beans::XPropertySet);
        *pTypes++ = ITYPE(container::XNamed);
        *pTypes++ = ITYPE(lang::XServiceInfo);
        *pTypes++ = ITYPE(util::XReplaceable);
        *pTypes++ = ITYPE(document::XLinkTargetSupplier);
        *pTypes++ = ITYPE( drawing::XShapeCombiner );
        *pTypes++ = ITYPE( drawing::XShapeBinder );

        if( bPresPage )
            *pTypes++ = ITYPE(presentation::XPresentationPage);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SdMasterPage::getImplementationId() throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XServiceInfo
OUString SAL_CALL SdMasterPage::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdMasterPage") );
}

uno::Sequence< OUString > SAL_CALL SdMasterPage::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SdGenericDrawPage::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.drawing.MasterPage" );

    return aSeq;
}

sal_Bool SAL_CALL SdMasterPage::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::supportsService( ServiceName );
}

// XElementAccess
sal_Bool SAL_CALL SdMasterPage::hasElements() throw(uno::RuntimeException)
{
    if( pPage == NULL )
        return sal_False;

    return (pPage->GetObjCount() > 1) || (!mbHasBackgroundObject && pPage->GetObjCount() == 1 );
}

uno::Type SAL_CALL SdMasterPage::getElementType()
    throw(uno::RuntimeException)
{
    return SdGenericDrawPage::getElementType();
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPage::getCount()
    throw(uno::RuntimeException)
{
    sal_Int32 nCount = SdGenericDrawPage::getCount();
    if( mbHasBackgroundObject )
        nCount--;

    return nCount;
}

uno::Any SAL_CALL SdMasterPage::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mbHasBackgroundObject )
        Index++;

    return SdGenericDrawPage::getByIndex(Index);
}

// intern
void SdMasterPage::setBackground( const uno::Any& rValue )
    throw( lang::IllegalArgumentException )
{
    // we need at least an beans::XPropertySet
    uno::Reference< beans::XPropertySet > xSet;

    rValue >>= xSet;

    if( !xSet.is() )
        throw lang::IllegalArgumentException();

    if( mpModel && mpModel->IsImpressDocument() )
    {
        uno::Reference< container::XNameAccess >  xFamilies = mpModel->getStyleFamilies();
        uno::Any aAny( xFamilies->getByName( getName() ) );

        uno::Reference< container::XNameAccess > xFamily;

        aAny >>= xFamily;

        if( xFamily.is() )
        {
            OUString aStyleName( OUString::createFromAscii(sUNO_PseudoSheet_Background) );

            try
            {
                aAny = xFamily->getByName( aStyleName );

                uno::Reference< style::XStyle >  xStyle( *(uno::Reference< style::XStyle > *)aAny.getValue() );
                uno::Reference< beans::XPropertySet >  xStyleSet( xStyle, uno::UNO_QUERY );
                if( xStyleSet.is() )
                {
                    uno::Reference< beans::XPropertySetInfo >  xSetInfo( xSet->getPropertySetInfo() );
                    uno::Reference< beans::XPropertyState > xSetStates( xSet, uno::UNO_QUERY );

                    const SfxItemPropertyMap* pMap = ImplGetPageBackgroundPropertyMap();
                    while( pMap->pName )
                    {
                        const OUString aPropName( OUString::createFromAscii(pMap->pName) );
                        if( xSetInfo->hasPropertyByName( aPropName ) )
                        {
                            if( !xSetStates.is() || xSetStates->getPropertyState( aPropName ) == beans::PropertyState_DIRECT_VALUE )
                                xStyleSet->setPropertyValue( aPropName,
                                        xSet->getPropertyValue( aPropName ) );
                            else
                                xSetStates->setPropertyToDefault( aPropName );
                        }

                        ++pMap;
                    }
                }
            }
            catch(...)
            {
                //
            }
        }
    }
    else
    {
        // first fill an item set
        // is it our own implementation?
        SdUnoPageBackground* pBack = SdUnoPageBackground::getImplementation( xSet );

        SfxItemSet aSet( GetModel()->GetDoc()->GetPool(), XATTR_FILL_FIRST, XATTR_FILLRESERVED_LAST );

        if( pBack )
        {
            pBack->fillItemSet( (SdDrawDocument*)GetPage()->GetModel(), aSet );
        }
        else
        {
            SdUnoPageBackground* pBackground = new SdUnoPageBackground();

            uno::Reference< beans::XPropertySetInfo >  xSetInfo( xSet->getPropertySetInfo() );
            uno::Reference< beans::XPropertySet >  xDestSet( (beans::XPropertySet*)pBackground );
            uno::Reference< beans::XPropertySetInfo >  xDestSetInfo( xDestSet->getPropertySetInfo() );

            uno::Sequence< beans::Property> aProperties( xDestSetInfo->getProperties() );
            sal_Int32 nCount = aProperties.getLength();
            beans::Property* pProp = aProperties.getArray();

            while( nCount-- )
            {
                const OUString aPropName( pProp->Name );
                if( xSetInfo->hasPropertyByName( aPropName ) )
                    xDestSet->setPropertyValue( aPropName,
                            xSet->getPropertyValue( aPropName ) );

                pProp++;
            }

            pBackground->fillItemSet( (SdDrawDocument*)pPage->GetModel(), aSet );
        }

        // if we find the background style, copy the set to the background
        SdDrawDocument* pDoc = (SdDrawDocument*)pPage->GetModel();
        SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = NULL;
        if(pSSPool)
        {
            String aStr(SdResId(STR_PSEUDOSHEET_BACKGROUND));
            pStyleSheet = pSSPool->Find( aStr, SFX_STYLE_FAMILY_PSEUDO);

            if( pStyleSheet )
            {
                SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
                rStyleSet.Put( aSet );
                pPage->SendRepaintBroadcast();
                return;
            }
        }


        // if no background style is available, try the background object
        SdrObject* pObj = GetPage()->GetPresObj(PRESOBJ_BACKGROUND);
        if( pObj == NULL )
            return;

        pObj->SetItemSet(aSet);

        pPage->SendRepaintBroadcast();
    }
}

void SdMasterPage::getBackground( uno::Any& rValue ) throw()
{
    if( mpModel == NULL )
        return;

    if( mpModel->IsImpressDocument() )
    {
        try
        {
            uno::Reference< container::XNameAccess >  xFamilies( mpModel->getStyleFamilies() );
            uno::Any aAny( xFamilies->getByName( getName() ) );
            uno::Reference< container::XNameAccess >  xFamily( *(uno::Reference< container::XNameAccess >*)aAny.getValue() );

            const OUString aStyleName( OUString::createFromAscii(sUNO_PseudoSheet_Background) );
            aAny = xFamily->getByName( aStyleName );
            uno::Reference< style::XStyle >  xStyle( *(uno::Reference< style::XStyle > *)aAny.getValue() );

            uno::Reference< beans::XPropertySet >  xStyleSet( xStyle, uno::UNO_QUERY );
            rValue <<= xStyleSet;
        }
        catch(...)
        {
            rValue.clear();
        }
    }
    else
    {
        SdDrawDocument* pDoc = (SdDrawDocument*)pPage->GetModel();
        SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = NULL;
        if(pSSPool)
        {
            String aStr(SdResId(STR_PSEUDOSHEET_BACKGROUND));
            pStyleSheet = pSSPool->Find( aStr, SFX_STYLE_FAMILY_PSEUDO);

            if( pStyleSheet )
            {
                SfxItemSet aStyleSet( pStyleSheet->GetItemSet());
                if( aStyleSet.Count() )
                {
                    uno::Reference< beans::XPropertySet >  xSet( new SdUnoPageBackground( pDoc, &aStyleSet ) );
                    rValue <<= xSet;
                    return;
                }
            }
        }

        // no stylesheet? try old fashion background rectangle
        SdrObject* pObj = NULL;
        if( pPage->GetObjCount() >= 1 )
        {
            pObj = pPage->GetObj(0);
            if( pObj->GetObjInventor() != SdrInventor || pObj->GetObjIdentifier() != OBJ_RECT )
                pObj = NULL;
        }

        if( pObj )
        {
            uno::Reference< beans::XPropertySet >  xSet( new SdUnoPageBackground( GetModel()->GetDoc(), pObj ) );
            rValue <<= xSet;
            return;
        }


        rValue.clear();
    }
}

// XNamed
void SAL_CALL SdMasterPage::setName( const OUString& aName )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pPage && GetPage()->GetPageKind() != PK_NOTES)
    {
        String aNewName( aName );
        GetPage()->SetName( aNewName );

        if(mpModel->GetDoc())
            mpModel->GetDoc()->RenameLayoutTemplate(GetPage()->GetLayoutName(), aNewName);

#ifndef SVX_LIGHT
        // fake a mode change to repaint the page tab bar
        SdDrawDocShell* pDocSh = mpModel->GetDocShell();
        SdViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : NULL;
        if( pViewSh && pViewSh->ISA( SdDrawViewShell ) )
        {
            SdDrawViewShell* pDrawViewSh = (SdDrawViewShell*)pViewSh;

            EditMode eMode = pDrawViewSh->GetEditMode();
            if( eMode == EM_MASTERPAGE )
            {
                BOOL bLayer = pDrawViewSh->GetLayerMode();

                pDrawViewSh->ChangeEditMode( eMode, !bLayer );
                pDrawViewSh->ChangeEditMode( eMode, bLayer );
            }
        }
#endif

        mpModel->SetModified();
    }
}

OUString SAL_CALL SdMasterPage::getName(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pPage)
    {
        String aLayoutName( GetPage()->GetLayoutName() );
        aLayoutName = aLayoutName.Erase(aLayoutName.Search( String( RTL_CONSTASCII_USTRINGPARAM((SD_LT_SEPARATOR)))));

        return aLayoutName;
    }

    return OUString();
}

// XPresentationPage
uno::Reference< drawing::XDrawPage > SAL_CALL SdMasterPage::getNotesPage()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pPage && mpModel && mpModel->GetDoc() )
    {
        SdPage* pNotesPage = mpModel->GetDoc()->GetMasterSdPage( (pPage->GetPageNum()-1)>>1, PK_NOTES );
        if( pNotesPage )
        {
            uno::Reference< drawing::XDrawPage > xPage( pNotesPage->getUnoPage(), uno::UNO_QUERY );
            return xPage;
        }
    }
    return NULL;
}

// XShapes
void SAL_CALL SdMasterPage::add( const uno::Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    SdGenericDrawPage::add( xShape );
}

void SAL_CALL SdMasterPage::remove( const uno::Reference< drawing::XShape >& xShape ) throw(uno::RuntimeException)
{
    SvxShape* pShape = SvxShape::getImplementation( xShape );
    if( pShape )
    {
        SdrObject* pObj = pShape->GetSdrObject();
        if( pObj )
            GetPage()->GetPresObjList()->Remove((void*) pObj);
    }

    SdGenericDrawPage::remove( xShape );
}


uno::Reference< uno::XInterface > createUnoPageImpl( SdPage* pPage )
{
    uno::Reference< uno::XInterface > xPage;

    if( pPage && pPage->GetModel() )
    {
        SdXImpressDocument* pModel = SdXImpressDocument::getImplementation( pPage->GetModel()->getUnoModel() );
        if( pModel )
        {
            if( pPage->IsMasterPage() )
            {
                xPage = (::cppu::OWeakObject*)new SdMasterPage( pModel, pPage );
            }
            else
            {
                xPage = (::cppu::OWeakObject*)new SdDrawPage( pModel, pPage );
            }
        }
    }

    return xPage;
}

