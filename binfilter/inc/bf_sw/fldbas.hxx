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
#ifndef _FLDBAS_HXX
#define _FLDBAS_HXX

#ifndef FIELDIDS_ONLY		// SWG-Testreader!!

#include <i18npool/lang.h>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <calbck.hxx>

#include <com/sun/star/uno/Any.hxx>
class Date; 
class Time; 

namespace binfilter {

class SvStringsDtor; 
class SvNumberFormatter; 
class SwDoc;
class _SetGetExpFlds;
class SwEditShell;
} //namespace binfilter
#endif		// FIELDIDS_ONLY

/*--------------------------------------------------------------------
    Beschreibung: 	die ID's fuer die Feldtypen
 --------------------------------------------------------------------*/
namespace binfilter {//STRIP009
enum RES_FIELDS
{
// Fuer die alten Dokumente muessen die Field-Which IDS erhalten bleiben !!!!
    RES_FIELDS_BEGIN,
    RES_DBFLD = RES_FIELDS_BEGIN,
    RES_USERFLD,
    RES_FILENAMEFLD,
    RES_DBNAMEFLD,
    RES_DATEFLD,
    RES_TIMEFLD,
    RES_PAGENUMBERFLD,
    RES_AUTHORFLD,
    RES_CHAPTERFLD,
    RES_DOCSTATFLD,
    RES_GETEXPFLD,
    RES_SETEXPFLD,
    RES_GETREFFLD,
    RES_HIDDENTXTFLD,
    RES_POSTITFLD,
    RES_FIXDATEFLD,
    RES_FIXTIMEFLD,
    RES_REGFLD,
    RES_VARREGFLD,
    RES_SETREFFLD,
    RES_INPUTFLD,
    RES_MACROFLD,
    RES_DDEFLD,
    RES_TABLEFLD,
    RES_HIDDENPARAFLD,
    RES_DOCINFOFLD,
    RES_TEMPLNAMEFLD,
    RES_DBNEXTSETFLD,
    RES_DBNUMSETFLD,
    RES_DBSETNUMBERFLD,
    RES_EXTUSERFLD,
    RES_REFPAGESETFLD,
    RES_REFPAGEGETFLD,
    RES_INTERNETFLD,
    RES_JUMPEDITFLD,
    RES_SCRIPTFLD,
    RES_DATETIMEFLD,
    RES_AUTHORITY,	//Table of authorities
    RES_COMBINED_CHARS,
    RES_DROPDOWN,
    RES_FIELDS_END
};

/*--------------------------------------------------------------------
    Beschreibung: Liste der FieldTypes am UI
 --------------------------------------------------------------------*/

enum SwFldTypesEnum
{
    TYP_BEGIN,
    TYP_DATEFLD = TYP_BEGIN,	// 0
    TYP_TIMEFLD,
    TYP_FILENAMEFLD,
    TYP_DBNAMEFLD,
    TYP_CHAPTERFLD,
    TYP_PAGENUMBERFLD,
    TYP_DOCSTATFLD,
    TYP_AUTHORFLD,
    TYP_SETFLD,
    TYP_GETFLD,
    TYP_FORMELFLD,  			// 10
    TYP_HIDDENTXTFLD,
    TYP_SETREFFLD,
    TYP_GETREFFLD,
    TYP_DDEFLD,
    TYP_MACROFLD,
    TYP_INPUTFLD,
    TYP_HIDDENPARAFLD,
    TYP_DOCINFOFLD,
    TYP_DBFLD,
    TYP_USERFLD,				// 20
    TYP_POSTITFLD,
    TYP_TEMPLNAMEFLD,
    TYP_SEQFLD,
    TYP_DBNEXTSETFLD,
    TYP_DBNUMSETFLD,
    TYP_DBSETNUMBERFLD,
    TYP_CONDTXTFLD,
    TYP_NEXTPAGEFLD,
    TYP_PREVPAGEFLD,
    TYP_EXTUSERFLD,				// 30
    TYP_FIXDATEFLD,
    TYP_FIXTIMEFLD,
    TYP_SETINPFLD,
    TYP_USRINPFLD,
    TYP_SETREFPAGEFLD,
    TYP_GETREFPAGEFLD,
    TYP_INTERNETFLD,
    TYP_JUMPEDITFLD,
    TYP_SCRIPTFLD,				// 40
    TYP_AUTHORITY,
    TYP_COMBINED_CHARS,
    TYP_DROPDOWN,
    TYP_END
};


enum SwFileNameFormat
{
    FF_BEGIN,
    FF_NAME = FF_BEGIN,
    FF_PATHNAME,
    FF_PATH,
    FF_NAME_NOEXT,
    FF_UI_NAME,
    FF_UI_RANGE,
    FF_END,
    FF_FIXED = 0x8000
};

enum SwVarFormat
{
    VVF_CMD			= 0x0010,	// Kommando anzeigen
    VVF_INVISIBLE   = 0x0040,   // unsichtbar
    VVF_XXP   		= 0x0400,   // 1234%
    VVF_XX_XXP   	= 0x0800,   // 1.234,56%
    VVF_CLEAR		= 0x000f,

// ab hier neue Formate
    VVF_SYS	 		= 0x2000, 	//Zahlenformat aus der Systemeinstellung
    VVF_X			= 0x2100,	// 1234
    VVF_X_X         = 0x2200,   // 1234,5
    VVF_X_XX		= 0x2300,   // 1245,56
    VVF_XX_X 		= 0x2400,   // 1.234,5
    VVF_XX_XX		= 0x2500,   // 1.234,56
    VVF_XX_XXX		= 0x2600,   // 1.234,567
    VVF_SYS_CUR		= 0x2700,	// W„hrungsformat aus der Systemeinstellung
    VVF_CUR_X		= 0x2800,   // DM 1234
    VVF_CUR_XX_XX	= 0x2900,   // DM 1234,56 DM 1234,00
    VVF_CUR_XX_X0 	= 0x2a00,   // DM 1234,56 DM 1234,--
    VVF_X_CUR		= 0x2b00,   // 1234 DM
    VVF_XX_XX_CUR	= 0x2c00,   // 1234,56 DM 1234,00 DM
    VVF_XX_X0_CUR	= 0x2d00,    // 1234,56 DM 1234,-- DM
// Kompatibilitaet:
    VF_CMD			= VVF_CMD,
    VF_INVISIBLE    = VVF_INVISIBLE,
    VF_XXP   		= VVF_XXP,
    VF_XX_XXP   	= VVF_XX_XXP,
    VF_VISIBLE		= VVF_SYS,
    VF_XX   		= VVF_X,
    VF_XX_XX   		= VVF_XX_XX,
    VF_XX_XX_CUR   	= VVF_SYS_CUR,
    VF_CLEAR		= VVF_CLEAR

};

enum SwGetSetExpType
{
    GSE_STRING 		= 0x0001,	// String
    GSE_EXPR		= 0x0002,	// Expression
    GSE_INP			= 0x0004,	// InputField
    GSE_SEQ			= 0x0008,	// Sequence
    GSE_FORMULA		= 0x0010	// Formel
};

enum SwExtendedSubType
{
    SUB_CMD			= 0x0100,	// Kommando anzeigen
    SUB_INVISIBLE	= 0x0200,	// unsichtbar
    SUB_OWN_FMT		= 0x0400	// SwDBField: Uebernimmt Formatierung nicht
                                // aus Datenbank
};

enum SwInputFieldSubType
{
    INP_TXT		= 0x01,
    INP_USR     = 0x02,
    INP_VAR     = 0x03
};


enum SwUserType
{
    UF_STRING 	= 0x01,
    UF_EXPR	  	= 0x02
};

enum SwDateTimeSubType
{
    FIXEDFLD = 1,
    DATEFLD  = 2,
    TIMEFLD  = 4
};

#ifndef FIELDIDS_ONLY		// SWG-Testreader!!


extern USHORT __FAR_DATA aTypeTab[];

/*--------------------------------------------------------------------
    Beschreibung: Allgemeine Tools
 --------------------------------------------------------------------*/

String	GetResult(double nVal, sal_uInt32 nNumFmt, USHORT nLang = LANGUAGE_SYSTEM);
void    SetErrorStr(const String& rStr);
//String	ExpandDate(const Date& rDate, ULONG nFmt, USHORT nLang);
//String 	ExpandTime(const Time& rTime, ULONG nFmt, USHORT nLang);
String  FormatNumber(USHORT nNum, sal_uInt32 nFormat);

/*--------------------------------------------------------------------
    Beschreibung: 	Instanzen von SwFields und Abgeleiteten kommen 0-n mal vor.
                    Zu jeder Klasse SwFields existiert
                    1 Instanz der zugehoerigen Typenklasse
                    Basisklasse aller Feldtypen ist SwFieldType
 --------------------------------------------------------------------*/
class SwFldNames;

DBG_NAMEEX(SwFieldType)//STRIP008

class SwFieldType : public SwModify
{
    USHORT nWhich;

    friend void _FinitUI();		// um den Pointer zu loeschen!
    static	SvStringsDtor*	pFldNames;

    static void _GetFldName();			// legt die FldNames an, fldmgr.cxx!
protected:
    SwFieldType( USHORT nWhichId );

public:

#ifdef DBG_UTIL
    virtual ~SwFieldType();
#endif
    static	const String&	GetTypeStr( USHORT nTypeId );

    // nur in abgeleiteten Klassen
    virtual const String&	GetName() const;
    virtual SwFieldType* 	Copy()    const = 0;
    virtual	BOOL QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const{DBG_BF_ASSERT(0, "STRIP"); return NULL;} //STRIP001 virtual	BOOL QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId ){DBG_BF_ASSERT(0, "STRIP"); return NULL;} //STRIP001 virtual	BOOL PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );

            USHORT 			Which() const { return nWhich; }

    inline  void 			UpdateFlds() const;

    static inline SvStringsDtor* GetFldNames();
};

inline void SwFieldType::UpdateFlds() const
{
    ((SwFieldType*)this)->Modify( 0, 0 );
}

/*--------------------------------------------------------------------
    Beschreibung:  Basisklasse aller Felder.
                   Ueber Which wird der Typ des Feldes abgefragt.
                   Ueber Expand() wird der expandierte Inhalt
                   des Feldes in Erfahrung gebracht.
 --------------------------------------------------------------------*/

class SwField
{
    USHORT				nLang;	// Immer ueber SetLanguage aendern!
    BOOL                bIsAutomaticLanguage;
    sal_uInt32			nFormat;

    SwFieldType* 		pType;
protected:
    void				SetFormat(sal_uInt32 nSet) {nFormat = nSet;}

    SwField(SwFieldType* pTyp, sal_uInt32 nFmt = 0, USHORT nLang = LANGUAGE_SYSTEM);
public:
    virtual 			~SwField();

    // Typ feststellen
    inline SwFieldType*	GetTyp() const;

    // neuen Typ setzen (wird fuer das Kopieren zwischen Dokumenten benutzt)
    virtual SwFieldType* ChgTyp( SwFieldType* );

    // Expandierung fuer die Anzeige
    virtual String		Expand() const = 0;

    // liefert den Namen oder den Inhalt
    virtual	String		GetCntnt(BOOL bName = FALSE) const;
    virtual SwField* 	Copy() const = 0;

    // ResId
    USHORT              Which() const
#ifndef DBG_UTIL
        { return pType->Which(); }
#else
        ;       // in fldbas.cxx implementiert
#endif

    // TYP_ID
    virtual USHORT		GetSubType() const;
    virtual void        SetSubType(USHORT);

    // Sprache an der Feldposition
    inline USHORT		GetLanguage() const;
    virtual void		SetLanguage(USHORT nLng);

    // Parameter fuer Dialog und BASIC erfragen
    inline sal_uInt32 GetFormat() const;
    virtual const String& GetPar1() const;
    virtual String		GetPar2() 	const;

    virtual String		GetFormula() const;

    virtual void  		ChangeFormat(sal_uInt32 n);
    virtual void        SetPar1(const String& rStr);
    virtual void        SetPar2(const String& rStr);

    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
    // hat das Feld eine Action auf dem ClickHandler ? (z.B. INetFelder,..)
    // ist es ein Fix-Feld?
    FASTBOOL 			IsFixed() const;

    BOOL                IsAutomaticLanguage() const { return bIsAutomaticLanguage;}
    void                SetAutomaticLanguage(BOOL bSet){bIsAutomaticLanguage = bSet;}
};

inline SwFieldType*	SwField::GetTyp() const
    { return pType; 	}

inline sal_uInt32 SwField::GetFormat() const
    { return nFormat; 	}

inline USHORT SwField::GetLanguage() const
    { return nLang;		}

/*--------------------------------------------------------------------
    Beschreibung:	Felder mit Values, die ueber der Numberformatter
                    formatiert werden muessen
 --------------------------------------------------------------------*/

class SwValueFieldType : public SwFieldType
{
    SwDoc*	pDoc;
    BOOL	bUseFormat;	// Numberformatter verwenden

protected:
    SwValueFieldType( SwDoc* pDocPtr, USHORT nWhichId );

public:
    inline SwDoc*	GetDoc() const						{ return pDoc; }
    inline void		SetDoc(SwDoc* pNewDoc)				{ pDoc = pNewDoc; }

    inline BOOL		UseFormat() const					{ return bUseFormat; }
    inline void		EnableFormat(BOOL bFormat = TRUE)	{ bUseFormat = bFormat; }

    String			ExpandValue(const double& rVal, sal_uInt32 nFmt, USHORT nLng=0) const;
    void			DoubleToString(String &rValue, const double &rVal, LanguageType eLng) const;
};

class SwValueField : public SwField
{
    double fValue;
//	String sExpand;

protected:
    SwValueField( SwValueFieldType* pFldType, sal_uInt32 nFmt = 0, USHORT nLang = LANGUAGE_SYSTEM, const double fVal = 0.0 );

public:
    virtual 				~SwValueField();

    virtual SwFieldType*    ChgTyp( SwFieldType* );
    virtual void			SetLanguage(USHORT nLng);
//  os: wozu war das denn da?
//	virtual void  			ChangeFormat(ULONG n);

    inline SwDoc*			GetDoc() const			{ return ((SwValueFieldType*)GetTyp())->GetDoc(); }

    virtual double			GetValue() const;
    virtual void 			SetValue( const double& rVal );

    inline String	ExpandValue(const double& rVal, sal_uInt32 nFmt, USHORT nLng=0) const
        { return ((SwValueFieldType*)GetTyp())->ExpandValue(rVal, nFmt, nLng); }

};

class SwFormulaField : public SwValueField
{
    String sFormula;

protected:
    SwFormulaField( SwValueFieldType* pFldType, sal_uInt32 nFmt = 0, const double fVal = 0.0 );

public:
    virtual String			GetFormula() const;
    void 					SetFormula(const String& rStr);

    void                    SetExpandedFormula(const String& rStr);
    String                  GetExpandedFormula() const;

};


/*--------------------------------------------------------------------
    Beschreibung:	Sortierung von Felder nach der Position
 --------------------------------------------------------------------*/

inline SvStringsDtor* SwFieldType::GetFldNames()
{
    if( !SwFieldType::pFldNames )
        SwFieldType::_GetFldName();
    return SwFieldType::pFldNames;
}

#endif 	// FIELDIDS_ONLY


} //namespace binfilter
#endif // _FLDBAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
