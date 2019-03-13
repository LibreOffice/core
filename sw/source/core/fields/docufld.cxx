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
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/DocumentStatistic.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <o3tl/any.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/unolingu.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <svl/urihelper.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/syslocale.hxx>
#include <svl/zforlist.hxx>
#include <libxml/xmlstring.h>
#include <libxml/xmlwriter.h>

#include <tools/time.hxx>
#include <tools/datetime.hxx>

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
#include <strings.hrc>

#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <calbck.hxx>
#include <docary.hxx>
#include <hints.hxx>

#define URL_DECODE  INetURLObject::DecodeMechanism::Unambiguous

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace nsSwDocInfoSubType;

SwPageNumberFieldType::SwPageNumberFieldType()
    : SwFieldType( SwFieldIds::PageNumber ),
    m_nNumberingType( SVX_NUM_ARABIC ),
    m_bVirtual( false )
{
}

OUString SwPageNumberFieldType::Expand( SvxNumType nFormat, short nOff,
         sal_uInt16 const nPageNumber, sal_uInt16 const nMaxPage,
         const OUString& rUserStr, LanguageType nLang ) const
{
    SvxNumType nTmpFormat = (SVX_NUM_PAGEDESC == nFormat) ? m_nNumberingType : nFormat;
    int const nTmp = nPageNumber + nOff;

    if (0 > nTmp || SVX_NUM_NUMBER_NONE == nTmpFormat || (!m_bVirtual && nTmp > nMaxPage))
        return OUString();

    if( SVX_NUM_CHAR_SPECIAL == nTmpFormat )
        return rUserStr;

    return FormatNumber( nTmp, nTmpFormat, nLang );
}

SwFieldType* SwPageNumberFieldType::Copy() const
{
    SwPageNumberFieldType *pTmp = new SwPageNumberFieldType();

    pTmp->m_nNumberingType = m_nNumberingType;
    pTmp->m_bVirtual  = m_bVirtual;

    return pTmp;
}

void SwPageNumberFieldType::ChangeExpansion( SwDoc* pDoc,
                                            bool bVirt,
                                            const SvxNumType* pNumFormat )
{
    if( pNumFormat )
        m_nNumberingType = *pNumFormat;

    m_bVirtual = false;
    if (bVirt && pDoc)
    {
        // check the flag since the layout NEVER sets it back
        const SfxItemPool &rPool = pDoc->GetAttrPool();
        sal_uInt32 nMaxItems = rPool.GetItemCount2( RES_PAGEDESC );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        {
            const SwFormatPageDesc *pDesc;
            if( nullptr != (pDesc = rPool.GetItem2( RES_PAGEDESC, n ) )
                && pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
            {
                const SwContentNode* pNd = dynamic_cast<const SwContentNode*>( pDesc->GetDefinedIn()  );
                if( pNd )
                {
                    if (SwIterator<SwFrame, SwContentNode, sw::IteratorMode::UnwrapMulti>(*pNd).First())
                    // sw_redlinehide: not sure if this should happen only if
                    // it's the first node, because that's where RES_PAGEDESC
                    // is effective?
                        m_bVirtual = true;
                }
                else if( dynamic_cast< const SwFormat* >(pDesc->GetDefinedIn()) !=  nullptr)
                {
                    SwAutoFormatGetDocNode aGetHt( &pDoc->GetNodes() );
                    m_bVirtual = !pDesc->GetDefinedIn()->GetInfo( aGetHt );
                    break;
                }
            }
        }
    }
}

SwPageNumberField::SwPageNumberField(SwPageNumberFieldType* pTyp,
          sal_uInt16 nSub, sal_uInt32 nFormat, short nOff,
          sal_uInt16 const nPageNumber, sal_uInt16 const nMaxPage)
    : SwField(pTyp, nFormat), m_nSubType(nSub), m_nOffset(nOff)
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

OUString SwPageNumberField::ExpandImpl(SwRootFrame const*const) const
{
    OUString sRet;
    SwPageNumberFieldType* pFieldType = static_cast<SwPageNumberFieldType*>(GetTyp());

    if( PG_NEXT == m_nSubType && 1 != m_nOffset )
    {
        sRet = pFieldType->Expand(static_cast<SvxNumType>(GetFormat()), 1, m_nPageNumber, m_nMaxPage, m_sUserStr, GetLanguage());
        if (!sRet.isEmpty())
        {
            sRet = pFieldType->Expand(static_cast<SvxNumType>(GetFormat()), m_nOffset, m_nPageNumber, m_nMaxPage, m_sUserStr, GetLanguage());
        }
    }
    else if( PG_PREV == m_nSubType && -1 != m_nOffset )
    {
        sRet = pFieldType->Expand(static_cast<SvxNumType>(GetFormat()), -1, m_nPageNumber, m_nMaxPage, m_sUserStr, GetLanguage());
        if (!sRet.isEmpty())
        {
            sRet = pFieldType->Expand(static_cast<SvxNumType>(GetFormat()), m_nOffset, m_nPageNumber, m_nMaxPage, m_sUserStr, GetLanguage());
        }
    }
    else
        sRet = pFieldType->Expand(static_cast<SvxNumType>(GetFormat()), m_nOffset, m_nPageNumber, m_nMaxPage, m_sUserStr, GetLanguage());
    return sRet;
}

std::unique_ptr<SwField> SwPageNumberField::Copy() const
{
    std::unique_ptr<SwPageNumberField> pTmp(new SwPageNumberField(
                static_cast<SwPageNumberFieldType*>(GetTyp()), m_nSubType,
                GetFormat(), m_nOffset, m_nPageNumber, m_nMaxPage));
    pTmp->SetLanguage( GetLanguage() );
    pTmp->SetUserString( m_sUserStr );
    return std::unique_ptr<SwField>(pTmp.release());
}

OUString SwPageNumberField::GetPar2() const
{
    return OUString::number(m_nOffset);
}

void SwPageNumberField::SetPar2(const OUString& rStr)
{
    m_nOffset = static_cast<short>(rStr.toInt32());
}

sal_uInt16 SwPageNumberField::GetSubType() const
{
    return m_nSubType;
}

bool SwPageNumberField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_FORMAT:
        rAny <<= static_cast<sal_Int16>(GetFormat());
        break;
    case FIELD_PROP_USHORT1:
        rAny <<= m_nOffset;
        break;
    case FIELD_PROP_SUBTYPE:
        {
            text::PageNumberType eType;
            eType = text::PageNumberType_CURRENT;
            if(m_nSubType == PG_PREV)
                eType = text::PageNumberType_PREV;
            else if(m_nSubType == PG_NEXT)
                eType = text::PageNumberType_NEXT;
            rAny <<= eType;
        }
        break;
    case FIELD_PROP_PAR1:
        rAny <<= m_sUserStr;
        break;

    default:
        assert(false);
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

        // TODO: where do the defines come from?
        if(nSet <= SVX_NUM_PAGEDESC )
            SetFormat(nSet);
        break;
    case FIELD_PROP_USHORT1:
        rAny >>= nSet;
        m_nOffset = nSet;
        break;
    case FIELD_PROP_SUBTYPE:
        switch( static_cast<text::PageNumberType>(SWUnoHelper::GetEnumAsInt32( rAny )) )
        {
            case text::PageNumberType_CURRENT:
                m_nSubType = PG_RANDOM;
            break;
            case text::PageNumberType_PREV:
                m_nSubType = PG_PREV;
            break;
            case text::PageNumberType_NEXT:
                m_nSubType = PG_NEXT;
            break;
            default:
                bRet = false;
        }
        break;
    case FIELD_PROP_PAR1:
        rAny >>= m_sUserStr;
        break;

    default:
        assert(false);
    }
    return bRet;
}

SwAuthorFieldType::SwAuthorFieldType()
    : SwFieldType( SwFieldIds::Author )
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
    m_aContent = SwAuthorFieldType::Expand(GetFormat());
}

OUString SwAuthorField::ExpandImpl(SwRootFrame const*const) const
{
    if (!IsFixed())
        const_cast<SwAuthorField*>(this)->m_aContent =
                    SwAuthorFieldType::Expand(GetFormat());

    return m_aContent;
}

std::unique_ptr<SwField> SwAuthorField::Copy() const
{
    std::unique_ptr<SwAuthorField> pTmp(new SwAuthorField( static_cast<SwAuthorFieldType*>(GetTyp()),
                                                GetFormat()));
    pTmp->SetExpansion(m_aContent);
    return std::unique_ptr<SwField>(pTmp.release());
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
        rAny <<= m_aContent;
        break;

    default:
        assert(false);
    }
    return true;
}

bool SwAuthorField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        SetFormat( *o3tl::doAccess<bool>(rAny) ? AF_NAME : AF_SHORTCUT );
        break;

    case FIELD_PROP_BOOL2:
        if( *o3tl::doAccess<bool>(rAny) )
            SetFormat( GetFormat() | AF_FIXED);
        else
            SetFormat( GetFormat() & ~AF_FIXED);
        break;

    case FIELD_PROP_PAR1:
        rAny >>= m_aContent;
        break;

    default:
        assert(false);
    }
    return true;
}

SwFileNameFieldType::SwFileNameFieldType(SwDoc *pDocument)
    : SwFieldType( SwFieldIds::Filename )
{
    m_pDoc = pDocument;
}

OUString SwFileNameFieldType::Expand(sal_uLong nFormat) const
{
    OUString aRet;
    const SwDocShell* pDShell = m_pDoc->GetDocShell();
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
                                    rURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                    INetURLObject::EncodeMechanism::WasEncoded, URL_DECODE );
                        const sal_Int32 nPos = aRet.indexOf(rURLObj.GetLastName( URL_DECODE ));
                        if (nPos>=0)
                        {
                            aRet = aRet.copy(0, nPos);
                        }
                    }
                }
                break;

            case FF_NAME:
                aRet = rURLObj.GetLastName( INetURLObject::DecodeMechanism::WithCharset );
                break;

            case FF_NAME_NOEXT:
                aRet = rURLObj.GetBase();
                break;

            default:
                if( INetProtocol::File == rURLObj.GetProtocol() )
                    aRet = rURLObj.GetFull();
                else
                    aRet = URIHelper::removePassword(
                                    rURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                    INetURLObject::EncodeMechanism::WasEncoded, URL_DECODE );
        }
    }
    return aRet;
}

SwFieldType* SwFileNameFieldType::Copy() const
{
    SwFieldType *pTmp = new SwFileNameFieldType(m_pDoc);
    return pTmp;
}

SwFileNameField::SwFileNameField(SwFileNameFieldType* pTyp, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat)
{
    m_aContent = static_cast<SwFileNameFieldType*>(GetTyp())->Expand(GetFormat());
}

OUString SwFileNameField::ExpandImpl(SwRootFrame const*const) const
{
    if (!IsFixed())
        const_cast<SwFileNameField*>(this)->m_aContent = static_cast<SwFileNameFieldType*>(GetTyp())->Expand(GetFormat());

    return m_aContent;
}

std::unique_ptr<SwField> SwFileNameField::Copy() const
{
    std::unique_ptr<SwFileNameField> pTmp(
        new SwFileNameField(static_cast<SwFileNameFieldType*>(GetTyp()), GetFormat()));
    pTmp->SetExpansion(m_aContent);

    return std::unique_ptr<SwField>(pTmp.release());
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
        rAny <<= m_aContent;
        break;

    default:
        assert(false);
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
        if( *o3tl::doAccess<bool>(rAny) )
            SetFormat( GetFormat() | FF_FIXED);
        else
            SetFormat( GetFormat() & ~FF_FIXED);
        break;

    case FIELD_PROP_PAR3:
        rAny >>= m_aContent;
        break;

    default:
        assert(false);
    }
    return true;
}

SwTemplNameFieldType::SwTemplNameFieldType(SwDoc *pDocument)
    : SwFieldType( SwFieldIds::TemplateName )
{
    m_pDoc = pDocument;
}

OUString SwTemplNameFieldType::Expand(sal_uLong nFormat) const
{
    OSL_ENSURE( nFormat < FF_END, "Expand: no valid Format!" );

    OUString aRet;
    SwDocShell *pDocShell(m_pDoc->GetDocShell());
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
    SwFieldType *pTmp = new SwTemplNameFieldType(m_pDoc);
    return pTmp;
}

SwTemplNameField::SwTemplNameField(SwTemplNameFieldType* pTyp, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat)
{}

OUString SwTemplNameField::ExpandImpl(SwRootFrame const*const) const
{
    return static_cast<SwTemplNameFieldType*>(GetTyp())->Expand(GetFormat());
}

std::unique_ptr<SwField> SwTemplNameField::Copy() const
{
    return std::make_unique<SwTemplNameField>(static_cast<SwTemplNameFieldType*>(GetTyp()), GetFormat());
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
        assert(false);
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
        assert(false);
    }
    return true;
}

SwDocStatFieldType::SwDocStatFieldType(SwDoc* pDocument)
    : SwFieldType( SwFieldIds::DocStat ), m_nNumberingType( SVX_NUM_ARABIC )
{
    m_pDoc = pDocument;
}

OUString SwDocStatFieldType::Expand(sal_uInt16 nSubType, SvxNumType nFormat) const
{
    sal_uInt32 nVal = 0;
    const SwDocStat& rDStat = m_pDoc->getIDocumentStatistics().GetDocStat();
    switch( nSubType )
    {
        case DS_TBL:  nVal = rDStat.nTable;   break;
        case DS_GRF:  nVal = rDStat.nGrf;   break;
        case DS_OLE:  nVal = rDStat.nOLE;   break;
        case DS_PARA: nVal = rDStat.nPara;  break;
        case DS_WORD: nVal = rDStat.nWord;  break;
        case DS_CHAR: nVal = rDStat.nChar;  break;
        case DS_PAGE:
            if( m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
                const_cast<SwDocStat &>(rDStat).nPage = m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetPageNum();
            nVal = rDStat.nPage;
            if( SVX_NUM_PAGEDESC == nFormat )
                nFormat = m_nNumberingType;
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
    SwDocStatFieldType *pTmp = new SwDocStatFieldType(m_pDoc);
    return pTmp;
}

/**
 * @param pTyp
 * @param nSub SubType
 * @param nFormat
 */
SwDocStatField::SwDocStatField(SwDocStatFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFormat)
    : SwField(pTyp, nFormat),
    m_nSubType(nSub)
{}

OUString SwDocStatField::ExpandImpl(SwRootFrame const*const) const
{
    return static_cast<SwDocStatFieldType*>(GetTyp())->Expand(m_nSubType, static_cast<SvxNumType>(GetFormat()));
}

std::unique_ptr<SwField> SwDocStatField::Copy() const
{
    return std::make_unique<SwDocStatField>(
                    static_cast<SwDocStatFieldType*>(GetTyp()), m_nSubType, GetFormat() );
}

sal_uInt16 SwDocStatField::GetSubType() const
{
    return m_nSubType;
}

void SwDocStatField::SetSubType(sal_uInt16 nSub)
{
    m_nSubType = nSub;
}

void SwDocStatField::ChangeExpansion( const SwFrame* pFrame )
{
    if( DS_PAGE == m_nSubType && SVX_NUM_PAGEDESC == GetFormat() )
        static_cast<SwDocStatFieldType*>(GetTyp())->SetNumFormat(
                pFrame->FindPageFrame()->GetPageDesc()->GetNumType().GetNumberingType() );
}

bool SwDocStatField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch ( nWhichId )
    {
    case FIELD_PROP_USHORT2:
        rAny <<= static_cast<sal_Int16>(GetFormat());
        break;

    default:
        assert(false);
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
        assert(false);
    }
    return bRet;
}

// Document info field type

SwDocInfoFieldType::SwDocInfoFieldType(SwDoc* pDc)
    : SwValueFieldType( pDc, SwFieldIds::DocInfo )
{
}

SwFieldType* SwDocInfoFieldType::Copy() const
{
    SwDocInfoFieldType* pTyp = new SwDocInfoFieldType(GetDoc());
    return pTyp;
}

static void lcl_GetLocalDataWrapper( LanguageType nLang,
                              const LocaleDataWrapper **ppAppLocalData,
                              const LocaleDataWrapper **ppLocalData )
{
    SvtSysLocale aLocale;
    *ppAppLocalData = &aLocale.GetLocaleData();
    *ppLocalData = *ppAppLocalData;
    if( nLang != (*ppLocalData)->getLanguageTag().getLanguageType() )
        *ppLocalData = new LocaleDataWrapper(LanguageTag( nLang ));
}

OUString SwDocInfoFieldType::Expand( sal_uInt16 nSub, sal_uInt32 nFormat,
                                    LanguageType nLang, const OUString& rName ) const
{
    const LocaleDataWrapper *pAppLocalData = nullptr, *pLocalData = nullptr;
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
    case DI_TITLE:  aStr = xDocProps->getTitle();       break;
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
                ;       // that's it !!
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
                        // start the number formatter
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
                        // start the number formatter
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
    SwValueField(pTyp, nFormat), m_nSubType(nSub)
{
    m_aName = rName;
    m_aContent = static_cast<SwDocInfoFieldType*>(GetTyp())->Expand(m_nSubType, nFormat, GetLanguage(), m_aName);
}

SwDocInfoField::SwDocInfoField(SwDocInfoFieldType* pTyp, sal_uInt16 nSub, const OUString& rName, const OUString& rValue, sal_uInt32 nFormat) :
    SwValueField(pTyp, nFormat), m_nSubType(nSub)
{
    m_aName = rName;
    m_aContent = rValue;
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

OUString SwDocInfoField::ExpandImpl(SwRootFrame const*const) const
{
    if ( ( m_nSubType & 0xFF ) == DI_CUSTOM )
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
            return m_aContent;
        try
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS( pDocShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xDocProps( xDPS->getDocumentProperties());
            uno::Reference < beans::XPropertySet > xSet( xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
            uno::Reference < beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();

            uno::Any aAny;
            if( xSetInfo->hasPropertyByName( m_aName ) )
                aAny = xSet->getPropertyValue( m_aName );
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
                        const Date& rNullDate = pFormatter->GetNullDate();
                        sVal = ExpandValue( lcl_DateToDouble<util::Date>( aDate, rNullDate ), GetFormat(), GetLanguage());
                    }
                    else if( aAny >>= aDateTime )
                    {
                        double fDateTime = lcl_TimeToDouble<util::DateTime>( aDateTime );
                        SvNumberFormatter* pFormatter = pDocShell->GetDoc()->GetNumberFormatter();
                        const Date& rNullDate = pFormatter->GetNullDate();
                        fDateTime += lcl_DateToDouble<util::DateTime>( aDateTime, rNullDate );
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
                    const_cast<SwDocInfoField*>(this)->m_aContent = sVal;
                }
            }
        }
        catch (uno::Exception&) {}
    }
    else if ( !IsFixed() )
        const_cast<SwDocInfoField*>(this)->m_aContent = static_cast<SwDocInfoFieldType*>(GetTyp())->Expand(m_nSubType, GetFormat(), GetLanguage(), m_aName);

    return m_aContent;
}

OUString SwDocInfoField::GetFieldName() const
{
    OUString aStr(SwFieldType::GetTypeStr(GetTypeId()) + ":");

    sal_uInt16 const nSub = m_nSubType & 0xff;

    switch (nSub)
    {
        case DI_CUSTOM:
            aStr += m_aName;
            break;

        default:
            aStr += SwViewShell::GetShellRes()
                     ->aDocInfoLst[ nSub - DI_SUBTYPE_BEGIN ];
            break;
    }
    if (IsFixed())
    {
        aStr += " " + SwViewShell::GetShellRes()->aFixedStr;
    }
    return aStr;
}

std::unique_ptr<SwField> SwDocInfoField::Copy() const
{
    std::unique_ptr<SwDocInfoField> pField(new SwDocInfoField(static_cast<SwDocInfoFieldType*>(GetTyp()), m_nSubType, m_aName, GetFormat()));
    pField->SetAutomaticLanguage(IsAutomaticLanguage());
    pField->m_aContent = m_aContent;

    return std::unique_ptr<SwField>(pField.release());
}

sal_uInt16 SwDocInfoField::GetSubType() const
{
    return m_nSubType;
}

void SwDocInfoField::SetSubType(sal_uInt16 nSub)
{
    m_nSubType = nSub;
}

void SwDocInfoField::SetLanguage(LanguageType nLng)
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
        rAny <<= m_aContent;
        break;

    case FIELD_PROP_PAR4:
        rAny <<= m_aName;
        break;

    case FIELD_PROP_USHORT1:
        rAny  <<= static_cast<sal_Int16>(m_aContent.toInt32());
        break;

    case FIELD_PROP_BOOL1:
        rAny <<= 0 != (m_nSubType & DI_SUB_FIXED);
        break;

    case FIELD_PROP_FORMAT:
        rAny  <<= static_cast<sal_Int32>(GetFormat());
        break;

    case FIELD_PROP_DOUBLE:
        {
            double fVal = GetValue();
            rAny <<= fVal;
        }
        break;
    case FIELD_PROP_PAR3:
        rAny <<= ExpandImpl(nullptr);
        break;
    case FIELD_PROP_BOOL2:
        {
            sal_uInt16 nExtSub = (m_nSubType & 0xff00) & ~DI_SUB_FIXED;
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
        if( m_nSubType & DI_SUB_FIXED )
            rAny >>= m_aContent;
        break;

    case FIELD_PROP_USHORT1:
        if( m_nSubType & DI_SUB_FIXED )
        {
            rAny >>= nValue;
            m_aContent = OUString::number(nValue);
        }
        break;

    case FIELD_PROP_BOOL1:
        if(*o3tl::doAccess<bool>(rAny))
            m_nSubType |= DI_SUB_FIXED;
        else
            m_nSubType &= ~DI_SUB_FIXED;
        break;
    case FIELD_PROP_FORMAT:
        {
            rAny >>= nValue;
            if( nValue >= 0)
                SetFormat(nValue);
        }
        break;

    case FIELD_PROP_PAR3:
        rAny >>= m_aContent;
        break;
    case FIELD_PROP_BOOL2:
        m_nSubType &= 0xf0ff;
        if(*o3tl::doAccess<bool>(rAny))
            m_nSubType |= DI_SUB_DATE;
        else
            m_nSubType |= DI_SUB_TIME;
        break;
    default:
        return SwField::PutValue(rAny, nWhichId);
    }
    return true;
}

SwHiddenTextFieldType::SwHiddenTextFieldType( bool bSetHidden )
    : SwFieldType( SwFieldIds::HiddenText ), m_bHidden( bSetHidden )
{
}

SwFieldType* SwHiddenTextFieldType::Copy() const
{
    return new SwHiddenTextFieldType( m_bHidden );
}

void SwHiddenTextFieldType::SetHiddenFlag( bool bSetHidden )
{
    if( m_bHidden != bSetHidden )
    {
        m_bHidden = bSetHidden;
        UpdateFields();       // notify all HiddenTexts
    }
}

SwHiddenTextField::SwHiddenTextField( SwHiddenTextFieldType* pFieldType,
                                    bool    bConditional,
                                    const OUString& rCond,
                                    const OUString& rStr,
                                    bool    bHidden,
                                    sal_uInt16  nSub) :
    SwField( pFieldType ), m_aCond(rCond), m_nSubType(nSub),
    m_bCanToggle(bConditional), m_bIsHidden(bHidden), m_bValid(false)
{
    if(m_nSubType == TYP_CONDTXTFLD)
    {
        sal_Int32 nPos = 0;
        m_aTRUEText = rStr.getToken(0, '|', nPos);

        if(nPos != -1)
        {
            m_aFALSEText = rStr.getToken(0, '|', nPos);
            if(nPos != -1)
            {
                m_aContent = rStr.getToken(0, '|', nPos);
                m_bValid = true;
            }
        }
    }
    else
        m_aTRUEText = rStr;
}

SwHiddenTextField::SwHiddenTextField( SwHiddenTextFieldType* pFieldType,
                                    const OUString& rCond,
                                    const OUString& rTrue,
                                    const OUString& rFalse,
                                    sal_uInt16 nSub)
    : SwField( pFieldType ), m_aTRUEText(rTrue), m_aFALSEText(rFalse), m_aCond(rCond), m_nSubType(nSub),
      m_bIsHidden(true), m_bValid(false)
{
    m_bCanToggle = !m_aCond.isEmpty();
}

OUString SwHiddenTextField::ExpandImpl(SwRootFrame const*const) const
{
    // Type: !Hidden  -> show always
    //        Hide    -> evaluate condition

    if( TYP_CONDTXTFLD == m_nSubType )
    {
        if( m_bValid )
            return m_aContent;

        if( m_bCanToggle && !m_bIsHidden )
            return m_aTRUEText;
    }
    else if( !static_cast<SwHiddenTextFieldType*>(GetTyp())->GetHiddenFlag() ||
        ( m_bCanToggle && m_bIsHidden ))
        return m_aTRUEText;

    return m_aFALSEText;
}

/// get current field value and cache it
void SwHiddenTextField::Evaluate(SwDoc* pDoc)
{
    OSL_ENSURE(pDoc, "got no document");

    if( TYP_CONDTXTFLD == m_nSubType )
    {
#if !HAVE_FEATURE_DBCONNECTIVITY
        (void) pDoc;
#else
        SwDBManager* pMgr = pDoc->GetDBManager();
#endif
        m_bValid = false;
        OUString sTmpName = (m_bCanToggle && !m_bIsHidden) ? m_aTRUEText : m_aFALSEText;

        // Database expressions need to be different from normal text. Therefore, normal text is set
        // in quotes. If the latter exist they will be removed. If not, check if potential DB name.
        // Only if there are two or more dots and no quotes, we assume a database.
        if (sTmpName.getLength()>1 &&
            sTmpName.startsWith("\"") &&
            sTmpName.endsWith("\""))
        {
            m_aContent = sTmpName.copy(1, sTmpName.getLength() - 2);
            m_bValid = true;
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
                sal_Int32 nIdx{ 0 };
                OUString sDBName( GetDBName( sTmpName, pDoc ));
                OUString sDataSource(sDBName.getToken(0, DB_DELIM, nIdx));
                OUString sDataTableOrQuery(sDBName.getToken(0, DB_DELIM, nIdx));
                if( pMgr->IsInMerge() && !sDBName.isEmpty() &&
                    pMgr->IsDataSourceOpen( sDataSource,
                                                sDataTableOrQuery, false))
                {
                    double fNumber;
                    pMgr->GetMergeColumnCnt(GetColumnName( sTmpName ),
                        GetLanguage(), m_aContent, &fNumber );
                    m_bValid = true;
                }
                else if( !sDBName.isEmpty() && !sDataSource.isEmpty() &&
                         !sDataTableOrQuery.isEmpty() )
                    m_bValid = true;
            }
#endif
        }
    }
}

OUString SwHiddenTextField::GetFieldName() const
{
    OUString aStr = SwFieldType::GetTypeStr(m_nSubType) +
        " " + m_aCond + " " + m_aTRUEText;

    if (m_nSubType == TYP_CONDTXTFLD)
    {
        aStr += " : " + m_aFALSEText;
    }
    return aStr;
}

std::unique_ptr<SwField> SwHiddenTextField::Copy() const
{
    std::unique_ptr<SwHiddenTextField> pField(
        new SwHiddenTextField(static_cast<SwHiddenTextFieldType*>(GetTyp()), m_aCond,
                              m_aTRUEText, m_aFALSEText));
    pField->m_bIsHidden = m_bIsHidden;
    pField->m_bValid    = m_bValid;
    pField->m_aContent  = m_aContent;
    pField->SetFormat(GetFormat());
    pField->m_nSubType  = m_nSubType;
    return std::unique_ptr<SwField>(pField.release());
}

/// set condition
void SwHiddenTextField::SetPar1(const OUString& rStr)
{
    m_aCond = rStr;
    m_bCanToggle = !m_aCond.isEmpty();
}

OUString SwHiddenTextField::GetPar1() const
{
    return m_aCond;
}

/// set True/False text
void SwHiddenTextField::SetPar2(const OUString& rStr)
{
    if (m_nSubType == TYP_CONDTXTFLD)
    {
        sal_Int32 nPos = rStr.indexOf('|');
        if (nPos == -1)
            m_aTRUEText = rStr;
        else
        {
            m_aTRUEText = rStr.copy(0, nPos);
            m_aFALSEText = rStr.copy(nPos + 1);
        }
    }
    else
        m_aTRUEText = rStr;
}

/// get True/False text
OUString SwHiddenTextField::GetPar2() const
{
    if(m_nSubType != TYP_CONDTXTFLD)
    {
        return m_aTRUEText;
    }
    return m_aTRUEText + "|" + m_aFALSEText;
}

sal_uInt16 SwHiddenTextField::GetSubType() const
{
    return m_nSubType;
}

bool SwHiddenTextField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_aCond;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_aTRUEText;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= m_aFALSEText;
        break;
    case FIELD_PROP_PAR4 :
        rAny <<= m_aContent;
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= m_bIsHidden;
        break;
    default:
        assert(false);
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
        rAny >>= m_aTRUEText;
        break;
    case FIELD_PROP_PAR3:
        rAny >>= m_aFALSEText;
        break;
    case FIELD_PROP_BOOL1:
        m_bIsHidden = *o3tl::doAccess<bool>(rAny);
        break;
    case FIELD_PROP_PAR4:
        rAny >>= m_aContent;
        m_bValid = true;
        break;
    default:
        assert(false);
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
    return aData.sDataSource + OUStringLiteral1(DB_DELIM) + aData.sCommand;
}

// [aFieldDefinition] value sample : " IF A == B \"TrueText\" \"FalseText\""
void SwHiddenTextField::ParseIfFieldDefinition(const OUString& aFieldDefinition,
                                               OUString& rCondition,
                                               OUString& rTrue,
                                               OUString& rFalse)
{
    // get all positions inside the input string where words are started
    //
    // In: " IF A == B \"TrueText\" \"FalseText\""
    //      0         1           2          3
    //      01234567890 123456789 01 2345678901 2
    //
    // result:
    //      [1, 4, 6, 9, 11, 22]
    std::vector<sal_Int32> wordPosition;
    {
        bool quoted = false;
        bool insideWord = false;
        for (sal_Int32 i = 0; i < aFieldDefinition.getLength(); i++)
        {
            if (quoted)
            {
                if (aFieldDefinition[i] == '\"')
                {
                    quoted = false;
                    insideWord = false;
                }
            }
            else
            {
                if (aFieldDefinition[i] == ' ')
                {
                    // word delimiter
                    insideWord = false;
                }
                else
                {
                    if (insideWord)
                    {
                        quoted = (aFieldDefinition[i] == '\"');
                    }
                    else
                    {
                        insideWord = true;
                        wordPosition.push_back(i);
                        quoted = (aFieldDefinition[i] == '\"');
                    }
                }
            }
        }
    }

    // first word is always "IF"
    // last two words are: true-case and false-case,
    // everything before is treated as condition expression
    // => we need at least 4 words to be inside the input string
    if (wordPosition.size() < 4)
    {
        return;
    }


    const sal_Int32 conditionBegin = wordPosition[1];
    const sal_Int32 trueBegin      = wordPosition[wordPosition.size() - 2];
    const sal_Int32 falseBegin     = wordPosition[wordPosition.size() - 1];

    const sal_Int32 conditionLength = trueBegin - conditionBegin;
    const sal_Int32 trueLength      = falseBegin - trueBegin;

    // Syntax
    // OUString::copy( sal_Int32 beginIndex, sal_Int32 count )
    rCondition = aFieldDefinition.copy(conditionBegin, conditionLength);
    rTrue = aFieldDefinition.copy(trueBegin, trueLength);
    rFalse = aFieldDefinition.copy(falseBegin);

    // trim
    rCondition = rCondition.trim();
    rTrue = rTrue.trim();
    rFalse = rFalse.trim();

    // remove quotes
    if (rCondition.getLength() >= 2)
    {
        if (rCondition[0] == '\"' && rCondition[rCondition.getLength() - 1] == '\"')
            rCondition = rCondition.copy(1, rCondition.getLength() - 2);
    }
    if (rTrue.getLength() >= 2)
    {
        if (rTrue[0] == '\"' && rTrue[rTrue.getLength() - 1] == '\"')
            rTrue = rTrue.copy(1, rTrue.getLength() - 2);
    }
    if (rFalse.getLength() >= 2)
    {
        if (rFalse[0] == '\"' && rFalse[rFalse.getLength() - 1] == '\"')
            rFalse = rFalse.copy(1, rFalse.getLength() - 2);
    }

    // Note: do not make trim once again, while this is a user defined data
}

// field type for line height 0

SwHiddenParaFieldType::SwHiddenParaFieldType()
    : SwFieldType( SwFieldIds::HiddenPara )
{
}

SwFieldType* SwHiddenParaFieldType::Copy() const
{
    SwHiddenParaFieldType* pTyp = new SwHiddenParaFieldType();
    return pTyp;
}

// field for line height 0

SwHiddenParaField::SwHiddenParaField(SwHiddenParaFieldType* pTyp, const OUString& rStr)
    : SwField(pTyp), m_aCond(rStr)
{
    m_bIsHidden = false;
}

OUString SwHiddenParaField::ExpandImpl(SwRootFrame const*const) const
{
    return OUString();
}

std::unique_ptr<SwField> SwHiddenParaField::Copy() const
{
    std::unique_ptr<SwHiddenParaField> pField(new SwHiddenParaField(static_cast<SwHiddenParaFieldType*>(GetTyp()), m_aCond));
    pField->m_bIsHidden = m_bIsHidden;
    return std::unique_ptr<SwField>(pField.release());
}

bool SwHiddenParaField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_aCond;
        break;
    case  FIELD_PROP_BOOL1:
        rAny <<= m_bIsHidden;
        break;

    default:
        assert(false);
    }
    return true;
}

bool SwHiddenParaField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= m_aCond;
        break;
    case FIELD_PROP_BOOL1:
        m_bIsHidden = *o3tl::doAccess<bool>(rAny);
        break;

    default:
        assert(false);
    }
    return true;
}

/// set condition
void SwHiddenParaField::SetPar1(const OUString& rStr)
{
    m_aCond = rStr;
}

OUString SwHiddenParaField::GetPar1() const
{
    return m_aCond;
}

// PostIt field type

SwPostItFieldType::SwPostItFieldType(SwDoc *pDoc)
    : SwFieldType( SwFieldIds::Postit )
    , mpDoc(pDoc)
{}

SwFieldType* SwPostItFieldType::Copy() const
{
    return new SwPostItFieldType(mpDoc);
}

// PostIt field

sal_uInt32 SwPostItField::m_nLastPostItId = 1;

SwPostItField::SwPostItField( SwPostItFieldType* pT,
        const OUString& rAuthor,
        const OUString& rText,
        const OUString& rInitials,
        const OUString& rName,
        const DateTime& rDateTime,
        const sal_uInt32 nPostItId)
    : SwField( pT )
    , m_sText( rText )
    , m_sAuthor( rAuthor )
    , m_sInitials( rInitials )
    , m_sName( rName )
    , m_aDateTime( rDateTime )
{
    m_nPostItId = nPostItId == 0 ? m_nLastPostItId++ : nPostItId;
}

SwPostItField::~SwPostItField()
{
    if ( m_xTextObject.is() )
    {
        m_xTextObject->DisposeEditSource();
    }

    mpText.reset();
}

OUString SwPostItField::ExpandImpl(SwRootFrame const*const) const
{
    return OUString();
}

OUString SwPostItField::GetDescription() const
{
    return SwResId(STR_NOTE);
}

std::unique_ptr<SwField> SwPostItField::Copy() const
{
    std::unique_ptr<SwPostItField> pRet(new SwPostItField( static_cast<SwPostItFieldType*>(GetTyp()), m_sAuthor, m_sText, m_sInitials, m_sName,
                                             m_aDateTime, m_nPostItId));
    if (mpText)
        pRet->SetTextObject( std::make_unique<OutlinerParaObject>(*mpText) );

    // Note: member <m_xTextObject> not copied.

    return std::unique_ptr<SwField>(pRet.release());
}

/// set author
void SwPostItField::SetPar1(const OUString& rStr)
{
    m_sAuthor = rStr;
}

/// get author
OUString SwPostItField::GetPar1() const
{
    return m_sAuthor;
}

/// set the PostIt's text
void SwPostItField::SetPar2(const OUString& rStr)
{
    m_sText = rStr;
}

/// get the PostIt's text
OUString SwPostItField::GetPar2() const
{
    return m_sText;
}


void SwPostItField::SetName(const OUString& rName)
{
    m_sName = rName;
}


void SwPostItField::SetTextObject( std::unique_ptr<OutlinerParaObject> pText )
{
    mpText = std::move(pText);
}

sal_Int32 SwPostItField::GetNumberOfParagraphs() const
{
    return mpText ? mpText->Count() : 1;
}

bool SwPostItField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_sAuthor;
        break;
    case FIELD_PROP_PAR2:
        {
        rAny <<= m_sText;
        break;
        }
    case FIELD_PROP_PAR3:
        rAny <<= m_sInitials;
        break;
    case FIELD_PROP_PAR4:
        rAny <<= m_sName;
        break;
    case FIELD_PROP_TEXT:
        {
            if ( !m_xTextObject.is() )
            {
                SwPostItFieldType* pGetType = static_cast<SwPostItFieldType*>(GetTyp());
                SwDoc* pDoc = pGetType->GetDoc();
                auto pObj = std::make_unique<SwTextAPIEditSource>( pDoc );
                const_cast <SwPostItField*> (this)->m_xTextObject = new SwTextAPIObject( std::move(pObj) );
            }

            if ( mpText )
                m_xTextObject->SetText( *mpText );
            else
                m_xTextObject->SetString( m_sText );

            uno::Reference < text::XText > xText( m_xTextObject.get() );
            rAny <<= xText;
            break;
        }
    case FIELD_PROP_DATE:
        {
            rAny <<= m_aDateTime.GetUNODate();
        }
        break;
    case FIELD_PROP_DATE_TIME:
        {
            rAny <<= m_aDateTime.GetUNODateTime();
        }
        break;
    default:
        assert(false);
    }
    return true;
}

bool SwPostItField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= m_sAuthor;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_sText;
        //#i100374# new string via api, delete complex text object so SwPostItNote picks up the new string
        mpText.reset();
        break;
    case FIELD_PROP_PAR3:
        rAny >>= m_sInitials;
        break;
    case FIELD_PROP_PAR4:
        rAny >>= m_sName;
        break;
    case FIELD_PROP_TEXT:
        OSL_FAIL("Not implemented!");
        break;
    case FIELD_PROP_DATE:
        if( auto aSetDate = o3tl::tryAccess<util::Date>(rAny) )
        {
            m_aDateTime = Date(aSetDate->Day, aSetDate->Month, aSetDate->Year);
        }
        break;
    case FIELD_PROP_DATE_TIME:
    {
        util::DateTime aDateTimeValue;
        if(!(rAny >>= aDateTimeValue))
            return false;
        m_aDateTime = DateTime(aDateTimeValue);
    }
    break;
    default:
        assert(false);
    }
    return true;
}

void SwPostItField::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwPostItField"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));

    SwField::dumpAsXml(pWriter);

    xmlTextWriterStartElement(pWriter, BAD_CAST("mpText"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", mpText.get());
    if (mpText)
        mpText->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);

    xmlTextWriterEndElement(pWriter);
}

// extended user information field type

SwExtUserFieldType::SwExtUserFieldType()
    : SwFieldType( SwFieldIds::ExtUser )
{
}

SwFieldType* SwExtUserFieldType::Copy() const
{
    SwExtUserFieldType* pTyp = new SwExtUserFieldType;
    return pTyp;
}

OUString SwExtUserFieldType::Expand(sal_uInt16 nSub )
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
    SwField(pTyp, nFormat), m_nType(nSubTyp)
{
    m_aContent = SwExtUserFieldType::Expand(m_nType);
}

OUString SwExtUserField::ExpandImpl(SwRootFrame const*const) const
{
    if (!IsFixed())
        const_cast<SwExtUserField*>(this)->m_aContent = SwExtUserFieldType::Expand(m_nType);

    return m_aContent;
}

std::unique_ptr<SwField> SwExtUserField::Copy() const
{
    std::unique_ptr<SwExtUserField> pField(new SwExtUserField(static_cast<SwExtUserFieldType*>(GetTyp()), m_nType, GetFormat()));
    pField->SetExpansion(m_aContent);

    return std::unique_ptr<SwField>(pField.release());
}

sal_uInt16 SwExtUserField::GetSubType() const
{
    return m_nType;
}

void SwExtUserField::SetSubType(sal_uInt16 nSub)
{
    m_nType = nSub;
}

bool SwExtUserField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_aContent;
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp = m_nType;
            rAny <<= nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= IsFixed();
        break;
    default:
        assert(false);
    }
    return true;
}

bool SwExtUserField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= m_aContent;
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp = 0;
            rAny >>= nTmp;
            m_nType = nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        if( *o3tl::doAccess<bool>(rAny) )
            SetFormat(GetFormat() | AF_FIXED);
        else
            SetFormat(GetFormat() & ~AF_FIXED);
        break;
    default:
        assert(false);
    }
    return true;
}

// field type for relative page numbers

SwRefPageSetFieldType::SwRefPageSetFieldType()
    : SwFieldType( SwFieldIds::RefPageSet )
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
    : SwField( pTyp ), m_nOffset( nOff ), m_bOn( bFlag )
{
}

OUString SwRefPageSetField::ExpandImpl(SwRootFrame const*const) const
{
    return OUString();
}

std::unique_ptr<SwField> SwRefPageSetField::Copy() const
{
    return std::make_unique<SwRefPageSetField>( static_cast<SwRefPageSetFieldType*>(GetTyp()), m_nOffset, m_bOn );
}

OUString SwRefPageSetField::GetPar2() const
{
    return OUString::number(GetOffset());
}

void SwRefPageSetField::SetPar2(const OUString& rStr)
{
    SetOffset( static_cast<short>(rStr.toInt32()) );
}

bool SwRefPageSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        rAny <<= m_bOn;
        break;
    case FIELD_PROP_USHORT1:
        rAny <<= static_cast<sal_Int16>(m_nOffset);
        break;
    default:
        assert(false);
    }
    return true;
}

bool SwRefPageSetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        m_bOn = *o3tl::doAccess<bool>(rAny);
        break;
    case FIELD_PROP_USHORT1:
        rAny >>=m_nOffset;
        break;
    default:
        assert(false);
    }
    return true;
}

// relative page numbers - query field

SwRefPageGetFieldType::SwRefPageGetFieldType( SwDoc* pDc )
    : SwFieldType( SwFieldIds::RefPageGet ), m_pDoc( pDc ), m_nNumberingType( SVX_NUM_ARABIC )
{
}

SwFieldType* SwRefPageGetFieldType::Copy() const
{
    SwRefPageGetFieldType* pNew = new SwRefPageGetFieldType( m_pDoc );
    pNew->m_nNumberingType = m_nNumberingType;
    return pNew;
}

void SwRefPageGetFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    auto const ModifyImpl = [this](SwRootFrame const*const pLayout)
    {
        // first collect all SetPageRefFields
        SetGetExpFields aTmpLst;
        if (MakeSetList(aTmpLst, pLayout))
        {
            SwIterator<SwFormatField,SwFieldType> aIter( *this );
            for ( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
            {
                    // update only the GetRef fields
                    if( pFormatField->GetTextField() )
                        UpdateField(pFormatField->GetTextField(), aTmpLst, pLayout);
            }
        }
    };

    // update all GetReference fields
    if( !pNew && !pOld && HasWriterListeners() )
    {
        SwRootFrame const* pLayout(nullptr);
        SwRootFrame const* pLayoutRLHidden(nullptr);
        for (SwRootFrame const*const pLay : m_pDoc->GetAllLayouts())
        {
            if (pLay->IsHideRedlines())
            {
                pLayoutRLHidden = pLay;
            }
            else
            {
                pLayout = pLay;
            }
        }
        ModifyImpl(pLayout);
        if (pLayoutRLHidden)
        {
            ModifyImpl(pLayoutRLHidden);
        }
    }

    // forward to text fields, they "expand" the text
    NotifyClients( pOld, pNew );
}

bool SwRefPageGetFieldType::MakeSetList(SetGetExpFields& rTmpLst,
        SwRootFrame const*const pLayout)
{
    IDocumentRedlineAccess const& rIDRA(m_pDoc->getIDocumentRedlineAccess());
    SwIterator<SwFormatField,SwFieldType> aIter(*m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::RefPageSet));
    for ( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
    {
        // update only the GetRef fields
        const SwTextField* pTField = pFormatField->GetTextField();
        if( pTField )
        {
            if (!pLayout || !pLayout->IsHideRedlines()
                || !sw::IsFieldDeletedInModel(rIDRA, *pTField))
            {
                const SwTextNode& rTextNd = pTField->GetTextNode();

                // Always the first! (in Tab-Headline, header/footer )
                Point aPt;
                std::pair<Point, bool> const tmp(aPt, false);
                const SwContentFrame *const pFrame = rTextNd.getLayoutFrame(
                    pLayout, nullptr, &tmp);

                std::unique_ptr<SetGetExpField> pNew;

                if( !pFrame ||
                     pFrame->IsInDocBody() ||
                    // #i31868#
                    // Check if pFrame is not yet connected to the layout.
                    !pFrame->FindPageFrame() )
                {
                    //  create index for determination of the TextNode
                    SwNodeIndex aIdx( rTextNd );
                    pNew.reset( new SetGetExpField( aIdx, pTField ) );
                }
                else
                {
                    //  create index for determination of the TextNode
                    SwPosition aPos( m_pDoc->GetNodes().GetEndOfPostIts() );
                    bool const bResult = GetBodyTextNode( *m_pDoc, aPos, *pFrame );
                    OSL_ENSURE(bResult, "where is the Field?");
                    pNew.reset( new SetGetExpField( aPos.nNode, pTField,
                                                &aPos.nContent ) );
                }

                rTmpLst.insert( std::move(pNew) );
            }
        }
    }

    return !rTmpLst.empty();
}

void SwRefPageGetFieldType::UpdateField( SwTextField const * pTextField,
                                        SetGetExpFields const & rSetList,
                                        SwRootFrame const*const pLayout)
{
    SwRefPageGetField* pGetField = const_cast<SwRefPageGetField*>(static_cast<const SwRefPageGetField*>(pTextField->GetFormatField().GetField()));
    pGetField->SetText( OUString(), pLayout );

    // then search the correct RefPageSet field
    SwTextNode* pTextNode = &pTextField->GetTextNode();
    if( pTextNode->StartOfSectionIndex() >
        m_pDoc->GetNodes().GetEndOfExtras().GetIndex() )
    {
        SwNodeIndex aIdx( *pTextNode );
        SetGetExpField aEndField( aIdx, pTextField );

        SetGetExpFields::const_iterator itLast = rSetList.lower_bound( &aEndField );

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
                std::pair<Point, bool> const tmp(aPt, false);
                const SwContentFrame *const pFrame = pTextNode->getLayoutFrame(
                    pLayout, nullptr, &tmp);
                const SwContentFrame *const pRefFrame = pRefTextField->GetTextNode().getLayoutFrame(
                    pLayout, nullptr, &tmp);
                const SwPageFrame* pPgFrame = nullptr;
                short nDiff = 1;
                if ( pFrame && pRefFrame )
                {
                    pPgFrame = pFrame->FindPageFrame();
                    nDiff = pPgFrame->GetPhyPageNum() -
                            pRefFrame->FindPageFrame()->GetPhyPageNum() + 1;
                }

                SvxNumType nTmpFormat = SVX_NUM_PAGEDESC == static_cast<SvxNumType>(pGetField->GetFormat())
                        ? ( !pPgFrame
                                ? SVX_NUM_ARABIC
                                : pPgFrame->GetPageDesc()->GetNumType().GetNumberingType() )
                        : static_cast<SvxNumType>(pGetField->GetFormat());
                const short nPageNum = std::max<short>(0, pSetField->GetOffset() + nDiff);
                pGetField->SetText(FormatNumber(nPageNum, nTmpFormat), pLayout);
            }
        }
    }
    // start formatting
    const_cast<SwFormatField&>(pTextField->GetFormatField()).ModifyNotification( nullptr, nullptr );
}

// queries for relative page numbering

SwRefPageGetField::SwRefPageGetField( SwRefPageGetFieldType* pTyp,
                                    sal_uInt32 nFormat )
    : SwField( pTyp, nFormat )
{
}

void SwRefPageGetField::SetText(const OUString& rText,
        SwRootFrame const*const pLayout)
{
    if (!pLayout || !pLayout->IsHideRedlines())
    {
        m_sText = rText;
    }
    if (!pLayout || pLayout->IsHideRedlines())
    {
        m_sTextRLHidden = rText;
    }
}

OUString SwRefPageGetField::ExpandImpl(SwRootFrame const*const pLayout) const
{
    return pLayout && pLayout->IsHideRedlines() ? m_sTextRLHidden : m_sText;
}

std::unique_ptr<SwField> SwRefPageGetField::Copy() const
{
    std::unique_ptr<SwRefPageGetField> pCpy(new SwRefPageGetField(
                        static_cast<SwRefPageGetFieldType*>(GetTyp()), GetFormat() ));
    pCpy->m_sText = m_sText;
    pCpy->m_sTextRLHidden = m_sTextRLHidden;
    return std::unique_ptr<SwField>(pCpy.release());
}

void SwRefPageGetField::ChangeExpansion(const SwFrame& rFrame,
                                        const SwTextField* pField )
{
    // only fields in Footer, Header, FootNote, Flys
    SwRefPageGetFieldType* pGetType = static_cast<SwRefPageGetFieldType*>(GetTyp());
    SwDoc* pDoc = pGetType->GetDoc();
    if( pField->GetTextNode().StartOfSectionIndex() >
        pDoc->GetNodes().GetEndOfExtras().GetIndex() )
        return;

    SwRootFrame const& rLayout(*rFrame.getRootFrame());
    OUString & rText(rLayout.IsHideRedlines() ? m_sTextRLHidden : m_sText);
    rText.clear();

    OSL_ENSURE(!rFrame.IsInDocBody(), "Flag incorrect, frame is in DocBody");

    // collect all SetPageRefFields
    SetGetExpFields aTmpLst;
    if (!pGetType->MakeSetList(aTmpLst, &rLayout))
        return ;

    //  create index for determination of the TextNode
    SwPosition aPos( SwNodeIndex( pDoc->GetNodes() ) );
    SwTextNode* pTextNode = const_cast<SwTextNode*>(GetBodyTextNode(*pDoc, aPos, rFrame));

    // If no layout exists, ChangeExpansion is called for header and
    // footer lines via layout formatting without existing TextNode.
    if(!pTextNode)
        return;

    SetGetExpField aEndField( aPos.nNode, pField, &aPos.nContent );

    SetGetExpFields::const_iterator itLast = aTmpLst.lower_bound( &aEndField );

    if( itLast == aTmpLst.begin() )
        return;        // there is no corresponding set-field in front
    --itLast;

    const SwTextField* pRefTextField = (*itLast)->GetTextField();
    const SwRefPageSetField* pSetField =
                        static_cast<const SwRefPageSetField*>(pRefTextField->GetFormatField().GetField());
    Point aPt;
    std::pair<Point, bool> const tmp(aPt, false);
    const SwContentFrame *const pRefFrame = pRefTextField->GetTextNode().getLayoutFrame(
            &rLayout, nullptr, &tmp);
    if( pSetField->IsOn() && pRefFrame )
    {
        // determine the correct offset
        const SwPageFrame* pPgFrame = rFrame.FindPageFrame();
        const short nDiff = pPgFrame->GetPhyPageNum() -
                            pRefFrame->FindPageFrame()->GetPhyPageNum() + 1;

        SwRefPageGetField* pGetField = const_cast<SwRefPageGetField*>(static_cast<const SwRefPageGetField*>(pField->GetFormatField().GetField()));
        SvxNumType nTmpFormat = SVX_NUM_PAGEDESC == pGetField->GetFormat()
                            ? pPgFrame->GetPageDesc()->GetNumType().GetNumberingType()
                            : static_cast<SvxNumType>(pGetField->GetFormat());
        const short nPageNum = std::max<short>(0, pSetField->GetOffset() + nDiff);
        pGetField->SetText(FormatNumber(nPageNum, nTmpFormat), &rLayout);
    }
}

bool SwRefPageGetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
        case FIELD_PROP_USHORT1:
            rAny <<= static_cast<sal_Int16>(GetFormat());
        break;
        case FIELD_PROP_PAR1:
            rAny <<= m_sText;
        break;
        default:
            assert(false);
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
        }
        break;
        case FIELD_PROP_PAR1:
            rAny >>= m_sText;
            m_sTextRLHidden = m_sText;
        break;
    default:
        assert(false);
    }
    return true;
}

// field type to jump to and edit

SwJumpEditFieldType::SwJumpEditFieldType( SwDoc* pD )
    : SwFieldType( SwFieldIds::JumpEdit ), m_pDoc( pD ), m_aDep( *this )
{
}

SwFieldType* SwJumpEditFieldType::Copy() const
{
    return new SwJumpEditFieldType( m_pDoc );
}

SwCharFormat* SwJumpEditFieldType::GetCharFormat()
{
    SwCharFormat* pFormat = m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( RES_POOLCHR_JUMPEDIT );
    m_aDep.StartListening(pFormat);
    return pFormat;
}

SwJumpEditField::SwJumpEditField( SwJumpEditFieldType* pTyp, sal_uInt32 nForm,
                                const OUString& rText, const OUString& rHelp )
    : SwField( pTyp, nForm ), m_sText( rText ), m_sHelp( rHelp )
{
}

OUString SwJumpEditField::ExpandImpl(SwRootFrame const*const) const
{
    return "<" + m_sText + ">";
}

std::unique_ptr<SwField> SwJumpEditField::Copy() const
{
    return std::make_unique<SwJumpEditField>( static_cast<SwJumpEditFieldType*>(GetTyp()), GetFormat(),
                                m_sText, m_sHelp );
}

/// get place holder text
OUString SwJumpEditField::GetPar1() const
{
    return m_sText;
}

/// set place holder text
void SwJumpEditField::SetPar1(const OUString& rStr)
{
    m_sText = rStr;
}

/// get hint text
OUString SwJumpEditField::GetPar2() const
{
    return m_sHelp;
}

/// set hint text
void SwJumpEditField::SetPar2(const OUString& rStr)
{
    m_sHelp = rStr;
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
        rAny <<= m_sHelp;
        break;
    case FIELD_PROP_PAR2 :
         rAny <<= m_sText;
         break;
    default:
        assert(false);
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
        rAny >>= m_sHelp;
        break;
    case FIELD_PROP_PAR2 :
         rAny >>= m_sText;
         break;
    default:
        assert(false);
    }
    return true;
}

// combined character field type

SwCombinedCharFieldType::SwCombinedCharFieldType()
    : SwFieldType( SwFieldIds::CombinedChars )
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
    m_sCharacters( rChars.copy( 0, std::min<sal_Int32>(rChars.getLength(), MAX_COMBINED_CHARACTERS) ))
{
}

OUString SwCombinedCharField::ExpandImpl(SwRootFrame const*const) const
{
    return m_sCharacters;
}

std::unique_ptr<SwField> SwCombinedCharField::Copy() const
{
    return std::make_unique<SwCombinedCharField>( static_cast<SwCombinedCharFieldType*>(GetTyp()),
                                        m_sCharacters );
}

OUString SwCombinedCharField::GetPar1() const
{
    return m_sCharacters;
}

void SwCombinedCharField::SetPar1(const OUString& rStr)
{
    m_sCharacters = rStr.copy(0, std::min<sal_Int32>(rStr.getLength(), MAX_COMBINED_CHARACTERS));
}

bool SwCombinedCharField::QueryValue( uno::Any& rAny,
                                        sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_sCharacters;
        break;
    default:
        assert(false);
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
            assert(false);
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
