/*************************************************************************
 *
 *  $RCSfile: svdotxln.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:15:38 $
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

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>

#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif

#include "svdotext.hxx"
#include "svditext.hxx"
#include "svdmodel.hxx"
#include "svdio.hxx"
#include "editdata.hxx"

#ifndef SVX_LIGHT
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#endif

#ifndef _SVXLINKMGR_HXX //autogen
#include <linkmgr.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <svtools/urihelper.hxx>

#ifndef SVX_LIGHT
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@  @@@@@  @@@@@@  @@@@@@ @@@@@@ @@  @@ @@@@@@  @@    @@ @@  @@ @@  @@
// @@  @@ @@  @@     @@    @@   @@      @@@@    @@    @@    @@ @@@ @@ @@ @@
// @@  @@ @@@@@      @@    @@   @@@@@    @@     @@    @@    @@ @@@@@@ @@@@
// @@  @@ @@  @@ @@  @@    @@   @@      @@@@    @@    @@    @@ @@ @@@ @@ @@
//  @@@@  @@@@@   @@@@     @@   @@@@@@ @@  @@   @@    @@@@@ @@ @@  @@ @@  @@
//
// ImpSdrObjTextLink zur Verbindung von SdrTextObj und LinkManager
//
// Einem solchen Link merke ich mir als SdrObjUserData am Objekt. Im Gegensatz
// zum Grafik-Link werden die ObjektDaten jedoch kopiert (fuer Paint, etc.).
// Die Information ob das Objekt ein Link ist besteht genau darin, dass dem
// Objekt ein entsprechender UserData-Record angehaengt ist oder nicht.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrObjTextLink: public ::so3::SvBaseLink
{
    SdrTextObj*                 pSdrObj;

public:
    ImpSdrObjTextLink( SdrTextObj* pObj1 )
        : ::so3::SvBaseLink( LINKUPDATE_ONCALL, FORMAT_FILE ),
            pSdrObj( pObj1 )
    {}
    virtual ~ImpSdrObjTextLink();

    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    BOOL Connect() { return 0 != SvBaseLink::GetRealObject(); }
};

ImpSdrObjTextLink::~ImpSdrObjTextLink()
{
}

void ImpSdrObjTextLink::Closed()
{
    if (pSdrObj )
    {
        // pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
        ImpSdrObjTextLinkUserData* pData=pSdrObj->GetLinkUserData();
        if (pData!=NULL) pData->pLink=NULL;
        pSdrObj->ReleaseTextLink();
    }
    SvBaseLink::Closed();
}


void ImpSdrObjTextLink::DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
{
    FASTBOOL bForceReload=FALSE;
    SdrModel* pModel = pSdrObj ? pSdrObj->GetModel() : 0;
    SvxLinkManager* pLinkManager= pModel ? pModel->GetLinkManager() : 0;
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
                bForceReload = TRUE;
            }
        }
    }
    if (pSdrObj )
        pSdrObj->ReloadLinkedText( bForceReload );
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

TYPEINIT1(ImpSdrObjTextLinkUserData,SdrObjUserData);

ImpSdrObjTextLinkUserData::ImpSdrObjTextLinkUserData(SdrTextObj* pObj1):
    SdrObjUserData(SdrInventor,SDRUSERDATA_OBJTEXTLINK,0),
    pObj(pObj1),
    pLink(NULL),
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
    pData->pLink=NULL;
    return pData;
}

void ImpSdrObjTextLinkUserData::WriteData(SvStream& rOut)
{
    SdrObjUserData::WriteData(rOut);
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
    aCompat.SetID("ImpSdrObjTextLinkUserData");
#endif

    String aRelFileName;

    if( aFileName.Len() )
    {
        aRelFileName = INetURLObject::AbsToRel( aFileName,
                                                INetURLObject::WAS_ENCODED,
                                                INetURLObject::DECODE_UNAMBIGUOUS );
    }

    rOut.WriteByteString( aRelFileName );

    // UNICODE: rOut << aFilterName;
    rOut.WriteByteString(aFilterName);

    rOut << UINT16(GetStoreCharSet(eCharSet));
    rOut << UINT32(aFileDate0.GetDate());
    rOut << UINT32(aFileDate0.GetTime());
}

void ImpSdrObjTextLinkUserData::ReadData(SvStream& rIn)
{
    SdrObjUserData::ReadData(rIn);
    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rIn, STREAM_READ);

#ifdef DBG_UTIL
    aCompat.SetID("ImpSdrObjTextLinkUserData");
#endif

    UINT32 nTmp32;
    UINT16 nTmp16;
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

    // UNICODE: rIn >> aFilterName;
    rIn.ReadByteString(aFilterName);

    rIn >> nTmp16; eCharSet = rtl_TextEncoding(nTmp16);
    rIn >> nTmp32; aFileDate0.SetDate(nTmp32);
    rIn >> nTmp32; aFileDate0.SetTime(nTmp32);
}

void ImpSdrObjTextLinkUserData::AfterRead()
{
    if (pObj!=NULL) {
        pObj->ImpLinkAnmeldung();
        // lt. Anweisung von MB kein automatisches Reload mehr
        //pObj->ReloadLinkedText();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrTextObj::SetTextLink(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet)
{
    if(eCharSet == RTL_TEXTENCODING_DONTKNOW)
        eCharSet = gsl_getSystemTextEncoding();

    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    if (pData!=NULL) {
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
    USHORT nAnz=GetUserDataCount();
    for (USHORT nNum=nAnz; nNum>0;) {
        nNum--;
        SdrObjUserData* pData=GetUserData(nNum);
        if (pData->GetInventor()==SdrInventor && pData->GetId()==SDRUSERDATA_OBJTEXTLINK) {
            DeleteUserData(nNum);
        }
    }
}

FASTBOOL SdrTextObj::ReloadLinkedText(FASTBOOL bForceLoad)
{
    ImpSdrObjTextLinkUserData*  pData = GetLinkUserData();
    FASTBOOL                    bRet = TRUE;

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
                INetURLObject aURL( pData->aFileName );
                DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

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

            if( bLoad )
            {
                bRet = LoadText( pData->aFileName, pData->aFilterName, pData->eCharSet );
            }

            pData->aFileDate0 = aFileDT;
        }
    }

    return bRet;
}

FASTBOOL SdrTextObj::LoadText(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet)
{
    INetURLObject   aFileURL( rFileName );
    BOOL            bRet = FALSE;

    if( aFileURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aFileURLStr;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aFileURLStr ) )
            aFileURL = INetURLObject( aFileURLStr );
        else
            aFileURL.SetSmartURL( rFileName );
    }

    DBG_ASSERT( aFileURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileURL.GetMainURL(), STREAM_READ );

    if( pIStm )
    {
        pIStm->SetStreamCharSet( eCharSet );
        char cRTF[5];
        cRTF[4] = 0;
        pIStm->Read(cRTF, 5);

        BOOL bRTF = cRTF[0] == '{' && cRTF[1] == '\\' && cRTF[2] == 'r' && cRTF[3] == 't' && cRTF[4] == 'f';

        pIStm->Seek(0);

        if( !pIStm->GetError() )
        {
            SetText( *pIStm, bRTF ? EE_FORMAT_RTF : EE_FORMAT_TEXT );
            bRet = TRUE;
        }

        delete pIStm;
    }

    return bRet;
}

ImpSdrObjTextLinkUserData* SdrTextObj::GetLinkUserData() const
{
    ImpSdrObjTextLinkUserData* pData=NULL;
    USHORT nAnz=GetUserDataCount();
    for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
        nNum--;
        pData=(ImpSdrObjTextLinkUserData*)GetUserData(nNum);
        if (pData->GetInventor()!=SdrInventor || pData->GetId()!=SDRUSERDATA_OBJTEXTLINK) {
            pData=NULL;
        }
    }
    return pData;
}

void SdrTextObj::ImpLinkAnmeldung()
{
#ifndef SVX_LIGHT
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    SvxLinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
    if (pLinkManager!=NULL && pData!=NULL && pData->pLink==NULL) { // Nicht 2x Anmelden
        pData->pLink=new ImpSdrObjTextLink(this);
#ifdef GCC
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     pData->aFilterName.Len() ?
                                      &pData->aFilterName : (const String *)NULL,
                                     (const String *)NULL);
#else
        pLinkManager->InsertFileLink(*pData->pLink,OBJECT_CLIENT_FILE,pData->aFileName,
                                     pData->aFilterName.Len() ? &pData->aFilterName : NULL,NULL);
#endif
        pData->pLink->Connect();
    }
#endif // SVX_LIGHT
}

void SdrTextObj::ImpLinkAbmeldung()
{
#ifndef SVX_LIGHT
    ImpSdrObjTextLinkUserData* pData=GetLinkUserData();
    SvxLinkManager* pLinkManager=pModel!=NULL ? pModel->GetLinkManager() : NULL;
    if (pLinkManager!=NULL && pData!=NULL && pData->pLink!=NULL) { // Nicht 2x Abmelden
        // Bei Remove wird *pLink implizit deleted
        pLinkManager->Remove( pData->pLink );
        pData->pLink=NULL;
    }
#endif // SVX_LIGHT
}

