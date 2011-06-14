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

#ifndef _SD_PPTIN_HXX
#define _SD_PPTIN_HXX

#include <filter/msfilter/svdfppt.hxx>
#include <svx/msdffdef.hxx>
#include <diadef.h>
#include <svx/svdtypes.hxx>
#include <filter/msfilter/msfiltertracer.hxx>
#include <com/sun/star/uno/Any.h>
#include <boost/shared_ptr.hpp>

class SdDrawDocument;
class SfxMedium;

/*************************************************************************
|*
|* lokaler Import
|*
\************************************************************************/

class SdPage;
class SdAnimationInfo;
struct PptInteractiveInfoAtom;
class Ppt97Animation;

typedef boost::shared_ptr< Ppt97Animation > Ppt97AnimationPtr;
typedef ::std::map < SdrObject*, Ppt97AnimationPtr > tAnimationMap;
typedef std::vector< std::pair< SdrObject*, Ppt97AnimationPtr > > tAnimationVector;

class ImplSdPPTImport : public SdrPowerPointImport
{
    SfxMedium&      mrMed;
    SvStorage&      mrStorage;
//  SvStream*       mpPicStream;
    DffRecordHeader maDocHd;
    List            maSlideNameList;
    sal_Bool            mbDocumentFound;
    sal_uInt32      mnFilterOptions;
    SdDrawDocument* mpDoc;
    PresChange      mePresChange;
    SdrLayerID      mnBackgroundLayerID;
    SdrLayerID      mnBackgroundObjectsLayerID;

    tAnimationMap   maAnimations;

    void            SetHeaderFooterPageSettings( SdPage* pPage, const PptSlidePersistEntry* pMasterPersist );
    void            ImportPageEffect( SdPage* pPage, const sal_Bool bNewAnimationsUsed );

    void            FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, String aMacroName );

    virtual         SdrObject* ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj );
    virtual         SdrObject* ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pText, SdPage* pPage,
                                            SfxStyleSheet*, SfxStyleSheet** ) const;

public:

    String          ReadSound( sal_uInt32 nSoundRef ) const;
    String          ReadMedia( sal_uInt32 nMediaRef ) const;

    ImplSdPPTImport( SdDrawDocument* pDoc, SvStorage& rStorage, SfxMedium& rMed, PowerPointImportParam& );
    ~ImplSdPPTImport();

    sal_Bool        Import();
};

class SdPPTImport
{
    ImplSdPPTImport* pFilter;

    public:

        SdPPTImport( SdDrawDocument* pDoc, SvStream& rDocStream, SvStorage& rStorage, SfxMedium& rMed, MSFilterTracer* pTracer = NULL );
        ~SdPPTImport();

        sal_Bool Import();
};

#endif // _SD_PPTIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
