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

#include <config_features.h>

#include <textapi.hxx>

#include <hintids.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/text/UserFieldFormat.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/DocumentStatistic.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <unotools/localedatawrapper.hxx>
#include <editeng/unolingu.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <svl/urihelper.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/syslocale.hxx>
#include <svl/zforlist.hxx>

#include <tools/time.hxx>
#include <tools/datetime.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>

#include <swmodule.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/doctempl.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <charfmt.hxx>
#include <docstat.hxx>
#include <pagedesc.hxx>
#include <fmtpdsc.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <viewsh.hxx>
#include <dbmgr.hxx>
#include <shellres.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <docfld.hxx>
#include <ndtxt.hxx>
#include <expfld.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <unofldmid.h>
#include <swunohelper.hxx>
#include <comcore.hrc>

#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <calbck.hxx>
#include <docary.hxx>

#define URL_DECODE  INetURLObject::DECODE_UNAMBIGUOUS

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace nsSwDocInfoSubType;

SwPageNumberFieldType::SwPageNumberFieldType()
    : SwFieldType( RES_PAGENUMBERFLD ),
    nNumberingType( SVX_NUM_ARABIC ),
    bVirtuell( false )
{
}

OUString SwPageNumberFieldType::Expand( sal_uInt32 nFormat, short nOff,
         sal_uInt16 const nPageNumber, sal_uInt16 const nMaxPage,
         const OUString& rUserStr ) const
{
    sal_uInt32 nTmpFormat = (SVX_NUM_PAGEDESC == nFormat) ? (sal_uInt32)nNumberingType : nFormat;
    int const nTmp = nPageNumber + nOff;

    if (0 > nTmp || SVX_NUM_NUMBER_NONE == nTmpFormat || (!bVirtuell && nTmp > nMaxPage))
        return OUString();

    if( SVX_NUM_CHAR_SPECIAL == nTmpFormat )
        return rUserStr;

    return FormatNumber( nTmp, nTmpFormat );
}

SwFieldType* SwPageNumberFieldType::Copy() const
{
    SwPageNumberFieldType *pTmp = new SwPageNumberFieldType();

    pTmp->nNumberingType = nNumberingType;
    pTmp->bVirtuell  = bVirtuell;

    return pTmp;
}

void SwPageNumberFieldType::ChangeExpansion( SwDoc* pDoc,
                                            bool bVirt,
                                            const sal_Int16* pNumFormat )
{
    if( pNumFormat )
        nNumberingType = *pNumFormat;

    bVirtuell = false;
    if (bVirt && pDoc)
    {
        // check the flag since the layout NEVER sets it back
        const SfxItemPool &rPool = pDoc->GetAttrPool();
        sal_uInt32 nMaxItems = rPool.GetItemCount2( RES_PAGEDESC );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        {
            const SwFormatPageDesc *pDesc;
            if( 0 != (pDesc = static_cast<const SwFormatPageDesc*>(rPool.GetItem2( RES_PAGEDESC, n )) )
                && pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
            {
                const SwContentNode* pNd = dynamic_cast<const SwContentNode*>( pDesc->GetDefinedIn()  );
                if( pNd )
                {
                    if ( SwIterator<SwFrm,SwContentNode>(*pNd).First() )
                        bVirtuell = true;
                }
                else if( dynamic_cast< const SwFormat* >(pDesc->GetDefinedIn()) !=  nullptr)
                {
                    SwAutoFormatGetDocNode aGetHt( &pDoc->GetNodes() );
                    bVirtuell = !pDesc->GetDefinedIn()->GetInfo( aGetHt );
                    break;
                }
            }
        }
    }
}

SwPageNumberField::SwPageNumberField(SwPageNumberFieldType* pTyp,
          sal_uInt16 nSub, sal_uInt32 nFormat, short nOff,
          sal_uInt16 const nPageNumber, sal_uInt16 const nMaxPage)
    : SwField(pTyp, nFormat), nSubType(nSub), nOffset(nOff)
    , m_nPageNumber(nPageNumber)
    , m_nMaxPage(nMaxPage)
{
}

void SwPageNumberField::ChangeExpansion(sal_uInt16 const nPageNumber,
        sal_uInt16 const nMaxPage)
{
    m_nPageNumber = nPageNumber;
    m_nMaxPage = nMaxPage;
}

OUString SwPageNumberField::Expand() const
{
    OUString sRet;
    SwPageNumberFieldType* pFieldType = static_cast<SwPageNumberFieldType*>(GetTyp());

    if( PG_NEXT == nSubType && 1 != nOffset )
    {
        sRet = pFieldType->Expand(GetFormat(), 1, m_nPageNumber, m_nMaxPage, sUserStr);
        if (!sRet.isEmpty())
        {
            sRet = pFieldType->Expand(GetFormat(), nOffset, m_nPageNumber, m_nMaxPage, sUserStr);
        }
    }
    else if( PG_PREV == nSubType && -1 != nOffset )
    {
        sRet = pFieldType->Expand(GetFormat(), -1, m_nPageNumber, m_nMaxPage, sUserStr);
        if (!sRet.isEmpty())
        {
            sRet = pFieldType->Expand(GetFormat(), nOffset, m_nPageNumber, m_nMaxPage, sUserStr);
        }
    }
    else
        sRet = pFieldType->Expand(GetFormat(), nOffset, m_nPageNumber, m_nMaxPage, sUserStr);
    return sRet;
}

SwField* SwPageNumberField::Copy() const
{
    SwPageNumberField *pTmp = new SwPageNumberField(
                static_cast<SwPageNumberFieldType*>(GetTyp()), nSubType,
                GetFormat(), nOffset, m_nPageNumber, m_nMaxPage);
    pTmp->SetLanguage( GetLanguage() );
    pTmp->SetUserString( sUserStr );
    return pTmp;
}

OUString SwPageNumberField::GetPar2() const
{
    return OUString::number(nOffset);
}

void SwPageNumberField::SetPar2(const OUString& rStr)
{
    nOffset = (short)rStr.toInt32();
}

sal_uInt16 SwPageNumberField::GetSubType() const
{
    return nSubType;
}

bool SwPageNumberField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int16)GetFormat();
        break;
    case FIELD_PROP_USHORT1:
        rAny <<= nOffset;
        break;
    case FIELD_PROP_SUBTYPE:
        {
             text::PageNumberType eType;
            eType = text::PageNumberType_CURRENT;
            if(nSubType == PG_PREV)
                eType = text::PageNumberType_PREV;
            else if(nSubType == PG_NEXT)
                eType = text::PageNumberType_NEXT;
            rAny.setValue(&eType, cppu::UnoType<text::PageNumberType>::get());
        }
        break;
    case FIELD_PROP_PAR1:
        rAny <<= sUserStr;
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwPageNumberField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    sal_Int16 nSet = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        rAny >>= nSet;

        // TODO: woher kommen die defines?
        if(nSet <= SVX_NUM_PAGEDESC )
            SetFormat(nSet);
        else {
        }
        break;
    case FIELD_PROP_USHORT1:
        rAny >>= nSet;
        nOffset = nSet;
        break;
    case FIELD_PROP_SUBTYPE:
        switch( SWUnoHelper::GetEnumAsInt32( rAny ) )
        {
            case text::PageNumberType_CURRENT:
                nSubType = PG_RANDOM;
            break;
            case text::PageNumberType_PREV:
                nSubType = PG_PREV;
            break;
            case text::PageNumberType_NEXT:
                nSubType = PG_NEXT;
            break;
            default:
                bRet = false;
        }
        break;
    case FIELD_PROP_PAR1:
        rAny >>= sUserStr;
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return bRet;
}

SwAuthorFieldType::SwAuthorFieldType()
    : SwFieldType( RES_AUTHORFLD )
{
}

OUString SwAuthorFieldType::Expand(sal_uLong nFormat)
{
    SvtUserOptions&  rOpt = SW_MOD()->GetUserOptions();
    if((nFormat & 0xff) == AF_NAME)
        return rOpt.GetFullName();

    return rOpt.GetID();
}

SwFieldType* SwAuthorFieldType::Copy() const
{
    return new SwAuthorFieldType;
}

SwAuthorField::SwAuthorField(SwAuthorFieldType* pTyp, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat)
{
    aContent = SwAuthorFieldType::Expand(GetFormat());
}

OUString SwAuthorField::Expand() const
{
    if (!IsFixed())
        const_cast<SwAuthorField*>(this)->aContent =
                    SwAuthorFieldType::Expand(GetFormat());

    return aContent;
}

SwField* SwAuthorField::Copy() const
{
    SwAuthorField *pTmp = new SwAuthorField( static_cast<SwAuthorFieldType*>(GetTyp()),
                                                GetFormat());
    pTmp->SetExpansion(aContent);
    return pTmp;
}

bool SwAuthorField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        rAny <<= (GetFormat() & 0xff) == AF_NAME;
        break;

    case FIELD_PROP_BOOL2:
        rAny <<= IsFixed();
        break;

    case FIELD_PROP_PAR1:
        rAny <<= GetContent();
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwAuthorField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        SetFormat( *static_cast<sal_Bool const *>(rAny.getValue()) ? AF_NAME : AF_SHORTCUT );
        break;

    case FIELD_PROP_BOOL2:
        if( *static_cast<sal_Bool const *>(rAny.getValue()) )
            SetFormat( GetFormat() | AF_FIXED);
        else
            SetFormat( GetFormat() & ~AF_FIXED);
        break;

    case FIELD_PROP_PAR1:
        rAny >>= aContent;
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

SwFileNameFieldType::SwFileNameFieldType(SwDoc *pDocument)
    : SwFieldType( RES_FILENAMEFLD )
{
    pDoc = pDocument;
}

OUString SwFileNameFieldType::Expand(sal_uLong nFormat) const
{
    OUString aRet;
    const SwDocShell* pDShell = pDoc->GetDocShell();
    if( pDShell && pDShell->HasName() )
    {
        const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
        switch( nFormat & ~FF_FIXED )
        {
            case FF_PATH:
                {
                    if( INetProtocol::File == rURLObj.GetProtocol() )
                    {
                        INetURLObject aTemp(rURLObj);
                        aTemp.removeSegment();
                        // last slash should belong to the pathname
                        aRet = aTemp.PathToFileName();
                    }
                    else
                    {
                        aRet = URIHelper::removePassword(
                                    rURLObj.GetMainURL( INetURLObject::NO_DECODE ),
                                    INetURLObject::WAS_ENCODED, URL_DECODE );
                        const sal_Int32 nPos = aRet.indexOf(rURLObj.GetLastName( URL_DECODE ));
                        if (nPos>=0)
                        {
                            aRet = aRet.copy(0, nPos);
                        }
                    }
                }
                break;

            case FF_NAME:
                aRet = rURLObj.GetLastName( INetURLObject::DECODE_WITH_CHARSET );
                break;

            case FF_NAME_NOEXT:
                aRet = rURLObj.GetBase();
                break;

            default:
                if( INetProtocol::File == rURLObj.GetProtocol() )
                    aRet = rURLObj.GetFull();
                else
                    aRet = URIHelper::removePassword(
                                    rURLObj.GetMainURL( INetURLObject::NO_DECODE ),
                                    INetURLObject::WAS_ENCODED, URL_DECODE );
        }
    }
    return aRet;
}

SwFieldType* SwFileNameFieldType::Copy() const
{
    SwFieldType *pTmp = new SwFileNameFieldType(pDoc);
    return pTmp;
}

SwFileNameField::SwFileNameField(SwFileNameFieldType* pTyp, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat)
{
    aContent = static_cast<SwFileNameFieldType*>(GetTyp())->Expand(GetFormat());
}

OUString SwFileNameField::Expand() const
{
    if (!IsFixed())
        const_cast<SwFileNameField*>(this)->aContent = static_cast<SwFileNameFieldType*>(GetTyp())->Expand(GetFormat());

    return aContent;
}

SwField* SwFileNameField::Copy() const
{
    SwFileNameField *pTmp =
        new SwFileNameField(static_cast<SwFileNameFieldType*>(GetTyp()), GetFormat());
    pTmp->SetExpansion(aContent);

    return pTmp;
}

bool SwFileNameField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        {
            sal_Int16 nRet;
            switch( GetFormat() &(~FF_FIXED) )
            {
                case FF_PATH:
                    nRet = text::FilenameDisplayFormat::PATH;
                break;
                case FF_NAME_NOEXT:
                    nRet = text::FilenameDisplayFormat::NAME;
                break;
                case FF_NAME:
                    nRet = text::FilenameDisplayFormat::NAME_AND_EXT;
                break;
                default:    nRet = text::FilenameDisplayFormat::FULL;
            }
            rAny <<= nRet;
        }
        break;

    case FIELD_PROP_BOOL2:
        rAny <<= IsFixed();
        break;

    case FIELD_PROP_PAR3:
        rAny <<= GetContent();
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwFileNameField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        {
            //JP 24.10.2001: int32 because in UnoField.cxx a putvalue is
            //              called with a int32 value! But normally we need
            //              here only a int16
            sal_Int32 nType = 0;
            rAny >>= nType;
            bool bFixed = IsFixed();
            switch( nType )
            {
                case text::FilenameDisplayFormat::PATH:
                    nType = FF_PATH;
                break;
                case text::FilenameDisplayFormat::NAME:
                    nType = FF_NAME_NOEXT;
                break;
                case text::FilenameDisplayFormat::NAME_AND_EXT:
                    nType = FF_NAME;
                break;
                default:    nType = FF_PATHNAME;
            }
            if(bFixed)
                nType |= FF_FIXED;
            SetFormat(nType);
        }
        break;

    case FIELD_PROP_BOOL2:
        if( *static_cast<sal_Bool const *>(rAny.getValue()) )
            SetFormat( GetFormat() | FF_FIXED);
        else
            SetFormat( GetFormat() & ~FF_FIXED);
        break;

    case FIELD_PROP_PAR3:
        rAny >>= aContent;
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

SwTemplNameFieldType::SwTemplNameFieldType(SwDoc *pDocument)
    : SwFieldType( RES_TEMPLNAMEFLD )
{
    pDoc = pDocument;
}

OUString SwTemplNameFieldType::Expand(sal_uLong nFormat) const
{
    OSL_ENSURE( nFormat < FF_END, "Expand: no valid Format!" );

    OUString aRet;
    SwDocShell *pDocShell(pDoc->GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");

        if( FF_UI_NAME == nFormat )
            aRet = xDocProps->getTemplateName();
        else if( !xDocProps->getTemplateURL().isEmpty() )
        {
            if( FF_UI_RANGE == nFormat )
            {
                // for getting region names!
                SfxDocumentTemplates aFac;
                OUString sTmp;
                OUString sRegion;
                aFac.GetLogicNames( xDocProps->getTemplateURL(), sRegion, sTmp );
                aRet = sRegion;
            }
            else
            {
                INetURLObject aPathName( xDocProps->getTemplateURL() );
                if( FF_NAME == nFormat )
                    aRet = aPathName.GetName(URL_DECODE);
                else if( FF_NAME_NOEXT == nFormat )
                    aRet = aPathName.GetBase();
                else
                {
                    if( FF_PATH == nFormat )
                    {
                        aPathName.removeSegment();
                        aRet = aPathName.GetFull();
                    }
                    else
                        aRet = aPathName.GetFull();
                }
            }
        }
    }
    return aRet;
}

SwFieldType* SwTemplNameFieldType::Copy() const
{
    SwFieldType *pTmp = new SwTemplNameFieldType(pDoc);
    return pTmp;
}

SwTemplNameField::SwTemplNameField(SwTemplNameFieldType* pTyp, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat)
{}

OUString SwTemplNameField::Expand() const
{
    return static_cast<SwTemplNameFieldType*>(GetTyp())->Expand(GetFormat());
}

SwField* SwTemplNameField::Copy() const
{
    SwTemplNameField *pTmp =
        new SwTemplNameField(static_cast<SwTemplNameFieldType*>(GetTyp()), GetFormat());
    return pTmp;
}

bool SwTemplNameField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch ( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        {
            sal_Int16 nRet;
            switch( GetFormat() )
            {
                case FF_PATH:       nRet = text::FilenameDisplayFormat::PATH; break;
                case FF_NAME_NOEXT: nRet = text::FilenameDisplayFormat::NAME; break;
                case FF_NAME:       nRet = text::FilenameDisplayFormat::NAME_AND_EXT; break;
                case FF_UI_RANGE:   nRet = text::TemplateDisplayFormat::AREA; break;
                case FF_UI_NAME:    nRet = text::TemplateDisplayFormat::TITLE;  break;
                default:    nRet = text::FilenameDisplayFormat::FULL;

            }
            rAny <<= nRet;
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwTemplNameField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch ( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        {
            //JP 24.10.2001: int32 because in UnoField.cxx a putvalue is
            //              called with a int32 value! But normally we need
            //              here only a int16
            sal_Int32 nType = 0;
            rAny >>= nType;
            switch( nType )
            {
            case text::FilenameDisplayFormat::PATH:
                SetFormat(FF_PATH);
            break;
            case text::FilenameDisplayFormat::NAME:
                SetFormat(FF_NAME_NOEXT);
            break;
            case text::FilenameDisplayFormat::NAME_AND_EXT:
                SetFormat(FF_NAME);
            break;
            case text::TemplateDisplayFormat::AREA  :
                SetFormat(FF_UI_RANGE);
            break;
            case text::TemplateDisplayFormat::TITLE  :
                SetFormat(FF_UI_NAME);
            break;
            default:    SetFormat(FF_PATHNAME);
            }
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

SwDocStatFieldType::SwDocStatFieldType(SwDoc* pDocument)
    : SwFieldType( RES_DOCSTATFLD ), nNumberingType( SVX_NUM_ARABIC )
{
    pDoc = pDocument;
}

OUString SwDocStatFieldType::Expand(sal_uInt16 nSubType, sal_uInt32 nFormat) const
{
    sal_uInt32 nVal = 0;
    const SwDocStat& rDStat = pDoc->getIDocumentStatistics().GetDocStat();
    switch( nSubType )
    {
        case DS_TBL:  nVal = rDStat.nTable;   break;
        case DS_GRF:  nVal = rDStat.nGrf;   break;
        case DS_OLE:  nVal = rDStat.nOLE;   break;
        case DS_PARA: nVal = rDStat.nPara;  break;
        case DS_WORD: nVal = rDStat.nWord;  break;
        case DS_CHAR: nVal = rDStat.nChar;  break;
        case DS_PAGE:
            if( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
                ((SwDocStat &)rDStat).nPage = pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetPageNum();
            nVal = rDStat.nPage;
            if( SVX_NUM_PAGEDESC == nFormat )
                nFormat = (sal_uInt32)nNumberingType;
            break;
        default:
            OSL_FAIL( "SwDocStatFieldType::Expand: unknown SubType" );
    }

    if( nVal <= SHRT_MAX )
        return FormatNumber( nVal, nFormat );

    return OUString::number( nVal );
}

SwFieldType* SwDocStatFieldType::Copy() const
{
    SwDocStatFieldType *pTmp = new SwDocStatFieldType(pDoc);
    return pTmp;
}

/**
 * @param pTyp
 * @param nSub SubType
 * @param nFormat
 */
SwDocStatField::SwDocStatField(SwDocStatFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat),
    nSubType(nSub)
{}

OUString SwDocStatField::Expand() const
{
    return static_cast<SwDocStatFieldType*>(GetTyp())->Expand(nSubType, GetFormat());
}

SwField* SwDocStatField::Copy() const
{
    SwDocStatField *pTmp = new SwDocStatField(
                    static_cast<SwDocStatFieldType*>(GetTyp()), nSubType, GetFormat() );
    return pTmp;
}

sal_uInt16 SwDocStatField::GetSubType() const
{
    return nSubType;
}

void SwDocStatField::SetSubType(sal_uInt16 nSub)
{
    nSubType = nSub;
}

void SwDocStatField::ChangeExpansion( const SwFrm* pFrm )
{
    if( DS_PAGE == nSubType && SVX_NUM_PAGEDESC == GetFormat() )
        static_cast<SwDocStatFieldType*>(GetTyp())->SetNumFormat(
                pFrm->FindPageFrm()->GetPageDesc()->GetNumType().GetNumberingType() );
}

bool SwDocStatField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch ( nWhichId )
    {
    case FIELD_PROP_USHORT2:
        rAny <<= (sal_Int16)GetFormat();
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwDocStatField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = false;
    switch ( nWhichId )
    {
    case FIELD_PROP_USHORT2:
        {
            sal_Int16 nSet = 0;
            rAny >>= nSet;
            if(nSet <= SVX_NUM_CHARS_LOWER_LETTER_N &&
                nSet != SVX_NUM_CHAR_SPECIAL &&
                    nSet != SVX_NUM_BITMAP)
            {
                SetFormat(nSet);
                bRet = true;
            }
        }
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return bRet;
}

// Document info field type

SwDocInfoFieldType::SwDocInfoFieldType(SwDoc* pDc)
    : SwValueFieldType( pDc, RES_DOCINFOFLD )
{
}

SwFieldType* SwDocInfoFieldType::Copy() const
{
    SwDocInfoFieldType* pTyp = new SwDocInfoFieldType(GetDoc());
    return pTyp;
}

static void lcl_GetLocalDataWrapper( sal_uLong nLang,
                              const LocaleDataWrapper **ppAppLocalData,
                              const LocaleDataWrapper **ppLocalData )
{
    SvtSysLocale aLocale;
    *ppAppLocalData = &aLocale.GetLocaleData();
    *ppLocalData = *ppAppLocalData;
    if( nLang != (*ppLocalData)->getLanguageTag().getLanguageType() )
        *ppLocalData = new LocaleDataWrapper(
                        LanguageTag( static_cast<LanguageType>(nLang) ));
}

OUString SwDocInfoFieldType::Expand( sal_uInt16 nSub, sal_uInt32 nFormat,
                                    sal_uInt16 nLang, const OUString& rName ) const
{
    const LocaleDataWrapper *pAppLocalData = 0, *pLocalData = 0;
    SwDocShell *pDocShell(GetDoc()->GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (!pDocShell) { return OUString(); }

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");

    sal_uInt16 nExtSub = nSub & 0xff00;
    nSub &= 0xff;   // do not consider extended SubTypes

    OUString aStr;
    switch(nSub)
    {
    case DI_TITEL:  aStr = xDocProps->getTitle();       break;
    case DI_THEMA:  aStr = xDocProps->getSubject();     break;
    case DI_KEYS:   aStr = ::comphelper::string::convertCommaSeparated(
                                xDocProps->getKeywords());
                    break;
    case DI_COMMENT:aStr = xDocProps->getDescription(); break;
    case DI_DOCNO:  aStr = OUString::number(
                                        xDocProps->getEditingCycles() );
                    break;
    case DI_EDIT:
        if ( !nFormat )
        {
            lcl_GetLocalDataWrapper( nLang, &pAppLocalData, &pLocalData );
            sal_Int32 dur = xDocProps->getEditingDuration();
            // If Seconds > 0 then bSec should be TRUE otherwise Seconds
            // information will be lost if file has EditTime in Seconds format.
            aStr = pLocalData->getTime( tools::Time(dur/3600, (dur%3600)/60, dur%60),
                                        dur%60 > 0);
        }
        else
        {
            sal_Int32 dur = xDocProps->getEditingDuration();
            double fVal = tools::Time(dur/3600, (dur%3600)/60, dur%60).GetTimeInDays();
            aStr = ExpandValue(fVal, nFormat, nLang);
        }
        break;
    case DI_CUSTOM:
        {
            OUString sVal;
            try
            {
                uno::Any aAny;
                uno::Reference < beans::XPropertySet > xSet(
                    xDocProps->getUserDefinedProperties(),
                    uno::UNO_QUERY_THROW);
                aAny = xSet->getPropertyValue( rName );

                uno::Reference < script::XTypeConverter > xConverter( script::Converter::create(comphelper::getProcessComponentContext()) );
                uno::Any aNew;
                    aNew = xConverter->convertToSimpleType( aAny, uno::TypeClass_STRING );
                aNew >>= sVal;
            }
            catch (uno::Exception&) {}
            return sVal;
        }

    default:
        {
            OUString aName( xDocProps->getAuthor() );
            util::DateTime uDT( xDocProps->getCreationDate() );
            DateTime aDate(uDT);
            if( nSub == DI_CREATE )
                ;       // das wars schon!!
            else if( nSub == DI_CHANGE )
            {
                aName = xDocProps->getModifiedBy();
                uDT = xDocProps->getModificationDate();
                aDate = DateTime(uDT);
            }
            else if( nSub == DI_PRINT )
            {
                aName = xDocProps->getPrintedBy();
                uDT = xDocProps->getPrintDate();
                aDate = DateTime(uDT);
            }
            else
                break;

            if (aDate.IsValidAndGregorian())
            {
                switch (nExtSub & ~DI_SUB_FIXED)
                {
                case DI_SUB_AUTHOR:
                    aStr = aName;
                    break;

                case DI_SUB_TIME:
                    if (!nFormat)
                    {
                        lcl_GetLocalDataWrapper( nLang, &pAppLocalData,
                                                        &pLocalData );
                        aStr = pLocalData->getTime( aDate,
                                                    false);
                    }
                    else
                    {
                        // Numberformatter anwerfen!
                        double fVal = SwDateTimeField::GetDateTime( GetDoc(),
                                                    aDate);
                        aStr = ExpandValue(fVal, nFormat, nLang);
                    }
                    break;

                case DI_SUB_DATE:
                    if (!nFormat)
                    {
                        lcl_GetLocalDataWrapper( nLang, &pAppLocalData,
                                                 &pLocalData );
                        aStr = pLocalData->getDate( aDate );
                    }
                    else
                    {
                        // Numberformatter anwerfen!
                        double fVal = SwDateTimeField::GetDateTime( GetDoc(),
                                                    aDate);
                        aStr = ExpandValue(fVal, nFormat, nLang);
                    }
                    break;
                }
            }
        }
        break;
    }

    if( pAppLocalData != pLocalData )
        delete pLocalData;

    return aStr;
}

// document info field

SwDocInfoField::SwDocInfoField(SwDocInfoFieldType* pTyp, sal_uInt16 nSub, const OUString& rName, sal_uInt32 nFormat) :
    SwValueField(pTyp, nFormat), nSubType(nSub)
{
    aName = rName;
    aContent = static_cast<SwDocInfoFieldType*>(GetTyp())->Expand(nSubType, nFormat, GetLanguage(), aName);
}

SwDocInfoField::SwDocInfoField(SwDocInfoFieldType* pTyp, sal_uInt16 nSub, const OUString& rName, const OUString& rValue, sal_uInt32 nFormat) :
    SwValueField(pTyp, nFormat), nSubType(nSub)
{
    aName = rName;
    aContent = rValue;
}

template<class T>
static double lcl_TimeToDouble( const T& rTime )
{
    const double fNanoSecondsPerDay = 86400000000000.0;
    return (  (rTime.Hours   * SAL_CONST_INT64(3600000000000))
            + (rTime.Minutes * SAL_CONST_INT64(  60000000000))
            + (rTime.Seconds * SAL_CONST_INT64(   1000000000))
            + (rTime.NanoSeconds))
        / fNanoSecondsPerDay;
}

template<class D>
static double lcl_DateToDouble( const D& rDate, const Date& rNullDate )
{
    long nDate = Date::DateToDays( rDate.Day, rDate.Month, rDate.Year );
    long nNullDate = Date::DateToDays( rNullDate.GetDay(), rNullDate.GetMonth(), rNullDate.GetYear() );
    return double( nDate - nNullDate );
}

OUString SwDocInfoField::Expand() const
{
    if ( ( nSubType & 0xFF ) == DI_CUSTOM )
    {
        // custom properties currently need special treatment
        // We don't have a secure way to detect "real" custom properties in Word import of text
        // fields, so we treat *every* unknown property as a custom property, even the "built-in"
        // section in Word's document summary information stream as these properties have not been
        // inserted when the document summary information was imported, we do it here.
        // This approach is still a lot better than the old one to import such fields as
        // "user fields" and simple text
        SwDocShell* pDocShell = GetDoc()->GetDocShell();
        if( !pDocShell )
            return aContent;
        try
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS( pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps( xDPS->getDocumentProperties());
            uno::Reference < beans::XPropertySet > xSet( xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
            uno::Reference < beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();

            uno::Any aAny;
            if( xSetInfo->hasPropertyByName( aName ) )
                aAny = xSet->getPropertyValue( aName );
            if ( aAny.getValueType() != cppu::UnoType<void>::get() )
            {
                // "void" type means that the property has not been inserted until now
                if ( !IsFixed() )
                {
                    // if the field is "fixed" we don't update it from the property
                    OUString sVal;
                    uno::Reference < script::XTypeConverter > xConverter( script::Converter::create(comphelper::getProcessComponentContext()) );
                    util::Date aDate;
                    util::DateTime aDateTime;
                    util::Duration aDuration;
                    if( aAny >>= aDate)
                    {
                        SvNumberFormatter* pFormatter = pDocShell->GetDoc()->GetNumberFormatter();
                        Date* pNullDate = pFormatter->GetNullDate();
                        sVal = ExpandValue( lcl_DateToDouble<util::Date>( aDate, *pNullDate ), GetFormat(), GetLanguage());
                    }
                    else if( aAny >>= aDateTime )
                    {
                        double fDateTime = lcl_TimeToDouble<util::DateTime>( aDateTime );
                        SvNumberFormatter* pFormatter = pDocShell->GetDoc()->GetNumberFormatter();
                        Date* pNullDate = pFormatter->GetNullDate();
                        fDateTime += lcl_DateToDouble<util::DateTime>( aDateTime, *pNullDate );
                        sVal = ExpandValue( fDateTime, GetFormat(), GetLanguage());
                    }
                    else if( aAny >>= aDuration )
                    {
                        sVal = OUString(aDuration.Negative ? '-' : '+')
                             + SwViewShell::GetShellRes()->sDurationFormat;
                        sVal = sVal.replaceFirst("%1", OUString::number( aDuration.Years  ) );
                        sVal = sVal.replaceFirst("%2", OUString::number( aDuration.Months ) );
                        sVal = sVal.replaceFirst("%3", OUString::number( aDuration.Days   ) );
                        sVal = sVal.replaceFirst("%4", OUString::number( aDuration.Hours  ) );
                        sVal = sVal.replaceFirst("%5", OUString::number( aDuration.Minutes) );
                        sVal = sVal.replaceFirst("%6", OUString::number( aDuration.Seconds) );
                    }
                    else
                    {
                        uno::Any aNew = xConverter->convertToSimpleType( aAny, uno::TypeClass_STRING );
                        aNew >>= sVal;
                    }
                    const_cast<SwDocInfoField*>(this)->aContent = sVal;
                }
            }
        }
        catch (uno::Exception&) {}
    }
    else if ( !IsFixed() )
        const_cast<SwDocInfoField*>(this)->aContent = static_cast<SwDocInfoFieldType*>(GetTyp())->Expand(nSubType, GetFormat(), GetLanguage(), aName);

    return aContent;
}

OUString SwDocInfoField::GetFieldName() const
{
    OUString aStr(SwFieldType::GetTypeStr(GetTypeId()) + ":");

    sal_uInt16 const nSub = nSubType & 0xff;

    switch (nSub)
    {
        case DI_CUSTOM:
            aStr += aName;
            break;

        default:
            aStr += SwViewShell::GetShellRes()
                     ->aDocInfoLst[ nSub - DI_SUBTYPE_BEGIN ];
            break;
    }
    if (IsFixed())
    {
        aStr += " " + OUString(SwViewShell::GetShellRes()->aFixedStr);
    }
    return aStr;
}

SwField* SwDocInfoField::Copy() const
{
    SwDocInfoField* pField = new SwDocInfoField(static_cast<SwDocInfoFieldType*>(GetTyp()), nSubType, aName, GetFormat());
    pField->SetAutomaticLanguage(IsAutomaticLanguage());
    pField->aContent = aContent;

    return pField;
}

sal_uInt16 SwDocInfoField::GetSubType() const
{
    return nSubType;
}

void SwDocInfoField::SetSubType(sal_uInt16 nSub)
{
    nSubType = nSub;
}

void SwDocInfoField::SetLanguage(sal_uInt16 nLng)
{
    if (!GetFormat())
        SwField::SetLanguage(nLng);
    else
        SwValueField::SetLanguage(nLng);
}

bool SwDocInfoField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aContent;
        break;

    case FIELD_PROP_PAR4:
        rAny <<= aName;
        break;

    case FIELD_PROP_USHORT1:
        rAny  <<= (sal_Int16)aContent.toInt32();
        break;

    case FIELD_PROP_BOOL1:
        rAny <<= 0 != (nSubType & DI_SUB_FIXED);
        break;

    case FIELD_PROP_FORMAT:
        rAny  <<= (sal_Int32)GetFormat();
        break;

    case FIELD_PROP_DOUBLE:
        {
            double fVal = GetValue();
            rAny.setValue(&fVal, cppu::UnoType<decltype(fVal)>::get());
        }
        break;
    case FIELD_PROP_PAR3:
        rAny <<= Expand();
        break;
    case FIELD_PROP_BOOL2:
        {
            sal_uInt16 nExtSub = (nSubType & 0xff00) & ~DI_SUB_FIXED;
            rAny <<= nExtSub == DI_SUB_DATE;
        }
        break;
    default:
        return SwField::QueryValue(rAny, nWhichId);
    }
    return true;
}

bool SwDocInfoField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    sal_Int32 nValue = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        if( nSubType & DI_SUB_FIXED )
            rAny >>= aContent;
        break;

    case FIELD_PROP_USHORT1:
        if( nSubType & DI_SUB_FIXED )
        {
            rAny >>= nValue;
            aContent = OUString::number(nValue);
        }
        break;

    case FIELD_PROP_BOOL1:
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType |= DI_SUB_FIXED;
        else
            nSubType &= ~DI_SUB_FIXED;
        break;
    case FIELD_PROP_FORMAT:
        {
            rAny >>= nValue;
            if( nValue >= 0)
                SetFormat(nValue);
        }
        break;

    case FIELD_PROP_PAR3:
        rAny >>= aContent;
        break;
    case FIELD_PROP_BOOL2:
        nSubType &= 0xf0ff;
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType |= DI_SUB_DATE;
        else
            nSubType |= DI_SUB_TIME;
        break;
    default:
        return SwField::PutValue(rAny, nWhichId);
    }
    return true;
}

SwHiddenTextFieldType::SwHiddenTextFieldType( bool bSetHidden )
    : SwFieldType( RES_HIDDENTXTFLD ), bHidden( bSetHidden )
{
}

SwFieldType* SwHiddenTextFieldType::Copy() const
{
    return new SwHiddenTextFieldType( bHidden );
}

void SwHiddenTextFieldType::SetHiddenFlag( bool bSetHidden )
{
    if( bHidden != bSetHidden )
    {
        bHidden = bSetHidden;
        UpdateFields();       // notify all HiddenTexts
    }
}

SwHiddenTextField::SwHiddenTextField( SwHiddenTextFieldType* pFieldType,
                                    bool    bConditional,
                                    const OUString& rCond,
                                    const OUString& rStr,
                                    bool    bHidden,
                                    sal_uInt16  nSub) :
    SwField( pFieldType ), aCond(rCond), nSubType(nSub),
    bCanToggle(bConditional), bIsHidden(bHidden), bValid(false)
{
    if(nSubType == TYP_CONDTXTFLD)
    {
        sal_Int32 nPos = 0;
        aTRUEText = rStr.getToken(0, '|', nPos);

        if(nPos != -1)
        {
            aFALSEText = rStr.getToken(0, '|', nPos);
            if(nPos != -1)
            {
                aContent = rStr.getToken(0, '|', nPos);
                bValid = true;
            }
        }
    }
    else
        aTRUEText = rStr;
}

SwHiddenTextField::SwHiddenTextField( SwHiddenTextFieldType* pFieldType,
                                    const OUString& rCond,
                                    const OUString& rTrue,
                                    const OUString& rFalse,
                                    sal_uInt16 nSub)
    : SwField( pFieldType ), aTRUEText(rTrue), aFALSEText(rFalse), aCond(rCond), nSubType(nSub),
      bIsHidden(true), bValid(false)
{
    bCanToggle = !aCond.isEmpty();
}

OUString SwHiddenTextField::Expand() const
{
    // Type: !Hidden  -> show always
    //        Hide    -> evaluate condition

    if( TYP_CONDTXTFLD == nSubType )
    {
        if( bValid )
            return aContent;

        if( bCanToggle && !bIsHidden )
            return aTRUEText;
    }
    else if( !static_cast<SwHiddenTextFieldType*>(GetTyp())->GetHiddenFlag() ||
        ( bCanToggle && bIsHidden ))
        return aTRUEText;

    return aFALSEText;
}

/// get current field value and cache it
void SwHiddenTextField::Evaluate(SwDoc* pDoc)
{
    OSL_ENSURE(pDoc, "got no document");

    if( TYP_CONDTXTFLD == nSubType )
    {
#if !HAVE_FEATURE_DBCONNECTIVITY
        (void) pDoc;
#else
        SwDBManager* pMgr = pDoc->GetDBManager();
#endif
        bValid = false;
        OUString sTmpName = (bCanToggle && !bIsHidden) ? aTRUEText : aFALSEText;

        // Database expressions need to be different from normal text. Therefore, normal text is set
        // in quotes. If the latter exist they will be removed. If not, check if potential DB name.
        // Only if there are two or more dots and no quotes, we assume a database.
        if (sTmpName.getLength()>1 &&
            sTmpName.startsWith("\"") &&
            sTmpName.endsWith("\""))
        {
            aContent = sTmpName.copy(1, sTmpName.getLength() - 2);
            bValid = true;
        }
        else if(sTmpName.indexOf('\"')<0 &&
            comphelper::string::getTokenCount(sTmpName, '.') > 2)
        {
            sTmpName = ::ReplacePoint(sTmpName);
            if(sTmpName.startsWith("[") && sTmpName.endsWith("]"))
            {   // remove brackets
                sTmpName = sTmpName.copy(1, sTmpName.getLength() - 2);
            }
#if HAVE_FEATURE_DBCONNECTIVITY
            if( pMgr)
            {
                OUString sDBName( GetDBName( sTmpName, pDoc ));
                OUString sDataSource(sDBName.getToken(0, DB_DELIM));
                OUString sDataTableOrQuery(sDBName.getToken(1, DB_DELIM));
                if( pMgr->IsInMerge() && !sDBName.isEmpty() &&
                    pMgr->IsDataSourceOpen( sDataSource,
                                                sDataTableOrQuery, false))
                {
                    double fNumber;
                    pMgr->GetMergeColumnCnt(GetColumnName( sTmpName ),
                        GetLanguage(), aContent, &fNumber );
                    bValid = true;
                }
                else if( !sDBName.isEmpty() && !sDataSource.isEmpty() &&
                         !sDataTableOrQuery.isEmpty() )
                    bValid = true;
            }
#endif
        }
    }
}

OUString SwHiddenTextField::GetFieldName() const
{
    OUString aStr = SwFieldType::GetTypeStr(nSubType) +
        " " + aCond + " " + aTRUEText;

    if (nSubType == TYP_CONDTXTFLD)
    {
        aStr += " : " + aFALSEText;
    }
    return aStr;
}

SwField* SwHiddenTextField::Copy() const
{
    SwHiddenTextField* pField =
        new SwHiddenTextField(static_cast<SwHiddenTextFieldType*>(GetTyp()), aCond,
                              aTRUEText, aFALSEText);
    pField->bIsHidden = bIsHidden;
    pField->bValid    = bValid;
    pField->aContent  = aContent;
    pField->SetFormat(GetFormat());
    pField->nSubType  = nSubType;
    return pField;
}

/// set condition
void SwHiddenTextField::SetPar1(const OUString& rStr)
{
    aCond = rStr;
    bCanToggle = !aCond.isEmpty();
}

OUString SwHiddenTextField::GetPar1() const
{
    return aCond;
}

/// set True/False text
void SwHiddenTextField::SetPar2(const OUString& rStr)
{
    if (nSubType == TYP_CONDTXTFLD)
    {
        sal_Int32 nPos = rStr.indexOf('|');
        if (nPos == -1)
            aTRUEText = rStr;
        else
        {
            aTRUEText = rStr.copy(0, nPos);
            aFALSEText = rStr.copy(nPos + 1);
        }
    }
    else
        aTRUEText = rStr;
}

/// get True/False text
OUString SwHiddenTextField::GetPar2() const
{
    if(nSubType != TYP_CONDTXTFLD)
    {
        return aTRUEText;
    }
    return aTRUEText + "|" + aFALSEText;
}

sal_uInt16 SwHiddenTextField::GetSubType() const
{
    return nSubType;
}

bool SwHiddenTextField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aCond;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aTRUEText;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= aFALSEText;
        break;
    case FIELD_PROP_PAR4 :
        rAny <<= aContent;
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= bIsHidden;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwHiddenTextField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        {
            OUString sVal;
            rAny >>= sVal;
            SetPar1(sVal);
        }
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aTRUEText;
        break;
    case FIELD_PROP_PAR3:
        rAny >>= aFALSEText;
        break;
    case FIELD_PROP_BOOL1:
        bIsHidden = *static_cast<sal_Bool const *>(rAny.getValue());
        break;
    case FIELD_PROP_PAR4:
        rAny >>= aContent;
        bValid = true;
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

OUString SwHiddenTextField::GetColumnName(const OUString& rName)
{
    sal_Int32 nPos = rName.indexOf(DB_DELIM);
    if( nPos>=0 )
    {
        nPos = rName.indexOf(DB_DELIM, nPos + 1);

        if( nPos>=0 )
            return rName.copy(nPos + 1);
    }
    return rName;
}

OUString SwHiddenTextField::GetDBName(const OUString& rName, SwDoc *pDoc)
{
    sal_Int32 nPos = rName.indexOf(DB_DELIM);
    if( nPos>=0 )
    {
        nPos = rName.indexOf(DB_DELIM, nPos + 1);

        if( nPos>=0 )
            return rName.copy(0, nPos);
    }

    SwDBData aData = pDoc->GetDBData();
    return aData.sDataSource + OUString(DB_DELIM) + aData.sCommand;
}

// field type for line height 0

SwHiddenParaFieldType::SwHiddenParaFieldType()
    : SwFieldType( RES_HIDDENPARAFLD )
{
}

SwFieldType* SwHiddenParaFieldType::Copy() const
{
    SwHiddenParaFieldType* pTyp = new SwHiddenParaFieldType();
    return pTyp;
}

// field for line height 0

SwHiddenParaField::SwHiddenParaField(SwHiddenParaFieldType* pTyp, const OUString& rStr)
    : SwField(pTyp), aCond(rStr)
{
    bIsHidden = false;
}

OUString SwHiddenParaField::Expand() const
{
    return OUString();
}

SwField* SwHiddenParaField::Copy() const
{
    SwHiddenParaField* pField = new SwHiddenParaField(static_cast<SwHiddenParaFieldType*>(GetTyp()), aCond);
    pField->bIsHidden = bIsHidden;

    return pField;
}

bool SwHiddenParaField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aCond;
        break;
    case  FIELD_PROP_BOOL1:
        rAny <<= bIsHidden;
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwHiddenParaField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aCond;
        break;
    case FIELD_PROP_BOOL1:
        bIsHidden = *static_cast<sal_Bool const *>(rAny.getValue());
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/// set condition
void SwHiddenParaField::SetPar1(const OUString& rStr)
{
    aCond = rStr;
}

OUString SwHiddenParaField::GetPar1() const
{
    return aCond;
}

// PostIt field type

SwPostItFieldType::SwPostItFieldType(SwDoc *pDoc)
    : SwFieldType( RES_POSTITFLD )
    , mpDoc(pDoc)
{}

SwFieldType* SwPostItFieldType::Copy() const
{
    return new SwPostItFieldType(mpDoc);
}

// PostIt field

SwPostItField::SwPostItField( SwPostItFieldType* pT,
        const OUString& rAuthor,
        const OUString& rText,
        const OUString& rInitials,
        const OUString& rName,
        const DateTime& rDateTime )
    : SwField( pT )
    , sText( rText )
    , sAuthor( rAuthor )
    , sInitials( rInitials )
    , sName( rName )
    , aDateTime( rDateTime )
    , mpText( NULL )
    , m_pTextObject( NULL )
{
}

SwPostItField::~SwPostItField()
{
    if ( m_pTextObject != NULL )
    {
        m_pTextObject->DisposeEditSource();
        m_pTextObject->release();
    }

    delete mpText;
}

OUString SwPostItField::Expand() const
{
    return OUString();
}

OUString SwPostItField::GetDescription() const
{
    return SW_RES(STR_NOTE);
}

SwField* SwPostItField::Copy() const
{
    SwPostItField* pRet = new SwPostItField( static_cast<SwPostItFieldType*>(GetTyp()), sAuthor, sText, sInitials, sName,
                                aDateTime);
    if (mpText)
        pRet->SetTextObject( new OutlinerParaObject(*mpText) );

    // Note: member <m_pTextObject> not copied.

    return pRet;
}

/// set author
void SwPostItField::SetPar1(const OUString& rStr)
{
    sAuthor = rStr;
}

/// get author
OUString SwPostItField::GetPar1() const
{
    return sAuthor;
}

/// set the PostIt's text
void SwPostItField::SetPar2(const OUString& rStr)
{
    sText = rStr;
}

/// get the PostIt's text
OUString SwPostItField::GetPar2() const
{
    return sText;
}


void SwPostItField::SetName(const OUString& rName)
{
    sName = rName;
}



void SwPostItField::SetTextObject( OutlinerParaObject* pText )
{
    delete mpText;
    mpText = pText;
}

sal_Int32 SwPostItField::GetNumberOfParagraphs() const
{
    return (mpText) ? mpText->Count() : 1;
}

bool SwPostItField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= sAuthor;
        break;
    case FIELD_PROP_PAR2:
        {
        rAny <<= sText;
        break;
        }
    case FIELD_PROP_PAR3:
        rAny <<= sInitials;
        break;
    case FIELD_PROP_PAR4:
        rAny <<= sName;
        break;
    case FIELD_PROP_TEXT:
        {
            if ( !m_pTextObject )
            {
                SwPostItFieldType* pGetType = static_cast<SwPostItFieldType*>(GetTyp());
                SwDoc* pDoc = pGetType->GetDoc();
                SwTextAPIEditSource* pObj = new SwTextAPIEditSource( pDoc );
                const_cast <SwPostItField*> (this)->m_pTextObject = new SwTextAPIObject( pObj );
                m_pTextObject->acquire();
            }

            if ( mpText )
                m_pTextObject->SetText( *mpText );
            else
                m_pTextObject->SetString( sText );

            uno::Reference < text::XText > xText( m_pTextObject );
            rAny <<= xText;
            break;
        }
    case FIELD_PROP_DATE:
        {
            css::util::Date aSetDate = aDateTime.GetUNODate();
            rAny.setValue(&aSetDate, ::cppu::UnoType<util::Date>::get());
        }
        break;
    case FIELD_PROP_DATE_TIME:
        {
            rAny <<= aDateTime.GetUNODateTime();
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwPostItField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= sAuthor;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= sText;
        //#i100374# new string via api, delete complex text object so SwPostItNote picks up the new string
        if (mpText)
        {
            delete mpText;
            mpText = 0;
        }
        break;
    case FIELD_PROP_PAR3:
        rAny >>= sInitials;
        break;
    case FIELD_PROP_PAR4:
        rAny >>= sName;
        break;
    case FIELD_PROP_TEXT:
        OSL_FAIL("Not implemented!");
        break;
    case FIELD_PROP_DATE:
        if( rAny.getValueType() == ::cppu::UnoType<util::Date>::get() )
        {
            util::Date aSetDate = *static_cast<util::Date const *>(rAny.getValue());
            aDateTime = Date(aSetDate.Day, aSetDate.Month, aSetDate.Year);
        }
        break;
    case FIELD_PROP_DATE_TIME:
    {
        util::DateTime aDateTimeValue;
        if(!(rAny >>= aDateTimeValue))
            return false;
        aDateTime = DateTime(aDateTimeValue);
    }
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// extended user information field type

SwExtUserFieldType::SwExtUserFieldType()
    : SwFieldType( RES_EXTUSERFLD )
{
}

SwFieldType* SwExtUserFieldType::Copy() const
{
    SwExtUserFieldType* pTyp = new SwExtUserFieldType;
    return pTyp;
}

OUString SwExtUserFieldType::Expand(sal_uInt16 nSub, sal_uInt32 )
{
    UserOptToken nRet = static_cast<UserOptToken>(USHRT_MAX);
    switch(nSub)
    {
    case EU_FIRSTNAME:      nRet = UserOptToken::FirstName; break;
    case EU_NAME:           nRet = UserOptToken::LastName;  break;
    case EU_SHORTCUT:       nRet = UserOptToken::ID; break;

    case EU_COMPANY:        nRet = UserOptToken::Company;        break;
    case EU_STREET:         nRet = UserOptToken::Street;         break;
    case EU_TITLE:          nRet = UserOptToken::Title;          break;
    case EU_POSITION:       nRet = UserOptToken::Position;       break;
    case EU_PHONE_PRIVATE:  nRet = UserOptToken::TelephoneHome;    break;
    case EU_PHONE_COMPANY:  nRet = UserOptToken::TelephoneWork;    break;
    case EU_FAX:            nRet = UserOptToken::Fax;            break;
    case EU_EMAIL:          nRet = UserOptToken::Email;          break;
    case EU_COUNTRY:        nRet = UserOptToken::Country;        break;
    case EU_ZIP:            nRet = UserOptToken::Zip;            break;
    case EU_CITY:           nRet = UserOptToken::City;           break;
    case EU_STATE:          nRet = UserOptToken::State;          break;
    case EU_FATHERSNAME:    nRet = UserOptToken::FathersName;    break;
    case EU_APARTMENT:      nRet = UserOptToken::Apartment;      break;
    default:             OSL_ENSURE( false, "Field unknown");
    }
    if( static_cast<UserOptToken>(USHRT_MAX) != nRet )
    {
        SvtUserOptions&  rUserOpt = SW_MOD()->GetUserOptions();
        return rUserOpt.GetToken( nRet );
    }
    return OUString();
}

// extended user information field

SwExtUserField::SwExtUserField(SwExtUserFieldType* pTyp, sal_uInt16 nSubTyp, sal_uInt32 nFormat) :
    SwField(pTyp, nFormat), nType(nSubTyp)
{
    aContent = SwExtUserFieldType::Expand(nType, GetFormat());
}

OUString SwExtUserField::Expand() const
{
    if (!IsFixed())
        const_cast<SwExtUserField*>(this)->aContent = SwExtUserFieldType::Expand(nType, GetFormat());

    return aContent;
}

SwField* SwExtUserField::Copy() const
{
    SwExtUserField* pField = new SwExtUserField(static_cast<SwExtUserFieldType*>(GetTyp()), nType, GetFormat());
    pField->SetExpansion(aContent);

    return pField;
}

sal_uInt16 SwExtUserField::GetSubType() const
{
    return nType;
}

void SwExtUserField::SetSubType(sal_uInt16 nSub)
{
    nType = nSub;
}

bool SwExtUserField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aContent;
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp = nType;
            rAny <<= nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= IsFixed();
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwExtUserField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aContent;
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp = 0;
            rAny >>= nTmp;
            nType = nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        if( *static_cast<sal_Bool const *>(rAny.getValue()) )
            SetFormat(GetFormat() | AF_FIXED);
        else
            SetFormat(GetFormat() & ~AF_FIXED);
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// field type for relative page numbers

SwRefPageSetFieldType::SwRefPageSetFieldType()
    : SwFieldType( RES_REFPAGESETFLD )
{
}

SwFieldType* SwRefPageSetFieldType::Copy() const
{
    return new SwRefPageSetFieldType;
}

// overridden since there is nothing to update
void SwRefPageSetFieldType::Modify( const SfxPoolItem*, const SfxPoolItem * )
{
}

// field for relative page numbers

SwRefPageSetField::SwRefPageSetField( SwRefPageSetFieldType* pTyp,
                    short nOff, bool bFlag )
    : SwField( pTyp ), nOffset( nOff ), bOn( bFlag )
{
}

OUString SwRefPageSetField::Expand() const
{
    return OUString();
}

SwField* SwRefPageSetField::Copy() const
{
    return new SwRefPageSetField( static_cast<SwRefPageSetFieldType*>(GetTyp()), nOffset, bOn );
}

OUString SwRefPageSetField::GetPar2() const
{
    return OUString::number(GetOffset());
}

void SwRefPageSetField::SetPar2(const OUString& rStr)
{
    SetOffset( (short) rStr.toInt32() );
}

bool SwRefPageSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        rAny <<= bOn;
        break;
    case FIELD_PROP_USHORT1:
        rAny <<= (sal_Int16)nOffset;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwRefPageSetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        bOn = *static_cast<sal_Bool const *>(rAny.getValue());
        break;
    case FIELD_PROP_USHORT1:
        rAny >>=nOffset;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// relative page numbers - query field

SwRefPageGetFieldType::SwRefPageGetFieldType( SwDoc* pDc )
    : SwFieldType( RES_REFPAGEGETFLD ), pDoc( pDc ), nNumberingType( SVX_NUM_ARABIC )
{
}

SwFieldType* SwRefPageGetFieldType::Copy() const
{
    SwRefPageGetFieldType* pNew = new SwRefPageGetFieldType( pDoc );
    pNew->nNumberingType = nNumberingType;
    return pNew;
}

void SwRefPageGetFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    // update all GetReference fields
    if( !pNew && !pOld && HasWriterListeners() )
    {
        // first collect all SetPageRefFields
        _SetGetExpFields aTmpLst;
        if( MakeSetList( aTmpLst ) )
        {
            SwIterator<SwFormatField,SwFieldType> aIter( *this );
            for ( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
                    // update only the GetRef fields
                    if( pFormatField->GetTextField() )
                        UpdateField( pFormatField->GetTextField(), aTmpLst );
        }
    }

    // forward to text fields, they "expand" the text
    NotifyClients( pOld, pNew );
}

bool SwRefPageGetFieldType::MakeSetList( _SetGetExpFields& rTmpLst )
{
    SwIterator<SwFormatField,SwFieldType> aIter(*pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_REFPAGESETFLD));
    for ( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
    {
            // update only the GetRef fields
            const SwTextField* pTField = pFormatField->GetTextField();
            if( pTField )
            {
                const SwTextNode& rTextNd = pTField->GetTextNode();

                // Always the first! (in Tab-Headline, header/footer )
                Point aPt;
                const SwContentFrm* pFrm = rTextNd.getLayoutFrm( rTextNd.GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );

                _SetGetExpField* pNew;

                if( !pFrm ||
                     pFrm->IsInDocBody() ||
                    // #i31868#
                    // Check if pFrm is not yet connected to the layout.
                    !pFrm->FindPageFrm() )
                {
                    //  create index for determination of the TextNode
                    SwNodeIndex aIdx( rTextNd );
                    pNew = new _SetGetExpField( aIdx, pTField );
                }
                else
                {
                    //  create index for determination of the TextNode
                    SwPosition aPos( pDoc->GetNodes().GetEndOfPostIts() );
                    bool const bResult = GetBodyTextNode( *pDoc, aPos, *pFrm );
                    OSL_ENSURE(bResult, "where is the Field?");
                    (void) bResult; // unused in non-debug
                    pNew = new _SetGetExpField( aPos.nNode, pTField,
                                                &aPos.nContent );
                }

                if( !rTmpLst.insert( pNew ).second)
                    delete pNew;
            }
    }

    return !rTmpLst.empty();
}

void SwRefPageGetFieldType::UpdateField( SwTextField* pTextField,
                                        _SetGetExpFields& rSetList )
{
    SwRefPageGetField* pGetField = const_cast<SwRefPageGetField*>(static_cast<const SwRefPageGetField*>(pTextField->GetFormatField().GetField()));
    pGetField->SetText( OUString() );

    // then search the correct RefPageSet field
    SwTextNode* pTextNode = &pTextField->GetTextNode();
    if( pTextNode->StartOfSectionIndex() >
        pDoc->GetNodes().GetEndOfExtras().GetIndex() )
    {
        SwNodeIndex aIdx( *pTextNode );
        _SetGetExpField aEndField( aIdx, pTextField );

        _SetGetExpFields::const_iterator itLast = rSetList.lower_bound( &aEndField );

        if( itLast != rSetList.begin() )
        {
            --itLast;
            const SwTextField* pRefTextField = (*itLast)->GetTextField();
            const SwRefPageSetField* pSetField =
                        static_cast<const SwRefPageSetField*>(pRefTextField->GetFormatField().GetField());
            if( pSetField->IsOn() )
            {
                // determine the correct offset
                Point aPt;
                const SwContentFrm* pFrm = pTextNode->getLayoutFrm( pTextNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
                const SwContentFrm* pRefFrm = pRefTextField->GetTextNode().getLayoutFrm( pRefTextField->GetTextNode().GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
                const SwPageFrm* pPgFrm = 0;
                const short nDiff = ( pFrm && pRefFrm )
                        ?   (pPgFrm = pFrm->FindPageFrm())->GetPhyPageNum() -
                            pRefFrm->FindPageFrm()->GetPhyPageNum() + 1
                        : 1;

                sal_uInt32 nTmpFormat = SVX_NUM_PAGEDESC == pGetField->GetFormat()
                        ? ( !pPgFrm
                                ? (sal_uInt32)SVX_NUM_ARABIC
                                : pPgFrm->GetPageDesc()->GetNumType().GetNumberingType() )
                        : pGetField->GetFormat();
                const short nPageNum = std::max<short>(0, pSetField->GetOffset() + nDiff);
                pGetField->SetText( FormatNumber( nPageNum, nTmpFormat ) );
            }
        }
    }
    // start formatting
    const_cast<SwFormatField&>(pTextField->GetFormatField()).ModifyNotification( 0, 0 );
}

// queries for relative page numbering

SwRefPageGetField::SwRefPageGetField( SwRefPageGetFieldType* pTyp,
                                    sal_uInt32 nFormat )
    : SwField( pTyp, nFormat )
{
}

OUString SwRefPageGetField::Expand() const
{
    return sText;
}

SwField* SwRefPageGetField::Copy() const
{
    SwRefPageGetField* pCpy = new SwRefPageGetField(
                        static_cast<SwRefPageGetFieldType*>(GetTyp()), GetFormat() );
    pCpy->SetText( sText );
    return pCpy;
}

void SwRefPageGetField::ChangeExpansion( const SwFrm* pFrm,
                                        const SwTextField* pField )
{
    // only fields in Footer, Header, FootNote, Flys
    SwRefPageGetFieldType* pGetType = static_cast<SwRefPageGetFieldType*>(GetTyp());
    SwDoc* pDoc = pGetType->GetDoc();
    if( pField->GetTextNode().StartOfSectionIndex() >
        pDoc->GetNodes().GetEndOfExtras().GetIndex() )
        return;

    sText.clear();

    OSL_ENSURE( !pFrm->IsInDocBody(), "Flag incorrect, frame is in DocBody" );

    // collect all SetPageRefFields
    _SetGetExpFields aTmpLst;
    if( !pGetType->MakeSetList( aTmpLst ) )
        return ;

    //  create index for determination of the TextNode
    SwPosition aPos( SwNodeIndex( pDoc->GetNodes() ) );
    SwTextNode* pTextNode = const_cast<SwTextNode*>(GetBodyTextNode( *pDoc, aPos, *pFrm ));

    // If no layout exists, ChangeExpansion is called for header and
    // footer lines via layout formatting without existing TextNode.
    if(!pTextNode)
        return;

    _SetGetExpField aEndField( aPos.nNode, pField, &aPos.nContent );

    _SetGetExpFields::const_iterator itLast = aTmpLst.lower_bound( &aEndField );

    if( itLast == aTmpLst.begin() )
        return;        // there is no corresponding set-field in front
    --itLast;

    const SwTextField* pRefTextField = (*itLast)->GetTextField();
    const SwRefPageSetField* pSetField =
                        static_cast<const SwRefPageSetField*>(pRefTextField->GetFormatField().GetField());
    Point aPt;
    const SwContentFrm* pRefFrm = pRefTextField->GetTextNode().getLayoutFrm( pFrm->getRootFrm(), &aPt, 0, false );
    if( pSetField->IsOn() && pRefFrm )
    {
        // determine the correct offset
        const SwPageFrm* pPgFrm = pFrm->FindPageFrm();
        const short nDiff = pPgFrm->GetPhyPageNum() -
                            pRefFrm->FindPageFrm()->GetPhyPageNum() + 1;

        SwRefPageGetField* pGetField = const_cast<SwRefPageGetField*>(static_cast<const SwRefPageGetField*>(pField->GetFormatField().GetField()));
        sal_uInt32 nTmpFormat = SVX_NUM_PAGEDESC == pGetField->GetFormat()
                            ? pPgFrm->GetPageDesc()->GetNumType().GetNumberingType()
                            : pGetField->GetFormat();
        const short nPageNum = std::max<short>(0, pSetField->GetOffset() + nDiff);
        pGetField->SetText( FormatNumber( nPageNum, nTmpFormat ) );
    }
}

bool SwRefPageGetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
        case FIELD_PROP_USHORT1:
            rAny <<= (sal_Int16)GetFormat();
        break;
        case FIELD_PROP_PAR1:
            rAny <<= sText;
        break;
        default:
            OSL_FAIL("illegal property");
    }
    return true;
}

bool SwRefPageGetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
        case FIELD_PROP_USHORT1:
        {
            sal_Int16 nSet = 0;
            rAny >>= nSet;
            if(nSet <= SVX_NUM_PAGEDESC )
                SetFormat(nSet);
            else {
            }
        }
        break;
        case FIELD_PROP_PAR1:
            rAny >>= sText;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// field type to jump to and edit

SwJumpEditFieldType::SwJumpEditFieldType( SwDoc* pD )
    : SwFieldType( RES_JUMPEDITFLD ), pDoc( pD ), aDep( this, 0 )
{
}

SwFieldType* SwJumpEditFieldType::Copy() const
{
    return new SwJumpEditFieldType( pDoc );
}

SwCharFormat* SwJumpEditFieldType::GetCharFormat()
{
    SwCharFormat* pFormat = pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( RES_POOLCHR_JUMPEDIT );

    // not registered yet?
    if( !aDep.GetRegisteredIn() )
        pFormat->Add( &aDep );     // register

    return pFormat;
}

SwJumpEditField::SwJumpEditField( SwJumpEditFieldType* pTyp, sal_uInt32 nForm,
                                const OUString& rText, const OUString& rHelp )
    : SwField( pTyp, nForm ), sText( rText ), sHelp( rHelp )
{
}

OUString SwJumpEditField::Expand() const
{
    return "<" + sText + ">";
}

SwField* SwJumpEditField::Copy() const
{
    return new SwJumpEditField( static_cast<SwJumpEditFieldType*>(GetTyp()), GetFormat(),
                                sText, sHelp );
}

/// get place holder text
OUString SwJumpEditField::GetPar1() const
{
    return sText;
}

/// set place holder text
void SwJumpEditField::SetPar1(const OUString& rStr)
{
    sText = rStr;
}

/// get hint text
OUString SwJumpEditField::GetPar2() const
{
    return sHelp;
}

/// set hint text
void SwJumpEditField::SetPar2(const OUString& rStr)
{
    sHelp = rStr;
}

bool SwJumpEditField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nRet;
            switch( GetFormat() )
            {
            case JE_FMT_TABLE:  nRet = text::PlaceholderType::TABLE; break;
            case JE_FMT_FRAME:  nRet = text::PlaceholderType::TEXTFRAME; break;
            case JE_FMT_GRAPHIC:nRet = text::PlaceholderType::GRAPHIC; break;
            case JE_FMT_OLE:    nRet = text::PlaceholderType::OBJECT; break;
            default:
                nRet = text::PlaceholderType::TEXT; break;
            }
            rAny <<= nRet;
        }
        break;
    case FIELD_PROP_PAR1 :
        rAny <<= sHelp;
        break;
    case FIELD_PROP_PAR2 :
         rAny <<= sText;
         break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwJumpEditField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        {
            //JP 24.10.2001: int32 because in UnoField.cxx a putvalue is
            //              called with a int32 value! But normally we need
            //              here only a int16
            sal_Int32 nSet = 0;
            rAny >>= nSet;
            switch( nSet )
            {
                case text::PlaceholderType::TEXT     : SetFormat(JE_FMT_TEXT); break;
                case text::PlaceholderType::TABLE    : SetFormat(JE_FMT_TABLE); break;
                case text::PlaceholderType::TEXTFRAME: SetFormat(JE_FMT_FRAME); break;
                case text::PlaceholderType::GRAPHIC  : SetFormat(JE_FMT_GRAPHIC); break;
                case text::PlaceholderType::OBJECT   : SetFormat(JE_FMT_OLE); break;
            }
        }
        break;
    case FIELD_PROP_PAR1 :
        rAny >>= sHelp;
        break;
    case FIELD_PROP_PAR2 :
         rAny >>= sText;
         break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// combined character field type

SwCombinedCharFieldType::SwCombinedCharFieldType()
    : SwFieldType( RES_COMBINED_CHARS )
{
}

SwFieldType* SwCombinedCharFieldType::Copy() const
{
    return new SwCombinedCharFieldType;
}

// combined character field

SwCombinedCharField::SwCombinedCharField( SwCombinedCharFieldType* pFTyp,
                                            const OUString& rChars )
    : SwField( pFTyp, 0 ),
    sCharacters( rChars.copy( 0, std::min<sal_Int32>(rChars.getLength(), MAX_COMBINED_CHARACTERS) ))
{
}

OUString SwCombinedCharField::Expand() const
{
    return sCharacters;
}

SwField* SwCombinedCharField::Copy() const
{
    return new SwCombinedCharField( static_cast<SwCombinedCharFieldType*>(GetTyp()),
                                        sCharacters );
}

OUString SwCombinedCharField::GetPar1() const
{
    return sCharacters;
}

void SwCombinedCharField::SetPar1(const OUString& rStr)
{
    sCharacters = rStr.copy(0, std::min<sal_Int32>(rStr.getLength(), MAX_COMBINED_CHARACTERS));
}

bool SwCombinedCharField::QueryValue( uno::Any& rAny,
                                        sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= sCharacters;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwCombinedCharField::PutValue( const uno::Any& rAny,
                                        sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
        case FIELD_PROP_PAR1:
        {
            OUString sTmp;
            rAny >>= sTmp;
            SetPar1(sTmp);
        }
        break;
        default:
            OSL_FAIL("illegal property");
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
