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

#ifndef INCLUDED_SD_SOURCE_FILTER_PPT_PPTIN_HXX
#define INCLUDED_SD_SOURCE_FILTER_PPT_PPTIN_HXX

#include <filter/msfilter/svdfppt.hxx>
#include <svx/msdffdef.hxx>
#include <diadef.h>
#include <svx/svdtypes.hxx>
#include <com/sun/star/uno/Any.h>
#include <memory>

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

typedef std::shared_ptr< Ppt97Animation > Ppt97AnimationPtr;
typedef ::std::map < SdrObject*, Ppt97AnimationPtr > tAnimationMap;
typedef std::vector< std::pair< SdrObject*, Ppt97AnimationPtr > > tAnimationVector;

class ImplSdPPTImport : public SdrPowerPointImport
{
    SfxMedium&      mrMed;
    SotStorage&      mrStorage;
//  SvStream*       mpPicStream;
    DffRecordHeader maDocHd;
    std::vector<OUString> maSlideNameList;
    bool            mbDocumentFound;
    sal_uInt32      mnFilterOptions;
    SdDrawDocument* mpDoc;
    PresChange      mePresChange;
    SdrLayerID      mnBackgroundLayerID;
    SdrLayerID      mnBackgroundObjectsLayerID;

    tAnimationMap   maAnimations;
    void            SetHeaderFooterPageSettings( SdPage* pPage, const PptSlidePersistEntry* pMasterPersist );
    void            ImportPageEffect( SdPage* pPage, const bool bNewAnimationsUsed );

    void            FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, const OUString& aMacroName );

    virtual         SdrObject* ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj ) override;
    virtual         SdrObject* ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pText, SdPageCapsule pPage,
                                            SfxStyleSheet*, SfxStyleSheet** ) const override;

public:

    OUString        ReadSound( sal_uInt32 nSoundRef ) const;
    OUString        ReadMedia( sal_uInt32 nMediaRef ) const;

    ImplSdPPTImport( SdDrawDocument* pDoc, SotStorage& rStorage, SfxMedium& rMed, PowerPointImportParam& );
    virtual ~ImplSdPPTImport();

    bool         Import();
    virtual bool ReadFormControl(  tools::SvRef<SotStorage>& rSrc1, css::uno::Reference< css::form::XFormComponent > & rFormComp ) const override;
};

class SdPPTImport
{
    ImplSdPPTImport* pFilter;

    public:

        SdPPTImport( SdDrawDocument* pDoc, SvStream& rDocStream, SotStorage& rStorage, SfxMedium& rMed );
        ~SdPPTImport();

        bool Import();
};

#endif // INCLUDED_SD_SOURCE_FILTER_PPT_PPTIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
