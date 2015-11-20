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

    css::presentation::AnimationEffect meEffect;
    css::presentation::AnimationEffect meTextEffect;
    css::presentation::AnimationSpeed  meEffectSpeed;

    sal_Int32       mnPresOrder;

    OUString        maBookmark;

    sal_Int32       mnDimColor;
    bool            mbDimHide;
    bool            mbDimPrev;

    bool            mbSoundOn;
    bool            mbPlayFull;
    OUString        maSoundURL;

    sal_Int32       mnBlueScreenColor;

    ShapeInfo() :
        mnID(0), mnX(0), mnY(0), mnWidth(0), mnHeight(0),
        meEffect( css::presentation::AnimationEffect_NONE ),
        meTextEffect( css::presentation::AnimationEffect_NONE ),
        meEffectSpeed( css::presentation::AnimationSpeed_MEDIUM ),
        mnPresOrder( 0 ),
        mnDimColor( 0 ),
        mbDimHide( false ),
        mbDimPrev( false ),
        mbSoundOn( false ),
        mbPlayFull( false ),
        mnBlueScreenColor( 0 ) {}
};


struct PageInfo
{
    sal_uInt16      mnBackgroundID;
    sal_uInt16      mnObjectsID;
    sal_uInt16      mnForegroundID;

    ::std::vector<ShapeInfo*>
                    maShapesVector;

    PageInfo();
    ~PageInfo();

};



class FlashExporter
{
public:
    FlashExporter(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,

        // #i56084# variables for selection export
        const css::uno::Reference< css::drawing::XShapes >& rxSelectedShapes,
        const css::uno::Reference< css::drawing::XDrawPage >& rxSelectedDrawPage,

        sal_Int32 nJPEGCompressMode = -1,
        bool bExportOLEAsJPEG = false);
    ~FlashExporter();

    void Flush();

    bool exportAll( css::uno::Reference< css::lang::XComponent > xDoc, css::uno::Reference< css::io::XOutputStream > &xOutputStream,    css::uno::Reference< css::task::XStatusIndicator> &xStatusIndicator );
    bool exportSlides( css::uno::Reference< css::drawing::XDrawPage > xDrawPage, css::uno::Reference< css::io::XOutputStream > &xOutputStream, sal_uInt16 nPage);
    sal_uInt16 exportBackgrounds( css::uno::Reference< css::drawing::XDrawPage > xDrawPage, css::uno::Reference< css::io::XOutputStream > &xOutputStream, sal_uInt16 nPage, bool bExportObjects );
    sal_uInt16 exportBackgrounds( css::uno::Reference< css::drawing::XDrawPage > xDrawPage, sal_uInt16 nPage, bool bExportObjects );

    ChecksumCache gMasterCache;
    ChecksumCache gPrivateCache;
    ChecksumCache gObjectCache;
    ChecksumCache gMetafileCache;

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    // #i56084# variables for selection export
    const css::uno::Reference< css::drawing::XShapes > mxSelectedShapes;
    const css::uno::Reference< css::drawing::XDrawPage > mxSelectedDrawPage;
    bool mbExportSelection;

    css::uno::Reference< css::drawing::XGraphicExportFilter > mxGraphicExporter;

    ::std::map<sal_uInt32, PageInfo> maPagesMap;

    sal_uInt16 exportDrawPageBackground(sal_uInt16 nPage, css::uno::Reference< css::drawing::XDrawPage >& xPage);
    sal_uInt16 exportMasterPageObjects(sal_uInt16 nPage, css::uno::Reference< css::drawing::XDrawPage >& xMasterPage);

    void exportDrawPageContents( const css::uno::Reference< css::drawing::XDrawPage >& xPage, bool bStream, bool bMaster  );
    void exportShapes( const css::uno::Reference< css::drawing::XShapes >& xShapes, bool bStream, bool bMaster );
    void exportShape( const css::uno::Reference< css::drawing::XShape >& xShape, bool bMaster);

    BitmapChecksum ActionSummer(css::uno::Reference< css::drawing::XShape >& xShape);
    BitmapChecksum ActionSummer(css::uno::Reference< css::drawing::XShapes >& xShapes);

    bool getMetaFile( css::uno::Reference< css::lang::XComponent >&xComponent, GDIMetaFile& rMtf, bool bOnlyBackground = false, bool bExportAsJPEG = false );

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
