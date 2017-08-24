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
#include <osl/file.hxx>
#include <vcl/metaact.hxx>
#include <svl/zforlist.hxx>
#include <tools/urlobj.hxx>

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
        return nullptr;

    // we do not support these fields from Writer, so make sure we do not throw
    // here - see fdo#63436 how to possibly extend Writer to make use of this
    uno::Any aAny;
    try {
        aAny = xPropSet->getPropertyValue(UNO_TC_PROP_TEXTFIELD_TYPE);
        if ( !aAny.has<sal_Int32>() )
            return nullptr;

        sal_Int32 nFieldType = aAny.get<sal_Int32>();

        switch (nFieldType)
        {
            case text::textfield::Type::TIME:
            case text::textfield::Type::EXTENDED_TIME:
            case text::textfield::Type::DATE:
                {
                    bool bIsDate = false;
                    xPropSet->getPropertyValue(UNO_TC_PROP_IS_DATE) >>= bIsDate;

                    if (bIsDate)
                    {
                        util::DateTime aDateTime = xPropSet->getPropertyValue(UNO_TC_PROP_DATE_TIME).get<util::DateTime>();
                        Date aDate(aDateTime.Day, aDateTime.Month, aDateTime.Year);
                        bool bIsFixed = false;
                        xPropSet->getPropertyValue(UNO_TC_PROP_IS_FIXED) >>= bIsFixed;

                        SvxDateField* pData = new SvxDateField(aDate, bIsFixed ? SvxDateType::Fix : SvxDateType::Var);
                        sal_Int32 nNumFmt = -1;
                        xPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                        if (nNumFmt >= SVXDATEFORMAT_APPDEFAULT && nNumFmt <= SVXDATEFORMAT_F)
                            pData->SetFormat(static_cast<SvxDateFormat>(nNumFmt));

                        return pData;
                    }

                    if (nFieldType != text::textfield::Type::TIME)
                    {
                        util::DateTime aDateTime = xPropSet->getPropertyValue(UNO_TC_PROP_DATE_TIME).get<util::DateTime>();
                        tools::Time aTime(aDateTime);

                        bool bIsFixed = false;
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
            case text::textfield::Type::PAGE_NAME:
                return new SvxPageTitleField();
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
                    bool bIsFixed = false;
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
                    bool bIsFixed = false;
                    bool bFullName = false;
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

                    sal_Int32 nPos = aContent.lastIndexOf(' ', 0);
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

                    if (!bIsFixed)
                    {
                        if (!bFullName)
                        {
                            pData->SetFormat(SVXAUTHORFORMAT_SHORTNAME);
                        }
                        else if (nFmt >= SVXAUTHORFORMAT_FULLNAME && nFmt <= SVXAUTHORFORMAT_SHORTNAME)
                        {
                            pData->SetFormat(static_cast<SvxAuthorFormat>(nFmt));
                        }
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
    } catch ( const beans::UnknownPropertyException& )
    {
        return nullptr;
    }

    return nullptr;
}


SV_IMPL_PERSIST1( SvxFieldData );


SvxFieldData::SvxFieldData()
{
}


SvxFieldData::~SvxFieldData()
{
}


SvxFieldData* SvxFieldData::Clone() const
{
    return new SvxFieldData;
}


bool SvxFieldData::operator==( const SvxFieldData& rFld ) const
{
    DBG_ASSERT( typeid(*this) == typeid(rFld), "==: Different Types" );
    (void)rFld;
    return true;    // Basic class is always the same.
}


MetaAction* SvxFieldData::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

MetaAction* SvxFieldData::createEndComment()
{
    return new MetaCommentAction( "FIELD_SEQ_END" );
}


SvxFieldItem::SvxFieldItem( const SvxFieldData& rField, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
    , mxField( rField.Clone() )
{
}


SvxFieldItem::SvxFieldItem( const SvxFieldItem& rItem ) :
    SfxPoolItem ( rItem )
    , mxField( rItem.GetField() ? rItem.GetField()->Clone() : nullptr )
{
}


SvxFieldItem::~SvxFieldItem()
{
}


SfxPoolItem* SvxFieldItem::Clone( SfxItemPool* ) const
{
    return new SvxFieldItem(*this);
}


bool SvxFieldItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const SvxFieldData* pOtherFld = static_cast<const SvxFieldItem&>(rItem).GetField();
    if( mxField.get() == pOtherFld )
        return true;
    if( mxField == nullptr || pOtherFld == nullptr )
        return false;
    return ( typeid(*mxField) == typeid(*pOtherFld) )
            && ( *mxField == *pOtherFld );
}


// The following are the derivatives of SvxFieldData ...


SV_IMPL_PERSIST1( SvxDateField );


SvxDateField::SvxDateField()
{
    nFixDate = Date( Date::SYSTEM ).GetDate();
    eType = SvxDateType::Var;
    eFormat = SVXDATEFORMAT_STDSMALL;
}


SvxDateField::SvxDateField( const Date& rDate, SvxDateType eT, SvxDateFormat eF )
{
    nFixDate = rDate.GetDate();
    eType = eT;
    eFormat = eF;
}


SvxFieldData* SvxDateField::Clone() const
{
    return new SvxDateField( *this );
}


bool SvxDateField::operator==( const SvxFieldData& rOther ) const
{
    if ( typeid(rOther) != typeid(*this) )
        return false;

    const SvxDateField& rOtherFld = static_cast<const SvxDateField&>(rOther);
    return ( ( nFixDate == rOtherFld.nFixDate ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}



OUString SvxDateField::GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLang ) const
{
    Date aDate( Date::EMPTY );
    if ( eType == SvxDateType::Fix )
        aDate.SetDate( nFixDate );
    else
        aDate = Date( Date::SYSTEM ); // current date

    return GetFormatted( aDate, eFormat, rFormatter, eLang );
}

OUString SvxDateField::GetFormatted( Date const & aDate, SvxDateFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLang )
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

    sal_uInt32 nFormatKey;

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
            nFormatKey = rFormatter.GetStandardFormat( css::util::NumberFormat::DATE, eLang );
    }

    double fDiffDate = aDate - rFormatter.GetNullDate();
    OUString aStr;
    Color* pColor = nullptr;
    rFormatter.GetOutputString( fDiffDate, nFormatKey, aStr, &pColor );
    return aStr;
}

MetaAction* SvxDateField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

SV_IMPL_PERSIST1( SvxURLField );


SvxURLField::SvxURLField()
{
    eFormat = SVXURLFORMAT_URL;
}


SvxURLField::SvxURLField( const OUString& rURL, const OUString& rRepres, SvxURLFormat eFmt )
    : aURL( rURL ), aRepresentation( rRepres )
{
    eFormat = eFmt;
}


SvxFieldData* SvxURLField::Clone() const
{
    return new SvxURLField( *this );
}


bool SvxURLField::operator==( const SvxFieldData& rOther ) const
{
    if ( typeid(rOther) != typeid(*this) )
        return false;

    const SvxURLField& rOtherFld = static_cast<const SvxURLField&>(rOther);
    return ( ( eFormat == rOtherFld.eFormat ) &&
                ( aURL == rOtherFld.aURL ) &&
                ( aRepresentation == rOtherFld.aRepresentation ) &&
                ( aTargetFrame == rOtherFld.aTargetFrame ) );
}


MetaAction* SvxURLField::createBeginComment() const
{
    // #i46618# Adding target URL to metafile comment
    return new MetaCommentAction( "FIELD_SEQ_BEGIN",
                                  0,
                                  reinterpret_cast<const sal_uInt8*>(aURL.getStr()),
                                  2*aURL.getLength() );
}

//
// SvxPageTitleField methods
//

SV_IMPL_PERSIST1( SvxPageTitleField );

SvxPageTitleField::SvxPageTitleField() {}

SvxFieldData* SvxPageTitleField::Clone() const
{
    return new SvxPageTitleField();
}

bool SvxPageTitleField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxPageTitleField *>(&rCmp) != nullptr );
}

MetaAction* SvxPageTitleField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN;PageTitleField" );
}

//
// SvxPagesField
//
// The fields that were removed from Calc:


SV_IMPL_PERSIST1( SvxPageField );

SvxPageField::SvxPageField() {}

SvxFieldData* SvxPageField::Clone() const
{
    return new SvxPageField;        // empty
}

bool SvxPageField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxPageField *>(&rCmp) != nullptr );
}

MetaAction* SvxPageField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN;PageField" );
}


SV_IMPL_PERSIST1( SvxPagesField );

SvxPagesField::SvxPagesField() {}

SvxFieldData* SvxPagesField::Clone() const
{
    return new SvxPagesField;   // empty
}

bool SvxPagesField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxPagesField *>(&rCmp) != nullptr);
}

SV_IMPL_PERSIST1( SvxTimeField );

SvxTimeField::SvxTimeField() {}

SvxFieldData* SvxTimeField::Clone() const
{
    return new SvxTimeField;    // empty
}

bool SvxTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxTimeField *>(&rCmp) != nullptr);
}

MetaAction* SvxTimeField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

SV_IMPL_PERSIST1( SvxFileField );

SvxFileField::SvxFileField() {}

SvxFieldData* SvxFileField::Clone() const
{
    return new SvxFileField;    // empty
}

bool SvxFileField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxFileField *>(&rCmp) != nullptr );
}

SV_IMPL_PERSIST1( SvxTableField );

SvxTableField::SvxTableField() : mnTab(0) {}

SvxTableField::SvxTableField(int nTab) : mnTab(nTab) {}

void SvxTableField::SetTab(int nTab)
{
    mnTab = nTab;
}


SvxFieldData* SvxTableField::Clone() const
{
    return new SvxTableField(mnTab);
}

bool SvxTableField::operator==( const SvxFieldData& rCmp ) const
{
    if (dynamic_cast<const SvxTableField *>(&rCmp) == nullptr)
        return false;

    return mnTab == static_cast<const SvxTableField&>(rCmp).mnTab;
}

//      SvxExtTimeField


SV_IMPL_PERSIST1( SvxExtTimeField );


SvxExtTimeField::SvxExtTimeField()
    : m_nFixTime( tools::Time(tools::Time::SYSTEM).GetTime() )
{
    eType = SVXTIMETYPE_VAR;
    eFormat = SVXTIMEFORMAT_STANDARD;
}


SvxExtTimeField::SvxExtTimeField( const tools::Time& rTime, SvxTimeType eT, SvxTimeFormat eF )
    : m_nFixTime( rTime.GetTime() )
{
    eType = eT;
    eFormat = eF;
}


SvxFieldData* SvxExtTimeField::Clone() const
{
    return new SvxExtTimeField( *this );
}


bool SvxExtTimeField::operator==( const SvxFieldData& rOther ) const
{
    if ( typeid(rOther) != typeid(*this) )
        return false;

    const SvxExtTimeField& rOtherFld = static_cast<const SvxExtTimeField&>(rOther);
    return ((m_nFixTime == rOtherFld.m_nFixTime) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}


OUString SvxExtTimeField::GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLang ) const
{
    tools::Time aTime( tools::Time::EMPTY );
    if ( eType == SVXTIMETYPE_FIX )
        aTime.SetTime(m_nFixTime);
    else
        aTime = tools::Time( tools::Time::SYSTEM ); // current time
    return GetFormatted( aTime, eFormat, rFormatter, eLang );
}

OUString SvxExtTimeField::GetFormatted( tools::Time const & aTime, SvxTimeFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLang )
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
            nFormatKey = rFormatter.GetStandardFormat( css::util::NumberFormat::TIME, eLang );
    }

    double fFracTime = aTime.GetTimeInDays();
    OUString aStr;
    Color* pColor = nullptr;
    rFormatter.GetOutputString( fFracTime, nFormatKey, aStr, &pColor );
    return aStr;
}

MetaAction* SvxExtTimeField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}


//      SvxExtFileField


SV_IMPL_PERSIST1( SvxExtFileField );


SvxExtFileField::SvxExtFileField()
{
    eType = SVXFILETYPE_VAR;
    eFormat = SVXFILEFORMAT_FULLPATH;
}


SvxExtFileField::SvxExtFileField( const OUString& rStr, SvxFileType eT, SvxFileFormat eF )
{
    aFile = rStr;
    eType = eT;
    eFormat = eF;
}


SvxFieldData* SvxExtFileField::Clone() const
{
    return new SvxExtFileField( *this );
}


bool SvxExtFileField::operator==( const SvxFieldData& rOther ) const
{
    if ( typeid(rOther) != typeid(*this) )
        return false;

    const SvxExtFileField& rOtherFld = static_cast<const SvxExtFileField&>(rOther);
    return ( ( aFile == rOtherFld.aFile ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}


OUString SvxExtFileField::GetFormatted() const
{
    OUString aString;

    INetURLObject aURLObj( aFile );

    if( INetProtocol::NotValid == aURLObj.GetProtocol() )
    {
        // invalid? try to interpret string as system file name
        OUString aURLStr;

        osl::FileBase::getFileURLFromSystemPath( aFile, aURLStr );

        aURLObj.SetURL( aURLStr );
    }

    // #92009# Be somewhat liberate when trying to
    // get formatted content out of the FileField
    if( INetProtocol::NotValid == aURLObj.GetProtocol() )
    {
        // still not valid? Then output as is
        aString = aFile;
    }
    else if( INetProtocol::File == aURLObj.GetProtocol() )
    {
        switch( eFormat )
        {
            case SVXFILEFORMAT_FULLPATH:
                aString = aURLObj.getFSysPath(FSysStyle::Detect);
            break;

            case SVXFILEFORMAT_PATH:
                aURLObj.removeSegment(INetURLObject::LAST_SEGMENT, false);
                // #101742# Leave trailing slash at the pathname
                aURLObj.setFinalSlash();
                aString = aURLObj.getFSysPath(FSysStyle::Detect);
            break;

            case SVXFILEFORMAT_NAME:
                aString = aURLObj.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::Unambiguous);
            break;

            case SVXFILEFORMAT_NAME_EXT:
                aString = aURLObj.getName(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::Unambiguous);
            break;
        }
    }
    else
    {
        switch( eFormat )
        {
            case SVXFILEFORMAT_FULLPATH:
                aString = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
            break;

            case SVXFILEFORMAT_PATH:
                aURLObj.removeSegment(INetURLObject::LAST_SEGMENT, false);
                // #101742# Leave trailing slash at the pathname
                aURLObj.setFinalSlash();
                aString = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
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


//      SvxAuthorField


SV_IMPL_PERSIST1( SvxAuthorField );


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


SvxFieldData* SvxAuthorField::Clone() const
{
    return new SvxAuthorField( *this );
}


bool SvxAuthorField::operator==( const SvxFieldData& rOther ) const
{
    if ( typeid(rOther) != typeid(*this) )
        return false;

    const SvxAuthorField& rOtherFld = static_cast<const SvxAuthorField&>(rOther);
    return ( ( aName == rOtherFld.aName ) &&
                ( aFirstName == rOtherFld.aFirstName ) &&
                ( aShortName == rOtherFld.aShortName ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}


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

SV_IMPL_PERSIST1( SvxHeaderField );

SvxHeaderField::SvxHeaderField() {}

SvxFieldData* SvxHeaderField::Clone() const
{
    return new SvxHeaderField;      // empty
}

bool SvxHeaderField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxHeaderField *>(&rCmp) != nullptr );
}

SV_IMPL_PERSIST1( SvxFooterField );

SvxFooterField::SvxFooterField() {}

SvxFieldData* SvxFooterField::Clone() const
{
    return new SvxFooterField;      // empty
}

bool SvxFooterField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxFooterField *>(&rCmp) != nullptr );
}

SV_IMPL_PERSIST1( SvxDateTimeField );

SvxFieldData* SvxDateTimeField::Clone() const
{
    return new SvxDateTimeField;        // empty
}

bool SvxDateTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxDateTimeField *>(&rCmp) != nullptr );
}

SvxDateTimeField::SvxDateTimeField() {}

OUString SvxDateTimeField::GetFormatted(
    Date const & rDate, tools::Time const & rTime, int eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage )
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
            aBuf.append(' ');

        aBuf.append(
            SvxExtTimeField::GetFormatted(rTime, eTimeFormat, rFormatter, eLanguage));

        aRet = aBuf.makeStringAndClear();
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
