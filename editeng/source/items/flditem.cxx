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

#include <comphelper/string.hxx>
#include <vcl/metaact.hxx>
#include <svl/zforlist.hxx>
#include <tools/urlobj.hxx>

#include <unotools/localfilehelper.hxx>

#include <editeng/flditem.hxx>
#include <editeng/measfld.hxx>
#include "editeng/unonames.hxx"

#include <tools/tenccvt.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/util/DateTime.hpp>

using namespace com::sun::star;

SvxFieldData* SvxFieldData::Create(const uno::Reference<text::XTextContent>& xTextContent)
{
    uno::Reference<beans::XPropertySet> xPropSet(xTextContent, uno::UNO_QUERY);
    if (!xPropSet.is())
        return NULL;

    // we do not support these fields from Writer, so make sure we do not throw
    // here - see fdo#63436 how to possibly extend Writer to make use of this
    uno::Any aAny;
    try {
        aAny = xPropSet->getPropertyValue(UNO_TC_PROP_TEXTFIELD_TYPE);
        if ( !aAny.has<sal_Int32>() )
            return NULL;
    } catch ( const beans::UnknownPropertyException& )
    {
        return NULL;
    }

    sal_Int32 nFieldType = aAny.get<sal_Int32>();

    switch (nFieldType)
    {
        case text::textfield::Type::TIME:
        case text::textfield::Type::EXTENDED_TIME:
        case text::textfield::Type::DATE:
        {
            sal_Bool bIsDate = false;
            xPropSet->getPropertyValue(UNO_TC_PROP_IS_DATE) >>= bIsDate;

            if (bIsDate)
            {
                util::DateTime aDateTime = xPropSet->getPropertyValue(UNO_TC_PROP_DATE_TIME).get<util::DateTime>();
                Date aDate(aDateTime.Day, aDateTime.Month, aDateTime.Year);
                sal_Bool bIsFixed = false;
                xPropSet->getPropertyValue(UNO_TC_PROP_IS_FIXED) >>= bIsFixed;

                SvxDateField* pData = new SvxDateField(aDate, bIsFixed ? SVXDATETYPE_FIX : SVXDATETYPE_VAR);
                sal_Int32 nNumFmt = -1;
                xPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                if (nNumFmt >= SVXDATEFORMAT_APPDEFAULT && nNumFmt <= SVXDATEFORMAT_F)
                    pData->SetFormat(static_cast<SvxDateFormat>(nNumFmt));

                return pData;
            }

            if (nFieldType != text::textfield::Type::TIME && nFieldType != text::textfield::Type::DATE)
            {
                util::DateTime aDateTime = xPropSet->getPropertyValue(UNO_TC_PROP_DATE_TIME).get<util::DateTime>();
                Time aTime(aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.NanoSeconds);

                sal_Bool bIsFixed = false;
                xPropSet->getPropertyValue(UNO_TC_PROP_IS_FIXED) >>= bIsFixed;

                SvxExtTimeField* pData = new SvxExtTimeField(aTime, bIsFixed ? SVXTIMETYPE_FIX : SVXTIMETYPE_VAR);

                sal_Int32 nNumFmt = -1;
                xPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                if (nNumFmt >= SVXTIMEFORMAT_APPDEFAULT && nNumFmt <= SVXTIMEFORMAT_AM_HMSH)
                    pData->SetFormat(static_cast<SvxTimeFormat>(nNumFmt));

                return pData;
            }

            return new SvxTimeField();
        }
        case text::textfield::Type::URL:
        {
            OUString aRep, aTarget, aURL;
            sal_Int16 nFmt = -1;
            xPropSet->getPropertyValue(UNO_TC_PROP_URL_REPRESENTATION) >>= aRep;
            xPropSet->getPropertyValue(UNO_TC_PROP_URL_TARGET) >>= aTarget;
            xPropSet->getPropertyValue(UNO_TC_PROP_URL) >>= aURL;
            xPropSet->getPropertyValue(UNO_TC_PROP_URL_FORMAT) >>= nFmt;
            SvxURLField* pData = new SvxURLField(aURL, aRep, aRep.isEmpty() ? SVXURLFORMAT_URL : SVXURLFORMAT_REPR);
            pData->SetTargetFrame(aTarget);
            if (nFmt >= SVXURLFORMAT_APPDEFAULT && nFmt <= SVXURLFORMAT_REPR)
                pData->SetFormat(static_cast<SvxURLFormat>(nFmt));

            return pData;
        }
        case text::textfield::Type::PAGE:
            return new SvxPageField();
        case text::textfield::Type::PAGES:
            return new SvxPagesField();
        case text::textfield::Type::DOCINFO_TITLE:
            return new SvxFileField();
        case text::textfield::Type::TABLE:
        {
            sal_Int32 nTab = 0;
            xPropSet->getPropertyValue(UNO_TC_PROP_TABLE_POSITION) >>= nTab;
            return new SvxTableField(nTab);
        }
        case text::textfield::Type::EXTENDED_FILE:
        {
            OUString aPresentation;
            sal_Bool bIsFixed = false;
            sal_Int16 nFmt = text::FilenameDisplayFormat::FULL;
            xPropSet->getPropertyValue(UNO_TC_PROP_IS_FIXED) >>= bIsFixed;
            xPropSet->getPropertyValue(UNO_TC_PROP_CURRENT_PRESENTATION) >>= aPresentation;
            xPropSet->getPropertyValue(UNO_TC_PROP_FILE_FORMAT) >>= nFmt;

            SvxFileFormat eFmt = SVXFILEFORMAT_NAME_EXT;
            switch (nFmt)
            {
                case text::FilenameDisplayFormat::FULL: eFmt = SVXFILEFORMAT_FULLPATH; break;
                case text::FilenameDisplayFormat::PATH: eFmt = SVXFILEFORMAT_PATH;     break;
                case text::FilenameDisplayFormat::NAME: eFmt = SVXFILEFORMAT_NAME;     break;
                default:;
            }

            // pass fixed attribute to constructor
            return new SvxExtFileField(
                aPresentation, bIsFixed ? SVXFILETYPE_FIX : SVXFILETYPE_VAR, eFmt);
        }
        case text::textfield::Type::AUTHOR:
        {
            sal_Bool bIsFixed = false;
            sal_Bool bFullName = false;
            sal_Int16 nFmt = -1;
            OUString aPresentation, aContent, aFirstName, aLastName;
            xPropSet->getPropertyValue(UNO_TC_PROP_IS_FIXED) >>= bIsFixed;
            xPropSet->getPropertyValue(UNO_TC_PROP_AUTHOR_FULLNAME) >>= bFullName;
            xPropSet->getPropertyValue(UNO_TC_PROP_CURRENT_PRESENTATION) >>= aPresentation;
            xPropSet->getPropertyValue(UNO_TC_PROP_AUTHOR_CONTENT) >>= aContent;
            xPropSet->getPropertyValue(UNO_TC_PROP_AUTHOR_FORMAT) >>= nFmt;

            // do we have CurrentPresentation given?  Mimic behaviour of
            // writer, which means: prefer CurrentPresentation over Content
            // if both are given.
            if (!aPresentation.isEmpty())
                aContent = aPresentation;

            sal_Int32 nPos = aContent.lastIndexOf(sal_Unicode(' '), 0);
            if (nPos > 0)
            {
                aFirstName = aContent.copy(0, nPos);
                aLastName = aContent.copy(nPos + 1);
            }
            else
            {
                aLastName = aContent;
            }

            // #92009# pass fixed attribute to constructor
            SvxAuthorField* pData = new SvxAuthorField(
                aFirstName, aLastName, OUString(), bIsFixed ? SVXAUTHORTYPE_FIX : SVXAUTHORTYPE_VAR);

            if (!bFullName)
            {
                pData->SetFormat(SVXAUTHORFORMAT_SHORTNAME);
            }
            else if (nFmt >= SVXAUTHORFORMAT_FULLNAME || nFmt <= SVXAUTHORFORMAT_SHORTNAME)
            {
                pData->SetFormat(static_cast<SvxAuthorFormat>(nFmt));
            }

            return pData;
        }
        case text::textfield::Type::MEASURE:
        {
            SdrMeasureFieldKind eKind = SDRMEASUREFIELD_VALUE;
            sal_Int16 nTmp = -1;
            xPropSet->getPropertyValue(UNO_TC_PROP_MEASURE_KIND) >>= nTmp;
            if (nTmp == static_cast<sal_Int16>(SDRMEASUREFIELD_UNIT) ||
                nTmp == static_cast<sal_Int16>(SDRMEASUREFIELD_ROTA90BLANCS))
                eKind = static_cast<SdrMeasureFieldKind>(nTmp);

            return new SdrMeasureField(eKind);
        }
        case text::textfield::Type::PRESENTATION_HEADER:
            return new SvxHeaderField();
        case text::textfield::Type::PRESENTATION_FOOTER:
            return new SvxFooterField();
        case text::textfield::Type::PRESENTATION_DATE_TIME:
            return new SvxDateTimeField();
        default:
            ;
    };

    return NULL;
}

TYPEINIT1( SvxFieldItem, SfxPoolItem );

SV_IMPL_PERSIST1( SvxFieldData, SvPersistBase );

// -----------------------------------------------------------------------

SvxFieldData::SvxFieldData()
{
}

// -----------------------------------------------------------------------

SvxFieldData::~SvxFieldData()
{
}

// -----------------------------------------------------------------------

SvxFieldData* SvxFieldData::Clone() const
{
    return new SvxFieldData;
}

// -----------------------------------------------------------------------

int SvxFieldData::operator==( const SvxFieldData& rFld ) const
{
    DBG_ASSERT( Type() == rFld.Type(), "==: Different Types" );
    (void)rFld;
    return sal_True;    // Basic class is always the same.
}

// -----------------------------------------------------------------------

void SvxFieldData::Load( SvPersistStream & /*rStm*/ )
{
}

// -----------------------------------------------------------------------

void SvxFieldData::Save( SvPersistStream & /*rStm*/ )
{
}


MetaAction* SvxFieldData::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

MetaAction* SvxFieldData::createEndComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_END" );
}

// -----------------------------------------------------------------------

SvxFieldItem::SvxFieldItem( SvxFieldData* pFld, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    pField = pFld;  // belongs directly to the item
}

// -----------------------------------------------------------------------

SvxFieldItem::SvxFieldItem( const SvxFieldData& rField, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    pField = rField.Clone();
}

// -----------------------------------------------------------------------

SvxFieldItem::SvxFieldItem( const SvxFieldItem& rItem ) :
    SfxPoolItem ( rItem )
{
    pField = rItem.GetField() ? rItem.GetField()->Clone() : 0;
}

// -----------------------------------------------------------------------

SvxFieldItem::~SvxFieldItem()
{
    delete pField;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFieldItem::Clone( SfxItemPool* ) const
{
    return new SvxFieldItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFieldItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    SvxFieldData* pData = 0;
    SvPersistStream aPStrm( GetClassManager(), &rStrm );
    aPStrm >> pData;

    if( aPStrm.IsEof() )
        aPStrm.SetError( SVSTREAM_GENERALERROR );

    if ( aPStrm.GetError() == ERRCODE_IO_NOFACTORY )
        aPStrm.ResetError();    // Actually a code for that not all were read Attr ...

    return new SvxFieldItem( pData, Which() );
}

// -----------------------------------------------------------------------

SvStream& SvxFieldItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    DBG_ASSERT( pField, "SvxFieldItem::Store: Field?!" );
    SvPersistStream aPStrm( GetClassManager(), &rStrm );
    // The reset error in the above Create method did not exist in 3.1,
    // therefore newer items can not be saved for 3.x-exports!
    if ( ( rStrm.GetVersion() <= SOFFICE_FILEFORMAT_31 ) && pField &&
            pField->GetClassId() == 50 /* SdrMeasureField */ )
    {
        // SvxFieldData not enough, because not registered on ClassMgr.
        SvxURLField aDummyData;
        aPStrm << &aDummyData;
    }
    else
        aPStrm << pField;

    return rStrm;
}

// -----------------------------------------------------------------------

int SvxFieldItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );

    const SvxFieldData* pOtherFld = ((const SvxFieldItem&)rItem).GetField();
    if ( !pField && !pOtherFld )
        return sal_True;

    if ( ( !pField && pOtherFld ) || ( pField && !pOtherFld ) )
        return sal_False;

    return ( ( pField->Type() == pOtherFld->Type() )
                && ( *pField == *pOtherFld ) );
}

// =================================================================
// The following are the derivatives of SvxFieldData ...
// =================================================================

SV_IMPL_PERSIST1( SvxDateField, SvxFieldData );

// -----------------------------------------------------------------------

SvxDateField::SvxDateField()
{
    nFixDate = Date( Date::SYSTEM ).GetDate();
    eType = SVXDATETYPE_VAR;
    eFormat = SVXDATEFORMAT_STDSMALL;
}

// -----------------------------------------------------------------------

SvxDateField::SvxDateField( const Date& rDate, SvxDateType eT, SvxDateFormat eF )
{
    nFixDate = rDate.GetDate();
    eType = eT;
    eFormat = eF;
}

// -----------------------------------------------------------------------

SvxFieldData* SvxDateField::Clone() const
{
    return new SvxDateField( *this );
}

// -----------------------------------------------------------------------

int SvxDateField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return sal_False;

    const SvxDateField& rOtherFld = (const SvxDateField&) rOther;
    return ( ( nFixDate == rOtherFld.nFixDate ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

// -----------------------------------------------------------------------

void SvxDateField::Load( SvPersistStream & rStm )
{
    sal_uInt16 nType, nFormat;

    rStm >> nFixDate;
    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxDateType)nType;
    eFormat= (SvxDateFormat)nFormat;
}

// -----------------------------------------------------------------------

void SvxDateField::Save( SvPersistStream & rStm )
{
    rStm << nFixDate;
    rStm << (sal_uInt16)eType;
    rStm << (sal_uInt16)eFormat;
}

// -----------------------------------------------------------------------

OUString SvxDateField::GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLang ) const
{
    Date aDate( Date::EMPTY );
    if ( eType == SVXDATETYPE_FIX )
        aDate.SetDate( nFixDate );
    else
        aDate = Date( Date::SYSTEM ); // current date

    return GetFormatted( aDate, eFormat, rFormatter, eLang );
}

OUString SvxDateField::GetFormatted( Date& aDate, SvxDateFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLang )
{
    if ( eFormat == SVXDATEFORMAT_SYSTEM )
    {
        OSL_FAIL( "SVXDATEFORMAT_SYSTEM not implemented!" );
        eFormat = SVXDATEFORMAT_STDSMALL;
    }
    else if ( eFormat == SVXDATEFORMAT_APPDEFAULT )
    {
        OSL_FAIL( "SVXDATEFORMAT_APPDEFAULT: take them from where? ");
        eFormat = SVXDATEFORMAT_STDSMALL;
    }

    sal_uLong nFormatKey;

    switch( eFormat )
    {
        case SVXDATEFORMAT_STDSMALL:
            // short
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLang );
        break;
        case SVXDATEFORMAT_STDBIG:
            // long
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYSTEM_LONG, eLang );
        break;
        case SVXDATEFORMAT_A:
            // 13.02.96
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DDMMYY, eLang );
        break;
        case SVXDATEFORMAT_B:
            // 13.02.1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DDMMYYYY, eLang );
        break;
        case SVXDATEFORMAT_C:
            // 13. Feb 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DMMMYYYY, eLang );
        break;
        case SVXDATEFORMAT_D:
            // 13. February 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DMMMMYYYY, eLang );
        break;
        case SVXDATEFORMAT_E:
            // The, 13. February 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_NNDMMMMYYYY, eLang );
        break;
        case SVXDATEFORMAT_F:
            // Tuesday, 13. February 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_NNNNDMMMMYYYY, eLang );
        break;
        default:
            nFormatKey = rFormatter.GetStandardFormat( NUMBERFORMAT_DATE, eLang );
    }

    double fDiffDate = aDate - *(rFormatter.GetNullDate());
    OUString aStr;
       Color* pColor = NULL;
    rFormatter.GetOutputString( fDiffDate, nFormatKey, aStr, &pColor );
    return aStr;
}

MetaAction* SvxDateField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

SV_IMPL_PERSIST1( SvxURLField, SvxFieldData );

// -----------------------------------------------------------------------

SvxURLField::SvxURLField()
{
    eFormat = SVXURLFORMAT_URL;
}

// -----------------------------------------------------------------------

SvxURLField::SvxURLField( const OUString& rURL, const OUString& rRepres, SvxURLFormat eFmt )
    : aURL( rURL ), aRepresentation( rRepres )
{
    eFormat = eFmt;
}

// -----------------------------------------------------------------------

SvxFieldData* SvxURLField::Clone() const
{
    return new SvxURLField( *this );
}

// -----------------------------------------------------------------------

int SvxURLField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return sal_False;

    const SvxURLField& rOtherFld = (const SvxURLField&) rOther;
    return ( ( eFormat == rOtherFld.eFormat ) &&
                ( aURL == rOtherFld.aURL ) &&
                ( aRepresentation == rOtherFld.aRepresentation ) &&
                ( aTargetFrame == rOtherFld.aTargetFrame ) );
}

// -----------------------------------------------------------------------

static void write_unicode( SvPersistStream & rStm, const OUString& rString )
{
    sal_uInt16 nL =  sal::static_int_cast<sal_uInt16>(rString.getLength());
    rStm << nL;
    //endian specific?, yipes!
    rStm.Write( rString.getStr(), nL*sizeof(sal_Unicode) );
}

static OUString read_unicode( SvPersistStream & rStm )
{
    rtl_uString *pStr = NULL;
    sal_uInt16 nL = 0;
    rStm >> nL;
    if ( nL )
    {
        pStr = rtl_uString_alloc(nL);
        //endian specific?, yipes!
        rStm.Read(pStr->buffer, nL*sizeof(sal_Unicode));
    }
    //take ownership of buffer and return, otherwise return empty string
    return pStr ? OUString(pStr, SAL_NO_ACQUIRE) : OUString();
}

void SvxURLField::Load( SvPersistStream & rStm )
{
    sal_uInt16 nFormat = 0;

    rStm >> nFormat;
    eFormat= (SvxURLFormat)nFormat;

    aURL = read_unicode( rStm );
    aRepresentation = read_unicode( rStm );
    aTargetFrame = read_unicode( rStm );
}

// -----------------------------------------------------------------------

void SvxURLField::Save( SvPersistStream & rStm )
{
    rStm << (sal_uInt16)eFormat;

    write_unicode( rStm, aURL );
    write_unicode( rStm, aRepresentation );
    write_unicode( rStm, aTargetFrame );
}

MetaAction* SvxURLField::createBeginComment() const
{
    // #i46618# Adding target URL to metafile comment
    return new MetaCommentAction( "FIELD_SEQ_BEGIN",
                                  0,
                                  reinterpret_cast<const sal_uInt8*>(aURL.getStr()),
                                  2*aURL.getLength() );
}

// =================================================================
// The fields that were removed from Calc:
// =================================================================

SV_IMPL_PERSIST1( SvxPageField, SvxFieldData );

SvxPageField::SvxPageField() {}

SvxFieldData* SvxPageField::Clone() const
{
    return new SvxPageField;        // empty
}

int SvxPageField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxPageField) );
}

void SvxPageField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxPageField::Save( SvPersistStream & /*rStm*/ )
{
}

MetaAction* SvxPageField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN;PageField" );
}


SV_IMPL_PERSIST1( SvxPagesField, SvxFieldData );

SvxPagesField::SvxPagesField() {}

SvxFieldData* SvxPagesField::Clone() const
{
    return new SvxPagesField;   // empty
}

int SvxPagesField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxPagesField) );
}

void SvxPagesField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxPagesField::Save( SvPersistStream & /*rStm*/ )
{
}

SV_IMPL_PERSIST1( SvxTimeField, SvxFieldData );

SvxTimeField::SvxTimeField() {}

SvxFieldData* SvxTimeField::Clone() const
{
    return new SvxTimeField;    // empty
}

int SvxTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxTimeField) );
}

void SvxTimeField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxTimeField::Save( SvPersistStream & /*rStm*/ )
{
}

MetaAction* SvxTimeField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

SV_IMPL_PERSIST1( SvxFileField, SvxFieldData );

SvxFileField::SvxFileField() {}

SvxFieldData* SvxFileField::Clone() const
{
    return new SvxFileField;    // empty
}

int SvxFileField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxFileField) );
}

void SvxFileField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxFileField::Save( SvPersistStream & /*rStm*/ )
{
}

SV_IMPL_PERSIST1( SvxTableField, SvxFieldData );

SvxTableField::SvxTableField() : mnTab(0) {}

SvxTableField::SvxTableField(int nTab) : mnTab(nTab) {}

void SvxTableField::SetTab(int nTab)
{
    mnTab = nTab;
}

int SvxTableField::GetTab() const
{
    return mnTab;
}

SvxFieldData* SvxTableField::Clone() const
{
    return new SvxTableField(mnTab);
}

int SvxTableField::operator==( const SvxFieldData& rCmp ) const
{
    if (rCmp.Type() != TYPE(SvxTableField))
        return false;

    return mnTab == static_cast<const SvxTableField&>(rCmp).mnTab;
}

void SvxTableField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxTableField::Save( SvPersistStream & /*rStm*/ )
{
}

//----------------------------------------------------------------------------
//      SvxExtTimeField
//----------------------------------------------------------------------------

SV_IMPL_PERSIST1( SvxExtTimeField, SvxFieldData );

//----------------------------------------------------------------------------

SvxExtTimeField::SvxExtTimeField()
{
    nFixTime = Time( Time::SYSTEM ).GetTime();
    eType = SVXTIMETYPE_VAR;
    eFormat = SVXTIMEFORMAT_STANDARD;
}

//----------------------------------------------------------------------------

SvxExtTimeField::SvxExtTimeField( const Time& rTime, SvxTimeType eT, SvxTimeFormat eF )
{
    nFixTime = rTime.GetTime();
    eType = eT;
    eFormat = eF;
}

//----------------------------------------------------------------------------

SvxFieldData* SvxExtTimeField::Clone() const
{
    return new SvxExtTimeField( *this );
}

//----------------------------------------------------------------------------

int SvxExtTimeField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return sal_False;

    const SvxExtTimeField& rOtherFld = (const SvxExtTimeField&) rOther;
    return ( ( nFixTime == rOtherFld.nFixTime ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

//----------------------------------------------------------------------------

void SvxExtTimeField::Load( SvPersistStream & rStm )
{
    sal_uInt16 nType, nFormat;

    rStm >> nFixTime;
    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxTimeType) nType;
    eFormat= (SvxTimeFormat) nFormat;
}

//----------------------------------------------------------------------------

void SvxExtTimeField::Save( SvPersistStream & rStm )
{
    rStm << nFixTime;
    rStm << (sal_uInt16) eType;
    rStm << (sal_uInt16) eFormat;
}

//----------------------------------------------------------------------------

OUString SvxExtTimeField::GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLang ) const
{
    Time aTime( Time::EMPTY );
    if ( eType == SVXTIMETYPE_FIX )
        aTime.SetTime( nFixTime );
    else
        aTime = Time( Time::SYSTEM ); // current time
    return GetFormatted( aTime, eFormat, rFormatter, eLang );
}

OUString SvxExtTimeField::GetFormatted( Time& aTime, SvxTimeFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLang )
{
    switch( eFormat )
    {
        case SVXTIMEFORMAT_SYSTEM :
            OSL_FAIL( "SVXTIMEFORMAT_SYSTEM: not implemented" );
            eFormat = SVXTIMEFORMAT_STANDARD;
        break;
        case SVXTIMEFORMAT_APPDEFAULT :
            OSL_FAIL( "SVXTIMEFORMAT_APPDEFAULT: not implemented" );
            eFormat = SVXTIMEFORMAT_STANDARD;
        break;
        default: ;//prevent warning
    }

    sal_uInt32 nFormatKey;

    switch( eFormat )
    {
        case SVXTIMEFORMAT_12_HM:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMMAMPM, eLang );
            break;
        case SVXTIMEFORMAT_12_HMSH:
        {
            // no builtin format available, try to insert or reuse
            OUString aFormatCode( "HH:MM:SS.00 AM/PM" );
            sal_Int32 nCheckPos;
            short nType;
            rFormatter.PutandConvertEntry( aFormatCode, nCheckPos, nType,
                                           nFormatKey, LANGUAGE_ENGLISH_US, eLang );
            DBG_ASSERT( nCheckPos == 0, "SVXTIMEFORMAT_12_HMSH: could not insert format code" );
            if ( nCheckPos )
            {
                nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HH_MMSS00, eLang );
            }
            break;
        }
        case SVXTIMEFORMAT_24_HM:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMM, eLang );
            break;
        case SVXTIMEFORMAT_24_HMSH:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HH_MMSS00, eLang );
            break;
        case SVXTIMEFORMAT_12_HMS:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMMSSAMPM, eLang );
            break;
        case SVXTIMEFORMAT_24_HMS:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMMSS, eLang );
            break;
        case SVXTIMEFORMAT_STANDARD:
        default:
            nFormatKey = rFormatter.GetStandardFormat( NUMBERFORMAT_TIME, eLang );
    }

    double fFracTime = aTime.GetTimeInDays();
    OUString aStr;
    Color* pColor = NULL;
    rFormatter.GetOutputString( fFracTime, nFormatKey, aStr, &pColor );
    return aStr;
}

MetaAction* SvxExtTimeField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

//----------------------------------------------------------------------------
//      SvxExtFileField
//----------------------------------------------------------------------------

SV_IMPL_PERSIST1( SvxExtFileField, SvxFieldData );

//----------------------------------------------------------------------------

SvxExtFileField::SvxExtFileField()
{
    eType = SVXFILETYPE_VAR;
    eFormat = SVXFILEFORMAT_FULLPATH;
}

//----------------------------------------------------------------------------

SvxExtFileField::SvxExtFileField( const OUString& rStr, SvxFileType eT, SvxFileFormat eF )
{
    aFile = rStr;
    eType = eT;
    eFormat = eF;
}

//----------------------------------------------------------------------------

SvxFieldData* SvxExtFileField::Clone() const
{
    return new SvxExtFileField( *this );
}

//----------------------------------------------------------------------------

int SvxExtFileField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return sal_False;

    const SvxExtFileField& rOtherFld = (const SvxExtFileField&) rOther;
    return ( ( aFile == rOtherFld.aFile ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

//----------------------------------------------------------------------------

void SvxExtFileField::Load( SvPersistStream & rStm )
{
    sal_uInt16 nType, nFormat;

    // UNICODE: rStm >> aFile;
    aFile = rStm.ReadUniOrByteString(rStm.GetStreamCharSet());

    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxFileType) nType;
    eFormat= (SvxFileFormat) nFormat;
}

//----------------------------------------------------------------------------

void SvxExtFileField::Save( SvPersistStream & rStm )
{
    // UNICODE: rStm << aFile;
    rStm.WriteUniOrByteString(aFile, rStm.GetStreamCharSet());

    rStm << (sal_uInt16) eType;
    rStm << (sal_uInt16) eFormat;
}

//----------------------------------------------------------------------------

OUString SvxExtFileField::GetFormatted() const
{
    OUString aString;

    INetURLObject aURLObj( aFile );

    if( INET_PROT_NOT_VALID == aURLObj.GetProtocol() )
    {
        // invalid? try to interpret string as system file name
        OUString aURLStr;

        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFile, aURLStr );

        aURLObj.SetURL( aURLStr );
    }

    // #92009# Be somewhat liberate when trying to
    // get formatted content out of the FileField
    if( INET_PROT_NOT_VALID == aURLObj.GetProtocol() )
    {
        // still not valid? Then output as is
        aString = aFile;
    }
    else if( INET_PROT_FILE == aURLObj.GetProtocol() )
    {
        switch( eFormat )
        {
            case SVXFILEFORMAT_FULLPATH:
                aString = aURLObj.getFSysPath(INetURLObject::FSYS_DETECT);
            break;

            case SVXFILEFORMAT_PATH:
                aURLObj.removeSegment(INetURLObject::LAST_SEGMENT, false);
                // #101742# Leave trailing slash at the pathname
                aURLObj.setFinalSlash();
                aString = aURLObj.getFSysPath(INetURLObject::FSYS_DETECT);
            break;

            case SVXFILEFORMAT_NAME:
                aString = aURLObj.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_UNAMBIGUOUS);
            break;

            case SVXFILEFORMAT_NAME_EXT:
                aString = aURLObj.getName(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_UNAMBIGUOUS);
            break;
        }
    }
    else
    {
        switch( eFormat )
        {
            case SVXFILEFORMAT_FULLPATH:
                aString = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
            break;

            case SVXFILEFORMAT_PATH:
                aURLObj.removeSegment(INetURLObject::LAST_SEGMENT, false);
                // #101742# Leave trailing slash at the pathname
                aURLObj.setFinalSlash();
                aString = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
            break;

            case SVXFILEFORMAT_NAME:
                aString = aURLObj.getBase();
            break;

            case SVXFILEFORMAT_NAME_EXT:
                aString = aURLObj.getName();
            break;
        }
    }

    return aString;
}

//----------------------------------------------------------------------------
//      SvxAuthorField
//----------------------------------------------------------------------------

SV_IMPL_PERSIST1( SvxAuthorField, SvxFieldData );

//----------------------------------------------------------------------------

SvxAuthorField::SvxAuthorField()
{
    eType = SVXAUTHORTYPE_VAR;
    eFormat = SVXAUTHORFORMAT_FULLNAME;
}

//----------------------------------------------------------------------------

SvxAuthorField::SvxAuthorField( const OUString& rFirstName,
                                const OUString& rLastName,
                                const OUString& rShortName,
                                    SvxAuthorType eT, SvxAuthorFormat eF )
{
    aName      = rLastName;
    aFirstName = rFirstName;
    aShortName = rShortName;
    eType   = eT;
    eFormat = eF;
}

//----------------------------------------------------------------------------

SvxFieldData* SvxAuthorField::Clone() const
{
    return new SvxAuthorField( *this );
}

//----------------------------------------------------------------------------

int SvxAuthorField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return sal_False;

    const SvxAuthorField& rOtherFld = (const SvxAuthorField&) rOther;
    return ( ( aName == rOtherFld.aName ) &&
                ( aFirstName == rOtherFld.aFirstName ) &&
                ( aShortName == rOtherFld.aShortName ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

//----------------------------------------------------------------------------

void SvxAuthorField::Load( SvPersistStream & rStm )
{
    sal_uInt16 nType = 0, nFormat = 0;

    aName = read_unicode( rStm );
    aFirstName = read_unicode( rStm );
    aShortName = read_unicode( rStm );

    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxAuthorType) nType;
    eFormat= (SvxAuthorFormat) nFormat;
}

//----------------------------------------------------------------------------

void SvxAuthorField::Save( SvPersistStream & rStm )
{
    write_unicode( rStm, aName );
    write_unicode( rStm, aFirstName );
    write_unicode( rStm, aShortName );

    rStm << (sal_uInt16) eType;
    rStm << (sal_uInt16) eFormat;
}

//----------------------------------------------------------------------------

OUString SvxAuthorField::GetFormatted() const
{
    OUString aString;

    switch( eFormat )
    {
        case SVXAUTHORFORMAT_FULLNAME:
            aString = aFirstName + " " + aName;
        break;
        case SVXAUTHORFORMAT_NAME:
            aString = aName;
        break;

        case SVXAUTHORFORMAT_FIRSTNAME:
            aString = aFirstName;
        break;

        case SVXAUTHORFORMAT_SHORTNAME:
            aString = aShortName;
        break;
    }

    return aString;
}

static SvClassManager* pClassMgr=0;

SvClassManager& SvxFieldItem::GetClassManager()
{
    if ( !pClassMgr )
    {
        pClassMgr = new SvClassManager;
        pClassMgr->Register(SvxFieldData::StaticClassId(),    SvxFieldData::CreateInstance);
        pClassMgr->Register(SvxURLField::StaticClassId(),     SvxURLField::CreateInstance);
        pClassMgr->Register(SvxDateField::StaticClassId(),    SvxDateField::CreateInstance);
        pClassMgr->Register(SvxPageField::StaticClassId(),    SvxPageField::CreateInstance);
        pClassMgr->Register(SvxTimeField::StaticClassId(),    SvxTimeField::CreateInstance);
        pClassMgr->Register(SvxExtTimeField::StaticClassId(), SvxExtTimeField::CreateInstance);
        pClassMgr->Register(SvxExtFileField::StaticClassId(), SvxExtFileField::CreateInstance);
        pClassMgr->Register(SvxAuthorField::StaticClassId(),  SvxAuthorField::CreateInstance);
    }

    return *pClassMgr;
}

///////////////////////////////////////////////////////////////////////

SV_IMPL_PERSIST1( SvxHeaderField, SvxFieldData );

SvxHeaderField::SvxHeaderField() {}

SvxFieldData* SvxHeaderField::Clone() const
{
    return new SvxHeaderField;      // empty
}

int SvxHeaderField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxHeaderField) );
}

void SvxHeaderField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxHeaderField::Save( SvPersistStream & /*rStm*/ )
{
}

///////////////////////////////////////////////////////////////////////

SV_IMPL_PERSIST1( SvxFooterField, SvxFieldData );
SvxFooterField::SvxFooterField() {}

SvxFieldData* SvxFooterField::Clone() const
{
    return new SvxFooterField;      // empty
}

int SvxFooterField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxFooterField) );
}

void SvxFooterField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxFooterField::Save( SvPersistStream & /*rStm*/ )
{
}

///////////////////////////////////////////////////////////////////////

SV_IMPL_PERSIST1( SvxDateTimeField, SvxFieldData );

SvxFieldData* SvxDateTimeField::Clone() const
{
    return new SvxDateTimeField;        // empty
}

int SvxDateTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxDateTimeField) );
}

void SvxDateTimeField::Load( SvPersistStream & /*rStm*/ )
{
}

void SvxDateTimeField::Save( SvPersistStream & /*rStm*/ )
{
}

SvxDateTimeField::SvxDateTimeField() {}

OUString SvxDateTimeField::GetFormatted(
    Date& rDate, Time& rTime, int eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage )
{
    OUString aRet;

    SvxDateFormat eDateFormat = (SvxDateFormat)(eFormat & 0x0f);

    if(eDateFormat)
    {
        aRet = SvxDateField::GetFormatted( rDate, eDateFormat, rFormatter, eLanguage );
    }

    SvxTimeFormat eTimeFormat = (SvxTimeFormat)((eFormat >> 4) & 0x0f);

    if(eTimeFormat)
    {
        OUStringBuffer aBuf(aRet);

        if (!aRet.isEmpty())
            aBuf.append(sal_Unicode(' '));

        aBuf.append(
            SvxExtTimeField::GetFormatted(rTime, eTimeFormat, rFormatter, eLanguage));

        aRet = aBuf.makeStringAndClear();
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
