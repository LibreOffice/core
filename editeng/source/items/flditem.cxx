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

#include <osl/file.hxx>
#include <vcl/metaact.hxx>
#include <svl/zforlist.hxx>
#include <tools/urlobj.hxx>

#include <editeng/flditem.hxx>
#include <editeng/CustomPropertyField.hxx>
#include <editeng/measfld.hxx>
#include <editeng/unonames.hxx>

#include <tools/tenccvt.hxx>
#include <tools/debug.hxx>

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
                        if (static_cast<SvxDateFormat>(nNumFmt) >= SvxDateFormat::AppDefault &&
                            static_cast<SvxDateFormat>(nNumFmt) <= SvxDateFormat::F)
                            pData->SetFormat(static_cast<SvxDateFormat>(nNumFmt));

                        return pData;
                    }

                    if (nFieldType != text::textfield::Type::TIME)
                    {
                        util::DateTime aDateTime = xPropSet->getPropertyValue(UNO_TC_PROP_DATE_TIME).get<util::DateTime>();
                        tools::Time aTime(aDateTime);

                        bool bIsFixed = false;
                        xPropSet->getPropertyValue(UNO_TC_PROP_IS_FIXED) >>= bIsFixed;

                        SvxExtTimeField* pData = new SvxExtTimeField(aTime, bIsFixed ? SvxTimeType::Fix : SvxTimeType::Var);

                        sal_Int32 nNumFmt = -1;
                        xPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                        if (static_cast<SvxTimeFormat>(nNumFmt) >= SvxTimeFormat::AppDefault &&
                            static_cast<SvxTimeFormat>(nNumFmt) <= SvxTimeFormat::HH12_MM_SS_00_AMPM)
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
                    SvxURLField* pData = new SvxURLField(aURL, aRep, aRep.isEmpty() ? SvxURLFormat::Url : SvxURLFormat::Repr);
                    pData->SetTargetFrame(aTarget);
                    if (static_cast<SvxURLFormat>(nFmt) >= SvxURLFormat::AppDefault &&
                        static_cast<SvxURLFormat>(nFmt) <= SvxURLFormat::Repr)
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

                    SvxFileFormat eFmt = SvxFileFormat::NameAndExt;
                    switch (nFmt)
                    {
                        case text::FilenameDisplayFormat::FULL: eFmt = SvxFileFormat::PathFull; break;
                        case text::FilenameDisplayFormat::PATH: eFmt = SvxFileFormat::PathOnly;     break;
                        case text::FilenameDisplayFormat::NAME: eFmt = SvxFileFormat::NameOnly;     break;
                        default:;
                    }

                    // pass fixed attribute to constructor
                    return new SvxExtFileField(
                            aPresentation, bIsFixed ? SvxFileType::Fix : SvxFileType::Var, eFmt);
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
                            aFirstName, aLastName, OUString(), bIsFixed ? SvxAuthorType::Fix : SvxAuthorType::Var);

                    if (!bIsFixed)
                    {
                        if (!bFullName)
                        {
                            pData->SetFormat(SvxAuthorFormat::ShortName);
                        }
                        else if (static_cast<SvxAuthorFormat>(nFmt) >= SvxAuthorFormat::FullName &&
                                 static_cast<SvxAuthorFormat>(nFmt) <= SvxAuthorFormat::ShortName)
                        {
                            pData->SetFormat(static_cast<SvxAuthorFormat>(nFmt));
                        }
                    }

                    return pData;
                }
            case text::textfield::Type::MEASURE:
                {
                    SdrMeasureFieldKind eKind = SdrMeasureFieldKind::Value;
                    sal_Int16 nTmp = -1;
                    xPropSet->getPropertyValue(UNO_TC_PROP_MEASURE_KIND) >>= nTmp;
                    if (nTmp == static_cast<sal_Int16>(SdrMeasureFieldKind::Unit) ||
                            nTmp == static_cast<sal_Int16>(SdrMeasureFieldKind::Rotate90Blanks))
                        eKind = static_cast<SdrMeasureFieldKind>(nTmp);

                    return new SdrMeasureField(eKind);
                }
            case text::textfield::Type::PRESENTATION_HEADER:
                return new SvxHeaderField();
            case text::textfield::Type::PRESENTATION_FOOTER:
                return new SvxFooterField();
            case text::textfield::Type::PRESENTATION_DATE_TIME:
                return new SvxDateTimeField();
            case text::textfield::Type::DOCINFO_CUSTOM:
                {
                    OUString sName;
                    xPropSet->getPropertyValue(UNO_TC_PROP_NAME) >>= sName;

                    OUString sCurrentPresentation;
                    xPropSet->getPropertyValue(UNO_TC_PROP_CURRENT_PRESENTATION) >>= sCurrentPresentation;

                    return new editeng::CustomPropertyField(sName, sCurrentPresentation);
                }
            default:
                ;
        };
    } catch ( const beans::UnknownPropertyException& )
    {
        return nullptr;
    }

    return nullptr;
}


SvxFieldData::SvxFieldData()
{
}


SvxFieldData::~SvxFieldData()
{
}


std::unique_ptr<SvxFieldData> SvxFieldData::Clone() const
{
    return std::make_unique<SvxFieldData>();
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


SvxFieldItem::SvxFieldItem( std::unique_ptr<SvxFieldData> pField, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
    , mpField( std::move(pField) )
{
}

SvxFieldItem::SvxFieldItem( const SvxFieldData& rField, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
    , mpField( rField.Clone() )
{
}


SvxFieldItem::SvxFieldItem( const SvxFieldItem& rItem ) :
    SfxPoolItem ( rItem )
    , mpField( rItem.mpField ? rItem.mpField->Clone() : nullptr )
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
    if( mpField.get() == pOtherFld )
        return true;
    if( mpField == nullptr || pOtherFld == nullptr )
        return false;
    return ( typeid(*mpField) == typeid(*pOtherFld) )
            && ( *mpField == *pOtherFld );
}


// The following are the derivatives of SvxFieldData ...


SvxDateField::SvxDateField()
{
    nFixDate = Date( Date::SYSTEM ).GetDate();
    eType = SvxDateType::Var;
    eFormat = SvxDateFormat::StdSmall;
}


SvxDateField::SvxDateField( const Date& rDate, SvxDateType eT, SvxDateFormat eF )
{
    nFixDate = rDate.GetDate();
    eType = eT;
    eFormat = eF;
}


std::unique_ptr<SvxFieldData> SvxDateField::Clone() const
{
    return std::make_unique<SvxDateField>( *this );
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
    if ( eFormat == SvxDateFormat::System )
    {
        OSL_FAIL( "SvxDateFormat::System not implemented!" );
        eFormat = SvxDateFormat::StdSmall;
    }
    else if ( eFormat == SvxDateFormat::AppDefault )
    {
        OSL_FAIL( "SvxDateFormat::AppDefault: take them from where? ");
        eFormat = SvxDateFormat::StdSmall;
    }

    sal_uInt32 nFormatKey;

    switch( eFormat )
    {
        case SvxDateFormat::StdSmall:
            // short
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLang );
        break;
        case SvxDateFormat::StdBig:
            // long
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYSTEM_LONG, eLang );
        break;
        case SvxDateFormat::A:
            // 13.02.96
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DDMMYY, eLang );
        break;
        case SvxDateFormat::B:
            // 13.02.1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DDMMYYYY, eLang );
        break;
        case SvxDateFormat::C:
            // 13. Feb 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DMMMYYYY, eLang );
        break;
        case SvxDateFormat::D:
            // 13. February 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_DMMMMYYYY, eLang );
        break;
        case SvxDateFormat::E:
            // The, 13. February 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_NNDMMMMYYYY, eLang );
        break;
        case SvxDateFormat::F:
            // Tuesday, 13. February 1996
            nFormatKey = rFormatter.GetFormatIndex( NF_DATE_SYS_NNNNDMMMMYYYY, eLang );
        break;
        default:
            nFormatKey = rFormatter.GetStandardFormat( SvNumFormatType::DATE, eLang );
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

SvxURLField::SvxURLField()
{
    eFormat = SvxURLFormat::Url;
}


SvxURLField::SvxURLField( const OUString& rURL, const OUString& rRepres, SvxURLFormat eFmt )
    : aURL( rURL ), aRepresentation( rRepres )
{
    eFormat = eFmt;
}


std::unique_ptr<SvxFieldData> SvxURLField::Clone() const
{
    return std::make_unique<SvxURLField>( *this );
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

SvxPageTitleField::SvxPageTitleField() {}

std::unique_ptr<SvxFieldData> SvxPageTitleField::Clone() const
{
    return std::make_unique<SvxPageTitleField>();
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


SvxPageField::SvxPageField() {}

std::unique_ptr<SvxFieldData> SvxPageField::Clone() const
{
    return std::make_unique<SvxPageField>();        // empty
}

bool SvxPageField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxPageField *>(&rCmp) != nullptr );
}

MetaAction* SvxPageField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN;PageField" );
}


SvxPagesField::SvxPagesField() {}

std::unique_ptr<SvxFieldData> SvxPagesField::Clone() const
{
    return std::make_unique<SvxPagesField>();   // empty
}

bool SvxPagesField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxPagesField *>(&rCmp) != nullptr);
}

SvxTimeField::SvxTimeField() {}

std::unique_ptr<SvxFieldData> SvxTimeField::Clone() const
{
    return std::make_unique<SvxTimeField>();    // empty
}

bool SvxTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxTimeField *>(&rCmp) != nullptr);
}

MetaAction* SvxTimeField::createBeginComment() const
{
    return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
}

SvxFileField::SvxFileField() {}

std::unique_ptr<SvxFieldData> SvxFileField::Clone() const
{
    return std::make_unique<SvxFileField>();    // empty
}

bool SvxFileField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxFileField *>(&rCmp) != nullptr );
}

SvxTableField::SvxTableField() : mnTab(0) {}

SvxTableField::SvxTableField(int nTab) : mnTab(nTab) {}

void SvxTableField::SetTab(int nTab)
{
    mnTab = nTab;
}


std::unique_ptr<SvxFieldData> SvxTableField::Clone() const
{
    return std::make_unique<SvxTableField>(mnTab);
}

bool SvxTableField::operator==( const SvxFieldData& rCmp ) const
{
    if (dynamic_cast<const SvxTableField *>(&rCmp) == nullptr)
        return false;

    return mnTab == static_cast<const SvxTableField&>(rCmp).mnTab;
}

//      SvxExtTimeField


SvxExtTimeField::SvxExtTimeField()
    : m_nFixTime( tools::Time(tools::Time::SYSTEM).GetTime() )
{
    eType = SvxTimeType::Var;
    eFormat = SvxTimeFormat::Standard;
}


SvxExtTimeField::SvxExtTimeField( const tools::Time& rTime, SvxTimeType eT, SvxTimeFormat eF )
    : m_nFixTime( rTime.GetTime() )
{
    eType = eT;
    eFormat = eF;
}


std::unique_ptr<SvxFieldData> SvxExtTimeField::Clone() const
{
    return std::make_unique<SvxExtTimeField>( *this );
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
    if ( eType == SvxTimeType::Fix )
        aTime.SetTime(m_nFixTime);
    else
        aTime = tools::Time( tools::Time::SYSTEM ); // current time
    return GetFormatted( aTime, eFormat, rFormatter, eLang );
}

OUString SvxExtTimeField::GetFormatted( tools::Time const & aTime, SvxTimeFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLang )
{
    switch( eFormat )
    {
        case SvxTimeFormat::System :
            OSL_FAIL( "SvxTimeFormat::System: not implemented" );
            eFormat = SvxTimeFormat::Standard;
        break;
        case SvxTimeFormat::AppDefault :
            OSL_FAIL( "SvxTimeFormat::AppDefault: not implemented" );
            eFormat = SvxTimeFormat::Standard;
        break;
        default: ;//prevent warning
    }

    sal_uInt32 nFormatKey;

    switch( eFormat )
    {
        case SvxTimeFormat::HH12_MM:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMMAMPM, eLang );
            break;
        case SvxTimeFormat::HH12_MM_SS_00:
        {
            // no builtin format available, try to insert or reuse
            OUString aFormatCode( "HH:MM:SS.00 AM/PM" );
            sal_Int32 nCheckPos;
            SvNumFormatType nType;
            rFormatter.PutandConvertEntry( aFormatCode, nCheckPos, nType,
                                           nFormatKey, LANGUAGE_ENGLISH_US, eLang, true);
            DBG_ASSERT( nCheckPos == 0, "SvxTimeFormat::HH12_MM_SS_00: could not insert format code" );
            if ( nCheckPos )
            {
                nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HH_MMSS00, eLang );
            }
            break;
        }
        case SvxTimeFormat::HH24_MM:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMM, eLang );
            break;
        case SvxTimeFormat::HH24_MM_SS_00:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HH_MMSS00, eLang );
            break;
        case SvxTimeFormat::HH12_MM_SS:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMMSSAMPM, eLang );
            break;
        case SvxTimeFormat::HH24_MM_SS:
            nFormatKey = rFormatter.GetFormatIndex( NF_TIME_HHMMSS, eLang );
            break;
        case SvxTimeFormat::Standard:
        default:
            nFormatKey = rFormatter.GetStandardFormat( SvNumFormatType::TIME, eLang );
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


SvxExtFileField::SvxExtFileField()
{
    eType = SvxFileType::Var;
    eFormat = SvxFileFormat::PathFull;
}


SvxExtFileField::SvxExtFileField( const OUString& rStr, SvxFileType eT, SvxFileFormat eF )
{
    aFile = rStr;
    eType = eT;
    eFormat = eF;
}


std::unique_ptr<SvxFieldData> SvxExtFileField::Clone() const
{
    return std::make_unique<SvxExtFileField>( *this );
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
            case SvxFileFormat::PathFull:
                aString = aURLObj.getFSysPath(FSysStyle::Detect);
            break;

            case SvxFileFormat::PathOnly:
                aURLObj.removeSegment(INetURLObject::LAST_SEGMENT, false);
                // #101742# Leave trailing slash at the pathname
                aURLObj.setFinalSlash();
                aString = aURLObj.getFSysPath(FSysStyle::Detect);
            break;

            case SvxFileFormat::NameOnly:
                aString = aURLObj.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::Unambiguous);
            break;

            case SvxFileFormat::NameAndExt:
                aString = aURLObj.getName(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::Unambiguous);
            break;
        }
    }
    else
    {
        switch( eFormat )
        {
            case SvxFileFormat::PathFull:
                aString = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
            break;

            case SvxFileFormat::PathOnly:
                aURLObj.removeSegment(INetURLObject::LAST_SEGMENT, false);
                // #101742# Leave trailing slash at the pathname
                aURLObj.setFinalSlash();
                aString = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
            break;

            case SvxFileFormat::NameOnly:
                aString = aURLObj.getBase();
            break;

            case SvxFileFormat::NameAndExt:
                aString = aURLObj.getName();
            break;
        }
    }

    return aString;
}


//      SvxAuthorField


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


std::unique_ptr<SvxFieldData> SvxAuthorField::Clone() const
{
    return std::make_unique<SvxAuthorField>( *this );
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
        case SvxAuthorFormat::FullName:
            aString = aFirstName + " " + aName;
        break;
        case SvxAuthorFormat::LastName:
            aString = aName;
        break;

        case SvxAuthorFormat::FirstName:
            aString = aFirstName;
        break;

        case SvxAuthorFormat::ShortName:
            aString = aShortName;
        break;
    }

    return aString;
}

SvxHeaderField::SvxHeaderField() {}

std::unique_ptr<SvxFieldData> SvxHeaderField::Clone() const
{
    return std::make_unique<SvxHeaderField>();      // empty
}

bool SvxHeaderField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxHeaderField *>(&rCmp) != nullptr );
}

SvxFooterField::SvxFooterField() {}

std::unique_ptr<SvxFieldData> SvxFooterField::Clone() const
{
    return std::make_unique<SvxFooterField>();      // empty
}

bool SvxFooterField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxFooterField *>(&rCmp) != nullptr );
}

std::unique_ptr<SvxFieldData> SvxDateTimeField::Clone() const
{
    return std::make_unique<SvxDateTimeField>();        // empty
}

bool SvxDateTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( dynamic_cast< const SvxDateTimeField *>(&rCmp) != nullptr );
}

SvxDateTimeField::SvxDateTimeField() {}

OUString SvxDateTimeField::GetFormatted(
    Date const & rDate, tools::Time const & rTime,
    SvxDateFormat eDateFormat, SvxTimeFormat eTimeFormat,
    SvNumberFormatter& rFormatter, LanguageType eLanguage )
{
    OUString aRet;

    if(eDateFormat != SvxDateFormat::AppDefault)
    {
        aRet = SvxDateField::GetFormatted( rDate, eDateFormat, rFormatter, eLanguage );
    }

    if(eTimeFormat != SvxTimeFormat::AppDefault)
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
