/*************************************************************************
 *
 *  $RCSfile: svdogrp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-07 12:58:28 $
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

#ifndef _SVXLINKMGR_HXX //autogen
#include <linkmgr.hxx>
#endif

#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif

#include "svdogrp.hxx"

#ifndef SVX_LIGHT
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <svtools/urihelper.hxx>

#include "xpool.hxx"
#include "xpoly.hxx"

#include "svdxout.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svditer.hxx"
#include "svdobj.hxx"
#include "svdtrans.hxx"
#include "svdio.hxx"
#include "svdetc.hxx"
#include "svdattrx.hxx"  // NotPersistItems
#include "svdoedge.hxx"  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname

#ifndef _PERSIST_HXX //autogen
#include <so3/persist.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef SVX_LIGHT

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@  @@@@@  @@@@@@   @@@@@ @@@@@   @@@@  @@  @@ @@@@@   @@    @@ @@  @@ @@  @@
// @@  @@ @@  @@     @@  @@     @@  @@ @@  @@ @@  @@ @@  @@  @@    @@ @@@ @@ @@ @@
// @@  @@ @@@@@      @@  @@ @@@ @@@@@  @@  @@ @@  @@ @@@@@   @@    @@ @@@@@@ @@@@
// @@  @@ @@  @@ @@  @@  @@  @@ @@  @@ @@  @@ @@  @@ @@      @@    @@ @@ @@@ @@ @@
//  @@@@  @@@@@   @@@@    @@@@@ @@  @@  @@@@   @@@@  @@      @@@@@ @@ @@  @@ @@  @@
//
// ImpSdrObjGroupLink zur Verbindung von SdrObjGroup und LinkManager
//
// Einem solchen Link merke ich mir als SdrObjUserData am Objekt. Im Gegensatz
// zum Grafik-Link werden die ObjektDaten jedoch kopiert (fuer Paint, etc.).
// Die Information ob das Objekt ein Link ist besteht genau darin, dass dem
// Objekt ein entsprechender UserData-Record angehaengt ist oder nicht.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrObjGroupLink: public SvBaseLink {
    SdrObject* pSdrObj;
public:
    ImpSdrObjGroupLink(SdrObject* pObj1): SvBaseLink(LINKUPDATE_ONCALL,FORMAT_FILE),pSdrObj(pObj1) {}
    virtual ~ImpSdrObjGroupLink();
    virtual void Closed();
    virtual void DataChanged(SvData& rData);
    FASTBOOL     Connect() { return 0 != SvBaseLink::GetRealObject(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////


ImpSdrObjGroupLink::~ImpSdrObjGroupLink()
{
}

// Closed() wird gerufen, wenn die Verknüpfung geloesst wird.

void ImpSdrObjGroupLink::Closed()
{
    if (pSdrObj!=NULL) {
        // pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
        ImpSdrObjGroupLinkUserData* pData=((SdrObjGroup*)pSdrObj)->GetLinkUserData();
        if (pData!=NULL) pData->pLink=NULL;
        ((SdrObjGroup*)pSdrObj)->ReleaseGroupLink();
    }
    SvBaseLink::Closed();
}


void ImpSdrObjGroupLink::DataChanged(SvData& rData)
{
    FASTBOOL bForceReload=FALSE;
    SdrModel*       pModel      = pSdrObj  ==NULL ? NULL : pSdrObj->GetModel();
    SvxLinkManager* pLinkManager= pModel==NULL ? NULL : pModel->GetLinkManager();
    if (pLinkManager!=NULL) {
        ImpSdrObjGroupLinkUserData* pData=((SdrObjGroup*)pSdrObj)->GetLinkUserData();
        if (pData!=NULL) {
            String aFile;
            String aName;
            pLinkManager->GetDisplayNames(*this,NULL,&aFile,&aName,NULL);

            if(!pData->aFileName.Equals(aFile) || !pData->aObjName.Equals(aName))
            {
                pData->aFileName=aFile;
                pData->aObjName=aName;
                pSdrObj->SetChanged();
                bForceReload=TRUE;
            }
        }
    }
    if (pSdrObj!=NULL) {
        ((SdrObjGroup*)pSdrObj)->ReloadLinkedGroup(bForceReload);
    }
}

#endif // SVX_LIGHT

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// @@    @@ @@  @@ @@  @@  @@  @@  @@@@@ @@@@@@ @@@@@   @@@@@   @@@@  @@@@@@  @@@@
// @@    @@ @@@ @@ @@ @@   @@  @@ @@     @@     @@  @@  @@  @@ @@  @@   @@   @@  @@
// @@    @@ @@@@@@ @@@@    @@  @@  @@@@  @@@@@  @@@@@   @@  @@ @@@@@@   @@   @@@@@@
// @@    @@ @@ @@@ @@@@@   @@  @@     @@ @@     @@  @@  @@  @@ @@  @@   @@   @@  @@
// @@@@@ @@ @@  @@ @@  @@   @@@@  @@@@@  @@@@@@ @@  @@  @@@@@  @@  @@   @@   @@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(ImpSdrObjGroupLinkUserData,SdrObjUserData);


ImpSdrObjGroupLinkUserData::ImpSdrObjGroupLinkUserData(SdrObject* pObj1):
    SdrObjUserData(SdrInventor,SDRUSERDATA_OBJGROUPLINK,0),
    pObj(pObj1),
    nDrehWink0(0),
    nShearWink0(0),
    pLink(NULL),
    nObjNum(0),
    nPageNum(0),
    bMasterPage(FALSE),
    bOrigPos(FALSE),
    bOrigSize(FALSE),
    bOrigRotate(FALSE),
    bOrigShear(FALSE)
{
}


ImpSdrObjGroupLinkUserData::~ImpSdrObjGroupLinkUserData()
{
#ifndef SVX_LIGHT
    delete pLink;
#endif
}


SdrObjUserData* ImpSdrObjGroupLinkUserData::Clone(SdrObject* pObj1) const
{
    ImpSdrObjGroupLinkUserData* pData=new ImpSdrObjGroupLinkUserData(pObj1);
    pData->aFileName  =aFileName;
    pData->aObjName   =aObjName;
    pData->aFileDate0 =aFileDate0;
    pData->aSnapRect0 =aSnapRect0;
    pData->nDrehWink0 =nDrehWink0;
    pData->nShearWink0=nShearWink0;
    pData->nObjNum    =nObjNum;
    pData->nPageNum   =nPageNum;
    pData->bMasterPage=bMasterPage;
    pData->bOrigPos   =bOrigPos;
    pData->bOrigSize  =bOrigSize;
    pData->bOrigRotate=bOrigRotate;
    pData->bOrigShear =bOrigShear;
    pData->pLink=NULL;
    //pObj1->ImpLinkAnmeldung();
    return pData;
}


void ImpSdrObjGroupLinkUserData::WriteData(SvStream& rOut)
{
    SdrObjUserData::WriteData(rOut);

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
    aCompat.SetID("ImpSdrObjGroupLinkUserData");
#endif

    String aRelFileName;

    if( aFileName.Len() )
    {
        aRelFileName = INetURLObject::AbsToRel( aFileName,
                                                INetURLObject::WAS_ENCODED,
                                                INetURLObject::DECODE_UNAMBIGUOUS );
    }

    rOut.WriteByteString( aRelFileName );

    // UNICODE: rOut << aObjName;
    rOut.WriteByteString(aObjName);

    rOut << UINT32(aFileDate0.GetDate());
    rOut << UINT32(aFileDate0.GetTime());
    rOut << aSnapRect0;
    rOut << nDrehWink0;
    rOut << nShearWink0;
    rOut << BOOL(bMasterPage);
    rOut << nPageNum;
    rOut << nObjNum;
    rOut << BOOL(bOrigPos);
    rOut << BOOL(bOrigSize);
    rOut << BOOL(bOrigRotate);
    rOut << BOOL(bOrigShear);
}

void ImpSdrObjGroupLinkUserData::ReadData(SvStream& rIn)
{
    SdrObjUserData::ReadData(rIn);
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rIn, STREAM_READ);

#ifdef DBG_UTIL
    aCompat.SetID("ImpSdrObjGroupLinkUserData");
#endif

    BOOL bTmp;
    UINT32 nTmp32;
    String aFileNameRel;

    rIn.ReadByteString(aFileNameRel);

    if( aFileNameRel.Len() )
    {
        aFileName = ::URIHelper::SmartRelToAbs( aFileNameRel, FALSE,
                                                INetURLObject::WAS_ENCODED,
                                                INetURLObject::DECODE_UNAMBIGUOUS );
    }
    else
        aFileName.Erase();

    // UNICODE: rIn >> aObjName;
    rIn.ReadByteString(aObjName);

    rIn >> nTmp32; aFileDate0.SetDate(nTmp32);
    rIn >> nTmp32; aFileDate0.SetTime(nTmp32);
    rIn >> aSnapRect0;
    rIn >> nDrehWink0;
    rIn >> nShearWink0;
    rIn >> bTmp; bMasterPage=bTmp;
    rIn >> nPageNum;
    rIn >> nObjNum;
    rIn >> bTmp; bOrigPos   =bTmp;
    rIn >> bTmp; bOrigSize  =bTmp;
    rIn >> bTmp; bOrigRotate=bTmp;
    rIn >> bTmp; bOrigShear =bTmp;
}


void ImpSdrObjGroupLinkUserData::AfterRead()
{
    if (pObj!=NULL) {
        ((SdrObjGroup*)pObj)->ImpLinkAnmeldung();
        // lt. Anweisung von MB kein automatisches Reload mehr
        //((SdrObjGroup*)pObj)->ReloadLinkedGroup();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@   @@@@  @@@@@   @@@@  @@  @@ @@@@@
//  @@  @@ @@  @@     @@  @@     @@  @@ @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@ @@@ @@@@@  @@  @@ @@  @@ @@@@@
//  @@  @@ @@  @@ @@  @@  @@  @@ @@  @@ @@  @@ @@  @@ @@
//   @@@@  @@@@@   @@@@    @@@@@ @@  @@  @@@@   @@@@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrObjGroup,SdrObject);


SdrObjGroup::SdrObjGroup()
{
    pSub=new SdrObjList(NULL,NULL);
    pSub->SetOwnerObj(this);
    pSub->SetListKind(SDROBJLIST_GROUPOBJ);
    bRefPoint=FALSE;
    nDrehWink=0;
    nShearWink=0;
    bClosedObj=FALSE;
    mpGroupItemSet = NULL;
}


SdrObjGroup::~SdrObjGroup()
{
    ReleaseGroupLink();
    delete pSub;
    if(mpGroupItemSet)
        delete mpGroupItemSet;
}


void SdrObjGroup::SetGroupLink(const String& rFileName, const String& rObjName)
{
    ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
    if (pData!=NULL) {
        nDrehWink-=pData->nDrehWink0;
        nShearWink-=pData->nShearWink0;
        ReleaseGroupLink();
    }
    aName=rObjName;
    pData=new ImpSdrObjGroupLinkUserData(this);
    pData->aFileName=rFileName;
    pData->aObjName=rObjName;
    InsertUserData(pData);
    ImpLinkAnmeldung();
}


void SdrObjGroup::ReleaseGroupLink()
{
    ImpLinkAbmeldung();
    USHORT nAnz=GetUserDataCount();
    for (USHORT nNum=nAnz; nNum>0;) {
        nNum--;
        SdrObjUserData* pData=GetUserData(nNum);
        if (pData->GetInventor()==SdrInventor && pData->GetId()==SDRUSERDATA_OBJGROUPLINK) {
            DeleteUserData(nNum);
        }
    }
}


ImpSdrObjGroupLinkUserData* SdrObjGroup::GetLinkUserData() const
{
    ImpSdrObjGroupLinkUserData* pData=NULL;
    USHORT nAnz=GetUserDataCount();
    for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
        nNum--;
        pData=(ImpSdrObjGroupLinkUserData*)GetUserData(nNum);
        if (pData->GetInventor()!=SdrInventor || pData->GetId()!=SDRUSERDATA_OBJGROUPLINK) {
            pData=NULL;
        }
    }
    return pData;
}


FASTBOOL SdrObjGroup::ReloadLinkedGroup(FASTBOOL bForceLoad)
{
    ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
    FASTBOOL                    bRet=TRUE;

    if( pData )
    {
        ::ucb::ContentBroker*   pBroker = ::ucb::ContentBroker::get();
        DateTime                aFileDT;
        BOOL                    bExists = FALSE, bLoad = FALSE;

        if( pBroker )
        {
            bExists = TRUE;

            try
            {
                INetURLObject aURL; aURL.SetSmartURL( pData->aFileName );
                ::ucb::Content aCnt( aURL.GetMainURL(), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
                ::com::sun::star::uno::Any aAny( aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ) ) );
                ::com::sun::star::util::DateTime aDateTime;

                aAny >>= aDateTime;
                ::utl::typeConvert( aDateTime, aFileDT );
            }
            catch( ... )
            {
                bExists = FALSE;
            }
        }

        if( bExists )
        {
            if( bForceLoad )
                bLoad = TRUE;
            else
                bLoad = ( aFileDT > pData->aFileDate0 );

            pData->aFileDate0 = aFileDT;
        }
        else
            bLoad = ( pModel!=NULL && pModel->LoadModel(pData->aFileName)!=NULL );

        if( bLoad )
        {
            Rectangle aMyRect(GetSnapRect());
            bRet=LoadGroup(pData->aFileName,pData->aObjName,&pData->nPageNum,&pData->bMasterPage,&pData->nObjNum);
            Rectangle aOrgRect(GetSnapRect());
            if (bRet && !aMyRect.IsEmpty() && !aOrgRect.IsEmpty())
            { // und nun noch zurechttransformieren
                if (aMyRect!=aOrgRect)
                {
                    // erstmal karo-einfach
                    NbcSetSnapRect(aMyRect);
                }
            }
            pData->aSnapRect0=aOrgRect; // letzte bekannte Groesse des Originalobjekts merken
        }
    }
    return bRet;
}


FASTBOOL SdrObjGroup::LoadGroup(const String& rFileName, const String& rObjName, USHORT* pnPgNum, FASTBOOL* pbMasterPg, ULONG* pnObjNum)
{
    FASTBOOL bRet=FALSE;

    if(pModel && rFileName.Len() && rObjName.Len())
    {
        const SdrModel* pTempModel=pModel->LoadModel(rFileName);
        if (pTempModel!=NULL) {
            SdrObjGroup* pRef=NULL;
            for (FASTBOOL bMPg=FALSE; bMPg!=TRUE && pRef==NULL;) {
                USHORT nPgAnz=bMPg ? pTempModel->GetMasterPageCount() : pTempModel->GetPageCount();
                for (USHORT nPgNum=0; nPgNum<nPgAnz && pRef==NULL; nPgNum++) {
                    const SdrPage* pPg=bMPg ? pTempModel->GetMasterPage(nPgNum) : pTempModel->GetPage(nPgNum);
                    ULONG nObjAnz=pPg->GetObjCount();
                    for (USHORT nObjNum=0; nObjNum<nObjAnz && pRef==NULL; nObjNum++) {
                        SdrObject* pObj=pPg->GetObj(nObjNum);
                        SdrObjGroup* pGrp=PTR_CAST(SdrObjGroup,pObj);

                        if(pGrp && pGrp->GetName().Equals(rObjName))
                        {
                            pRef = pGrp;

                            if(pnPgNum)
                                *pnPgNum = nPgNum;

                            if(pbMasterPg)
                                *pbMasterPg = bMPg;

                            if(pnObjNum)
                                *pnObjNum = nObjNum;

                            bRet = TRUE;
                        }
                    }
                }
                bMPg=TRUE; // soz. von FALSE auf TRUE inkrementieren (fuer die obige for-Schleife)
            }
            if (pRef!=NULL) {
                Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
                SendRepaintBroadcast();
                // zunaechst diverse Daten des Obj kopieren
                nLayerId=pRef->GetLayer(); // hier noch ueberarbeiten !!!
                aAnchor =pRef->aAnchor;
                bVirtObj=pRef->bVirtObj;
                bSizProt=pRef->bSizProt;
                bMovProt=pRef->bMovProt;
                bNoPrint=pRef->bNoPrint;
                bEmptyPresObj=pRef->bEmptyPresObj;
                bNotVisibleAsMaster=pRef->bNotVisibleAsMaster;
                // und nun die Objekte rueberhohlen
                pSub->Clear();
                pSub->CopyObjects(*pRef->GetSubList());
                SetChanged();
                SendRepaintBroadcast();
                SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
            }
            //delete pTempModel;
        }
        if (!pModel->IsLoading()) pModel->DisposeLoadedModels();
    }
    return bRet;
}


void SdrObjGroup::ImpLinkAnmeldung()
{
#ifndef SVX_LIGHT
    ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
    SvxLinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
    if ( pLinkManager         &&
         pData!=NULL          &&
         pData->pLink == NULL &&
         pModel->GetPersist() )
    {
        // Nicht 2x Anmelden
        INetURLObject aURLObj( ::URIHelper::SmartRelToAbs( pModel->GetPersist()->GetFileName(), FALSE,
                                                           INetURLObject::WAS_ENCODED,
                                                           INetURLObject::DECODE_UNAMBIGUOUS ) );
        INetURLObject aLinkURLObj( ::URIHelper::SmartRelToAbs( pData->aFileName, FALSE,
                                                               INetURLObject::WAS_ENCODED,
                                                               INetURLObject::DECODE_UNAMBIGUOUS ) );

        if(!aURLObj.PathToFileName().Equals(aLinkURLObj.PathToFileName()))
        {
            // Keine gelinkten Objekte im eigenen Model
            pData->pLink = new ImpSdrObjGroupLink(this);
            pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,
                                         pData->aFileName,NULL,&pData->aObjName);
            pData->pLink->Connect();
        }
    }
#endif // SVX_LIGHT
}


void SdrObjGroup::ImpLinkAbmeldung()
{
#ifndef SVX_LIGHT
    ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
    SvxLinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
    if (pLinkManager!=NULL && pData!=NULL && pData->pLink!=NULL) { // Nicht 2x Abmelden
        // Bei Remove wird *pLink implizit deleted
        pLinkManager->Remove(*pData->pLink);
        pData->pLink=NULL;
    }
#endif // SVX_LIGHT
}


void SdrObjGroup::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
//    rInfo.bCanConvToPath          =FALSE;
//    rInfo.bCanConvToPoly          =FALSE;
//    rInfo.bCanConvToPathLineToArea=FALSE;
//    rInfo.bCanConvToPolyLineToArea=FALSE;
    rInfo.bNoContortion=FALSE;
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (!aInfo.bMoveAllowed            ) rInfo.bMoveAllowed            =FALSE;
        if (!aInfo.bResizeFreeAllowed      ) rInfo.bResizeFreeAllowed      =FALSE;
        if (!aInfo.bResizePropAllowed      ) rInfo.bResizePropAllowed      =FALSE;
        if (!aInfo.bRotateFreeAllowed      ) rInfo.bRotateFreeAllowed      =FALSE;
        if (!aInfo.bRotate90Allowed        ) rInfo.bRotate90Allowed        =FALSE;
        if (!aInfo.bMirrorFreeAllowed      ) rInfo.bMirrorFreeAllowed      =FALSE;
        if (!aInfo.bMirror45Allowed        ) rInfo.bMirror45Allowed        =FALSE;
        if (!aInfo.bMirror90Allowed        ) rInfo.bMirror90Allowed        =FALSE;
        if (!aInfo.bShearAllowed           ) rInfo.bShearAllowed           =FALSE;
        if (!aInfo.bNoOrthoDesired         ) rInfo.bNoOrthoDesired         =FALSE;
        if (aInfo.bNoContortion            ) rInfo.bNoContortion           =TRUE;
        if (!aInfo.bCanConvToPath          ) rInfo.bCanConvToPath          =FALSE;

        if(!aInfo.bCanConvToContour)
            rInfo.bCanConvToContour = FALSE;

        if (!aInfo.bCanConvToPoly          ) rInfo.bCanConvToPoly          =FALSE;
        if (!aInfo.bCanConvToPathLineToArea) rInfo.bCanConvToPathLineToArea=FALSE;
        if (!aInfo.bCanConvToPolyLineToArea) rInfo.bCanConvToPolyLineToArea=FALSE;
    }
    if (nObjAnz==0) {
        rInfo.bRotateFreeAllowed=FALSE;
        rInfo.bRotate90Allowed  =FALSE;
        rInfo.bMirrorFreeAllowed=FALSE;
        rInfo.bMirror45Allowed  =FALSE;
        rInfo.bMirror90Allowed  =FALSE;
        rInfo.bTransparenceAllowed = FALSE;
        rInfo.bGradientAllowed = FALSE;
        rInfo.bShearAllowed     =FALSE;
        rInfo.bNoContortion     =TRUE;
    }
    if(nObjAnz != 1)
    {
        // only allowed if single object selected
        rInfo.bTransparenceAllowed = FALSE;
        rInfo.bGradientAllowed = FALSE;
    }
    if (pPlusData!=NULL && nObjAnz!=0) {
        ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
        if (pData!=NULL) {
            if (pData->bOrigPos   ) rInfo.bMoveAllowed =FALSE;
            if (pData->bOrigSize  ) { rInfo.bResizeFreeAllowed=FALSE; rInfo.bResizePropAllowed=FALSE; }
            if (pData->bOrigRotate) rInfo.bMoveAllowed =FALSE;
            if (pData->bOrigShear ) rInfo.bMoveAllowed =FALSE;
            // erstmal alles abschalten
            //rInfo.bResizeFreeAllowed=FALSE;
            //rInfo.bResizePropAllowed=FALSE;
            rInfo.bRotateFreeAllowed=FALSE;
            rInfo.bRotate90Allowed  =FALSE;
            rInfo.bMirrorFreeAllowed=FALSE;
            rInfo.bMirror45Allowed=FALSE;
            rInfo.bMirror90Allowed=FALSE;
            rInfo.bShearAllowed=FALSE;
            rInfo.bNoContortion=TRUE;
            // default: Proportionen beibehalten
            rInfo.bNoOrthoDesired=FALSE;
        }
    }
}


UINT16 SdrObjGroup::GetObjIdentifier() const
{
    return UINT16(OBJ_GRUP);
}


SdrLayerID SdrObjGroup::GetLayer() const
{
    FASTBOOL b1st=TRUE;
    SdrLayerID nLay=SdrLayerID(nLayerId);
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        SdrLayerID nLay1=pOL->GetObj(i)->GetLayer();
        if (b1st) { nLay=nLay1; b1st=FALSE; }
        else if (nLay1!=nLay) return 0;
    }
    return nLay;
}


void SdrObjGroup::NbcSetLayer(SdrLayerID nLayer)
{
    SdrObject::NbcSetLayer(nLayer);
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        pOL->GetObj(i)->NbcSetLayer(nLayer);
    }
}


void SdrObjGroup::SetObjList(SdrObjList* pNewObjList)
{
    SdrObject::SetObjList(pNewObjList);
    pSub->SetUpList(pNewObjList);
}


void SdrObjGroup::SetPage(SdrPage* pNewPage)
{
    FASTBOOL bLinked=IsLinkedGroup();
    FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
    FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;

    if (bLinked && bRemove) {
        ImpLinkAbmeldung();
    }

    SdrObject::SetPage(pNewPage);
    pSub->SetPage(pNewPage);

    if (bLinked && bInsert) {
        ImpLinkAnmeldung();
    }
}


void SdrObjGroup::SetModel(SdrModel* pNewModel)
{
    FASTBOOL bLinked=IsLinkedGroup();
    FASTBOOL bChg=pNewModel!=pModel;
    if (bLinked && bChg) {
        ImpLinkAbmeldung();
    }

    SdrObject::SetModel(pNewModel);
    pSub->SetModel(pNewModel);

    if (bLinked && bChg) {
        ImpLinkAnmeldung();
    }
}


FASTBOOL SdrObjGroup::HasRefPoint() const
{
    return bRefPoint;
}


Point SdrObjGroup::GetRefPoint() const
{
    return aRefPoint;
}


void SdrObjGroup::SetRefPoint(const Point& rPnt)
{
    bRefPoint=TRUE;
    aRefPoint=rPnt;
}


SdrObjList* SdrObjGroup::GetSubList() const
{
    return pSub;
}

FASTBOOL SdrObjGroup::HasSetName() const
{
    return TRUE;
}


void SdrObjGroup::SetName(const XubString& rStr)
{
    aName=rStr; SetChanged();
}


XubString SdrObjGroup::GetName() const
{
    return aName;
}


const Rectangle& SdrObjGroup::GetBoundRect() const
{
    if (pSub->GetObjCount()!=0) {
        // hier auch das aOutRect=AllObjSnapRect setzen, da GetSnapRect zu selten gerufen wird.
        ((SdrObjGroup*)this)->aOutRect=pSub->GetAllObjSnapRect();
        return pSub->GetAllObjBoundRect();
    } else {
        return aOutRect;
    }
}


const Rectangle& SdrObjGroup::GetSnapRect() const
{
    if (pSub->GetObjCount()!=0) {
        ((SdrObjGroup*)this)->aOutRect=pSub->GetAllObjSnapRect();
    }
    return aOutRect;
}


FASTBOOL SdrObjGroup::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    FASTBOOL bOk=TRUE;
    if (pSub->GetObjCount()!=0) {
        bOk=pSub->Paint(rXOut,rInfoRec);
    } else { // ansonsten ist es eine leere Gruppe
        if (!rInfoRec.bPrinter && rInfoRec.aPaintLayer.IsSet(nLayerId)) {
            OutputDevice* pOutDev=rXOut.GetOutDev();
            pOutDev->SetFillInBrush(Brush(BRUSH_NULL));
            pOutDev->SetPen(Color(COL_LIGHTGRAY));
            pOutDev->DrawRect(aOutRect);
        }
    }
    if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
        bOk=PaintGluePoints(rXOut,rInfoRec);
    }
    return bOk;
}


SdrObject* SdrObjGroup::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if (pSub->GetObjCount()!=0) {
        return pSub->CheckHit(rPnt,nTol,pVisiLayer);
    } else { // ansonsten ist es eine leere Gruppe
        if (pVisiLayer==NULL || pVisiLayer->IsSet(nLayerId)) {
            Rectangle aAussen(aOutRect);
            aAussen.Top()   -=nTol;
            aAussen.Left()  -=nTol;
            aAussen.Bottom()+=nTol;
            aAussen.Right() +=nTol;
            nTol++;
            Rectangle aInnen(aOutRect);
            aInnen.Top()   +=nTol;
            aInnen.Left()  +=nTol;
            aInnen.Bottom()-=nTol;
            aInnen.Right() -=nTol;
            if (aAussen.IsInside(rPnt) && !aInnen.IsInside(rPnt)) {
                return (SdrObject*)this;
            }
        }
    }
    return NULL;
}

/*SdrObject* SdrObjGroup::Clone() const
{
    SdrObjGroup* pObj=new SdrObjGroup();
    if (pObj!=NULL) {
        *pObj=*this;
    }
    return pObj;
}*/


void SdrObjGroup::operator=(const SdrObject& rObj)
{
    if (rObj.IsGroupObject()) {
        SdrObject::operator=(rObj);
        pSub->CopyObjects(*rObj.GetSubList());
        nDrehWink  =((SdrObjGroup&)rObj).nDrehWink;
        nShearWink =((SdrObjGroup&)rObj).nShearWink;
        aName      =((SdrObjGroup&)rObj).aName;
        aRefPoint  =((SdrObjGroup&)rObj).aRefPoint;
        bRefPoint  =((SdrObjGroup&)rObj).bRefPoint;
    }
}


void SdrObjGroup::TakeObjNameSingul(XubString& rName) const
{
    if(IsLinkedGroup())
    {
        rName = ImpGetResStr(STR_ObjNameSingulGRUPLNK);
    }
    else if(!pSub->GetObjCount())
    {
        rName = ImpGetResStr(STR_ObjNameSingulGRUPEMPTY);
    }
    else
    {
        rName = ImpGetResStr(STR_ObjNameSingulGRUP);
    }

    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}


void SdrObjGroup::TakeObjNamePlural(XubString& rName) const
{
    if (IsLinkedGroup()) {
        rName=ImpGetResStr(STR_ObjNamePluralGRUPLNK);
    } else if (pSub->GetObjCount()==0) {
        rName=ImpGetResStr(STR_ObjNamePluralGRUPEMPTY);
    } else {
        rName=ImpGetResStr(STR_ObjNamePluralGRUP);
    }
}


void SdrObjGroup::RecalcSnapRect()
{
    // nicht erforderlich, da die Rects von der SubList verwendet werden.
}


void MergePoly(XPolyPolygon& rDst, const XPolyPolygon& rSrc)
{
    USHORT nAnz=rSrc.Count();
    USHORT i;
    for (i=0; i<nAnz; i++) {
        rDst.Insert(rSrc.GetObject(i));
    }
}


void SdrObjGroup::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    rPoly.Clear();
    ULONG nAnz=pSub->GetObjCount();
    ULONG i=0;
    while (i<nAnz) {
        SdrObject* pObj=pSub->GetObj(i);
        XPolyPolygon aPP;
        pObj->TakeXorPoly(aPP,bDetail);
        MergePoly(rPoly,aPP);
        i++;
    }
    if (rPoly.Count()==0) {
        rPoly.Insert(XPolygon(aOutRect));
    }
}

void SdrObjGroup::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}


FASTBOOL SdrObjGroup::BegDrag(SdrDragStat& rDrag) const
{
    return FALSE;
}


FASTBOOL SdrObjGroup::BegCreate(SdrDragStat& rStat)
{
    return FALSE;
}


long SdrObjGroup::GetRotateAngle() const
{
    return nDrehWink;
}


long SdrObjGroup::GetShearAngle(FASTBOOL bVertical) const
{
    return nShearWink;
}


void SdrObjGroup::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aOld(GetSnapRect());
    long nMulX=rRect.Right()-rRect.Left();
    long nDivX=aOld.Right()-aOld.Left();
    long nMulY=rRect.Bottom()-rRect.Top();
    long nDivY=aOld.Bottom()-aOld.Top();
    if (nDivX==0) { nMulX=1; nDivX=1; }
    if (nDivY==0) { nMulY=1; nDivY=1; }
    if (nMulX!=nDivX || nMulY!=nDivY) {
        Fraction aX(nMulX,nDivX);
        Fraction aY(nMulY,nDivY);
        NbcResize(aOld.TopLeft(),aX,aY);
    }
    if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
        NbcMove(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
    }
}


void SdrObjGroup::NbcSetLogicRect(const Rectangle& rRect)
{
    NbcSetSnapRect(rRect);
}


void SdrObjGroup::NbcMove(const Size& rSiz)
{
    MovePoint(aRefPoint,rSiz);
    if (pSub->GetObjCount()!=0) {
        SdrObjList* pOL=pSub;
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG i=0; i<nObjAnz; i++) {
            SdrObject* pObj=pOL->GetObj(i);
            pObj->NbcMove(rSiz);
        }
    } else {
        MoveRect(aOutRect,rSiz);
        SetRectsDirty();
    }
}


void SdrObjGroup::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
    if (bXMirr || bYMirr) {
        Point aRef1(GetSnapRect().Center());
        if (bXMirr) {
            Point aRef2(aRef1);
            aRef2.Y()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.X()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }
    ResizePoint(aRefPoint,rRef,xFact,yFact);
    if (pSub->GetObjCount()!=0) {
        SdrObjList* pOL=pSub;
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG i=0; i<nObjAnz; i++) {
            SdrObject* pObj=pOL->GetObj(i);
            pObj->NbcResize(rRef,xFact,yFact);
        }
    } else {
        ResizeRect(aOutRect,rRef,xFact,yFact);
        SetRectsDirty();
    }
}


void SdrObjGroup::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SetGlueReallyAbsolute(TRUE);
    nDrehWink=NormAngle360(nDrehWink+nWink);
    RotatePoint(aRefPoint,rRef,sn,cs);
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcRotate(rRef,nWink,sn,cs);
    }
    NbcRotateGluePoints(rRef,nWink,sn,cs);
    SetGlueReallyAbsolute(FALSE);
}


void SdrObjGroup::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(TRUE);
    MirrorPoint(aRefPoint,rRef1,rRef2); // fehlende Implementation in SvdEtc !!!
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcMirror(rRef1,rRef2);
    }
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(FALSE);
}


void SdrObjGroup::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SetGlueReallyAbsolute(TRUE);
    nShearWink+=nWink;
    ShearPoint(aRefPoint,rRef,tn);
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcShear(rRef,nWink,tn,bVShear);
    }
    NbcShearGluePoints(rRef,nWink,tn,bVShear);
    SetGlueReallyAbsolute(FALSE);
}


void SdrObjGroup::NbcSetAnchorPos(const Point& rPnt)
{
    aAnchor=rPnt;
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    MovePoint(aRefPoint,aSiz);
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcSetAnchorPos(rPnt);
    }
}


void SdrObjGroup::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    Rectangle aOld(GetSnapRect());
    long nMulX=rRect.Right()-rRect.Left();
    long nDivX=aOld.Right()-aOld.Left();
    long nMulY=rRect.Bottom()-rRect.Top();
    long nDivY=aOld.Bottom()-aOld.Top();
    if (nDivX==0) { nMulX=1; nDivX=1; }
    if (nDivY==0) { nMulY=1; nDivY=1; }
    if (nMulX!=nDivX || nMulY!=nDivY) {
        Fraction aX(nMulX,nDivX);
        Fraction aY(nMulY,nDivY);
        Resize(aOld.TopLeft(),aX,aY);
    }
    if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
        Move(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
    }
    SendRepaintBroadcast(TRUE);
    SetChanged();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::SetLogicRect(const Rectangle& rRect)
{
    SetSnapRect(rRect);
}


void SdrObjGroup::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        MovePoint(aRefPoint,rSiz);
        if (pSub->GetObjCount()!=0) {
            // #32383# Erst die Verbinder verschieben, dann den Rest
            SdrObjList* pOL=pSub;
            ULONG nObjAnz=pOL->GetObjCount();
            ULONG i;
            for (i=0; i<nObjAnz; i++) {
                SdrObject* pObj=pOL->GetObj(i);
                if (pObj->IsEdgeObj()) pObj->Move(rSiz);
            }
            for (i=0; i<nObjAnz; i++) {
                SdrObject* pObj=pOL->GetObj(i);
                if (!pObj->IsEdgeObj()) pObj->Move(rSiz);
            }
            SendRepaintBroadcast(TRUE);
        } else {
            SendRepaintBroadcast();
            MoveRect(aOutRect,rSiz);
            SetRectsDirty();
            SendRepaintBroadcast();
        }
        SetChanged();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}


void SdrObjGroup::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
        FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
        if (bXMirr || bYMirr) {
            Point aRef1(GetSnapRect().Center());
            if (bXMirr) {
                Point aRef2(aRef1);
                aRef2.Y()++;
                NbcMirrorGluePoints(aRef1,aRef2);
            }
            if (bYMirr) {
                Point aRef2(aRef1);
                aRef2.X()++;
                NbcMirrorGluePoints(aRef1,aRef2);
            }
        }
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        ResizePoint(aRefPoint,rRef,xFact,yFact);
        if (pSub->GetObjCount()!=0) {
            // #32383# Erst die Verbinder verschieben, dann den Rest
            SdrObjList* pOL=pSub;
            ULONG nObjAnz=pOL->GetObjCount();
            ULONG i;
            for (i=0; i<nObjAnz; i++) {
                SdrObject* pObj=pOL->GetObj(i);
                if (pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact);
            }
            for (i=0; i<nObjAnz; i++) {
                SdrObject* pObj=pOL->GetObj(i);
                if (!pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact);
            }
            SendRepaintBroadcast(TRUE);
        } else {
            SendRepaintBroadcast();
            ResizeRect(aOutRect,rRef,xFact,yFact);
            SetRectsDirty();
            SendRepaintBroadcast();
        }
        SetChanged();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
    if (nWink!=0) {
        SetGlueReallyAbsolute(TRUE);
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        nDrehWink=NormAngle360(nDrehWink+nWink);
        RotatePoint(aRefPoint,rRef,sn,cs);
        // #32383# Erst die Verbinder verschieben, dann den Rest
        SdrObjList* pOL=pSub;
        ULONG nObjAnz=pOL->GetObjCount();
        ULONG i;
        for (i=0; i<nObjAnz; i++) {
            SdrObject* pObj=pOL->GetObj(i);
            if (pObj->IsEdgeObj()) pObj->Rotate(rRef,nWink,sn,cs);
        }
        for (i=0; i<nObjAnz; i++) {
            SdrObject* pObj=pOL->GetObj(i);
            if (!pObj->IsEdgeObj()) pObj->Rotate(rRef,nWink,sn,cs);
        }
        NbcRotateGluePoints(rRef,nWink,sn,cs);
        SetGlueReallyAbsolute(FALSE);
        SendRepaintBroadcast(TRUE);
        SetChanged();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::Mirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(TRUE);
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    MirrorPoint(aRefPoint,rRef1,rRef2); // fehlende Implementation in SvdEtc !!!
    // #32383# Erst die Verbinder verschieben, dann den Rest
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    ULONG i;
    for (i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        if (pObj->IsEdgeObj()) pObj->Mirror(rRef1,rRef2);
    }
    for (i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        if (!pObj->IsEdgeObj()) pObj->Mirror(rRef1,rRef2);
    }
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(FALSE);
    SendRepaintBroadcast(TRUE);
    SetChanged();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    if (nWink!=0) {
        SetGlueReallyAbsolute(TRUE);
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        nShearWink+=nWink;
        ShearPoint(aRefPoint,rRef,tn);
        // #32383# Erst die Verbinder verschieben, dann den Rest
        SdrObjList* pOL=pSub;
        ULONG nObjAnz=pOL->GetObjCount();
        ULONG i;
        for (i=0; i<nObjAnz; i++) {
            SdrObject* pObj=pOL->GetObj(i);
            if (pObj->IsEdgeObj()) pObj->Shear(rRef,nWink,tn,bVShear);
        }
        for (i=0; i<nObjAnz; i++) {
            SdrObject* pObj=pOL->GetObj(i);
            if (!pObj->IsEdgeObj()) pObj->Shear(rRef,nWink,tn,bVShear);
        }
        NbcShearGluePoints(rRef,nWink,tn,bVShear);
        SetGlueReallyAbsolute(FALSE);
        SendRepaintBroadcast(TRUE);
        SetChanged();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::SetAnchorPos(const Point& rPnt)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    FASTBOOL bChg=aAnchor!=rPnt;
    aAnchor=rPnt;
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    MovePoint(aRefPoint,aSiz);
    // #32383# Erst die Verbinder verschieben, dann den Rest
    SdrObjList* pOL=pSub;
    ULONG nObjAnz=pOL->GetObjCount();
    ULONG i;
    for (i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        if (pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
    }
    for (i=0; i<nObjAnz; i++) {
        SdrObject* pObj=pOL->GetObj(i);
        if (!pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
    }
    if (bChg) {
        SendRepaintBroadcast(TRUE);
        SetChanged();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}



void SdrObjGroup::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}


void SdrObjGroup::SetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    if (aSiz.Width()!=0 || aSiz.Height()!=0) Move(aSiz); // Der ruft auch das SetRectsDirty() und Broadcast, ...
}


////////////////////////////////////////////////////////////////////////////////////////////////////

//-/void SdrObjGroup::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    const SfxItemSet* pAttr=&rAttr;
//-/    //
//-/    //SfxItemSet aSet(rAttr);
//-/    //pAttr=&aSet;
//-/    //for (USHORT nWhich=SDRATTR_NOTPERSIST_FIRST; nWhich<=SDRATTR_NOTPERSIST_LAST; nWhich++) {
//-/    //    aSet.InvalidateItem(nWhich);
//-/    //}
//-/
//-/    if (!IsLinkedGroup()) {
//-/        SdrObjList* pOL=pSub;
//-/        ULONG nObjAnz=pOL->GetObjCount();
//-/        for (ULONG i=0; i<nObjAnz; i++) {
//-/            pOL->GetObj(i)->NbcSetAttributes(*pAttr,bReplaceAll);
//-/        }
//-/    }
//-/
//-/    // NbcApplyNotPersistAttr(rAttr);
//-/}

//-/void SdrObjGroup::SetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    if (!IsLinkedGroup()) {
//-/        SdrObjList* pOL=pSub;
//-/        ULONG nObjAnz=pOL->GetObjCount();
//-/        for (ULONG i=0; i<nObjAnz; i++) {
//-/            pOL->GetObj(i)->SetAttributes(rAttr,bReplaceAll);
//-/        }
//-/    }
//-/}

//-/void SdrObjGroup::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
//-/{
//-/    SdrObjList* pOL=pSub;
//-/    ULONG nObjAnz=pOL->GetObjCount();
//-/    for (ULONG i=0; i<nObjAnz; i++) {
//-/        pOL->GetObj(i)->TakeAttributes(rAttr,TRUE,bOnlyHardAttr);
//-/    }
//-/
//-/    //if (bMerge) {
//-/        // NotPersist-Items erstmal mit Put, damit die Werte der Sub-Objekte ueberschrieben werden
//-/        // Todo: Muss mit den urspruenglichen Werten Gemerged werden!
//-/    //}
//-/    // TakeNotPersistAttr(rAttr,FALSE);
//-/}

////////////////////////////////////////////////////////////////////////////////////////////////////

// groups may contain 3d objects(?)
//-/SfxItemSet* SdrObjGroup::CreateNewItemSet(SfxItemPool& rPool)
//-/{
//-/    return new SfxItemSet(rPool,
//-/        SDRATTR_START,  SDRATTR_END,
//-/        SID_ATTR_3D_START, SID_ATTR_3D_END,
//-/        0, 0);
//-/}

const SfxItemSet& SdrObjGroup::GetItemSet() const
{
    if(!mpGroupItemSet)
    {
        ((SdrObjGroup*)this)->mpGroupItemSet =
        ((SdrObjGroup*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
        DBG_ASSERT(mpGroupItemSet, "Could not create an SfxItemSet(!)");
    }

    // collect all ItemSets in mpGroupItemSet
    mpGroupItemSet->ClearItem();

    sal_uInt32 nCount(pSub->GetObjCount());
    for(sal_uInt32 a(0); a < nCount; a++)
    {
//-/        mpGroupItemSet->MergeValues(pSub->GetObj(a)->GetItemSet(), TRUE);
        const SfxItemSet& rSet = pSub->GetObj(a)->GetItemSet();
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(nWhich)
        {
            const SfxPoolItem* pItem = NULL;
            rSet.GetItemState(nWhich, TRUE, &pItem);

            if(pItem)
            {
                if(pItem == (SfxPoolItem *)-1)
                    mpGroupItemSet->InvalidateItem(nWhich);
                else
                    mpGroupItemSet->MergeValue(*pItem, TRUE);
            }
            nWhich = aIter.NextWhich();
        }
    }

    return *mpGroupItemSet;
}

void SdrObjGroup::SetItem( const SfxPoolItem& rItem )
{
    if(!IsLinkedGroup())
    {
        sal_uInt32 nCount(pSub->GetObjCount());
        for(sal_uInt32 a(0); a < nCount; a++)
            pSub->GetObj(a)->SetItem( rItem );
    }
}

void SdrObjGroup::ClearItem( USHORT nWhich )
{
    if(!IsLinkedGroup())
    {
        sal_uInt32 nCount(pSub->GetObjCount());
        for(sal_uInt32 a(0); a < nCount; a++)
            pSub->GetObj(a)->ClearItem( nWhich );
    }
}

void SdrObjGroup::SetItemSet( const SfxItemSet& rSet )
{
    if(!IsLinkedGroup())
    {
        sal_uInt32 nCount(pSub->GetObjCount());
        for(sal_uInt32 a(0); a < nCount; a++)
            pSub->GetObj(a)->SetItemSet( rSet );
    }
}

SfxItemSet* SdrObjGroup::CreateNewItemSet(SfxItemPool& rPool)
{
    // include ALL items
    return new SfxItemSet(rPool, SDRATTR_START, SDRATTR_END);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void SdrObjGroup::PreSave()
{
    if(!IsLinkedGroup())
    {
        sal_uInt32 nCount(pSub->GetObjCount());
        for(sal_uInt32 a(0); a < nCount; a++)
            pSub->GetObj(a)->PreSave();
    }
}

void SdrObjGroup::PostSave()
{
    if(!IsLinkedGroup())
    {
        sal_uInt32 nCount(pSub->GetObjCount());
        for(sal_uInt32 a(0); a < nCount; a++)
            pSub->GetObj(a)->PostSave();
    }
}

//-/void SdrObjGroup::BroadcastItemChange(const SdrBroadcastItemChange& rChange)
//-/{
//-/    if(!IsLinkedGroup())
//-/    {
//-/        sal_uInt32 nCount(pSub->GetObjCount());
//-/        for(sal_uInt32 a(0); a < nCount; a++)
//-/            pSub->GetObj(a)->BroadcastItemChange(rOldRect);
//-/    }
//-/}

////////////////////////////////////////////////////////////////////////////////////////////////////

SfxStyleSheet* SdrObjGroup::GetStyleSheet() const
{
    // Hier mit 'nem Iterator. Es koennte sonst passieren dass ein
    // verschachteltes Gruppenobjekt wegen DontCare NULL liefert.
    // Das koennte ich dann nicht unterscheiden von NotSet.
    SfxStyleSheet* pRet=NULL;
    FASTBOOL b1st=TRUE;
    SdrObjListIter aIter(*this,IM_DEEPNOGROUPS);
    while (aIter.IsMore()) {
        SdrObject* pObj=aIter.Next();
        if (b1st) {
            b1st=FALSE;
            pRet=pObj->GetStyleSheet(); // Der Erste
        } else {
            if (pObj->GetStyleSheet()!=pRet) return NULL; // Unterschiedlich!
        }
    }
    // Wenn hier angekommen, sind alle gleich.
    return pRet;
}


void SdrObjGroup::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    if (!IsLinkedGroup()) {
        SdrObjList* pOL=pSub;
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG i=0; i<nObjAnz; i++) {
            pOL->GetObj(i)->NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
        }
    }
}


void SdrObjGroup::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    if (!IsLinkedGroup()) {
        SdrObjList* pOL=pSub;
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG i=0; i<nObjAnz; i++) {
            pOL->GetObj(i)->SetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
        }
    }
}


void SdrObjGroup::NbcReformatText()
{
    pSub->NbcReformatAllTextObjects();
}


void SdrObjGroup::ReformatText()
{
    pSub->ReformatAllTextObjects();
}


void SdrObjGroup::BurnInStyleSheetAttributes()
{
    pSub->BurnInStyleSheetAttributes();
}


void SdrObjGroup::RestartAnimation(SdrPageView* pPageView) const
{
    pSub->RestartAllAnimations(pPageView);
}


SdrObject* SdrObjGroup::DoConvertToPolyObj(BOOL bBezier) const
{
    SdrObject* pGroup = new SdrObjGroup;
    pGroup->SetModel(GetModel());

    for(UINT32 a=0;a<pSub->GetObjCount();a++)
    {
        SdrObject* pIterObj = pSub->GetObj(a);
        pGroup->GetSubList()->NbcInsertObject(pIterObj->DoConvertToPolyObj(bBezier));
    }

    return pGroup;
}


void SdrObjGroup::WriteData(SvStream& rOut) const
{
    SdrObject::WriteData(rOut);
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
    aCompat.SetID("SdrObjGroup");
#endif

    // UNICODE: rOut << aName;
    rOut.WriteByteString(aName);

    UINT8 nTemp = bRefPoint; rOut << nTemp;
    rOut << aRefPoint;
    pSub->Save(rOut);
    rOut << INT32(nDrehWink);
    rOut << INT32(nShearWink);
}


void SdrObjGroup::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if(rIn.GetError())
        return;

    SdrObject::ReadData(rHead, rIn);
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rIn, STREAM_READ);

#ifdef DBG_UTIL
    aCompat.SetID("SdrObjGroup");
#endif

    // UNICODE: rIn >> aName;
    rIn.ReadByteString(aName);

    UINT8 nTemp; rIn >> nTemp; bRefPoint = nTemp;
    rIn >> aRefPoint;
    pSub->Load(rIn, *pPage);

    if(rHead.GetVersion() >= 2)
    {
        INT32 n32;

        rIn >> n32; nDrehWink = n32;
        rIn >> n32; nShearWink = n32;
    }
}


void SdrObjGroup::AfterRead()
{
    SdrObject::AfterRead();
    pSub->AfterRead();
}

// ItemPool fuer dieses Objekt wechseln
void SdrObjGroup::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool)
{
    if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
    {
        // call parent
        SdrObject::MigrateItemPool(pSrcPool, pDestPool);

        // own reaction
        SdrObjList* pOL = pSub;
        sal_uInt32 nObjAnz(pOL->GetObjCount());

        for(sal_uInt32 a(0); a < nObjAnz; a++)
        {
            pOL->GetObj(a)->MigrateItemPool(pSrcPool, pDestPool);
        }
    }
}

