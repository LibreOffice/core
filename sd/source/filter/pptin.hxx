/*************************************************************************
 *
 *  $RCSfile: pptin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:26:53 $
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

#ifndef _SD_PPTIN_HXX
#define _SD_PPTIN_HXX

#ifndef _SVDFPPT_HXX //autogen
#include <svx/svdfppt.hxx>
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

class SdDrawDocument;
class SfxMedium;

#define PPT_TRANSITION_TYPE_NONE            0
#define PPT_TRANSITION_TYPE_RANDOM          1
#define PPT_TRANSITION_TYPE_BLINDS          2
#define PPT_TRANSITION_TYPE_CHECKER         3
#define PPT_TRANSITION_TYPE_COVER           4
#define PPT_TRANSITION_TYPE_DISSOLVE        5
#define PPT_TRANSITION_TYPE_FADE            6
#define PPT_TRANSITION_TYPE_PULL            7
#define PPT_TRANSITION_TYPE_RANDOM_BARS     8
#define PPT_TRANSITION_TYPE_STRIPS          9
#define PPT_TRANSITION_TYPE_WIPE           10
#define PPT_TRANSITION_TYPE_ZOOM           11
#define PPT_TRANSITION_TYPE_SPLIT          13

/*************************************************************************
|*
|* lokaler Import
|*
\************************************************************************/

class SdPage;
class SdAnimationInfo;
struct PptAnimationInfoAtom;
struct PptInteractiveInfoAtom;

class ImplSdPPTImport : public SdrPowerPointImport
{
    SfxMedium&      rMed;
    SvStorage&      rStorage;
    SvStream*       pPicStream;
    DffRecordHeader aDocHd;
    List            aSlideNameList;
    BOOL            bDocumentFound;
    sal_uInt32      nFilterOptions;
    SdDrawDocument* pDoc;
    PresChange      ePresChange;
    SdrLayerID      nBackgroundLayerID;
    SdrLayerID      nBackgroundObjectsLayerID;

    void            ImportPageEffect( SdPage* pPage );

    void            FillSdAnimationInfo( SdAnimationInfo* pInfo, PptInteractiveInfoAtom* pIAtom, String aMacroName );
    void            FillSdAnimationInfo( SdAnimationInfo* pInfo, PptAnimationInfoAtom* pAnim );

    virtual         SdrObject* ProcessObj( SvStream& rSt, DffObjData& rData, void* pData, Rectangle& rTextRect, SdrObject* pObj );
    virtual         SdrObject* ApplyTextObj( PPTTextObj* pTextObj, SdrTextObj* pText, SdPage* pPage,
                                            SfxStyleSheet*, SfxStyleSheet** ) const;

    String          ReadSound( UINT32 nSoundRef ) const;

public:

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

