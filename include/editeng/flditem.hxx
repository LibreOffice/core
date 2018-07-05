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
#ifndef INCLUDED_EDITENG_FLDITEM_HXX
#define INCLUDED_EDITENG_FLDITEM_HXX

#include <i18nlangtag/lang.h>
#include <tools/time.hxx>
#include <tools/date.hxx>
#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

#include <com/sun/star/text/textfield/Type.hpp>

namespace com { namespace sun { namespace star { namespace text {

class XTextContent;

}}}}

class SvNumberFormatter;
class MetaAction;

// class SvxFieldItem ---------------------------------------------------


class EDITENG_DLLPUBLIC SvxFieldData
{
public:
    static const sal_Int32 UNKNOWN_FIELD;

    static SvxFieldData* Create(const css::uno::Reference<css::text::XTextContent>& xContent);

    static constexpr auto CLASS_ID = css::text::textfield::Type::UNSPECIFIED;
    virtual sal_Int32  GetClassId() const { return CLASS_ID; }

                            SvxFieldData();
    virtual                 ~SvxFieldData();

    SvxFieldData(SvxFieldData const &) = default;
    SvxFieldData(SvxFieldData &&) = default;
    SvxFieldData & operator =(SvxFieldData const &) = default;
    SvxFieldData & operator =(SvxFieldData &&) = default;

    virtual std::unique_ptr<SvxFieldData> Clone() const;
    virtual bool            operator==( const SvxFieldData& ) const;

    virtual MetaAction*     createBeginComment() const;
    static MetaAction*      createEndComment();
};

/**
 * This item stores a field (SvxFieldData).  The field is controlled by or
 * belongs to the item.  The field itself is determined by a derivation from
 * SvxFieldData (RTTI)
 */
class EDITENG_DLLPUBLIC SvxFieldItem : public SfxPoolItem
{
    std::unique_ptr<SvxFieldData>  mpField;
public:
            SvxFieldItem( std::unique_ptr<SvxFieldData> pField, const sal_uInt16 nId  );
            SvxFieldItem( const SvxFieldData& rField, const sal_uInt16 nId  );
            SvxFieldItem( const SvxFieldItem& rItem );
            virtual ~SvxFieldItem() override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const SvxFieldData*     GetField() const    { return mpField.get(); }
};


// The following are the derivatives of SvxFieldData ...


enum class SvxDateType { Fix, Var };
enum class SvxDateFormat {
    AppDefault = 0, // Set as in App
    System,         // Set as in System
    StdSmall,
    StdBig,
    A,              // 13.02.96
    B,              // 13.02.1996
    C,              // 13.Feb 1996
    D,              // 13.February 1996
    E,              // Tue, 13.February 1996
    F               // Tuesday, 13.February 1996
};

class EDITENG_DLLPUBLIC SvxDateField : public SvxFieldData
{
    sal_Int32               nFixDate;
    SvxDateType             eType;
    SvxDateFormat           eFormat;

public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::DATE;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }

                            SvxDateField();
    explicit                SvxDateField( const Date& rDate,
                                SvxDateType eType,
                                SvxDateFormat eFormat = SvxDateFormat::StdSmall );

    sal_Int32               GetFixDate() const { return nFixDate; }
    void                    SetFixDate( const Date& rDate ) { nFixDate = rDate.GetDate(); }

    SvxDateType             GetType() const { return eType; }
    void                    SetType( SvxDateType eTp ) { eType = eTp; }

    SvxDateFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxDateFormat eFmt ) { eFormat = eFmt; }

                            // If eLanguage==LANGUAGE_DONTKNOW the language/country
                            // used in number formatter initialization is taken.
    OUString                GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLanguage ) const;
    static OUString         GetFormatted( Date const & rDate, SvxDateFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};


enum class SvxURLFormat {
    AppDefault = 0, // Set as in App
    Url,            // Represent URL
    Repr            // Constitute representation
};

class EDITENG_DLLPUBLIC SvxURLField : public SvxFieldData
{
private:
    SvxURLFormat            eFormat;
    OUString                aURL;               // URL-Address
    OUString                aRepresentation;    // What is shown
    OUString                aTargetFrame;       // In what Frame

public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::URL;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }

                            SvxURLField();
                            SvxURLField( const OUString& rURL, const OUString& rRepres, SvxURLFormat eFmt = SvxURLFormat::Url );

    const OUString&         GetURL() const { return aURL; }
    void                    SetURL( const OUString& rURL ) { aURL = rURL; }

    const OUString&         GetRepresentation() const { return aRepresentation; }
    void                    SetRepresentation( const OUString& rRep ) { aRepresentation= rRep; }

    const OUString&         GetTargetFrame() const { return aTargetFrame; }
    void                    SetTargetFrame( const OUString& rFrm ) { aTargetFrame = rFrm; }

    SvxURLFormat            GetFormat() const { return eFormat; }
    void                    SetFormat( SvxURLFormat eFmt ) { eFormat = eFmt; }

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxPageField final: public SvxFieldData
{
public:
    virtual sal_Int32  GetClassId() const override { return css::text::textfield::Type::PAGE; }
    SvxPageField();

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxPageTitleField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::PAGE_NAME;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxPageTitleField();

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxPagesField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::PAGES;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxPagesField();

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

class EDITENG_DLLPUBLIC SvxTimeField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::TIME;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxTimeField();

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxFileField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::DOCINFO_TITLE;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxFileField();

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

class EDITENG_DLLPUBLIC SvxTableField final: public SvxFieldData
{
    int mnTab;
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::TABLE;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxTableField();
    explicit SvxTableField(int nTab);

    void SetTab(int nTab);
    int GetTab() const { return mnTab;}

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};


enum class SvxTimeType {
    Fix,
    Var
};
enum class SvxTimeFormat {
    AppDefault = 0,    // Set as in App
    System,            // Set as in System
    Standard,
    HH24_MM,           // 13:49
    HH24_MM_SS,        // 13:49:38
    HH24_MM_SS_00,     // 13:49:38.78
    HH12_MM,           // 01:49
    HH12_MM_SS,        // 01:49:38
    HH12_MM_SS_00,     // 01:49:38.78
    HH12_MM_AMPM,      // 01:49 PM
    HH12_MM_SS_AMPM,   // 01:49:38 PM
    HH12_MM_SS_00_AMPM // 01:49:38.78 PM
};

class EDITENG_DLLPUBLIC SvxExtTimeField : public SvxFieldData
{
private:
    sal_Int64               m_nFixTime;
    SvxTimeType             eType;
    SvxTimeFormat           eFormat;

public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::EXTENDED_TIME;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
                            SvxExtTimeField();
    explicit                SvxExtTimeField( const tools::Time& rTime,
                                SvxTimeType eType,
                                SvxTimeFormat eFormat = SvxTimeFormat::Standard );

    sal_Int64               GetFixTime() const { return m_nFixTime; }
    void                    SetFixTime( const tools::Time& rTime ) { m_nFixTime = rTime.GetTime(); }

    SvxTimeType             GetType() const { return eType; }
    void                    SetType( SvxTimeType eTp ) { eType = eTp; }

    SvxTimeFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxTimeFormat eFmt ) { eFormat = eFmt; }

                            // If eLanguage==LANGUAGE_DONTKNOW the language/country
                            // used in number formatter initialization is taken.
    OUString                GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLanguage ) const;
    static OUString         GetFormatted( tools::Time const & rTime, SvxTimeFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};


enum class SvxFileType {
    Fix,
    Var
};
enum class SvxFileFormat {
    NameAndExt = 0, // File name with Extension
    PathFull,       // full path
    PathOnly,       // only path
    NameOnly        // only file name
};


class EDITENG_DLLPUBLIC SvxExtFileField : public SvxFieldData
{
private:
    OUString           aFile;
    SvxFileType             eType;
    SvxFileFormat           eFormat;

public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::EXTENDED_FILE;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
                            SvxExtFileField();
    explicit                SvxExtFileField( const OUString& rString,
                                SvxFileType eType = SvxFileType::Var,
                                SvxFileFormat eFormat = SvxFileFormat::PathFull );

    const OUString&         GetFile() const { return aFile; }
    void                    SetFile( const OUString& rString ) { aFile = rString; }

    SvxFileType             GetType() const { return eType; }
    void                    SetType( SvxFileType eTp ) { eType = eTp; }

    SvxFileFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxFileFormat eFmt ) { eFormat = eFmt; }

    OUString                GetFormatted() const;

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};


enum class SvxAuthorType {
    Fix, Var
};
enum class SvxAuthorFormat {
    FullName,   // full name
    LastName,   // Only Last name
    FirstName,  // Only first name
    ShortName   // Initials
};

class EDITENG_DLLPUBLIC SvxAuthorField : public SvxFieldData
{
private:
    OUString   aName;
    OUString   aFirstName;
    OUString   aShortName;
    SvxAuthorType   eType;
    SvxAuthorFormat eFormat;

public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::AUTHOR;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
                            SvxAuthorField(
                                const OUString& rFirstName,
                                const OUString& rLastName,
                                const OUString& rShortName,
                                SvxAuthorType eType = SvxAuthorType::Var,
                                SvxAuthorFormat eFormat = SvxAuthorFormat::FullName );

    SvxAuthorType           GetType() const { return eType; }
    void                    SetType( SvxAuthorType eTp ) { eType = eTp; }

    SvxAuthorFormat         GetFormat() const { return eFormat; }
    void                    SetFormat( SvxAuthorFormat eFmt ) { eFormat = eFmt; }

    OUString                GetFormatted() const;

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class EDITENG_DLLPUBLIC SvxHeaderField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::PRESENTATION_HEADER;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxHeaderField();

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class EDITENG_DLLPUBLIC SvxFooterField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::PRESENTATION_FOOTER;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxFooterField();
    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class EDITENG_DLLPUBLIC SvxDateTimeField final: public SvxFieldData
{
public:
    static constexpr auto CLASS_ID = css::text::textfield::Type::PRESENTATION_DATE_TIME;
    virtual sal_Int32  GetClassId() const override { return CLASS_ID; }
    SvxDateTimeField();

    static OUString    GetFormatted( Date const & rDate, tools::Time const & rTime,
                                     SvxDateFormat eDateFormat, SvxTimeFormat eTimeFormat,
                                     SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
