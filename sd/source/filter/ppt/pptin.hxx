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
    bool            mbDocumentFound;
    sal_uInt32      mnFilterOptions;
    SdDrawDocument* mpDoc;
    PresChange      mePresChange;
    SdrLayerID      mnBackgroundLayerID;
    SdrLayerID      mnBackgroundObjectsLayerID;

    tAnimationMap   maAnimations;

    void            SetHeaderFooterPageSettings( SdPage* pPage, const PptSlidePersistEntry* pMasterPersist );
    void            ImportPageEffect( SdPage* pPage, const sal_Bool bNewAnimationsUsed );

    void            FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, String aMacroName );

    virtual         SdrObject* ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, basegfx::B2DRange& rTextRect, SdrObject* pObj );
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
