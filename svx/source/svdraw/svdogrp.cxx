/*************************************************************************
 *
 *  $RCSfile: svdogrp.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 10:59:55 $
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
#include <sfx2/lnkbase.hxx>
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

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef _SVDPOOL_HXX
#include <svdpool.hxx>
#endif

#ifndef _SDR_PROPERTIES_GROUPPROPERTIES_HXX
#include <svx/sdr/properties/groupproperties.hxx>
#endif

// #110094#
#ifndef _SDR_CONTACT_VIEWCONTACTOFGROUP_HXX
#include <svx/sdr/contact/viewcontactofgroup.hxx>
#endif

//BFS01#ifndef SVX_LIGHT

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

//BFS01class ImpSdrObjGroupLink: public so3::SvBaseLink
//BFS01{
//BFS01 SdrObject* pSdrObj;
//BFS01public:
//BFS01 ImpSdrObjGroupLink( SdrObject* pObj1 )
//BFS01     : ::so3::SvBaseLink( so3::LINKUPDATE_ONCALL, FORMAT_FILE ),
//BFS01     pSdrObj( pObj1 )
//BFS01 {}
//BFS01 virtual ~ImpSdrObjGroupLink();
//BFS01 virtual void Closed();
//BFS01 virtual void DataChanged( const String& rMimeType,
//BFS01                             const ::com::sun::star::uno::Any & rValue );
//BFS01
//BFS01 FASTBOOL     Connect() { return 0 != GetRealObject(); }
//BFS01};

////////////////////////////////////////////////////////////////////////////////////////////////////


//BFS01ImpSdrObjGroupLink::~ImpSdrObjGroupLink()
//BFS01{
//BFS01}

// Closed() wird gerufen, wenn die Verknüpfung geloesst wird.

//BFS01void ImpSdrObjGroupLink::Closed()
//BFS01{
//BFS01 if (pSdrObj!=NULL) {
//BFS01     // pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
//BFS01     ImpSdrObjGroupLinkUserData* pData=((SdrObjGroup*)pSdrObj)->GetLinkUserData();
//BFS01     if (pData!=NULL) pData->pLink=NULL;
//BFS01     ((SdrObjGroup*)pSdrObj)->ReleaseGroupLink();
//BFS01 }
//BFS01 SvBaseLink::Closed();
//BFS01}


//BFS01void ImpSdrObjGroupLink::DataChanged( const String& ,
//BFS01                                   const ::com::sun::star::uno::Any& )
//BFS01{
//BFS01 FASTBOOL bForceReload=FALSE;
//BFS01 SdrModel* pModel = pSdrObj ? pSdrObj->GetModel() : 0;
//BFS01 SvxLinkManager* pLinkManager= pModel ? pModel->GetLinkManager() : 0;
//BFS01 if( pLinkManager )
//BFS01 {
//BFS01     ImpSdrObjGroupLinkUserData* pData=
//BFS01                             ((SdrObjGroup*)pSdrObj)->GetLinkUserData();
//BFS01     if( pData )
//BFS01     {
//BFS01         String aFile;
//BFS01         String aName;
//BFS01         pLinkManager->GetDisplayNames( this, 0, &aFile, &aName, 0 );
//BFS01
//BFS01         if( !pData->aFileName.Equals( aFile ) ||
//BFS01             !pData->aObjName.Equals( aName ))
//BFS01         {
//BFS01             pData->aFileName=aFile;
//BFS01             pData->aObjName=aName;
//BFS01             pSdrObj->SetChanged();
//BFS01             bForceReload=TRUE;
//BFS01         }
//BFS01     }
//BFS01 }
//BFS01 if( pSdrObj )
//BFS01     ((SdrObjGroup*)pSdrObj)->ReloadLinkedGroup( bForceReload );
//BFS01}

//BFS01#endif // SVX_LIGHT

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// @@    @@ @@  @@ @@  @@  @@  @@  @@@@@ @@@@@@ @@@@@   @@@@@   @@@@  @@@@@@  @@@@
// @@    @@ @@@ @@ @@ @@   @@  @@ @@     @@     @@  @@  @@  @@ @@  @@   @@   @@  @@
// @@    @@ @@@@@@ @@@@    @@  @@  @@@@  @@@@@  @@@@@   @@  @@ @@@@@@   @@   @@@@@@
// @@    @@ @@ @@@ @@@@@   @@  @@     @@ @@     @@  @@  @@  @@ @@  @@   @@   @@  @@
// @@@@@ @@ @@  @@ @@  @@   @@@@  @@@@@  @@@@@@ @@  @@  @@@@@  @@  @@   @@   @@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//BFS01TYPEINIT1(ImpSdrObjGroupLinkUserData,SdrObjUserData);


//BFS01ImpSdrObjGroupLinkUserData::ImpSdrObjGroupLinkUserData(SdrObject* pObj1):
//BFS01 SdrObjUserData(SdrInventor,SDRUSERDATA_OBJGROUPLINK,0),
//BFS01 pObj(pObj1),
//BFS01 nDrehWink0(0),
//BFS01 nShearWink0(0),
//BFS01 pLink(NULL),
//BFS01 nObjNum(0),
//BFS01 nPageNum(0),
//BFS01 bMasterPage(FALSE),
//BFS01 bOrigPos(FALSE),
//BFS01 bOrigSize(FALSE),
//BFS01 bOrigRotate(FALSE),
//BFS01 bOrigShear(FALSE)
//BFS01{
//BFS01}


//BFS01ImpSdrObjGroupLinkUserData::~ImpSdrObjGroupLinkUserData()
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 delete pLink;
//BFS01#endif
//BFS01}


//BFS01SdrObjUserData* ImpSdrObjGroupLinkUserData::Clone(SdrObject* pObj1) const
//BFS01{
//BFS01 ImpSdrObjGroupLinkUserData* pData=new ImpSdrObjGroupLinkUserData(pObj1);
//BFS01 pData->aFileName  =aFileName;
//BFS01 pData->aObjName   =aObjName;
//BFS01 pData->aFileDate0 =aFileDate0;
//BFS01 pData->aSnapRect0 =aSnapRect0;
//BFS01 pData->nDrehWink0 =nDrehWink0;
//BFS01 pData->nShearWink0=nShearWink0;
//BFS01 pData->nObjNum    =nObjNum;
//BFS01 pData->nPageNum   =nPageNum;
//BFS01 pData->bMasterPage=bMasterPage;
//BFS01 pData->bOrigPos   =bOrigPos;
//BFS01 pData->bOrigSize  =bOrigSize;
//BFS01 pData->bOrigRotate=bOrigRotate;
//BFS01 pData->bOrigShear =bOrigShear;
//BFS01 pData->pLink=NULL;
//BFS01 //pObj1->ImpLinkAnmeldung();
//BFS01 return pData;
//BFS01}


//BFS01void ImpSdrObjGroupLinkUserData::WriteData(SvStream& rOut)
//BFS01{
//BFS01 SdrObjUserData::WriteData(rOut);
//BFS01
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("ImpSdrObjGroupLinkUserData");
//BFS01#endif
//BFS01
//BFS01 String aRelFileName;
//BFS01
//BFS01 if( aFileName.Len() )
//BFS01 {
//BFS01     aRelFileName = INetURLObject::AbsToRel( aFileName,
//BFS01                                             INetURLObject::WAS_ENCODED,
//BFS01                                             INetURLObject::DECODE_UNAMBIGUOUS );
//BFS01 }
//BFS01
//BFS01 rOut.WriteByteString( aRelFileName );
//BFS01
//BFS01 // UNICODE: rOut << aObjName;
//BFS01 rOut.WriteByteString(aObjName);
//BFS01
//BFS01 rOut << UINT32(aFileDate0.GetDate());
//BFS01 rOut << UINT32(aFileDate0.GetTime());
//BFS01 rOut << aSnapRect0;
//BFS01 rOut << nDrehWink0;
//BFS01 rOut << nShearWink0;
//BFS01 rOut << BOOL(bMasterPage);
//BFS01 rOut << nPageNum;
//BFS01 rOut << nObjNum;
//BFS01 rOut << BOOL(bOrigPos);
//BFS01 rOut << BOOL(bOrigSize);
//BFS01 rOut << BOOL(bOrigRotate);
//BFS01 rOut << BOOL(bOrigShear);
//BFS01}

//BFS01void ImpSdrObjGroupLinkUserData::ReadData(SvStream& rIn)
//BFS01{
//BFS01 SdrObjUserData::ReadData(rIn);
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("ImpSdrObjGroupLinkUserData");
//BFS01#endif
//BFS01
//BFS01 BOOL bTmp;
//BFS01 UINT32 nTmp32;
//BFS01 String aFileNameRel;
//BFS01
//BFS01 rIn.ReadByteString(aFileNameRel);
//BFS01
//BFS01 if( aFileNameRel.Len() )
//BFS01 {
//BFS01     aFileName = ::URIHelper::SmartRelToAbs( aFileNameRel, FALSE,
//BFS01                                             INetURLObject::WAS_ENCODED,
//BFS01                                             INetURLObject::DECODE_UNAMBIGUOUS );
//BFS01 }
//BFS01 else
//BFS01     aFileName.Erase();
//BFS01
//BFS01 // UNICODE: rIn >> aObjName;
//BFS01 rIn.ReadByteString(aObjName);
//BFS01
//BFS01 rIn >> nTmp32; aFileDate0.SetDate(nTmp32);
//BFS01 rIn >> nTmp32; aFileDate0.SetTime(nTmp32);
//BFS01 rIn >> aSnapRect0;
//BFS01 rIn >> nDrehWink0;
//BFS01 rIn >> nShearWink0;
//BFS01 rIn >> bTmp; bMasterPage=bTmp;
//BFS01 rIn >> nPageNum;
//BFS01 rIn >> nObjNum;
//BFS01 rIn >> bTmp; bOrigPos   =bTmp;
//BFS01 rIn >> bTmp; bOrigSize  =bTmp;
//BFS01 rIn >> bTmp; bOrigRotate=bTmp;
//BFS01 rIn >> bTmp; bOrigShear =bTmp;
//BFS01}


//BFS01void ImpSdrObjGroupLinkUserData::AfterRead()
//BFS01{
//BFS01 if (pObj!=NULL) {
//BFS01     ((SdrObjGroup*)pObj)->ImpLinkAnmeldung();
//BFS01     // lt. Anweisung von MB kein automatisches Reload mehr
//BFS01     //((SdrObjGroup*)pObj)->ReloadLinkedGroup();
//BFS01 }
//BFS01}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@   @@@@  @@@@@   @@@@  @@  @@ @@@@@
//  @@  @@ @@  @@     @@  @@     @@  @@ @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@ @@@ @@@@@  @@  @@ @@  @@ @@@@@
//  @@  @@ @@  @@ @@  @@  @@  @@ @@  @@ @@  @@ @@  @@ @@
//   @@@@  @@@@@   @@@@    @@@@@ @@  @@  @@@@   @@@@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrObjGroup::CreateObjectSpecificProperties()
{
    return new sdr::properties::GroupProperties((SdrObject&)(*this));
}

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* SdrObjGroup::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfGroup(*this);
}

//////////////////////////////////////////////////////////////////////////////

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
}


SdrObjGroup::~SdrObjGroup()
{
    //BFS01ReleaseGroupLink();
    delete pSub;
}


//BFS01void SdrObjGroup::SetGroupLink(const String& rFileName, const String& rObjName)
//BFS01{
//BFS01 ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
//BFS01 if (pData!=NULL) {
//BFS01     nDrehWink-=pData->nDrehWink0;
//BFS01     nShearWink-=pData->nShearWink0;
//BFS01     ReleaseGroupLink();
//BFS01 }
//BFS01 aName=rObjName;
//BFS01 pData=new ImpSdrObjGroupLinkUserData(this);
//BFS01 pData->aFileName=rFileName;
//BFS01 pData->aObjName=rObjName;
//BFS01 InsertUserData(pData);
//BFS01 ImpLinkAnmeldung();
//BFS01}


//BFS01void SdrObjGroup::ReleaseGroupLink()
//BFS01{
//BFS01 ImpLinkAbmeldung();
//BFS01 USHORT nAnz=GetUserDataCount();
//BFS01 for (USHORT nNum=nAnz; nNum>0;) {
//BFS01     nNum--;
//BFS01     SdrObjUserData* pData=GetUserData(nNum);
//BFS01     if (pData->GetInventor()==SdrInventor && pData->GetId()==SDRUSERDATA_OBJGROUPLINK) {
//BFS01         DeleteUserData(nNum);
//BFS01     }
//BFS01 }
//BFS01}


//BFS01ImpSdrObjGroupLinkUserData* SdrObjGroup::GetLinkUserData() const
//BFS01{
//BFS01 ImpSdrObjGroupLinkUserData* pData=NULL;
//BFS01 USHORT nAnz=GetUserDataCount();
//BFS01 for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
//BFS01     nNum--;
//BFS01     pData=(ImpSdrObjGroupLinkUserData*)GetUserData(nNum);
//BFS01     if (pData->GetInventor()!=SdrInventor || pData->GetId()!=SDRUSERDATA_OBJGROUPLINK) {
//BFS01         pData=NULL;
//BFS01     }
//BFS01 }
//BFS01 return pData;
//BFS01}


//BFS01FASTBOOL SdrObjGroup::ReloadLinkedGroup(FASTBOOL bForceLoad)
//BFS01{
//BFS01 ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
//BFS01 FASTBOOL                    bRet=TRUE;
//BFS01
//BFS01 if( pData )
//BFS01 {
//BFS01     ::ucb::ContentBroker*   pBroker = ::ucb::ContentBroker::get();
//BFS01     DateTime                aFileDT;
//BFS01     BOOL                    bExists = FALSE, bLoad = FALSE;
//BFS01
//BFS01     if( pBroker )
//BFS01     {
//BFS01         bExists = TRUE;
//BFS01
//BFS01         try
//BFS01         {
//BFS01             INetURLObject aURL( pData->aFileName );
//BFS01             DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
//BFS01
//BFS01             ::ucb::Content aCnt( aURL.GetMainURL( INetURLObject::NO_DECODE ), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
//BFS01             ::com::sun::star::uno::Any aAny( aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ) ) );
//BFS01             ::com::sun::star::util::DateTime aDateTime;
//BFS01
//BFS01             aAny >>= aDateTime;
//BFS01             ::utl::typeConvert( aDateTime, aFileDT );
//BFS01         }
//BFS01         catch( ... )
//BFS01         {
//BFS01             bExists = FALSE;
//BFS01         }
//BFS01     }
//BFS01
//BFS01     if( bExists )
//BFS01     {
//BFS01         if( bForceLoad )
//BFS01             bLoad = TRUE;
//BFS01         else
//BFS01             bLoad = ( aFileDT > pData->aFileDate0 );
//BFS01
//BFS01         pData->aFileDate0 = aFileDT;
//BFS01     }
//BFS01     else
//BFS01         bLoad = ( pModel!=NULL && pModel->LoadModel(pData->aFileName)!=NULL );
//BFS01
//BFS01     if( bLoad )
//BFS01     {
//BFS01         Rectangle aMyRect(GetSnapRect());
//BFS01         bRet=LoadGroup(pData->aFileName,pData->aObjName,&pData->nPageNum,&pData->bMasterPage,&pData->nObjNum);
//BFS01         Rectangle aOrgRect(GetSnapRect());
//BFS01         if (bRet && !aMyRect.IsEmpty() && !aOrgRect.IsEmpty())
//BFS01         { // und nun noch zurechttransformieren
//BFS01             if (aMyRect!=aOrgRect)
//BFS01             {
//BFS01                 // erstmal karo-einfach
//BFS01                 NbcSetSnapRect(aMyRect);
//BFS01             }
//BFS01         }
//BFS01         pData->aSnapRect0=aOrgRect; // letzte bekannte Groesse des Originalobjekts merken
//BFS01     }
//BFS01 }
//BFS01 return bRet;
//BFS01}


//BFS01FASTBOOL SdrObjGroup::LoadGroup(const String& rFileName, const String& rObjName, USHORT* pnPgNum, FASTBOOL* pbMasterPg, ULONG* pnObjNum)
//BFS01{
//BFS01 FASTBOOL bRet=FALSE;
//BFS01
//BFS01 if(pModel && rFileName.Len() && rObjName.Len())
//BFS01 {
//BFS01     const SdrModel* pTempModel=pModel->LoadModel(rFileName);
//BFS01     if (pTempModel!=NULL) {
//BFS01         SdrObjGroup* pRef=NULL;
//BFS01         for (FASTBOOL bMPg=FALSE; bMPg!=TRUE && pRef==NULL;) {
//BFS01             USHORT nPgAnz=bMPg ? pTempModel->GetMasterPageCount() : pTempModel->GetPageCount();
//BFS01             for (USHORT nPgNum=0; nPgNum<nPgAnz && pRef==NULL; nPgNum++) {
//BFS01                 const SdrPage* pPg=bMPg ? pTempModel->GetMasterPage(nPgNum) : pTempModel->GetPage(nPgNum);
//BFS01                 ULONG nObjAnz=pPg->GetObjCount();
//BFS01                 for (USHORT nObjNum=0; nObjNum<nObjAnz && pRef==NULL; nObjNum++) {
//BFS01                     SdrObject* pObj=pPg->GetObj(nObjNum);
//BFS01                     SdrObjGroup* pGrp=PTR_CAST(SdrObjGroup,pObj);
//BFS01
//BFS01                     if(pGrp && pGrp->GetName().Equals(rObjName))
//BFS01                     {
//BFS01                         pRef = pGrp;
//BFS01
//BFS01                         if(pnPgNum)
//BFS01                             *pnPgNum = nPgNum;
//BFS01
//BFS01                         if(pbMasterPg)
//BFS01                             *pbMasterPg = bMPg;
//BFS01
//BFS01                         if(pnObjNum)
//BFS01                             *pnObjNum = nObjNum;
//BFS01
//BFS01                         bRet = TRUE;
//BFS01                     }
//BFS01                 }
//BFS01             }
//BFS01             bMPg=TRUE; // soz. von FALSE auf TRUE inkrementieren (fuer die obige for-Schleife)
//BFS01         }
//BFS01         if (pRef!=NULL) {
//BFS01             Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
//BFS01             // #110094#-14 SendRepaintBroadcast();
//BFS01             // zunaechst diverse Daten des Obj kopieren
//BFS01             nLayerId=pRef->GetLayer(); // hier noch ueberarbeiten !!!
//BFS01             aAnchor =pRef->aAnchor;
//BFS01             bVirtObj=pRef->bVirtObj;
//BFS01             bSizProt=pRef->bSizProt;
//BFS01             bMovProt=pRef->bMovProt;
//BFS01             bNoPrint=pRef->bNoPrint;
//BFS01             bEmptyPresObj=pRef->bEmptyPresObj;
//BFS01             bNotVisibleAsMaster=pRef->bNotVisibleAsMaster;
//BFS01             // und nun die Objekte rueberhohlen
//BFS01             pSub->Clear();
//BFS01             pSub->CopyObjects(*pRef->GetSubList());
//BFS01             SetChanged();
//BFS01             BroadcastObjectChange();
//BFS01             SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
//BFS01         }
//BFS01         //delete pTempModel;
//BFS01     }
//BFS01     if (!pModel->IsLoading()) pModel->DisposeLoadedModels();
//BFS01 }
//BFS01 return bRet;
//BFS01}


//BFS01void SdrObjGroup::ImpLinkAnmeldung()
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
//BFS01 SvxLinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
//BFS01 if ( pLinkManager         &&
//BFS01      pData!=NULL          &&
//BFS01      pData->pLink == NULL &&
//BFS01      pModel->GetPersist() )
//BFS01 {
//BFS01     // Nicht 2x Anmelden
//BFS01     INetURLObject aURLObj( ::URIHelper::SmartRelToAbs( pModel->GetPersist()->GetFileName(), FALSE,
//BFS01                                                        INetURLObject::WAS_ENCODED,
//BFS01                                                        INetURLObject::DECODE_UNAMBIGUOUS ) );
//BFS01     INetURLObject aLinkURLObj( ::URIHelper::SmartRelToAbs( pData->aFileName, FALSE,
//BFS01                                                            INetURLObject::WAS_ENCODED,
//BFS01                                                            INetURLObject::DECODE_UNAMBIGUOUS ) );
//BFS01
//BFS01     if( !aURLObj.GetMainURL( INetURLObject::NO_DECODE ).Equals( aLinkURLObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
//BFS01     {
//BFS01         // Keine gelinkten Objekte im eigenen Model
//BFS01         pData->pLink = new ImpSdrObjGroupLink(this);
//BFS01         pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,
//BFS01                                      pData->aFileName,NULL,&pData->aObjName);
//BFS01         pData->pLink->Connect();
//BFS01     }
//BFS01 }
//BFS01#endif // SVX_LIGHT
//BFS01}


//BFS01void SdrObjGroup::ImpLinkAbmeldung()
//BFS01{
//BFS01#ifndef SVX_LIGHT
//BFS01 ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
//BFS01 SvxLinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
//BFS01 if (pLinkManager!=NULL && pData!=NULL && pData->pLink!=NULL) { // Nicht 2x Abmelden
//BFS01     // Bei Remove wird *pLink implizit deleted
//BFS01     pLinkManager->Remove( pData->pLink );
//BFS01     pData->pLink=NULL;
//BFS01 }
//BFS01#endif // SVX_LIGHT
//BFS01}


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
        if (!aInfo.bEdgeRadiusAllowed      ) rInfo.bEdgeRadiusAllowed      =FALSE;
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
        rInfo.bEdgeRadiusAllowed=FALSE;
        rInfo.bNoContortion     =TRUE;
    }
    if(nObjAnz != 1)
    {
        // only allowed if single object selected
        rInfo.bTransparenceAllowed = FALSE;
        rInfo.bGradientAllowed = FALSE;
    }
    //BFS01if (pPlusData!=NULL && nObjAnz!=0) {
    //BFS01 ImpSdrObjGroupLinkUserData* pData=GetLinkUserData();
    //BFS01 if (pData!=NULL) {
    //BFS01     if (pData->bOrigPos   ) rInfo.bMoveAllowed =FALSE;
    //BFS01     if (pData->bOrigSize  ) { rInfo.bResizeFreeAllowed=FALSE; rInfo.bResizePropAllowed=FALSE; }
    //BFS01     if (pData->bOrigRotate) rInfo.bMoveAllowed =FALSE;
    //BFS01     if (pData->bOrigShear ) rInfo.bMoveAllowed =FALSE;
    //BFS01     // erstmal alles abschalten
    //BFS01     //rInfo.bResizeFreeAllowed=FALSE;
    //BFS01     //rInfo.bResizePropAllowed=FALSE;
    //BFS01     rInfo.bRotateFreeAllowed=FALSE;
    //BFS01     rInfo.bRotate90Allowed  =FALSE;
    //BFS01     rInfo.bMirrorFreeAllowed=FALSE;
    //BFS01     rInfo.bMirror45Allowed=FALSE;
    //BFS01     rInfo.bMirror90Allowed=FALSE;
    //BFS01     rInfo.bShearAllowed=FALSE;
    //BFS01     rInfo.bShearAllowed=FALSE;
    //BFS01     rInfo.bNoContortion=TRUE;
    //BFS01     // default: Proportionen beibehalten
    //BFS01     rInfo.bNoOrthoDesired=FALSE;
    //BFS01 }
    //BFS01}
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
    //BFS01FASTBOOL bLinked=IsLinkedGroup();
    FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
    FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;

    //BFS01if (bLinked && bRemove) {
    //BFS01 ImpLinkAbmeldung();
    //BFS01}

    SdrObject::SetPage(pNewPage);
    pSub->SetPage(pNewPage);

    //BFS01if (bLinked && bInsert) {
    //BFS01 ImpLinkAnmeldung();
    //BFS01}
}


void SdrObjGroup::SetModel(SdrModel* pNewModel)
{
    // #i30648#
    // This method also needs to migrate the used ItemSet
    // when the destination model uses a different pool
    // than the current one. Else it is possible to create
    // SdrObjGroups which reference the old pool which might
    // be destroyed (as the bug shows).
    SdrModel* pOldModel = pModel;
    //BFS01const sal_Bool bLinked(IsLinkedGroup());
    const sal_Bool bChg(pNewModel!=pModel);

    //BFS01if(bLinked && bChg)
    //BFS01{
    //BFS01 ImpLinkAbmeldung();
    //BFS01}

    // test for correct pool in ItemSet; move to new pool if necessary
    if(pNewModel && GetObjectItemPool() && GetObjectItemPool() != &pNewModel->GetItemPool())
    {
        MigrateItemPool(GetObjectItemPool(), &pNewModel->GetItemPool(), pNewModel);
    }

    // call parent
    SdrObject::SetModel(pNewModel);

    // set new model at content
    pSub->SetModel(pNewModel);

    //BFS01if(bLinked && bChg)
    //BFS01{
    //BFS01 ImpLinkAnmeldung();
    //BFS01}

    // modify properties
    GetProperties().SetModel(pOldModel, pNewModel);
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
    aName = rStr;
    SetChanged();
}


XubString SdrObjGroup::GetName() const
{
    return aName;
}


const Rectangle& SdrObjGroup::GetCurrentBoundRect() const
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
    //BFS01if(IsLinkedGroup())
    //BFS01{
    //BFS01 rName = ImpGetResStr(STR_ObjNameSingulGRUPLNK);
    //BFS01}
    //BFS01else
    if(!pSub->GetObjCount())
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
    //BFS01if (IsLinkedGroup()) {
    //BFS01 rName=ImpGetResStr(STR_ObjNamePluralGRUPLNK);
    //BFS01} else
    if (pSub->GetObjCount()==0) {
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

//#110094#-12
//void SdrObjGroup::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
//{
//}


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
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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

    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::SetLogicRect(const Rectangle& rRect)
{
    SetSnapRect(rRect);
}


void SdrObjGroup::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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
        } else {
            // #110094#-14 SendRepaintBroadcast();
            MoveRect(aOutRect,rSiz);
            SetRectsDirty();
        }

        SetChanged();
        BroadcastObjectChange();
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
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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
        } else {
            // #110094#-14 SendRepaintBroadcast();
            ResizeRect(aOutRect,rRef,xFact,yFact);
            SetRectsDirty();
        }

        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
    if (nWink!=0) {
        SetGlueReallyAbsolute(TRUE);
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::Mirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(TRUE);
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    if (nWink!=0) {
        SetGlueReallyAbsolute(TRUE);
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::SetAnchorPos(const Point& rPnt)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
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
        SetChanged();
        BroadcastObjectChange();
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

void SdrObjGroup::NbcReformatText()
{
    pSub->NbcReformatAllTextObjects();
}

void SdrObjGroup::ReformatText()
{
    pSub->ReformatAllTextObjects();
}

SdrObject* SdrObjGroup::DoConvertToPolyObj(BOOL bBezier) const
{
    SdrObject* pGroup = new SdrObjGroup;
    pGroup->SetModel(GetModel());

    for(UINT32 a=0;a<pSub->GetObjCount();a++)
    {
        SdrObject* pIterObj = pSub->GetObj(a);
        SdrObject* pResult = pIterObj->DoConvertToPolyObj(bBezier);

        // pResult can be NULL e.g. for empty objects
        if( pResult )
            pGroup->GetSubList()->NbcInsertObject(pResult);
    }

    return pGroup;
}


//BFS01void SdrObjGroup::WriteData(SvStream& rOut) const
//BFS01{
//BFS01 SdrObject::WriteData(rOut);
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rOut, STREAM_WRITE);
//BFS01
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("SdrObjGroup");
//BFS01#endif
//BFS01
//BFS01 // UNICODE: rOut << aName;
//BFS01 rOut.WriteByteString(aName);
//BFS01
//BFS01 UINT8 nTemp = bRefPoint; rOut << nTemp;
//BFS01 rOut << aRefPoint;
//BFS01 pSub->Save(rOut);
//BFS01 rOut << INT32(nDrehWink);
//BFS01 rOut << INT32(nShearWink);
//BFS01}


//BFS01void SdrObjGroup::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
//BFS01{
//BFS01 if(rIn.GetError())
//BFS01     return;
//BFS01
//BFS01 SdrObject::ReadData(rHead, rIn);
//BFS01 // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//BFS01 SdrDownCompat aCompat(rIn, STREAM_READ);
//BFS01
//BFS01#ifdef DBG_UTIL
//BFS01 aCompat.SetID("SdrObjGroup");
//BFS01#endif
//BFS01
//BFS01 // UNICODE: rIn >> aName;
//BFS01 rIn.ReadByteString(aName);
//BFS01
//BFS01 UINT8 nTemp; rIn >> nTemp; bRefPoint = nTemp;
//BFS01 rIn >> aRefPoint;
//BFS01 pSub->Load(rIn, *pPage);
//BFS01
//BFS01 if(rHead.GetVersion() >= 2)
//BFS01 {
//BFS01     INT32 n32;
//BFS01
//BFS01     rIn >> n32; nDrehWink = n32;
//BFS01     rIn >> n32; nShearWink = n32;
//BFS01 }
//BFS01}

//BFS01void SdrObjGroup::AfterRead()
//BFS01{
//BFS01 SdrObject::AfterRead();
//BFS01 pSub->AfterRead();
//BFS01
//BFS01 // #80049# as fix for errors after #69055#
//BFS01 if(aAnchor.X() || aAnchor.Y())
//BFS01     NbcSetAnchorPos(aAnchor);
//BFS01}

// eof
