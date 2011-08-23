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
#ifndef _DBFLD_HXX
#define _DBFLD_HXX

#include "fldbas.hxx"
#ifndef _SWDBDATA_HXX
#include <swdbdata.hxx>
#endif
namespace binfilter {

class SwDoc;
class SwTxtFld;
class SwFrm;

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld
 --------------------------------------------------------------------*/

class SwDBFieldType : public SwValueFieldType
{
    SwDBData 	aDBData;		//
    String 		sName;			// only used in ::GetName() !
    String 		sColumn;
    long   		nRefCnt;

public:

    SwDBFieldType(SwDoc* pDocPtr, const String& rColumnName, const SwDBData& rDBData);

    virtual const String& GetName() const;
    virtual SwFieldType*  Copy() const;

    inline void		AddRef() { nRefCnt++; }
    void			ReleaseRef();
    inline long		GetRefCount() { return nRefCnt; }

    const String&	GetColumnName()	const {return sColumn;}
    const SwDBData&	GetDBData()	const {return aDBData;}

    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

/*--------------------------------------------------------------------
    Beschreibung:
    von SwFields abgeleitete Klassen. Sie ueberlagern die Expand-Funktion.
    Der Inhalt wird entsprechend dem Format, soweit vorhanden, formatiert.
 --------------------------------------------------------------------*/

class SwDBField : public SwValueField
{
    String  aContent;
    USHORT	nSubType;
    BOOL 	bIsInBodyTxt	: 1;
    BOOL	bValidValue		: 1;
    BOOL	bInitialized	: 1;

public:
    SwDBField(SwDBFieldType*, ULONG nFmt = 0);
    virtual ~SwDBField();

    virtual SwFieldType*    ChgTyp( SwFieldType* );

    // Der aktuelle Text
    inline  void		SetExpansion(const String& rStr);
    virtual String 		Expand() const;
    virtual SwField*	Copy() const;

    virtual USHORT		GetSubType() const;
    virtual void		SetSubType(USHORT nType);

    // Name oder Inhalt

    // fuer Berechnungen in Ausdruecken
    void                ChgValue( double d, BOOL bVal );

    // Evaluierung ueber den DBMgr String rauspulen
    void                Evaluate();

    // Evaluierung fuer Kopf und Fusszeilen
    void 			    ChangeExpansion( const SwFrm*, const SwTxtFld* );
    void				InitContent();
    void				InitContent(const String& rExpansion);
    String				GetOldContent();

    inline void 		ChgBodyTxtFlag( BOOL bIsInBody );
    inline BOOL 		IsInBodyTxt() const;

    inline BOOL 		IsValidValue() const	{ return bValidValue; }

    inline BOOL 		IsInitialized() const	{ return bInitialized; }
    inline void			ClearInitialized()		{ bInitialized = FALSE; }
    inline void			SetInitialized()		{ bInitialized = TRUE; }

    // Name erfragen
    virtual const String& GetPar1() const;

    // DBName
    inline const SwDBData& 	GetDBData() const { return ((SwDBFieldType*)GetTyp())->GetDBData(); }
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

inline  void SwDBField::SetExpansion(const String& rStr)
    { aContent = rStr; }

// wird von der Formatierung abgefragt
inline BOOL SwDBField::IsInBodyTxt() const
    { return bIsInBodyTxt; }

// wird von UpdateExpFlds gesetzt (dort ist die Node-Position bekannt)
inline void SwDBField::ChgBodyTxtFlag( BOOL bIsInBody )
    { bIsInBodyTxt = bIsInBody; }

/*--------------------------------------------------------------------
    Beschreibung: Basisklasse fuer alle weiteren Datenbankfelder
 --------------------------------------------------------------------*/

class SwDBNameInfField : public SwField
{
    SwDBData 	aDBData;
    USHORT      nSubType;

protected:
    const SwDBData& GetDBData() const {return aDBData;}
    SwDBData& 		GetDBData() {return aDBData;}

    SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, ULONG nFmt = 0);

public:
    // DBName
    inline const SwDBData&	GetRealDBData() { return aDBData; }

    SwDBData				GetDBData(SwDoc* pDoc);
    inline void				SetDBData(const SwDBData& rDBData) { aDBData = rDBData; }

    // Name oder Inhalt
    virtual	BOOL       		QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual BOOL            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
    virtual USHORT          GetSubType() const;
    virtual void            SetSubType(USHORT nType);
};


/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld Naechster Satz
 --------------------------------------------------------------------*/

class SwDBNextSetFieldType : public SwFieldType
{
public:
    SwDBNextSetFieldType();

    virtual SwFieldType* 	Copy() const;
};


/*--------------------------------------------------------------------
    Beschreibung: Naechsten Datensatz mit Bedingung
 --------------------------------------------------------------------*/

class SwDBNextSetField : public SwDBNameInfField
{
    String 	aCond;
    BOOL	bCondValid;

public:
    SwDBNextSetField( SwDBNextSetFieldType*,
                      const String& rCond, const String& rDummy, const SwDBData& rDBData);

    virtual String 			Expand() const;
    virtual SwField*		Copy() const;

    void					Evaluate(SwDoc*);
    inline void				SetCondValid(BOOL bCond);
    inline BOOL				IsCondValid() const;

    // Condition
    virtual const String&	GetPar1() const;
    virtual void            SetPar1(const String& rStr);
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual BOOL        PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

inline BOOL	SwDBNextSetField::IsCondValid() const
    { return bCondValid; }

inline void	SwDBNextSetField::SetCondValid(BOOL bCond)
    { bCondValid = bCond; }

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld Naechster Satz
 --------------------------------------------------------------------*/

class SwDBNumSetFieldType : public SwFieldType
{
public:
    SwDBNumSetFieldType();

     virtual SwFieldType*	Copy() const;
};


/*--------------------------------------------------------------------
    Beschreibung: Datensatz mit Nummer xxx
                  Die Nummer steht in nFormat
                  ! kleiner Missbrauch
 --------------------------------------------------------------------*/

class SwDBNumSetField : public SwDBNameInfField
{
    String 	aCond;
    String 	aPar2;
    BOOL 	bCondValid;

public:
    SwDBNumSetField(SwDBNumSetFieldType*, const String& rCond, const String& rDBNum, const SwDBData& rDBData);

    virtual String 			Expand() const;
    virtual SwField*		Copy() const;

    inline BOOL				IsCondValid() const;
    inline void				SetCondValid(BOOL bCond);
    void					Evaluate(SwDoc*);

    // Condition
    virtual const String&	GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    // Datensatznummer
    virtual String			GetPar2() 	const;
    virtual void            SetPar2(const String& rStr);

    // Die Datensatznummer steht in nFormat !!
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual BOOL        PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

inline BOOL	SwDBNumSetField::IsCondValid() const
    { return bCondValid; }

inline void	SwDBNumSetField::SetCondValid(BOOL bCond)
    { bCondValid = bCond; }

/*--------------------------------------------------------------------
    Beschreibung: Datenbankname
 --------------------------------------------------------------------*/

class SwDBNameFieldType : public SwFieldType
{
        SwDoc *pDoc;
public:
    SwDBNameFieldType(SwDoc*);

    String 					Expand(ULONG) const;
     virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld
 --------------------------------------------------------------------*/

class SwDBNameField : public SwDBNameInfField
{
public:
    SwDBNameField(SwDBNameFieldType*, const SwDBData& rDBData, ULONG nFmt = 0);

    virtual String	 Expand() const;
    virtual SwField* Copy() const;
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual BOOL        PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

/*--------------------------------------------------------------------
    Beschreibung: Datensatznummer
 --------------------------------------------------------------------*/

class SwDBSetNumberFieldType : public SwFieldType
{
public:
    SwDBSetNumberFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld
 --------------------------------------------------------------------*/

class SwDBSetNumberField : public SwDBNameInfField
{
    long	nNumber;

public:
    SwDBSetNumberField(SwDBSetNumberFieldType*, const SwDBData& rDBData, ULONG nFmt = 0);

    virtual String	Expand() const;
    virtual 		SwField* Copy() const;
    void			Evaluate(SwDoc*);

    inline long		GetSetNumber() const;
    inline void		SetSetNumber(long nNum);
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual BOOL        PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

inline long	SwDBSetNumberField::GetSetNumber() const
    { return nNumber; }

inline void	SwDBSetNumberField::SetSetNumber(long nNum)
    { nNumber = nNum; }


} //namespace binfilter
#endif // _DBFLD_HXX
