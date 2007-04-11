/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flditem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:49:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_FLDITEM_HXX
#define _SVX_FLDITEM_HXX

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _PSTM_HXX //autogen
#include <tools/pstm.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvNumberFormatter;
class MetaAction;

// class SvxFieldItem ---------------------------------------------------

#ifdef ITEMID_FIELD

class SVX_DLLPUBLIC SvxFieldData : public SvPersistBase
{
public:
                            SV_DECL_PERSIST1( SvxFieldData, SvPersistBase, 1 )

                            SvxFieldData();
    virtual                 ~SvxFieldData();

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;

    virtual MetaAction* createBeginComment() const;
    virtual MetaAction* createEndComment() const;
};

/*
[Beschreibung]
In diesem Item wird ein Feld (SvxFieldData) gespeichert.
Das Feld gehoert dem Item.
Das Feld selbst wird durch eine Ableitung von SvxFieldData bestimmt. (RTTI)
*/

class SVX_DLLPUBLIC SvxFieldItem : public SfxPoolItem
{
private:
    SvxFieldData*           pField;

                    SVX_DLLPRIVATE SvxFieldItem( SvxFieldData* pField, const USHORT nId );

public:
            TYPEINFO();

            SvxFieldItem( const SvxFieldData& rField, const USHORT nId = ITEMID_FIELD );
            SvxFieldItem( const SvxFieldItem& rItem );
            ~SvxFieldItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& , USHORT nItemVersion ) const;

    const SvxFieldData*     GetField() const    { return pField; }
    static SvClassManager&  GetClassManager();
};

// =================================================================
// Es folgen die Ableitungen von SvxFieldData...
// =================================================================

#define SVX_DATEFIELD       2
#define SVX_URLFIELD        3

#define SVX_PAGEFIELD       100     // Ex-Calc-Felder
#define SVX_PAGESFIELD      101
#define SVX_TIMEFIELD       102
#define SVX_FILEFIELD       103
#define SVX_TABLEFIELD      104
#define SVX_EXT_TIMEFIELD   105
#define SVX_EXT_FILEFIELD   106
#define SVX_AUTHORFIELD     107
#define SVX_HEADERFIELD     108
#define SVX_FOOTERFIELD     109
#define SVX_DATEFIMEFIELD   110

#define SVX_USERFIELD       200 // Ab hier eigene Felder, nicht im SVX

enum SvxDateType { SVXDATETYPE_FIX, SVXDATETYPE_VAR };
enum SvxDateFormat {    SVXDATEFORMAT_APPDEFAULT,   // Wie in App eingestellt
                        SVXDATEFORMAT_SYSTEM,       // Wie im System eingestellt
                        SVXDATEFORMAT_STDSMALL,
                        SVXDATEFORMAT_STDBIG,
                        SVXDATEFORMAT_A,    // 13.02.96
                        SVXDATEFORMAT_B,    // 13.02.1996
                        SVXDATEFORMAT_C,    // 13.Feb 1996
                        SVXDATEFORMAT_D,    // 13.Februar 1996
                        SVXDATEFORMAT_E,    // Die, 13.Februar 1996
                        SVXDATEFORMAT_F     // Dienstag, 13.Februar 1996
                    };

class SVX_DLLPUBLIC SvxDateField : public SvxFieldData
{
    sal_uInt32              nFixDate;
    SvxDateType             eType;
    SvxDateFormat           eFormat;

public:
                            SV_DECL_PERSIST1( SvxDateField, SvxFieldData, SVX_DATEFIELD )

                            SvxDateField();
                            SvxDateField( const Date& rDate,
                                SvxDateType eType = SVXDATETYPE_VAR,
                                SvxDateFormat eFormat = SVXDATEFORMAT_STDSMALL );

    sal_uInt32              GetFixDate() const { return nFixDate; }
    void                    SetFixDate( const Date& rDate ) { nFixDate = rDate.GetDate(); }

    SvxDateType             GetType() const { return eType; }
    void                    SetType( SvxDateType eTp ) { eType = eTp; }

    SvxDateFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxDateFormat eFmt ) { eFormat = eFmt; }

                            // deprecated, to be removed
    String                  GetFormatted( LanguageType eLanguage, LanguageType eFormat ) const;
                            // use this instead
                            // If eLanguage==LANGUAGE_DONTKNOW the language/country
                            // used in number formatter initialization is taken.
    String                  GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLanguage ) const;
    static String           GetFormatted( Date& rDate, SvxDateFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;

    virtual MetaAction* createBeginComment() const;
};


enum SvxURLFormat   {   SVXURLFORMAT_APPDEFAULT,    // Wie in App eingestellt
                        SVXURLFORMAT_URL,           // URL darstellen
                        SVXURLFORMAT_REPR           // Repraesentation darstellen
                    };

class SVX_DLLPUBLIC SvxURLField : public SvxFieldData
{
private:
    SvxURLFormat            eFormat;
    XubString               aURL;               // URL-Adresse
    XubString               aRepresentation;    // Was wird dargestellt
    XubString               aTargetFrame;       // In welchem Frame

public:
                            SV_DECL_PERSIST1( SvxURLField, SvxFieldData, SVX_URLFIELD )

                            SvxURLField();
                            SvxURLField( const XubString& rURL, const XubString& rRepres, SvxURLFormat eFmt = SVXURLFORMAT_URL );

    const XubString&            GetURL() const { return aURL; }
    void                    SetURL( const XubString& rURL ) { aURL = rURL; }

    const XubString&            GetRepresentation() const { return aRepresentation; }
    void                    SetRepresentation( const XubString& rRep ) { aRepresentation= rRep; }

    const XubString&            GetTargetFrame() const { return aTargetFrame; }
    void                    SetTargetFrame( const XubString& rFrm ) { aTargetFrame = rFrm; }

    SvxURLFormat            GetFormat() const { return eFormat; }
    void                    SetFormat( SvxURLFormat eFmt ) { eFormat = eFmt; }

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;

    virtual MetaAction* createBeginComment() const;
};

class SVX_DLLPUBLIC SvxPageField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxPageField, SvxFieldData, SVX_PAGEFIELD )
                            SvxPageField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;

    virtual MetaAction* createBeginComment() const;
};

class SVX_DLLPUBLIC SvxPagesField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxPagesField, SvxFieldData, SVX_PAGESFIELD )
                            SvxPagesField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};

class SVX_DLLPUBLIC SvxTimeField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxTimeField, SvxFieldData, SVX_TIMEFIELD )
                            SvxTimeField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;

    virtual MetaAction* createBeginComment() const;
};

class SVX_DLLPUBLIC SvxFileField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxFileField, SvxFieldData, SVX_FILEFIELD )
                            SvxFileField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};

class SVX_DLLPUBLIC SvxTableField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxTableField, SvxFieldData, SVX_TABLEFIELD )
                            SvxTableField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};


enum SvxTimeType { SVXTIMETYPE_FIX, SVXTIMETYPE_VAR };
enum SvxTimeFormat {    SVXTIMEFORMAT_APPDEFAULT,   // Wie in App eingestellt
                        SVXTIMEFORMAT_SYSTEM,       // Wie im System eingestellt
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

class SVX_DLLPUBLIC SvxExtTimeField : public SvxFieldData
{
private:
    sal_uInt32              nFixTime;
    SvxTimeType             eType;
    SvxTimeFormat           eFormat;

public:
                            SV_DECL_PERSIST1( SvxExtTimeField, SvxFieldData, SVX_EXT_TIMEFIELD )
                            SvxExtTimeField();
                            SvxExtTimeField( const Time& rTime,
                                SvxTimeType eType = SVXTIMETYPE_VAR,
                                SvxTimeFormat eFormat = SVXTIMEFORMAT_STANDARD );

    sal_uInt32              GetFixTime() const { return nFixTime; }
    void                    SetFixTime( const Time& rTime ) { nFixTime = rTime.GetTime(); }

    SvxTimeType             GetType() const { return eType; }
    void                    SetType( SvxTimeType eTp ) { eType = eTp; }

    SvxTimeFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxTimeFormat eFmt ) { eFormat = eFmt; }

                            // deprecated, to be removed
    XubString               GetFormatted( LanguageType eLanguage, LanguageType eFormat ) const;
                            // use this instead
                            // If eLanguage==LANGUAGE_DONTKNOW the language/country
                            // used in number formatter initialization is taken.
    String                  GetFormatted( SvNumberFormatter& rFormatter, LanguageType eLanguage ) const;
    static String           GetFormatted( Time& rTime, SvxTimeFormat eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;

    virtual MetaAction* createBeginComment() const;
};


enum SvxFileType { SVXFILETYPE_FIX, SVXFILETYPE_VAR };
enum SvxFileFormat {    SVXFILEFORMAT_NAME_EXT, // Dateiname mit Extension
                        SVXFILEFORMAT_FULLPATH, // vollst„ndiger Pfad
                        SVXFILEFORMAT_PATH,     // nur Pfad
                        SVXFILEFORMAT_NAME      // nur Dateiname
                   };



class SVX_DLLPUBLIC SvxExtFileField : public SvxFieldData
{
private:
    XubString               aFile;
    SvxFileType             eType;
    SvxFileFormat           eFormat;

public:
                            SV_DECL_PERSIST1( SvxExtFileField, SvxFieldData, SVX_EXT_FILEFIELD )
                            SvxExtFileField();
                            SvxExtFileField( const XubString& rString,
                                SvxFileType eType = SVXFILETYPE_VAR,
                                SvxFileFormat eFormat = SVXFILEFORMAT_FULLPATH );

    String                  GetFile() const { return aFile; }
    void                    SetFile( const XubString& rString ) { aFile = rString; }

    SvxFileType             GetType() const { return eType; }
    void                    SetType( SvxFileType eTp ) { eType = eTp; }

    SvxFileFormat           GetFormat() const { return eFormat; }
    void                    SetFormat( SvxFileFormat eFmt ) { eFormat = eFmt; }

    XubString               GetFormatted() const;

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};


enum SvxAuthorType { SVXAUTHORTYPE_FIX, SVXAUTHORTYPE_VAR };
enum SvxAuthorFormat {  SVXAUTHORFORMAT_FULLNAME,   // vollst„ndiger Name
                        SVXAUTHORFORMAT_NAME,       // nur Nachname
                        SVXAUTHORFORMAT_FIRSTNAME,  // nur Vorname
                        SVXAUTHORFORMAT_SHORTNAME   // Initialen
                    };

class SVX_DLLPUBLIC SvxAuthorField : public SvxFieldData
{
private:
    XubString       aName;
    XubString       aFirstName;
    XubString       aShortName;
    SvxAuthorType   eType;
    SvxAuthorFormat eFormat;

public:
                            SV_DECL_PERSIST1( SvxAuthorField, SvxFieldData, SVX_AUTHORFIELD )
                            SvxAuthorField();
                            SvxAuthorField(
                                const XubString& rFirstName,
                                const XubString& rLastName,
                                const XubString& rShortName,
                                SvxAuthorType eType = SVXAUTHORTYPE_VAR,
                                SvxAuthorFormat eFormat = SVXAUTHORFORMAT_FULLNAME );

    XubString               GetName() const { return aName; }
    void                    SetName( const XubString& rString ) { aName = rString; }

    XubString               GetFirstName() const { return aFirstName; }
    void                    SetFirstName( const XubString& rString ) { aFirstName = rString; }

    XubString               GetShortName() const { return aShortName; }
    void                    SetShortName( const XubString& rString ) { aShortName = rString; }

    SvxAuthorType           GetType() const { return eType; }
    void                    SetType( SvxAuthorType eTp ) { eType = eTp; }

    SvxAuthorFormat         GetFormat() const { return eFormat; }
    void                    SetFormat( SvxAuthorFormat eFmt ) { eFormat = eFmt; }

    XubString               GetFormatted() const;

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class SVX_DLLPUBLIC SvxHeaderField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxHeaderField, SvxFieldData, SVX_HEADERFIELD )
                            SvxHeaderField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class SVX_DLLPUBLIC SvxFooterField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxFooterField, SvxFieldData, SVX_FOOTERFIELD )
                            SvxFooterField() {}
    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};

/** this field is used as a placeholder for a header&footer in impress. The actual
    value is stored at the page */
class SVX_DLLPUBLIC SvxDateTimeField : public SvxFieldData
{
public:
                            SV_DECL_PERSIST1( SvxDateTimeField, SvxFieldData, SVX_DATEFIMEFIELD )
                            SvxDateTimeField() {}

    static String           GetFormatted( Date& rDate, Time& rTime, int eFormat, SvNumberFormatter& rFormatter, LanguageType eLanguage );

    virtual SvxFieldData*   Clone() const;
    virtual int             operator==( const SvxFieldData& ) const;
};

#endif


#endif

