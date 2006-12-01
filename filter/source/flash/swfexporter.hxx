/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swfexporter.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:26:01 $
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
#ifndef _FLASH_EXPORTER_HXX
#define _FLASH_EXPORTER_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <vector>
#include <map>

#include <stdio.h>

typedef ::std::map<sal_uInt32, sal_uInt16> ChecksumCache;

class GDIMetaFile;

inline ::rtl::OUString STR(const sal_Char * in)
{
    return ::rtl::OUString::createFromAscii(in);
}

inline ::rtl::OUString VAL(sal_Int32 in)
{
    return ::rtl::OUString::valueOf(in);
}

namespace swf {

class Writer;
// -----------------------------------------------------------------------------

class ShapeInfo
{
public:
    sal_uInt16      mnID;                   // the character id for the sprite definition of this shape

    sal_Int32       mnX;
    sal_Int32       mnY;

    sal_Int32       mnWidth;
    sal_Int32       mnHeight;

    ::com::sun::star::presentation::AnimationEffect meEffect;
    ::com::sun::star::presentation::AnimationEffect meTextEffect;
    ::com::sun::star::presentation::AnimationSpeed  meEffectSpeed;

    sal_Int32       mnPresOrder;

    ::com::sun::star::presentation::ClickAction     meClickAction;
    ::rtl::OUString maBookmark;

    sal_Int32       mnDimColor;
    sal_Bool        mbDimHide;
    sal_Bool        mbDimPrev;

    sal_Bool        mbSoundOn;
    sal_Bool        mbPlayFull;
    ::rtl::OUString maSoundURL;

    sal_Int32       mnBlueScreenColor;

    ShapeInfo() :
        mnID(0), mnX(0), mnY(0),
        meEffect( ::com::sun::star::presentation::AnimationEffect_NONE ),
        meTextEffect( ::com::sun::star::presentation::AnimationEffect_NONE ),
        meEffectSpeed( ::com::sun::star::presentation::AnimationSpeed_MEDIUM ),
        mnPresOrder( 0 ),
        meClickAction( ::com::sun::star::presentation::ClickAction_NONE ),
        mnDimColor( 0 ),
        mbDimHide( false ),
        mbDimPrev( false ),
        mbSoundOn( false ),
        mbPlayFull( false ),
        mnBlueScreenColor( 0 ) {}
};

typedef ::std::vector<ShapeInfo*> ShapeInfoVector;

// -----------------------------------------------------------------------------

struct ShapeAnimationInfo
{
    ShapeInfo* mpShapeInfo;
    sal_uInt16  mnDepth;

    ShapeAnimationInfo( ShapeInfo* pShapeInfo, sal_uInt16 nDepth ) : mpShapeInfo( pShapeInfo ), mnDepth( nDepth ) {}
};

typedef std::vector<ShapeAnimationInfo> ShapeAnimationInfoVector;

// -----------------------------------------------------------------------------

struct PageInfo
{
    ::com::sun::star::presentation::FadeEffect      meFadeEffect;
    ::com::sun::star::presentation::AnimationSpeed  meFadeSpeed;

    sal_Int32       mnDuration;
    sal_Int32       mnChange;

    sal_uInt16      mnBackgroundID;
    sal_uInt16      mnObjectsID;
    sal_uInt16      mnForegroundID;

    sal_Bool mbBackgroundVisible;
    sal_Bool mbBackgroundObjectsVisible;

    ShapeInfoVector maShapesVector;

    PageInfo();
    ~PageInfo();

    void addShape( ShapeInfo* pShapeInfo );

};

// -----------------------------------------------------------------------------

typedef ::std::map<sal_uInt32, PageInfo> PageInfoMap;

// -----------------------------------------------------------------------------

class FlashExporter
{
public:
    FlashExporter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF, sal_Int32 nJPEGCompressMode = -1, sal_Bool bExportOLEAsJPEG = false);
    ~FlashExporter();

    void Flush();

    sal_Bool exportAll( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xDoc, com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream,    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator> &xStatusIndicator );
    sal_Bool exportSlides( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage, com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream, sal_uInt16 nPage);
    sal_uInt16 exportBackgrounds( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage, com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream, sal_uInt16 nPage, sal_Bool bExportObjects );
    sal_uInt16 exportBackgrounds( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage, sal_uInt16 nPage, sal_Bool bExportObjects );

#ifdef AUGUSTUS
    sal_Bool exportSound( com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream, const char* wavfilename );
#endif

    ChecksumCache gMasterCache;
    ChecksumCache gPrivateCache;
    ChecksumCache gObjectCache;
    ChecksumCache gMetafileCache;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XExporter > mxGraphicExporter;

    PageInfoMap maPagesMap;

    sal_uInt16 exportDrawPageBackground(sal_uInt16 nPage, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage);
    sal_uInt16 exportMasterPageObjects(sal_uInt16 nPage, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xMasterPage);

    void exportDrawPageContents( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, bool bStream, bool bMaster  );
    void exportShapes( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes, bool bStream, bool bMaster );
    void exportShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, bool bMaster);

    sal_uInt32 ActionSummer(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape);
    sal_uInt32 ActionSummer(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes);

    void animateShape( ShapeAnimationInfo& rAnimInfo );
    void animatePage( PageInfo* pPageInfo );

    bool getMetaFile( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&xComponent, GDIMetaFile& rMtf, bool bOnlyBackground = false, bool bExportAsJPEG = false );

    Writer* mpWriter;

    sal_Int32 mnDocWidth;
    sal_Int32 mnDocHeight;

    sal_Int32 mnJPEGcompressMode;

    sal_Bool mbExportOLEAsJPEG;

    sal_Bool mbPresentation;

    sal_Int32 mnPageNumber;
};

}

#endif
