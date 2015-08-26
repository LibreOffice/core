/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_FILTER_SOURCE_FLASH_SWFEXPORTER_HXX
#define INCLUDED_FILTER_SOURCE_FLASH_SWFEXPORTER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XGraphicExportFilter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <osl/file.hxx>
#include <vcl/checksum.hxx>

#include <vector>
#include <map>

typedef ::std::map<BitmapChecksum, sal_uInt16> ChecksumCache;

class GDIMetaFile;

inline OUString STR(const sal_Char * in)
{
    return OUString::createFromAscii(in);
}

inline OUString VAL(sal_Int32 in)
{
    return OUString::number(in);
}

namespace swf {

class Writer;


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
    OUString maBookmark;

    sal_Int32       mnDimColor;
    bool        mbDimHide;
    bool        mbDimPrev;

    bool        mbSoundOn;
    bool        mbPlayFull;
    OUString maSoundURL;

    sal_Int32       mnBlueScreenColor;

    ShapeInfo() :
        mnID(0), mnX(0), mnY(0), mnWidth(0), mnHeight(0),
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



struct ShapeAnimationInfo
{
    ShapeInfo* mpShapeInfo;
    sal_uInt16  mnDepth;

    ShapeAnimationInfo( ShapeInfo* pShapeInfo, sal_uInt16 nDepth ) : mpShapeInfo( pShapeInfo ), mnDepth( nDepth ) {}
};




struct PageInfo
{
    ::com::sun::star::presentation::FadeEffect      meFadeEffect;
    ::com::sun::star::presentation::AnimationSpeed  meFadeSpeed;

    sal_Int32       mnDuration;
    sal_Int32       mnChange;

    sal_uInt16      mnBackgroundID;
    sal_uInt16      mnObjectsID;
    sal_uInt16      mnForegroundID;

    bool mbBackgroundVisible;
    bool mbBackgroundObjectsVisible;

    ShapeInfoVector maShapesVector;

    PageInfo();
    ~PageInfo();

};



typedef ::std::map<sal_uInt32, PageInfo> PageInfoMap;



class FlashExporter
{
public:
    FlashExporter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,

        // #i56084# variables for selection export
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxSelectedShapes,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxSelectedDrawPage,

        sal_Int32 nJPEGCompressMode = -1,
        bool bExportOLEAsJPEG = false);
    ~FlashExporter();

    void Flush();

    bool exportAll( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xDoc, com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream,    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator> &xStatusIndicator );
    bool exportSlides( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage, com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream, sal_uInt16 nPage);
    sal_uInt16 exportBackgrounds( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage, com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > &xOutputStream, sal_uInt16 nPage, bool bExportObjects );
    sal_uInt16 exportBackgrounds( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage, sal_uInt16 nPage, bool bExportObjects );

    ChecksumCache gMasterCache;
    ChecksumCache gPrivateCache;
    ChecksumCache gObjectCache;
    ChecksumCache gMetafileCache;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;

    // #i56084# variables for selection export
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxSelectedShapes;
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxSelectedDrawPage;
    bool mbExportSelection;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XGraphicExportFilter > mxGraphicExporter;

    PageInfoMap maPagesMap;

    sal_uInt16 exportDrawPageBackground(sal_uInt16 nPage, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage);
    sal_uInt16 exportMasterPageObjects(sal_uInt16 nPage, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xMasterPage);

    void exportDrawPageContents( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, bool bStream, bool bMaster  );
    void exportShapes( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes, bool bStream, bool bMaster );
    void exportShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, bool bMaster);

    BitmapChecksum ActionSummer(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape);
    BitmapChecksum ActionSummer(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes);

    void animateShape( ShapeAnimationInfo& rAnimInfo );
    void animatePage( PageInfo* pPageInfo );

    bool getMetaFile( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&xComponent, GDIMetaFile& rMtf, bool bOnlyBackground = false, bool bExportAsJPEG = false );

    Writer* mpWriter;

    sal_Int32 mnDocWidth;
    sal_Int32 mnDocHeight;

    sal_Int32 mnJPEGcompressMode;

    bool mbExportOLEAsJPEG;

    bool mbPresentation;

    sal_Int32 mnPageNumber;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
