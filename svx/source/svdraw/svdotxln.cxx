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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <unotools/datetime.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/editdata.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <tools/tenccvt.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrObjTextLink: public ::sfx2::SvBaseLink
{
    SdrTextObj*                 pSdrObj;

public:
    ImpSdrObjTextLink( SdrTextObj* pObj1 )
        : ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, FORMAT_FILE ),
            pSdrObj( pObj1 )
    {}
    virtual ~ImpSdrObjTextLink();

    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    bool Connect() { return 0 != SvBaseLink::GetRealObject(); }
};

ImpSdrObjTextLink::~ImpSdrObjTextLink()
{
}

void ImpSdrObjTextLink::Closed()
{
    if (pSdrObj )
    {
        // pLink des Objekts auf 0 setzen, da die Link-Instanz ja gerade destruiert wird.
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if (pData!=0) pData->pLink=0;
        pSdrObj->ReleaseTextLink();
    }
    SvBaseLink::Closed();
}


void ImpSdrObjTextLink::DataChanged( const String& /*rMimeType*/,
                                const ::com::sun::star::uno::Any & /*rValue */)
{
    bool bForceReload=false;
    SdrModel* pModel = pSdrObj ? &pSdrObj->getSdrModelFromSdrObject() : 0;
    sfx2::LinkManager* pLinkManager= pModel ? pModel->GetLinkManager() : 0;
    if( pLinkManager )
    {
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if( pData )
        {
            String aFile;
            String aFilter;
            pLinkManager->GetDisplayNames( this, 0,&aFile, 0, &aFilter );

            if( !pData->aFileName.Equals( aFile ) ||
                !pData->aFilterName.Equals( aFilter ))
            {
                pData->aFileName = aFile;
                pData->aFilterName = aFilter;
                pSdrObj->SetChanged();
                bForceReload = true;
            }
        }
    }
    if (pSdrObj )
        pSdrObj->ReloadLinkedText( bForceReload );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpSdrObjTextLinkUserData::ImpSdrObjTextLinkUserData(SdrTextObj* pObj1):
    SdrObjUserData(SdrInventor,SDRUSERDATA_OBJTEXTLINK),
    pObj(pObj1),
    pLink(0),
    eCharSet(RTL_TEXTENCODING_DONTKNOW)
{
}

ImpSdrObjTextLinkUserData::~ImpSdrObjTextLinkUserData()
{
#ifndef SVX_LIGHT
    delete pLink;
#endif
}

SdrObjUserData* ImpSdrObjTextLinkUserData::Clone(SdrObject* pObj1) const
{
    ImpSdrObjTextLinkUserData* pData=new ImpSdrObjTextLinkUserData((SdrTextObj*)pObj1);
    pData->aFileName  =aFileName;
    pData->aFilterName=aFilterName;
    pData->aFileDate0 =aFileDate0;
    pData->eCharSet   =eCharSet;
    pData->pLink=0;
    return pData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrTextObj::SetTextLink(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet)
{
    if(eCharSet == RTL_TEXTENCODING_DONTKNOW)
        eCharSet = gsl_getSystemTextEncoding();

    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    if (pData!=0) {
        ReleaseTextLink();
    }
    pData=new ImpSdrObjTextLinkUserData(this);
    pData->aFileName=rFileName;
    pData->aFilterName=rFilterName;
    pData->eCharSet=eCharSet;
    InsertUserData(pData);
    ImpLinkAnmeldung();
}

void SdrTextObj::ReleaseTextLink()
{
    ImpLinkAbmeldung();
    const sal_uInt32 nAnz(GetUserDataCount());

    for (sal_uInt32 nNum(nAnz); nNum > 0;)
    {
        nNum--;
        SdrObjUserData* pData=GetUserData(nNum);

        if(pData->GetInventor()==SdrInventor && pData->GetId()==SDRUSERDATA_OBJTEXTLINK)
        {
            DeleteUserData(nNum);
        }
    }
}

bool SdrTextObj::ReloadLinkedText( bool bForceLoad)
{
    ImpSdrObjTextLinkUserData*  pData = GetLinkUserData();
    bool                    bRet = true;

    if( pData )
    {
        ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
        DateTime                    aFileDT;
        bool bExists = false, bLoad = false;

        if( pBroker )
        {
            bExists = true;

            try
            {
                INetURLObject aURL( pData->aFileName );
                DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

                ::ucbhelper::Content aCnt( aURL.GetMainURL( INetURLObject::NO_DECODE ), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
                ::com::sun::star::uno::Any aAny( aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ) ) );
                ::com::sun::star::util::DateTime aDateTime;

                aAny >>= aDateTime;
                ::utl::typeConvert( aDateTime, aFileDT );
            }
            catch( ... )
            {
                bExists = false;
            }
        }

        if( bExists )
        {
            if( bForceLoad )
                bLoad = true;
            else
                bLoad = ( aFileDT > pData->aFileDate0 );

            if( bLoad )
            {
                bRet = LoadText( pData->aFileName, pData->aFilterName, pData->eCharSet );
            }

            pData->aFileDate0 = aFileDT;
        }
    }

    return bRet;
}

bool SdrTextObj::LoadText(const String& rFileName, const String& /*rFilterName*/, rtl_TextEncoding eCharSet)
{
    INetURLObject   aFileURL( rFileName );
    bool            bRet = false;

    if( aFileURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aFileURLStr;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aFileURLStr ) )
            aFileURL = INetURLObject( aFileURLStr );
        else
            aFileURL.SetSmartURL( rFileName );
    }

    DBG_ASSERT( aFileURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        // #90477# pIStm->SetStreamCharSet( eCharSet );
        pIStm->SetStreamCharSet(GetSOLoadTextEncoding(eCharSet, (sal_uInt16)pIStm->GetVersion()));

        char cRTF[5];
        cRTF[4] = 0;
        pIStm->Read(cRTF, 5);

        bool bRTF = cRTF[0] == '{' && cRTF[1] == '\\' && cRTF[2] == 'r' && cRTF[3] == 't' && cRTF[4] == 'f';

        pIStm->Seek(0);

        if( !pIStm->GetError() )
        {
            SetText( *pIStm, aFileURL.GetMainURL( INetURLObject::NO_DECODE ), sal::static_int_cast< sal_uInt16 >( bRTF ? EE_FORMAT_RTF : EE_FORMAT_TEXT ) );
            bRet = true;
        }

        delete pIStm;
    }

    return bRet;
}

ImpSdrObjTextLinkUserData* SdrTextObj::GetLinkUserData() const
{
    ImpSdrObjTextLinkUserData* pData=0;
    const sal_uInt32 nAnz(GetUserDataCount());

    for(sal_uInt32 nNum(nAnz); nNum > 0 && !pData;)
    {
        nNum--;
        pData=(ImpSdrObjTextLinkUserData*)GetUserData(nNum);

        if(pData->GetInventor()!=SdrInventor || pData->GetId()!=SDRUSERDATA_OBJTEXTLINK)
        {
            pData = 0;
        }
    }

    return pData;
}

void SdrTextObj::ImpLinkAnmeldung()
{
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    sfx2::LinkManager* pLinkManager = getSdrModelFromSdrObject().GetLinkManager();
    if (pLinkManager!=0 && pData!=0 && pData->pLink==0) { // Nicht 2x Anmelden
        pData->pLink=new ImpSdrObjTextLink(this);
#ifdef GCC
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     pData->aFilterName.Len() ?
                                      &pData->aFilterName : (const String *)0,
                                     (const String *)0);
#else
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     pData->aFilterName.Len() ? &pData->aFilterName : 0,0);
#endif
        pData->pLink->Connect();
    }
}

void SdrTextObj::ImpLinkAbmeldung()
{
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    sfx2::LinkManager* pLinkManager = getSdrModelFromSdrObject().GetLinkManager();
    if (pLinkManager!=0 && pData!=0 && pData->pLink!=0) { // Nicht 2x Abmelden
        // Bei Remove wird *pLink implizit deleted
        pLinkManager->Remove( pData->pLink );
        pData->pLink=0;
    }
}

