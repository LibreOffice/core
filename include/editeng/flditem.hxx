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
#include <tools/pstm.hxx>
#include <editeng/editengdllapi.h>

#include <com/sun/star/text/textfield/Type.hpp>

namespace com { namespace sun { namespace star { namespace text {

class XTextContent;

}}}}

class SvNumberFormatter;
class MetaAction;

// class SvxFieldItem ---------------------------------------------------



class EDITENG_DLLPUBLIC SvxFieldData : public SvPersistBase
{
public:
    static const sal_Int32 UNKNOWN_FIELD;

    static SvxFieldData* Create(const css::uno::Reference<css::text::XTextContent>& xContent);

    SV_DECL_PERSIST1( SvxFieldData, SvPersistBase, css::text::textfield::Type::UNSPECIFIED)

                            SvxFieldData();
    virtual                 ~SvxFieldData();

    virtual SvxFieldData*   Clone() const;
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
private:
    SvxFieldData*           pField;

                    EDITENG_DLLPRIVATE SvxFieldItem( SvxFieldData* pField, const sal_uInt16 nId );

public:
            TYPEINFO_OVERRIDE();

            SvxFieldItem( const SvxFieldData& rField, const sal_uInt16 nId  );
            SvxFieldItem( const SvxFieldItem& rItem );
            virtual ~SvxFieldItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nVer ) const override;
    virtual SvStream&       Store( SvStream& , sal_uInt16 nItemVersion ) const override;

    const SvxFieldData*     GetField() const    { return pField; }
    static SvClassManager&  GetClassManager();
};


// The following are the derivatives of SvxFieldData ...


enum SvxDateType { SVXDATETYPE_FIX, SVXDATETYPE_VAR };
enum SvxDateFormat {    SVXDATEFORMAT_APPDEFAULT = 0,   // Set as in App
                        SVXDATEFORMAT_SYSTEM,       // Set as in System
                        SVXDATEFORMAT_STDSMALL,
                        SVXDATEFORMAT_STDBIG,
                        SVXDATEFORMAT_A,    // 13.02.96
                        SVXDATEFORMAT_B,    // 13.02.1996
                        SVXDATEFORMAT_C,    // 13.Feb 1996
                        SVXDATEFORMAT_D,    // 13.February 1996
                        SVXDATEFORMAT_E,    // Tue, 13.February 1996
                        SVXDATEFORMAT_F     // Tuesday, 13.February 1996
                    };

class EDITENG_DLLPUBLIC SvxDateField : public SvxFieldData
{
    sal_uInt32              nFixDate;
    SvxDateType             eType;
    SvxDateFormat           eFormat;

public:
    SV_DECL_PERSIST1( SvxDateField, SvxFieldData, css::text::textfield::Type::DATE )

                            SvxDateField();
    explicit                SvxDateField( const Date& rDate,
                                SvxDateType eType = SVXDATETYPE_VAR,
                                SvxDateFormat eFormat = SVXDATEFORMAT_STDSMALL );

    sal_uInt32              GetFixDate() const { return nFixDate; }
    void                    SetFixDate( const Date& rDate ) { nFixDate = rDate.GetDate(); }

    SvxDateType             GetType() const { return eType; }
    void                    SetType( SvxDateType eTp ) { eType = eTp; }

    SvxDateFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxDateFormat eFmt ) { eFormat = eFmt; }

                            // If eLanguage==LANGUAGE_DONTKNOW the language/country
                            // used in number formatter initialization is taken.
    OUString           GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLanguage ) const;
    static OUString    GetFormatted( Date& rDate, SvxDateFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};


enum SvxURLFormat   {   SVXURLFORMAT_APPDEFAULT = 0,    // Set as in App
                        SVXURLFORMAT_URL,           // Represent URL
                        SVXURLFORMAT_REPR           // Constitute repraesentation
                    };

class EDITENG_DLLPUBLIC SvxURLField : public SvxFieldData
{
private:
    SvxURLFormat            eFormat;
    OUString           aURL;               // URL-Address
    OUString           aRepresentation;    // What is shown
    OUString           aTargetFrame;       // In what Frame

public:
    SV_DECL_PERSIST1( SvxURLField, SvxFieldData, css::text::textfield::Type::URL )

                            SvxURLField();
                            SvxURLField( const OUString& rURL, const OUString& rRepres, SvxURLFormat eFmt = SVXURLFORMAT_URL );

    const OUString&    GetURL() const { return aURL; }
    void                    SetURL( const OUString& rURL ) { aURL = rURL; }

    const OUString&    GetRepresentation() const { return aRepresentation; }
    void                    SetRepresentation( const OUString& rRep ) { aRepresentation= rRep; }

    const OUString&    GetTargetFrame() const { return aTargetFrame; }
    void                    SetTargetFrame( const OUString& rFrm ) { aTargetFrame = rFrm; }

    SvxURLFormat            GetFormat() const { return eFormat; }
    void                    SetFormat( SvxURLFormat eFmt ) { eFormat = eFmt; }

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxPageField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxPageField, SvxFieldData, css::text::textfield::Type::PAGE )
    SvxPageField();

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxPageTitleField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxPageTitleField, SvxFieldData, css::text::textfield::Type::PAGE_NAME )
    SvxPageTitleField();

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxPagesField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxPagesField, SvxFieldData, css::text::textfield::Type::PAGES )
    SvxPagesField();

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

class EDITENG_DLLPUBLIC SvxTimeField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxTimeField, SvxFieldData, css::text::textfield::Type::TIME )
    SvxTimeField();

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};

class EDITENG_DLLPUBLIC SvxFileField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxFileField, SvxFieldData, css::text::textfield::Type::DOCINFO_TITLE )
    SvxFileField();

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

class EDITENG_DLLPUBLIC SvxTableField : public SvxFieldData
{
    int mnTab;
public:
    SV_DECL_PERSIST1( SvxTableField, SvxFieldData, css::text::textfield::Type::TABLE )
    SvxTableField();
    explicit SvxTableField(int nTab);

    void SetTab(int nTab);
    int GetTab() const { return mnTab;}

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};


enum SvxTimeType { SVXTIMETYPE_FIX, SVXTIMETYPE_VAR };
enum SvxTimeFormat {    SVXTIMEFORMAT_APPDEFAULT = 0,   // Set as in App
                        SVXTIMEFORMAT_SYSTEM,       // Set as in System
                        SVXTIMEFORMAT_STANDARD,
                        SVXTIMEFORMAT_24_HM,    // 13:49
                        SVXTIMEFORMAT_24_HMS,   // 13:49:38
                        SVXTIMEFORMAT_24_HMSH,  // 13:49:38.78
                        SVXTIMEFORMAT_12_HM,    // 01:49
                        SVXTIMEFORMAT_12_HMS,   // 01:49:38
                        SVXTIMEFORMAT_12_HMSH,  // 01:49:38.78
                        SVXTIMEFORMAT_AM_HM,    // 01:49 PM
                        SVXTIMEFORMAT_AM_HMS,   // 01:49:38 PM
                        SVXTIMEFORMAT_AM_HMSH   // 01:49:38.78 PM
                    };

class EDITENG_DLLPUBLIC SvxExtTimeField : public SvxFieldData
{
private:
    sal_Int64               m_nFixTime;
    SvxTimeType             eType;
    SvxTimeFormat           eFormat;

public:
    SV_DECL_PERSIST1( SvxExtTimeField, SvxFieldData, css::text::textfield::Type::EXTENDED_TIME )
                            SvxExtTimeField();
    explicit                SvxExtTimeField( const tools::Time& rTime,
                                SvxTimeType eType = SVXTIMETYPE_VAR,
                                SvxTimeFormat eFormat = SVXTIMEFORMAT_STANDARD );

    sal_Int64               GetFixTime() const { return m_nFixTime; }
    void SetFixTime( const tools::Time& rTime ) { m_nFixTime = rTime.GetTime(); }

    SvxTimeType             GetType() const { return eType; }
    void                    SetType( SvxTimeType eTp ) { eType = eTp; }

    SvxTimeFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxTimeFormat eFmt ) { eFormat = eFmt; }

                            // If eLanguage==LANGUAGE_DONTKNOW the language/country
                            // used in number formatter initialization is taken.
    OUString           GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLanguage ) const;
    static OUString    GetFormatted( tools::Time& rTime, SvxTimeFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;

    virtual MetaAction* createBeginComment() const override;
};


enum SvxFileType { SVXFILETYPE_FIX, SVXFILETYPE_VAR };
enum SvxFileFormat {    SVXFILEFORMAT_NAME_EXT = 0, // File name with Extension
                        SVXFILEFORMAT_FULLPATH, // full path
                        SVXFILEFORMAT_PATH,     // only path
                        SVXFILEFORMAT_NAME      // only file name
                   };



class EDITENG_DLLPUBLIC SvxExtFileField : public SvxFieldData
{
private:
    OUString           aFile;
    SvxFileType             eType;
    SvxFileFormat           eFormat;

public:
    SV_DECL_PERSIST1( SvxExtFileField, SvxFieldData, css::text::textfield::Type::EXTENDED_FILE )
                            SvxExtFileField();
    explicit                SvxExtFileField( const OUString& rString,
                                SvxFileType eType = SVXFILETYPE_VAR,
                                SvxFileFormat eFormat = SVXFILEFORMAT_FULLPATH );

    OUString           GetFile() const { return aFile; }
    void                    SetFile( const OUString& rString ) { aFile = rString; }

    SvxFileType             GetType() const { return eType; }
    void                    SetType( SvxFileType eTp ) { eType = eTp; }

    SvxFileFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxFileFormat eFmt ) { eFormat = eFmt; }

    OUString           GetFormatted() const;

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};


enum SvxAuthorType { SVXAUTHORTYPE_FIX, SVXAUTHORTYPE_VAR };
enum SvxAuthorFormat {  SVXAUTHORFORMAT_FULLNAME = 0, // full name
                        SVXAUTHORFORMAT_NAME,       // Only Last name
                        SVXAUTHORFORMAT_FIRSTNAME,  // Only first name
                        SVXAUTHORFORMAT_SHORTNAME   // Initials
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
    SV_DECL_PERSIST1( SvxAuthorField, SvxFieldData, css::text::textfield::Type::AUTHOR )
                            SvxAuthorField();
                            SvxAuthorField(
                                const OUString& rFirstName,
                                const OUString& rLastName,
                                const OUString& rShortName,
                                SvxAuthorType eType = SVXAUTHORTYPE_VAR,
                                SvxAuthorFormat eFormat = SVXAUTHORFORMAT_FULLNAME );

    SvxAuthorType           GetType() const { return eType; }
    void                    SetType( SvxAuthorType eTp ) { eType = eTp; }

    SvxAuthorFormat         GetFormat() const { return eFormat; }
    void                    SetFormat( SvxAuthorFormat eFmt ) { eFormat = eFmt; }

    OUString                GetFormatted() const;

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class EDITENG_DLLPUBLIC SvxHeaderField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxHeaderField, SvxFieldData, css::text::textfield::Type::PRESENTATION_HEADER )
    SvxHeaderField();

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class EDITENG_DLLPUBLIC SvxFooterField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxFooterField, SvxFieldData, css::text::textfield::Type::PRESENTATION_FOOTER )
    SvxFooterField();
    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class EDITENG_DLLPUBLIC SvxDateTimeField : public SvxFieldData
{
public:
    SV_DECL_PERSIST1( SvxDateTimeField, SvxFieldData, css::text::textfield::Type::PRESENTATION_DATE_TIME )
    SvxDateTimeField();

    static OUString    GetFormatted( Date& rDate, tools::Time& rTime, int eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual SvxFieldData*   Clone() const override;
    virtual bool            operator==( const SvxFieldData& ) const override;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
