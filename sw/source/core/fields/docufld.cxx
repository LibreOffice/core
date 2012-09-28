/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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

#include <tools/shl.hxx>
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
#include <rootfrm.hxx>      // AuthorField
#include <pagefrm.hxx>      //
#include <cntfrm.hxx>       //
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
#include <switerator.hxx>

#define URL_DECODE  INetURLObject::DECODE_UNAMBIGUOUS

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace nsSwDocInfoSubType;

/*--------------------------------------------------------------------
    Beschreibung: SwPageNumberFieldType
 --------------------------------------------------------------------*/

SwPageNumberFieldType::SwPageNumberFieldType()
    : SwFieldType( RES_PAGENUMBERFLD ),
    nNumberingType( SVX_NUM_ARABIC ),
    nNum( 0 ),
    nMax( USHRT_MAX ),
    bVirtuell( sal_False )
{
}

String& SwPageNumberFieldType::Expand( sal_uInt32 nFmt, short nOff,
                                const String& rUserStr, String& rRet ) const
{
    sal_uInt32 nTmpFmt = (SVX_NUM_PAGEDESC == nFmt) ? (sal_uInt32)nNumberingType : nFmt;
    long nTmp = nNum + nOff;

    if( 0 >= nTmp || SVX_NUM_NUMBER_NONE == nTmpFmt || (!bVirtuell && nTmp > nMax) )
        rRet = aEmptyStr;
    else if( SVX_NUM_CHAR_SPECIAL == nTmpFmt )
        rRet = rUserStr;
    else
        rRet = FormatNumber( (sal_uInt16)nTmp, nTmpFmt );
    return rRet;
}

SwFieldType* SwPageNumberFieldType::Copy() const
{
    SwPageNumberFieldType *pTmp = new SwPageNumberFieldType();

    pTmp->nNum       = nNum;
    pTmp->nMax       = nMax;
    pTmp->nNumberingType = nNumberingType;
    pTmp->bVirtuell  = bVirtuell;

    return pTmp;
}

/*--------------------------------------------------------------------
    Beschreibung: Verschiedene Expandierung
 --------------------------------------------------------------------*/

void SwPageNumberFieldType::ChangeExpansion( SwDoc* pDoc, sal_uInt16 nPage,
                                            sal_uInt16 nNumPages, sal_Bool bVirt,
                                            const sal_Int16* pNumFmt )
{
    nNum = nPage;
    nMax = nNumPages;
    if( pNumFmt )
        nNumberingType = *pNumFmt;

    bVirtuell = sal_False;
    if( bVirt )
    {
        // dann muss das Flag ueberprueft werden, denn das Layout setzt
        // es NIE zurueck
        const SfxItemPool &rPool = pDoc->GetAttrPool();
        const SwFmtPageDesc *pDesc;
        sal_uInt32 nMaxItems = rPool.GetItemCount2( RES_PAGEDESC );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pDesc = (SwFmtPageDesc*)rPool.GetItem2( RES_PAGEDESC, n ) )
                && pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
            {
                SwCntntNode* pNd = PTR_CAST( SwCntntNode, pDesc->GetDefinedIn() );
                if( pNd )
                {
                    if ( SwIterator<SwFrm,SwCntntNode>::FirstElement(*pNd) )
                        bVirtuell = sal_True;
                }
                else if( pDesc->GetDefinedIn()->ISA( SwFmt ))
                {
                    SwAutoFmtGetDocNode aGetHt( &pDoc->GetNodes() );
                    bVirtuell = !pDesc->GetDefinedIn()->GetInfo( aGetHt );
                    break;
                }
            }
    }
}

/*--------------------------------------------------------------------
    Beschreibung: SwPageNumberField
 --------------------------------------------------------------------*/

SwPageNumberField::SwPageNumberField(SwPageNumberFieldType* pTyp,
                                     sal_uInt16 nSub, sal_uInt32 nFmt, short nOff)
    : SwField(pTyp, nFmt), nSubType(nSub), nOffset(nOff)
{
}

String SwPageNumberField::Expand() const
{
    String sRet;
    SwPageNumberFieldType* pFldType = (SwPageNumberFieldType*)GetTyp();

    if( PG_NEXT == nSubType && 1 != nOffset )
    {
        if( pFldType->Expand( GetFormat(), 1, sUserStr, sRet ).Len() )
            pFldType->Expand( GetFormat(), nOffset, sUserStr, sRet );
    }
    else if( PG_PREV == nSubType && -1 != nOffset )
    {
        if( pFldType->Expand( GetFormat(), -1, sUserStr, sRet ).Len() )
            pFldType->Expand( GetFormat(), nOffset, sUserStr, sRet );
    }
    else
        pFldType->Expand( GetFormat(), nOffset, sUserStr, sRet );
    return sRet;
}

SwField* SwPageNumberField::Copy() const
{
    SwPageNumberField *pTmp =
        new SwPageNumberField((SwPageNumberFieldType*)GetTyp(), nSubType, GetFormat(), nOffset);
    pTmp->SetLanguage( GetLanguage() );
    pTmp->SetUserString( sUserStr );
    return pTmp;
}

rtl::OUString SwPageNumberField::GetPar2() const
{
    return rtl::OUString::valueOf(static_cast<sal_Int32>(nOffset));
}

void SwPageNumberField::SetPar2(const rtl::OUString& rStr)
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
            rAny.setValue(&eType, ::getCppuType((const text::PageNumberType*)0));
        }
        break;
    case FIELD_PROP_PAR1:
        rAny <<= OUString(sUserStr);
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
        ::GetString( rAny, sUserStr );
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return bRet;
}
/*--------------------------------------------------------------------
    Beschreibung: SwAuthorFieldType
 --------------------------------------------------------------------*/

SwAuthorFieldType::SwAuthorFieldType()
    : SwFieldType( RES_AUTHORFLD )
{
}

String SwAuthorFieldType::Expand(sal_uLong nFmt) const
{
    String sRet;
    SvtUserOptions&  rOpt = SW_MOD()->GetUserOptions();
    if((nFmt & 0xff) == AF_NAME)
        sRet = rOpt.GetFullName();
    else
        sRet = rOpt.GetID();
    return sRet;
}

SwFieldType* SwAuthorFieldType::Copy() const
{
    return new SwAuthorFieldType;
}

/*--------------------------------------------------------------------
    Beschreibung: SwAuthorField
 --------------------------------------------------------------------*/

SwAuthorField::SwAuthorField(SwAuthorFieldType* pTyp, sal_uInt32 nFmt)
    : SwField(pTyp, nFmt)
{
    aContent = ((SwAuthorFieldType*)GetTyp())->Expand(GetFormat());
}

String SwAuthorField::Expand() const
{
    if (!IsFixed())
        ((SwAuthorField*)this)->aContent =
                    ((SwAuthorFieldType*)GetTyp())->Expand(GetFormat());

    return aContent;
}

SwField* SwAuthorField::Copy() const
{
    SwAuthorField *pTmp = new SwAuthorField( (SwAuthorFieldType*)GetTyp(),
                                                GetFormat());
    pTmp->SetExpansion(aContent);
    return pTmp;
}

bool SwAuthorField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bVal;
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        bVal = (GetFormat() & 0xff) == AF_NAME;
        rAny.setValue(&bVal, ::getBooleanCppuType());
        break;

    case FIELD_PROP_BOOL2:
        bVal = IsFixed();
        rAny.setValue(&bVal, ::getBooleanCppuType());
        break;

    case FIELD_PROP_PAR1:
        rAny <<= rtl::OUString(GetContent());
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
        SetFormat( *(sal_Bool*)rAny.getValue() ? AF_NAME : AF_SHORTCUT );
        break;

    case FIELD_PROP_BOOL2:
        if( *(sal_Bool*)rAny.getValue() )
            SetFormat( GetFormat() | AF_FIXED);
        else
            SetFormat( GetFormat() & ~AF_FIXED);
        break;

    case FIELD_PROP_PAR1:
        ::GetString( rAny, aContent );
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/*--------------------------------------------------------------------
    Beschreibung: SwFileNameFieldType
 --------------------------------------------------------------------*/

SwFileNameFieldType::SwFileNameFieldType(SwDoc *pDocument)
    : SwFieldType( RES_FILENAMEFLD )
{
    pDoc = pDocument;
}

String SwFileNameFieldType::Expand(sal_uLong nFmt) const
{
    String aRet;
    const SwDocShell* pDShell = pDoc->GetDocShell();
    if( pDShell && pDShell->HasName() )
    {
        const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
        switch( nFmt & ~FF_FIXED )
        {
            case FF_PATH:
                {
                    if( INET_PROT_FILE == rURLObj.GetProtocol() )
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
                        aRet.Erase( aRet.Search( String(rURLObj.GetLastName(
                                                    URL_DECODE )) ) );
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
                if( INET_PROT_FILE == rURLObj.GetProtocol() )
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
/*--------------------------------------------------------------------
    Beschreibung: SwFileNameField
 --------------------------------------------------------------------*/

SwFileNameField::SwFileNameField(SwFileNameFieldType* pTyp, sal_uInt32 nFmt)
    : SwField(pTyp, nFmt)
{
    aContent = ((SwFileNameFieldType*)GetTyp())->Expand(GetFormat());
}

String SwFileNameField::Expand() const
{
    if (!IsFixed())
        ((SwFileNameField*)this)->aContent = ((SwFileNameFieldType*)GetTyp())->Expand(GetFormat());

    return aContent;
}

SwField* SwFileNameField::Copy() const
{
    SwFileNameField *pTmp =
        new SwFileNameField((SwFileNameFieldType*)GetTyp(), GetFormat());
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
        {
            sal_Bool bVal = IsFixed();
            rAny.setValue(&bVal, ::getBooleanCppuType());
        }
        break;

    case FIELD_PROP_PAR3:
        rAny <<= OUString(GetContent());
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
            sal_Bool bFixed = IsFixed();
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
        if( *(sal_Bool*)rAny.getValue() )
            SetFormat( GetFormat() | FF_FIXED);
        else
            SetFormat( GetFormat() & ~FF_FIXED);
        break;

    case FIELD_PROP_PAR3:
        ::GetString( rAny, aContent );
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}
/*--------------------------------------------------------------------
    Beschreibung: SwTemplNameFieldType
 --------------------------------------------------------------------*/

SwTemplNameFieldType::SwTemplNameFieldType(SwDoc *pDocument)
    : SwFieldType( RES_TEMPLNAMEFLD )
{
    pDoc = pDocument;
}

String SwTemplNameFieldType::Expand(sal_uLong nFmt) const
{
    OSL_ENSURE( nFmt < FF_END, "Expand: kein guelt. Fmt!" );

    String aRet;
    SwDocShell *pDocShell(pDoc->GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps(
            xDPS->getDocumentProperties());
        OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");

        if( FF_UI_NAME == nFmt )
            aRet = xDocProps->getTemplateName();
        else if( !xDocProps->getTemplateURL().isEmpty() )
        {
            if( FF_UI_RANGE == nFmt )
            {
                // fuers besorgen vom RegionNamen !!
                SfxDocumentTemplates aFac;
                aFac.Construct();
                String sTmp;
                aFac.GetLogicNames( xDocProps->getTemplateURL(), aRet, sTmp );
            }
            else
            {
                INetURLObject aPathName( xDocProps->getTemplateURL() );
                if( FF_NAME == nFmt )
                    aRet = aPathName.GetName(URL_DECODE);
                else if( FF_NAME_NOEXT == nFmt )
                    aRet = aPathName.GetBase();
                else
                {
                    if( FF_PATH == nFmt )
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
/*--------------------------------------------------------------------
    Beschreibung: SwTemplNameField
 --------------------------------------------------------------------*/

SwTemplNameField::SwTemplNameField(SwTemplNameFieldType* pTyp, sal_uInt32 nFmt)
    : SwField(pTyp, nFmt)
{}

String SwTemplNameField::Expand() const
{
    return((SwTemplNameFieldType*)GetTyp())->Expand(GetFormat());
}

SwField* SwTemplNameField::Copy() const
{
    SwTemplNameField *pTmp =
        new SwTemplNameField((SwTemplNameFieldType*)GetTyp(), GetFormat());
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
/*--------------------------------------------------------------------
    Beschreibung: SwDocStatFieldType
 --------------------------------------------------------------------*/

SwDocStatFieldType::SwDocStatFieldType(SwDoc* pDocument)
    : SwFieldType( RES_DOCSTATFLD ), nNumberingType( SVX_NUM_ARABIC )
{
    pDoc = pDocument;
}

String SwDocStatFieldType::Expand(sal_uInt16 nSubType, sal_uInt32 nFmt) const
{
    sal_uInt32 nVal = 0;
    const SwDocStat& rDStat = pDoc->GetDocStat();
    switch( nSubType )
    {
        case DS_TBL:  nVal = rDStat.nTbl;   break;
        case DS_GRF:  nVal = rDStat.nGrf;   break;
        case DS_OLE:  nVal = rDStat.nOLE;   break;
        case DS_PARA: nVal = rDStat.nPara;  break;
        case DS_WORD: nVal = rDStat.nWord;  break;
        case DS_CHAR: nVal = rDStat.nChar;  break;
        case DS_PAGE:
            if( pDoc->GetCurrentLayout() )//swmod 080218
                ((SwDocStat &)rDStat).nPage = pDoc->GetCurrentLayout()->GetPageNum();   //swmod 080218
            nVal = rDStat.nPage;
            if( SVX_NUM_PAGEDESC == nFmt )
                nFmt = (sal_uInt32)nNumberingType;
            break;
        default:
            OSL_FAIL( "SwDocStatFieldType::Expand: unbekannter SubType" );
    }

    String sRet;
    if( nVal <= SHRT_MAX )
        sRet = FormatNumber( (sal_uInt16)nVal, nFmt );
    else
        sRet = String::CreateFromInt32( nVal );
    return sRet;
}

SwFieldType* SwDocStatFieldType::Copy() const
{
    SwDocStatFieldType *pTmp = new SwDocStatFieldType(pDoc);
    return pTmp;
}

/*--------------------------------------------------------------------
    Beschreibung: SwDocStatFieldType
                  Aus historischen Gruenden steht in nFormat der
                  SubType
 --------------------------------------------------------------------*/

SwDocStatField::SwDocStatField(SwDocStatFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFmt)
    : SwField(pTyp, nFmt),
    nSubType(nSub)
{}

String SwDocStatField::Expand() const
{
    return((SwDocStatFieldType*)GetTyp())->Expand(nSubType, GetFormat());
}

SwField* SwDocStatField::Copy() const
{
    SwDocStatField *pTmp = new SwDocStatField(
                    (SwDocStatFieldType*)GetTyp(), nSubType, GetFormat() );
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
        ((SwDocStatFieldType*)GetTyp())->SetNumFormat(
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

/*--------------------------------------------------------------------
    Beschreibung: DokumentinfoFields
 --------------------------------------------------------------------*/

SwDocInfoFieldType::SwDocInfoFieldType(SwDoc* pDc)
    : SwValueFieldType( pDc, RES_DOCINFOFLD )
{
}

SwFieldType* SwDocInfoFieldType::Copy() const
{
    SwDocInfoFieldType* pTyp = new SwDocInfoFieldType(GetDoc());
    return pTyp;
}

void lcl_GetLocalDataWrapper( sal_uLong nLang,
                              const LocaleDataWrapper **ppAppLocalData,
                              const LocaleDataWrapper **ppLocalData )
{
    SvtSysLocale aLocale;
    *ppAppLocalData = &aLocale.GetLocaleData();
    *ppLocalData = *ppAppLocalData;
    if( nLang != SvxLocaleToLanguage( (*ppLocalData)->getLocale() ) )
        *ppLocalData = new LocaleDataWrapper(
                        ::comphelper::getProcessServiceFactory(),
                        SvxCreateLocale( static_cast<LanguageType>(nLang) ) );
}

String SwDocInfoFieldType::Expand( sal_uInt16 nSub, sal_uInt32 nFormat,
                                    sal_uInt16 nLang, const String& rName ) const
{
    String aStr;
    const LocaleDataWrapper *pAppLocalData = 0, *pLocalData = 0;
    SwDocShell *pDocShell(GetDoc()->GetDocShell());
    OSL_ENSURE(pDocShell, "no SwDocShell");
    if (!pDocShell) { return aStr; }

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    OSL_ENSURE(xDocProps.is(), "Doc has no DocumentProperties");

    sal_uInt16 nExtSub = nSub & 0xff00;
    nSub &= 0xff;   // ExtendedSubTypes nicht beachten

    switch(nSub)
    {
    case DI_TITEL:  aStr = xDocProps->getTitle();       break;
    case DI_THEMA:  aStr = xDocProps->getSubject();     break;
    case DI_KEYS:   aStr = ::comphelper::string::convertCommaSeparated(
                                xDocProps->getKeywords());
                    break;
    case DI_COMMENT:aStr = xDocProps->getDescription(); break;
    case DI_DOCNO:  aStr = String::CreateFromInt32(
                                        xDocProps->getEditingCycles() );
                    break;
    case DI_EDIT:
        if ( !nFormat )
        {
            lcl_GetLocalDataWrapper( nLang, &pAppLocalData, &pLocalData );
            sal_Int32 dur = xDocProps->getEditingDuration();
            aStr = pLocalData->getTime( Time(dur/3600, (dur%3600)/60, dur%60),
                                        sal_False, sal_False);
        }
        else
        {
            sal_Int32 dur = xDocProps->getEditingDuration();
            double fVal = Time(dur/3600, (dur%3600)/60, dur%60).GetTimeInDays();
            aStr = ExpandValue(fVal, nFormat, nLang);
        }
        break;
    case DI_CUSTOM:
        {
            ::rtl::OUString sVal;
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
            String aName( xDocProps->getAuthor() );
            util::DateTime uDT( xDocProps->getCreationDate() );
            Date aD(uDT.Day, uDT.Month, uDT.Year);
            Time aT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
            DateTime aDate(aD,aT);
            if( nSub == DI_CREATE )
                ;       // das wars schon!!
            else if( nSub == DI_CHANGE )
            {
                aName = xDocProps->getModifiedBy();
                uDT = xDocProps->getModificationDate();
                Date bD(uDT.Day, uDT.Month, uDT.Year);
                Time bT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
                DateTime bDate(bD,bT);
                aDate = bDate;
            }
            else if( nSub == DI_PRINT )
            {
                aName = xDocProps->getPrintedBy();
                uDT = xDocProps->getPrintDate();
                Date bD(uDT.Day, uDT.Month, uDT.Year);
                Time bT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
                DateTime bDate(bD,bT);
                aDate = bDate;
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
                                                    sal_False, sal_False);
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

SwDocInfoField::SwDocInfoField(SwDocInfoFieldType* pTyp, sal_uInt16 nSub, const String& rName, sal_uInt32 nFmt) :
    SwValueField(pTyp, nFmt), nSubType(nSub)
{
    aName = rName;
    aContent = ((SwDocInfoFieldType*)GetTyp())->Expand(nSubType, nFmt, GetLanguage(), aName);
}

SwDocInfoField::SwDocInfoField(SwDocInfoFieldType* pTyp, sal_uInt16 nSub, const String& rName, const String& rValue, sal_uInt32 nFmt) :
    SwValueField(pTyp, nFmt), nSubType(nSub)
{
    aName = rName;
    aContent = rValue;
}


template<class T>
double lcl_TimeToDouble( const T& rTime )
{
    const double fMilliSecondsPerDay = 86400000.0;
    return ((rTime.Hours*3600000)+(rTime.Minutes*60000)+(rTime.Seconds*1000)+(rTime.HundredthSeconds*10)) / fMilliSecondsPerDay;
}

template<class D>
double lcl_DateToDouble( const D& rDate, const Date& rNullDate )
{
    long nDate = Date::DateToDays( rDate.Day, rDate.Month, rDate.Year );
    long nNullDate = Date::DateToDays( rNullDate.GetDay(), rNullDate.GetMonth(), rNullDate.GetYear() );
    return double( nDate - nNullDate );
}

String SwDocInfoField::Expand() const
{
    if ( ( nSubType & 0xFF ) == DI_CUSTOM )
    {
         // custom properties currently need special treatment
         // we don't have a secure way to detect "real" custom properties in Word  Import of text fields
        // so we treat *every* unknown property as a custom property, even the "built-in" section in Word's document summary information stream
        // as these properties have not been inserted when the document summary information was imported, we do it here
        // this approach is still a lot better than the old one to import such fields as "user fields" and simple text
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
            if ( aAny.getValueType() != ::getVoidCppuType() )
            {
                // "void" type means that the property has not been inserted until now
                if ( !IsFixed() )
                {
                    // if the field is "fixed" we don't update it from the property
                    ::rtl::OUString sVal;
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
                        String sText = aDuration.Negative ? rtl::OUString('-') : rtl::OUString('+');
                        sText += ViewShell::GetShellRes()->sDurationFormat;
                        sText.SearchAndReplace(rtl::OUString("%1"), String::CreateFromInt32( aDuration.Years ) );
                        sText.SearchAndReplace(rtl::OUString("%2"), String::CreateFromInt32( aDuration.Months ) );
                        sText.SearchAndReplace(rtl::OUString("%3"), String::CreateFromInt32( aDuration.Days   ) );
                        sText.SearchAndReplace(rtl::OUString("%4"), String::CreateFromInt32( aDuration.Hours  ) );
                        sText.SearchAndReplace(rtl::OUString("%5"), String::CreateFromInt32( aDuration.Minutes) );
                        sText.SearchAndReplace(rtl::OUString("%6"), String::CreateFromInt32( aDuration.Seconds) );
                        sVal = sText;
                    }
                    else
                    {
                        uno::Any aNew = xConverter->convertToSimpleType( aAny, uno::TypeClass_STRING );
                        aNew >>= sVal;
                    }
                    ((SwDocInfoField*)this)->aContent = sVal;
                }
            }
        }
        catch (uno::Exception&) {}
    }
    else if ( !IsFixed() )
        ((SwDocInfoField*)this)->aContent = ((SwDocInfoFieldType*)GetTyp())->Expand(nSubType, GetFormat(), GetLanguage(), aName);

    return aContent;
}

String SwDocInfoField::GetFieldName() const
{
    String aStr(SwFieldType::GetTypeStr(GetTypeId()));
    aStr += ':';

    sal_uInt16 const nSub = nSubType & 0xff;

    switch (nSub)
    {
        case DI_CUSTOM:
            aStr += aName;
            break;

        default:
            aStr += ViewShell::GetShellRes()
                     ->aDocInfoLst[ nSub - DI_SUBTYPE_BEGIN ];
            break;
    }
    if (IsFixed())
    {
        aStr += ' ';
        aStr += ViewShell::GetShellRes()->aFixedStr;
    }
    return aStr;
}

SwField* SwDocInfoField::Copy() const
{
    SwDocInfoField* pFld = new SwDocInfoField((SwDocInfoFieldType*)GetTyp(), nSubType, aName, GetFormat());
    pFld->SetAutomaticLanguage(IsAutomaticLanguage());
    pFld->aContent = aContent;

    return pFld;
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
        rAny <<= OUString(aContent);
        break;

    case FIELD_PROP_PAR4:
        rAny <<= OUString(aName);
        break;

    case FIELD_PROP_USHORT1:
        rAny  <<= (sal_Int16)aContent.ToInt32();
        break;

    case FIELD_PROP_BOOL1:
        {
            sal_Bool bVal = 0 != (nSubType & DI_SUB_FIXED);
            rAny.setValue(&bVal, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_FORMAT:
        rAny  <<= (sal_Int32)GetFormat();
        break;

    case FIELD_PROP_DOUBLE:
        {
            double fVal = GetValue();
            rAny.setValue(&fVal, ::getCppuType(&fVal));
        }
        break;
    case FIELD_PROP_PAR3:
        rAny <<= rtl::OUString(Expand());
        break;
    case FIELD_PROP_BOOL2:
        {
            sal_uInt16 nExtSub = (nSubType & 0xff00) & ~DI_SUB_FIXED;
            sal_Bool bVal = (nExtSub == DI_SUB_DATE);
            rAny.setValue(&bVal, ::getBooleanCppuType());
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
            ::GetString( rAny, aContent );
        break;

    case FIELD_PROP_USHORT1:
        if( nSubType & DI_SUB_FIXED )
        {
            rAny >>= nValue;
            aContent = String::CreateFromInt32(nValue);
        }
        break;

    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rAny.getValue())
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
        ::GetString( rAny, aContent );
        break;
    case FIELD_PROP_BOOL2:
        nSubType &= 0xf0ff;
        if(*(sal_Bool*)rAny.getValue())
            nSubType |= DI_SUB_DATE;
        else
            nSubType |= DI_SUB_TIME;
        break;
    default:
        return SwField::PutValue(rAny, nWhichId);
    }
    return true;
}

/*--------------------------------------------------------------------
    Beschreibung: SwHiddenTxtFieldType by JP
 --------------------------------------------------------------------*/

SwHiddenTxtFieldType::SwHiddenTxtFieldType( sal_Bool bSetHidden )
    : SwFieldType( RES_HIDDENTXTFLD ), bHidden( bSetHidden )
{
}

SwFieldType* SwHiddenTxtFieldType::Copy() const
{
    return new SwHiddenTxtFieldType( bHidden );
}

void SwHiddenTxtFieldType::SetHiddenFlag( sal_Bool bSetHidden )
{
    if( bHidden != bSetHidden )
    {
        bHidden = bSetHidden;
        UpdateFlds();       // alle HiddenText benachrichtigen
    }
}

SwHiddenTxtField::SwHiddenTxtField( SwHiddenTxtFieldType* pFldType,
                                    sal_Bool    bConditional,
                                    const   String& rCond,
                                    const   String& rStr,
                                    sal_Bool    bHidden,
                                    sal_uInt16  nSub) :
    SwField( pFldType ), aCond(rCond), nSubType(nSub),
    bCanToggle(bConditional), bIsHidden(bHidden), bValid(sal_False)
{
    if(nSubType == TYP_CONDTXTFLD)
    {
        sal_uInt16 nPos = 0;
        aTRUETxt = rStr.GetToken(0, '|', nPos);

        if(nPos != STRING_NOTFOUND)
        {
            aFALSETxt = rStr.GetToken(0, '|', nPos);
            if(nPos != STRING_NOTFOUND)
            {
                aContent = rStr.GetToken(0, '|', nPos);
                bValid = sal_True;
            }
        }
    }
    else
        aTRUETxt = rStr;
}

SwHiddenTxtField::SwHiddenTxtField( SwHiddenTxtFieldType* pFldType,
                                    const String& rCond,
                                    const String& rTrue,
                                    const String& rFalse,
                                    sal_uInt16 nSub)
    : SwField( pFldType ), aTRUETxt(rTrue), aFALSETxt(rFalse), aCond(rCond), nSubType(nSub),
      bIsHidden(sal_True), bValid(sal_False)
{
    bCanToggle  = aCond.getLength() > 0;
}

String SwHiddenTxtField::Expand() const
{
    // Type: !Hidden  -> immer anzeigen
    //        Hide    -> Werte die Bedingung aus

    if( TYP_CONDTXTFLD == nSubType )
    {
        if( bValid )
            return aContent;

        if( bCanToggle && !bIsHidden )
            return aTRUETxt;
    }
    else if( !((SwHiddenTxtFieldType*)GetTyp())->GetHiddenFlag() ||
        ( bCanToggle && bIsHidden ))
        return aTRUETxt;

    return aFALSETxt;
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuellen Field-Value holen und cachen
 --------------------------------------------------------------------*/

void SwHiddenTxtField::Evaluate(SwDoc* pDoc)
{
    OSL_ENSURE(pDoc, "Wo ist das Dokument Seniore");

    if( TYP_CONDTXTFLD == nSubType )
    {
        SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();

        bValid = sal_False;
        String sTmpName;

        if (bCanToggle && !bIsHidden)
            sTmpName = aTRUETxt;
        else
            sTmpName = aFALSETxt;

// Datenbankausdruecke muessen sich von
//              einfachem Text unterscheiden. also wird der einfache Text
//              bevorzugt in Anfuehrungszeichen gesetzt.
//              Sind diese vorhanden werden umschliessende entfernt.
//              Wenn nicht, dann wird auf die Tauglichkeit als Datenbankname
//              geprueft. Nur wenn zwei oder mehr Punkte vorhanden sind und kein
//              Anfuehrungszeichen enthalten ist, gehen wir von einer DB aus.
        if(sTmpName.Len() > 1 && sTmpName.GetChar(0) == '\"' &&
            sTmpName.GetChar((sTmpName.Len() - 1))== '\"')
        {
            aContent = sTmpName.Copy(1, sTmpName.Len() - 2);
            bValid = sal_True;
        }
        else if(sTmpName.Search('\"') == STRING_NOTFOUND &&
            comphelper::string::getTokenCount(sTmpName, '.') > 2)
        {
            ::ReplacePoint(sTmpName);
            if(sTmpName.GetChar(0) == '[' && sTmpName.GetChar(sTmpName.Len()-1) == ']')
            {   // Eckige Klammern entfernen
                sTmpName.Erase(0, 1);
                sTmpName.Erase(sTmpName.Len()-1, 1);
            }

            if( pMgr)
            {
                String sDBName( GetDBName( sTmpName, pDoc ));
                String sDataSource(sDBName.GetToken(0, DB_DELIM));
                String sDataTableOrQuery(sDBName.GetToken(1, DB_DELIM));
                if( pMgr->IsInMerge() && sDBName.Len() &&
                    pMgr->IsDataSourceOpen( sDataSource,
                                                sDataTableOrQuery, sal_False))
                {
                    double fNumber;
                    sal_uInt32 nTmpFormat;
                    pMgr->GetMergeColumnCnt(GetColumnName( sTmpName ),
                        GetLanguage(), aContent, &fNumber, &nTmpFormat );
                    bValid = sal_True;
                }
                else if( sDBName.Len() && sDataSource.Len() &&
                         sDataTableOrQuery.Len() )
                    bValid = sal_True;
            }
        }
    }
}

String SwHiddenTxtField::GetFieldName() const
{
    String aStr(SwFieldType::GetTypeStr(nSubType));
    aStr += ' ';
    aStr += aCond;
    aStr += ' ';
    aStr += aTRUETxt;

    if (nSubType == TYP_CONDTXTFLD)
    {
        aStr.AppendAscii(" : ");
        aStr += aFALSETxt;
    }
    return aStr;
}

SwField* SwHiddenTxtField::Copy() const
{
    SwHiddenTxtField* pFld =
        new SwHiddenTxtField((SwHiddenTxtFieldType*)GetTyp(), aCond,
                              aTRUETxt, aFALSETxt);
    pFld->bIsHidden = bIsHidden;
    pFld->bValid    = bValid;
    pFld->aContent  = aContent;
    pFld->SetFormat(GetFormat());
    pFld->nSubType  = nSubType;
    return pFld;
}


/*--------------------------------------------------------------------
    Beschreibung: Bedingung setzen
 --------------------------------------------------------------------*/

void SwHiddenTxtField::SetPar1(const rtl::OUString& rStr)
{
    aCond = rStr;
    bCanToggle = aCond.getLength() > 0;
}

const rtl::OUString& SwHiddenTxtField::GetPar1() const
{
    return aCond;
}

/*--------------------------------------------------------------------
    Beschreibung: True/False Text
 --------------------------------------------------------------------*/

void SwHiddenTxtField::SetPar2(const rtl::OUString& rStr)
{
    if (nSubType == TYP_CONDTXTFLD)
    {
        sal_Int32 nPos = rStr.indexOf('|');
        if (nPos == STRING_NOTFOUND)
            aTRUETxt = rStr;
        else
        {
            aTRUETxt = rStr.copy(0, nPos);
            aFALSETxt = rStr.copy(nPos + 1);
        }
    }
    else
        aTRUETxt = rStr;
}

rtl::OUString SwHiddenTxtField::GetPar2() const
{
    String aRet(aTRUETxt);
    if(nSubType == TYP_CONDTXTFLD)
    {
        aRet += '|';
        aRet += aFALSETxt;
    }
    return aRet;
}

sal_uInt16 SwHiddenTxtField::GetSubType() const
{
    return nSubType;
}

bool SwHiddenTxtField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    const rtl::OUString* pOut = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        pOut = &aCond;
        break;
    case FIELD_PROP_PAR2:
        pOut = &aTRUETxt;
        break;
    case FIELD_PROP_PAR3:
        pOut = &aFALSETxt;
        break;
    case FIELD_PROP_PAR4 :
        pOut = &aContent;
    break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bHidden = bIsHidden;
            rAny.setValue(&bHidden, ::getBooleanCppuType());
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    if( pOut )
        rAny <<= *pOut;
    return true;
}

bool SwHiddenTxtField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        {
            rtl::OUString sVal;
            rAny >>= sVal;
            SetPar1(sVal);
        }
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aTRUETxt;
        break;
    case FIELD_PROP_PAR3:
        rAny >>= aFALSETxt;
        break;
    case FIELD_PROP_BOOL1:
        bIsHidden = *(sal_Bool*)rAny.getValue();
        break;
    case FIELD_PROP_PAR4:
        rAny >>= aContent;
        bValid = sal_True;
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

//------------------------------------------------------------------------------

String SwHiddenTxtField::GetColumnName(const String& rName)
{
    sal_uInt16 nPos = rName.Search(DB_DELIM);
    if( STRING_NOTFOUND != nPos )
    {
        nPos = rName.Search(DB_DELIM, nPos + 1);

        if( STRING_NOTFOUND != nPos )
            return rName.Copy(nPos + 1);
    }
    return rName;
}

//------------------------------------------------------------------------------

String SwHiddenTxtField::GetDBName(const String& rName, SwDoc *pDoc)
{
    sal_uInt16 nPos = rName.Search(DB_DELIM);
    if( STRING_NOTFOUND != nPos )
    {
        nPos = rName.Search(DB_DELIM, nPos + 1);

        if( STRING_NOTFOUND != nPos )
            return rName.Copy( 0, nPos );
    }
    SwDBData aData = pDoc->GetDBData();
    String sRet = aData.sDataSource;
    sRet += DB_DELIM;
    sRet += String(aData.sCommand);
    return sRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Der Feldtyp fuer Zeilenhoehe 0
 --------------------------------------------------------------------*/

SwHiddenParaFieldType::SwHiddenParaFieldType()
    : SwFieldType( RES_HIDDENPARAFLD )
{
}

SwFieldType* SwHiddenParaFieldType::Copy() const
{
    SwHiddenParaFieldType* pTyp = new SwHiddenParaFieldType();
    return pTyp;
}

/*--------------------------------------------------------------------
    Beschreibung: Das Feld Zeilenhoehe 0
 --------------------------------------------------------------------*/

SwHiddenParaField::SwHiddenParaField(SwHiddenParaFieldType* pTyp, const String& rStr)
    : SwField(pTyp), aCond(rStr)
{
    bIsHidden = sal_False;
}

String SwHiddenParaField::Expand() const
{
    return aEmptyStr;
}

SwField* SwHiddenParaField::Copy() const
{
    SwHiddenParaField* pFld = new SwHiddenParaField((SwHiddenParaFieldType*)GetTyp(), aCond);
    pFld->bIsHidden = bIsHidden;

    return pFld;
}

bool SwHiddenParaField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch ( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= OUString(aCond);
        break;
    case  FIELD_PROP_BOOL1:
        {
            sal_Bool bHidden = bIsHidden;
            rAny.setValue(&bHidden, ::getBooleanCppuType());
        }
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
        bIsHidden = *(sal_Bool*)rAny.getValue();
        break;

    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/*--------------------------------------------------------------------
    Beschreibung: Bedingung setzen
 --------------------------------------------------------------------*/

void SwHiddenParaField::SetPar1(const rtl::OUString& rStr)
{
    aCond = rStr;
}

const rtl::OUString& SwHiddenParaField::GetPar1() const
{
    return aCond;
}

/*--------------------------------------------------------------------
    Beschreibung: PostIt
 --------------------------------------------------------------------*/

SwPostItFieldType::SwPostItFieldType(SwDoc *pDoc)
    : SwFieldType( RES_POSTITFLD ),mpDoc(pDoc)
{}

SwFieldType* SwPostItFieldType::Copy() const
{
    return new SwPostItFieldType(mpDoc);
}


/*--------------------------------------------------------------------
    Beschreibung: SwPostItFieldType
 --------------------------------------------------------------------*/

SwPostItField::SwPostItField( SwPostItFieldType* pT,
        const String& rAuthor, const String& rTxt, const String& rInitials, const String& rName, const DateTime& rDateTime )
    : SwField( pT ), sTxt( rTxt ), sAuthor( rAuthor ), sInitials( rInitials ), sName( rName ), aDateTime( rDateTime ), mpText(0), m_pTextObject(0)
{
}


SwPostItField::~SwPostItField()
{
    if ( m_pTextObject )
    {
        m_pTextObject->DisposeEditSource();
        m_pTextObject->release();
    }
}


String SwPostItField::Expand() const
{
    return aEmptyStr;
}


String SwPostItField::GetDescription() const
{
    return SW_RES(STR_NOTE);
}


SwField* SwPostItField::Copy() const
{
    SwPostItField* pRet = new SwPostItField( (SwPostItFieldType*)GetTyp(), sAuthor, sTxt, sInitials, sName,
                                aDateTime);
    if (mpText)
        pRet->SetTextObject( new OutlinerParaObject(*mpText) );
    return pRet;
}
/*--------------------------------------------------------------------
    Beschreibung: Author setzen
 --------------------------------------------------------------------*/

void SwPostItField::SetPar1(const rtl::OUString& rStr)
{
    sAuthor = rStr;
}

const rtl::OUString& SwPostItField::GetPar1() const
{
    return sAuthor;
}

/*--------------------------------------------------------------------
    Beschreibung: Text fuers PostIt setzen
 --------------------------------------------------------------------*/

void SwPostItField::SetPar2(const rtl::OUString& rStr)
{
    sTxt = rStr;
}

rtl::OUString SwPostItField::GetPar2() const
{
        return sTxt;
}

const rtl::OUString& SwPostItField::GetInitials() const
{
    return sInitials;
}

void SwPostItField::SetName(const rtl::OUString& rName)
{
    sName = rName;
}

const rtl::OUString& SwPostItField::GetName() const
{
    return sName;
}

const OutlinerParaObject* SwPostItField::GetTextObject() const
{
    return mpText;
}

void SwPostItField::SetTextObject( OutlinerParaObject* pText )
{
    delete mpText;
    mpText = pText;
}

sal_uInt32 SwPostItField::GetNumberOfParagraphs() const
{
    return (mpText) ? mpText->Count() : 1;
}

bool SwPostItField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= OUString(sAuthor);
        break;
    case FIELD_PROP_PAR2:
        {
        rAny <<= OUString(sTxt);
        break;
        }
    case FIELD_PROP_PAR3:
        rAny <<= OUString(sInitials);
        break;
    case FIELD_PROP_PAR4:
        rAny <<= OUString(sName);
        break;
    case FIELD_PROP_TEXT:
        {
            if ( !m_pTextObject )
            {
                SwPostItFieldType* pGetType = (SwPostItFieldType*)GetTyp();
                SwDoc* pDoc = pGetType->GetDoc();
                SwTextAPIEditSource* pObj = new SwTextAPIEditSource( pDoc );
                const_cast <SwPostItField*> (this)->m_pTextObject = new SwTextAPIObject( pObj );
                m_pTextObject->acquire();
            }

            if ( mpText )
                m_pTextObject->SetText( *mpText );
            else
                m_pTextObject->SetString( sTxt );

            uno::Reference < text::XText > xText( m_pTextObject );
            rAny <<= xText;
            break;
        }
    case FIELD_PROP_DATE:
        {
            util::Date aSetDate;
            aSetDate.Day = aDateTime.GetDay();
            aSetDate.Month = aDateTime.GetMonth();
            aSetDate.Year = aDateTime.GetYear();
            rAny.setValue(&aSetDate, ::getCppuType((util::Date*)0));
        }
        break;
    case FIELD_PROP_DATE_TIME:
        {
                util::DateTime DateTimeValue;
                DateTimeValue.HundredthSeconds = aDateTime.Get100Sec();
                DateTimeValue.Seconds = aDateTime.GetSec();
                DateTimeValue.Minutes = aDateTime.GetMin();
                DateTimeValue.Hours = aDateTime.GetHour();
                DateTimeValue.Day = aDateTime.GetDay();
                DateTimeValue.Month = aDateTime.GetMonth();
                DateTimeValue.Year = aDateTime.GetYear();
                rAny <<= DateTimeValue;
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
        rAny >>= sTxt;
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
        if( rAny.getValueType() == ::getCppuType((util::Date*)0) )
        {
            util::Date aSetDate = *(util::Date*)rAny.getValue();
            aDateTime = Date(aSetDate.Day, aSetDate.Month, aSetDate.Year);
        }
        break;
    case FIELD_PROP_DATE_TIME:
    {
        util::DateTime aDateTimeValue;
        if(!(rAny >>= aDateTimeValue))
            return sal_False;
        aDateTime.Set100Sec(aDateTimeValue.HundredthSeconds);
        aDateTime.SetSec(aDateTimeValue.Seconds);
        aDateTime.SetMin(aDateTimeValue.Minutes);
        aDateTime.SetHour(aDateTimeValue.Hours);
        aDateTime.SetDay(aDateTimeValue.Day);
        aDateTime.SetMonth(aDateTimeValue.Month);
        aDateTime.SetYear(aDateTimeValue.Year);
    }
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}
/*--------------------------------------------------------------------
    Beschreibung: DokumentinfoFields
 --------------------------------------------------------------------*/

SwExtUserFieldType::SwExtUserFieldType()
    : SwFieldType( RES_EXTUSERFLD )
{
}

SwFieldType* SwExtUserFieldType::Copy() const
{
    SwExtUserFieldType* pTyp = new SwExtUserFieldType;
    return pTyp;
}

String SwExtUserFieldType::Expand(sal_uInt16 nSub, sal_uInt32 ) const
{
    String aRet;
    sal_uInt16 nRet = USHRT_MAX;
    switch(nSub)
    {
    case EU_FIRSTNAME:      nRet = USER_OPT_FIRSTNAME; break;
    case EU_NAME:           nRet = USER_OPT_LASTNAME;  break;
    case EU_SHORTCUT:       nRet = USER_OPT_ID; break;

    case EU_COMPANY:        nRet = USER_OPT_COMPANY;        break;
    case EU_STREET:         nRet = USER_OPT_STREET;         break;
    case EU_TITLE:          nRet = USER_OPT_TITLE;          break;
    case EU_POSITION:       nRet = USER_OPT_POSITION;       break;
    case EU_PHONE_PRIVATE:  nRet = USER_OPT_TELEPHONEHOME;    break;
    case EU_PHONE_COMPANY:  nRet = USER_OPT_TELEPHONEWORK;    break;
    case EU_FAX:            nRet = USER_OPT_FAX;            break;
    case EU_EMAIL:          nRet = USER_OPT_EMAIL;          break;
    case EU_COUNTRY:        nRet = USER_OPT_COUNTRY;        break;
    case EU_ZIP:            nRet = USER_OPT_ZIP;            break;
    case EU_CITY:           nRet = USER_OPT_CITY;           break;
    case EU_STATE:          nRet = USER_OPT_STATE;          break;
    case EU_FATHERSNAME:    nRet = USER_OPT_FATHERSNAME;    break;
    case EU_APARTMENT:      nRet = USER_OPT_APARTMENT;      break;
    default:             OSL_ENSURE( !this, "Field unknown");
    }
    if( USHRT_MAX != nRet )
    {
        SvtUserOptions&  rUserOpt = SW_MOD()->GetUserOptions();
        aRet = rUserOpt.GetToken( nRet );
    }
    return aRet;
}

SwExtUserField::SwExtUserField(SwExtUserFieldType* pTyp, sal_uInt16 nSubTyp, sal_uInt32 nFmt) :
    SwField(pTyp, nFmt), nType(nSubTyp)
{
    aContent = ((SwExtUserFieldType*)GetTyp())->Expand(nType, GetFormat());
}

String SwExtUserField::Expand() const
{
    if (!IsFixed())
        ((SwExtUserField*)this)->aContent = ((SwExtUserFieldType*)GetTyp())->Expand(nType, GetFormat());

    return aContent;
}

SwField* SwExtUserField::Copy() const
{
    SwExtUserField* pFld = new SwExtUserField((SwExtUserFieldType*)GetTyp(), nType, GetFormat());
    pFld->SetExpansion(aContent);

    return pFld;
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
        rAny <<= OUString(aContent);
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp = nType;
            rAny <<= nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bTmp = IsFixed();
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
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
        ::GetString( rAny, aContent );
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp = 0;
            rAny >>= nTmp;
            nType = nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        if( *(sal_Bool*)rAny.getValue() )
            SetFormat(GetFormat() | AF_FIXED);
        else
            SetFormat(GetFormat() & ~AF_FIXED);
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}
//-------------------------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung: Relatives Seitennummern - Feld
 --------------------------------------------------------------------*/

SwRefPageSetFieldType::SwRefPageSetFieldType()
    : SwFieldType( RES_REFPAGESETFLD )
{
}

SwFieldType* SwRefPageSetFieldType::Copy() const
{
    return new SwRefPageSetFieldType;
}

// ueberlagert, weil es nichts zum Updaten gibt!
void SwRefPageSetFieldType::Modify( const SfxPoolItem*, const SfxPoolItem * )
{
}

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung
 --------------------------------------------------------------------*/

SwRefPageSetField::SwRefPageSetField( SwRefPageSetFieldType* pTyp,
                    short nOff, sal_Bool bFlag )
    : SwField( pTyp ), nOffset( nOff ), bOn( bFlag )
{
}

String SwRefPageSetField::Expand() const
{
    return aEmptyStr;
}

SwField* SwRefPageSetField::Copy() const
{
    return new SwRefPageSetField( (SwRefPageSetFieldType*)GetTyp(), nOffset, bOn );
}

rtl::OUString SwRefPageSetField::GetPar2() const
{
    return rtl::OUString::valueOf(static_cast<sal_Int32>(GetOffset()));
}

void SwRefPageSetField::SetPar2(const rtl::OUString& rStr)
{
    SetOffset( (short) rStr.toInt32() );
}

bool SwRefPageSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        rAny.setValue(&bOn, ::getBooleanCppuType());
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
        bOn = *(sal_Bool*)rAny.getValue();
        break;
    case FIELD_PROP_USHORT1:
        rAny >>=nOffset;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}
/*--------------------------------------------------------------------
    Beschreibung: relatives Seitennummern - Abfrage Feld
 --------------------------------------------------------------------*/

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
    // Update auf alle GetReferenz-Felder
    if( !pNew && !pOld && GetDepends() )
    {
        // sammel erstmal alle SetPageRefFelder ein.
        _SetGetExpFlds aTmpLst;
        if( MakeSetList( aTmpLst ) )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *this );
            for ( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
                    // nur die GetRef-Felder Updaten
                    if( pFmtFld->GetTxtFld() )
                        UpdateField( pFmtFld->GetTxtFld(), aTmpLst );
        }
    }

    // weiter an die Text-Felder, diese "Expandieren" den Text
    NotifyClients( pOld, pNew );
}

sal_uInt16 SwRefPageGetFieldType::MakeSetList( _SetGetExpFlds& rTmpLst )
{
    SwIterator<SwFmtFld,SwFieldType> aIter(*pDoc->GetSysFldType( RES_REFPAGESETFLD));
    for ( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
    {
            // nur die GetRef-Felder Updaten
            const SwTxtFld* pTFld = pFmtFld->GetTxtFld();
            if( pTFld )
            {
                const SwTxtNode& rTxtNd = pTFld->GetTxtNode();

                // immer den ersten !! (in Tab-Headline, Kopf-/Fuss )
                Point aPt;
                const SwCntntFrm* pFrm = rTxtNd.getLayoutFrm( rTxtNd.GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False );

                _SetGetExpFld* pNew;

                if( !pFrm ||
                     pFrm->IsInDocBody() ||
                    // #i31868#
                    // Check if pFrm is not yet connected to the layout.
                    !pFrm->FindPageFrm() )
                {
                    // einen sdbcx::Index fuers bestimmen vom TextNode anlegen
                    SwNodeIndex aIdx( rTxtNd );
                    pNew = new _SetGetExpFld( aIdx, pTFld );
                }
                else
                {
                    // einen sdbcx::Index fuers bestimmen vom TextNode anlegen
                    SwPosition aPos( pDoc->GetNodes().GetEndOfPostIts() );
                    bool const bResult = GetBodyTxtNode( *pDoc, aPos, *pFrm );
                    OSL_ENSURE(bResult, "where is the Field?");
                    (void) bResult; // unused in non-debug
                    pNew = new _SetGetExpFld( aPos.nNode, pTFld,
                                                &aPos.nContent );
                }

                if( !rTmpLst.insert( pNew ).second)
                    delete pNew;
            }
    }

    return rTmpLst.size();
}

void SwRefPageGetFieldType::UpdateField( SwTxtFld* pTxtFld,
                                        _SetGetExpFlds& rSetList )
{
    SwRefPageGetField* pGetFld = (SwRefPageGetField*)pTxtFld->GetFld().GetFld();
    pGetFld->SetText( aEmptyStr );

    // dann suche mal das richtige RefPageSet-Field
    SwTxtNode* pTxtNode = (SwTxtNode*)&pTxtFld->GetTxtNode();
    if( pTxtNode->StartOfSectionIndex() >
        pDoc->GetNodes().GetEndOfExtras().GetIndex() )
    {
        SwNodeIndex aIdx( *pTxtNode );
        _SetGetExpFld aEndFld( aIdx, pTxtFld );

        _SetGetExpFlds::const_iterator itLast = rSetList.lower_bound( &aEndFld );

        if( itLast != rSetList.begin() )
        {
            --itLast;
            const SwTxtFld* pRefTxtFld = (*itLast)->GetFld();
            const SwRefPageSetField* pSetFld =
                        (SwRefPageSetField*)pRefTxtFld->GetFld().GetFld();
            if( pSetFld->IsOn() )
            {
                // dann bestimme mal den entsp. Offset
                Point aPt;
                const SwCntntFrm* pFrm = pTxtNode->getLayoutFrm( pTxtNode->GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False );
                const SwCntntFrm* pRefFrm = pRefTxtFld->GetTxtNode().getLayoutFrm( pRefTxtFld->GetTxtNode().GetDoc()->GetCurrentLayout(), &aPt, 0, sal_False );
                const SwPageFrm* pPgFrm = 0;
                sal_uInt16 nDiff = ( pFrm && pRefFrm )
                        ?   (pPgFrm = pFrm->FindPageFrm())->GetPhyPageNum() -
                            pRefFrm->FindPageFrm()->GetPhyPageNum() + 1
                        : 1;

                sal_uInt32 nTmpFmt = SVX_NUM_PAGEDESC == pGetFld->GetFormat()
                        ? ( !pPgFrm
                                ? (sal_uInt32)SVX_NUM_ARABIC
                                : pPgFrm->GetPageDesc()->GetNumType().GetNumberingType() )
                        : pGetFld->GetFormat();
                short nPageNum = static_cast<short>(Max(0, pSetFld->GetOffset() + (short)nDiff));
                pGetFld->SetText( FormatNumber( nPageNum, nTmpFmt ) );
            }
        }
    }
    // dann die Formatierung anstossen
    ((SwFmtFld&)pTxtFld->GetFld()).ModifyNotification( 0, 0 );
}

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung Abfragen
 --------------------------------------------------------------------*/

SwRefPageGetField::SwRefPageGetField( SwRefPageGetFieldType* pTyp,
                                    sal_uInt32 nFmt )
    : SwField( pTyp, nFmt )
{
}

String SwRefPageGetField::Expand() const
{
    return sTxt;
}

SwField* SwRefPageGetField::Copy() const
{
    SwRefPageGetField* pCpy = new SwRefPageGetField(
                        (SwRefPageGetFieldType*)GetTyp(), GetFormat() );
    pCpy->SetText( sTxt );
    return pCpy;
}

void SwRefPageGetField::ChangeExpansion( const SwFrm* pFrm,
                                        const SwTxtFld* pFld )
{
    // nur Felder in Footer, Header, FootNote, Flys
    SwTxtNode* pTxtNode = (SwTxtNode*)&pFld->GetTxtNode();
    SwRefPageGetFieldType* pGetType = (SwRefPageGetFieldType*)GetTyp();
    SwDoc* pDoc = pGetType->GetDoc();
    if( pFld->GetTxtNode().StartOfSectionIndex() >
        pDoc->GetNodes().GetEndOfExtras().GetIndex() )
        return;

    sTxt.Erase();

    OSL_ENSURE( !pFrm->IsInDocBody(), "Flag ist nicht richtig, Frame steht im DocBody" );

    // sammel erstmal alle SetPageRefFelder ein.
    _SetGetExpFlds aTmpLst;
    if( !pGetType->MakeSetList( aTmpLst ) )
        return ;

    // einen sdbcx::Index fuers bestimmen vom TextNode anlegen
    SwPosition aPos( SwNodeIndex( pDoc->GetNodes() ) );
    pTxtNode = (SwTxtNode*) GetBodyTxtNode( *pDoc, aPos, *pFrm );

    // Wenn kein Layout vorhanden, kommt es in Kopf und Fusszeilen dazu
    // das ChangeExpansion uebers Layout-Formatieren aufgerufen wird
    // aber kein TxtNode vorhanden ist
    //
    if(!pTxtNode)
        return;

    _SetGetExpFld aEndFld( aPos.nNode, pFld, &aPos.nContent );

    _SetGetExpFlds::const_iterator itLast = aTmpLst.lower_bound( &aEndFld );

    if( itLast == aTmpLst.begin() )
        return;        // es gibt kein entsprechendes Set - Feld vor mir
    --itLast;

    const SwTxtFld* pRefTxtFld = (*itLast)->GetFld();
    const SwRefPageSetField* pSetFld =
                        (SwRefPageSetField*)pRefTxtFld->GetFld().GetFld();
    Point aPt;
    const SwCntntFrm* pRefFrm = pRefTxtFld ? pRefTxtFld->GetTxtNode().getLayoutFrm( pFrm->getRootFrm(), &aPt, 0, sal_False ) : 0;
    if( pSetFld->IsOn() && pRefFrm )
    {
        // dann bestimme mal den entsp. Offset
        const SwPageFrm* pPgFrm = pFrm->FindPageFrm();
        sal_uInt16 nDiff = pPgFrm->GetPhyPageNum() -
                            pRefFrm->FindPageFrm()->GetPhyPageNum() + 1;

        SwRefPageGetField* pGetFld = (SwRefPageGetField*)pFld->GetFld().GetFld();
        sal_uInt32 nTmpFmt = SVX_NUM_PAGEDESC == pGetFld->GetFormat()
                            ? pPgFrm->GetPageDesc()->GetNumType().GetNumberingType()
                            : pGetFld->GetFormat();
        short nPageNum = static_cast<short>(Max(0, pSetFld->GetOffset() + (short)nDiff ));
        pGetFld->SetText( FormatNumber( nPageNum, nTmpFmt ) );
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
            rAny <<= OUString(sTxt);
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
        {
            OUString sTmp;
            rAny >>= sTmp;
            sTxt = sTmp;
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/*--------------------------------------------------------------------
    Beschreibung: Feld zum Anspringen und Editieren
 --------------------------------------------------------------------*/

SwJumpEditFieldType::SwJumpEditFieldType( SwDoc* pD )
    : SwFieldType( RES_JUMPEDITFLD ), pDoc( pD ), aDep( this, 0 )
{
}

SwFieldType* SwJumpEditFieldType::Copy() const
{
    return new SwJumpEditFieldType( pDoc );
}

SwCharFmt* SwJumpEditFieldType::GetCharFmt()
{
    SwCharFmt* pFmt = pDoc->GetCharFmtFromPool( RES_POOLCHR_JUMPEDIT );

    // noch nicht registriert ?
    if( !aDep.GetRegisteredIn() )
        pFmt->Add( &aDep );     // anmelden

    return pFmt;
}

SwJumpEditField::SwJumpEditField( SwJumpEditFieldType* pTyp, sal_uInt32 nForm,
                                const String& rTxt, const String& rHelp )
    : SwField( pTyp, nForm ), sTxt( rTxt ), sHelp( rHelp )
{
}

String SwJumpEditField::Expand() const
{
    return rtl::OUStringBuffer().append('<').
        append(sTxt).append('>').makeStringAndClear();
}

SwField* SwJumpEditField::Copy() const
{
    return new SwJumpEditField( (SwJumpEditFieldType*)GetTyp(), GetFormat(),
                                sTxt, sHelp );
}

// Platzhalter-Text

const rtl::OUString& SwJumpEditField::GetPar1() const
{
    return sTxt;
}

void SwJumpEditField::SetPar1(const rtl::OUString& rStr)
{
    sTxt = rStr;
}

// HinweisText

rtl::OUString SwJumpEditField::GetPar2() const
{
    return sHelp;
}

void SwJumpEditField::SetPar2(const rtl::OUString& rStr)
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
        rAny <<= OUString(sHelp);
        break;
    case FIELD_PROP_PAR2 :
         rAny <<= OUString(sTxt);
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
         rAny >>= sTxt;
         break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}


/*--------------------------------------------------------------------
    Beschreibung: Combined Character Fieldtype / Field
 --------------------------------------------------------------------*/

SwCombinedCharFieldType::SwCombinedCharFieldType()
    : SwFieldType( RES_COMBINED_CHARS )
{
}

SwFieldType* SwCombinedCharFieldType::Copy() const
{
    return new SwCombinedCharFieldType;
}

/* --------------------------------------------------------------------*/

SwCombinedCharField::SwCombinedCharField( SwCombinedCharFieldType* pFTyp,
                                            const String& rChars )
    : SwField( pFTyp, 0 ),
    sCharacters( rChars.Copy( 0, MAX_COMBINED_CHARACTERS ))
{
}

String  SwCombinedCharField::Expand() const
{
    return sCharacters;
}

SwField* SwCombinedCharField::Copy() const
{
    return new SwCombinedCharField( (SwCombinedCharFieldType*)GetTyp(),
                                        sCharacters );
}

const rtl::OUString& SwCombinedCharField::GetPar1() const
{
    return sCharacters;
}

void SwCombinedCharField::SetPar1(const rtl::OUString& rStr)
{
    sCharacters = rStr.copy(0, std::min<sal_Int32>(rStr.getLength(), MAX_COMBINED_CHARACTERS));
}

bool SwCombinedCharField::QueryValue( uno::Any& rAny,
                                        sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= rtl::OUString( sCharacters );
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
            rtl::OUString sTmp;
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
