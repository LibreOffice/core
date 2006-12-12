/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptin.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:35:48 $
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

#ifndef _SD_PPTIN_HXX
#define _SD_PPTIN_HXX

#ifndef _SVDFPPT_HXX //autogen
#include <svx/svdfppt.hxx>
#endif
#ifndef _MSDFFDEF_HXX
#include <svx/msdffdef.hxx>
#endif
#ifndef _SD_DIADEF_H
#include <diadef.h>
#endif
#ifndef _SVDTYPES_HXX //autogen
#include <svx/svdtypes.hxx>
#endif
#ifndef _MS_FILTERTRACER_HXX
#include <svx/msfiltertracer.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _SD_PPT_ANIMATIONS_HXX
#include <ppt/pptanimations.hxx>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

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
    BOOL            mbDocumentFound;
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
