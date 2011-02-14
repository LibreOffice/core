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
#ifndef SW_DBFLD_HXX
#define SW_DBFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"
#include "swdbdata.hxx"

class SwDoc;
class SwTxtFld;
class SwFrm;

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBFieldType : public SwValueFieldType
{
    SwDBData    aDBData;        //
    String      sName;          // only used in ::GetName() !
    String      sColumn;
    long        nRefCnt;

public:

    SwDBFieldType(SwDoc* pDocPtr, const String& rColumnName, const SwDBData& rDBData);
    ~SwDBFieldType();

    virtual const String& GetName() const;
    virtual SwFieldType*  Copy() const;

    inline void     AddRef() { nRefCnt++; }
    void            ReleaseRef();

    const String&   GetColumnName() const {return sColumn;}
    const SwDBData& GetDBData() const {return aDBData;}

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung:
    von SwFields abgeleitete Klassen. Sie ueberlagern die Expand-Funktion.
    Der Inhalt wird entsprechend dem Format, soweit vorhanden, formatiert.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBField : public SwValueField
{
    String  aContent;
    String  sFieldCode; // contains Word's field code
    sal_uInt16  nSubType;
    sal_Bool    bIsInBodyTxt    : 1;
    sal_Bool    bValidValue     : 1;
    sal_Bool    bInitialized    : 1;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

public:
    SwDBField(SwDBFieldType*, sal_uLong nFmt = 0);
    virtual ~SwDBField();

    virtual SwFieldType*    ChgTyp( SwFieldType* );

    // Der aktuelle Text
    inline  void        SetExpansion(const String& rStr);

    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16 nType);

    virtual String      GetFieldName() const;

    // fuer Berechnungen in Ausdruecken
    void                ChgValue( double d, sal_Bool bVal );

    // Evaluierung ueber den DBMgr String rauspulen
    void                Evaluate();

    // Evaluierung fuer Kopf und Fusszeilen
    void                ChangeExpansion( const SwFrm*, const SwTxtFld* );
    void                InitContent();
    void                InitContent(const String& rExpansion);

    inline void         ChgBodyTxtFlag( sal_Bool bIsInBody );

    inline sal_Bool         IsInitialized() const   { return bInitialized; }
    inline void         ClearInitialized()      { bInitialized = sal_False; }
    inline void         SetInitialized()        { bInitialized = sal_True; }

    // Name erfragen
    virtual const String& GetPar1() const;

    // access to the command string
    const String&       GetFieldCode()   const
                        { return sFieldCode;}
    void                SetFieldCode(const String& rStr)
                        { sFieldCode = rStr; }

    // DBName
    inline const SwDBData&  GetDBData() const { return ((SwDBFieldType*)GetTyp())->GetDBData(); }
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline  void SwDBField::SetExpansion(const String& rStr)
    { aContent = rStr; }

// wird von UpdateExpFlds gesetzt (dort ist die Node-Position bekannt)
inline void SwDBField::ChgBodyTxtFlag( sal_Bool bIsInBody )
    { bIsInBodyTxt = bIsInBody; }

/*--------------------------------------------------------------------
    Beschreibung: Basisklasse fuer alle weiteren Datenbankfelder
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNameInfField : public SwField
{
    SwDBData    aDBData;
    sal_uInt16      nSubType;

protected:
    const SwDBData& GetDBData() const {return aDBData;}
    SwDBData&       GetDBData() {return aDBData;}

    SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uLong nFmt = 0);

public:
    // DBName
    inline const SwDBData&  GetRealDBData() { return aDBData; }

    SwDBData                GetDBData(SwDoc* pDoc);
    void                    SetDBData(const SwDBData& rDBData); // #111840#

    virtual String          GetFieldName() const;

    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
    virtual sal_uInt16          GetSubType() const;
    virtual void            SetSubType(sal_uInt16 nType);
};


/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld Naechster Satz
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNextSetFieldType : public SwFieldType
{
public:
    SwDBNextSetFieldType();

    virtual SwFieldType*    Copy() const;
};


/*--------------------------------------------------------------------
    Beschreibung: Naechsten Datensatz mit Bedingung
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNextSetField : public SwDBNameInfField
{
    String  aCond;
    sal_Bool    bCondValid;

public:
    SwDBNextSetField( SwDBNextSetFieldType*,
                      const String& rCond, const String& rDummy, const SwDBData& rDBData);

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    void                    Evaluate(SwDoc*);
    inline void             SetCondValid(sal_Bool bCond);
    inline sal_Bool             IsCondValid() const;

    // Condition
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_Bool SwDBNextSetField::IsCondValid() const
    { return bCondValid; }

inline void SwDBNextSetField::SetCondValid(sal_Bool bCond)
    { bCondValid = bCond; }

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld Naechster Satz
 --------------------------------------------------------------------*/

class SwDBNumSetFieldType : public SwFieldType
{
public:
    SwDBNumSetFieldType();

    virtual SwFieldType*    Copy() const;
};


/*--------------------------------------------------------------------
    Beschreibung: Datensatz mit Nummer xxx
                  Die Nummer steht in nFormat
                  ! kleiner Missbrauch
 --------------------------------------------------------------------*/

class SwDBNumSetField : public SwDBNameInfField
{
    String  aCond;
    String  aPar2;
    sal_Bool    bCondValid;

public:
    SwDBNumSetField(SwDBNumSetFieldType*, const String& rCond, const String& rDBNum, const SwDBData& rDBData);

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    inline sal_Bool             IsCondValid() const;
    inline void             SetCondValid(sal_Bool bCond);
    void                    Evaluate(SwDoc*);

    // Condition
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    // Datensatznummer
    virtual String          GetPar2()   const;
    virtual void            SetPar2(const String& rStr);

    // Die Datensatznummer steht in nFormat !!
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_Bool SwDBNumSetField::IsCondValid() const
    { return bCondValid; }

inline void SwDBNumSetField::SetCondValid(sal_Bool bCond)
    { bCondValid = bCond; }

/*--------------------------------------------------------------------
    Beschreibung: Datenbankname
 --------------------------------------------------------------------*/

class SwDBNameFieldType : public SwFieldType
{
        SwDoc *pDoc;
public:
    SwDBNameFieldType(SwDoc*);

    String                  Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNameField : public SwDBNameInfField
{
public:
    SwDBNameField(SwDBNameFieldType*, const SwDBData& rDBData, sal_uLong nFmt = 0);

    virtual String   Expand() const;
    virtual SwField* Copy() const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Datensatznummer
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBSetNumberFieldType : public SwFieldType
{
public:
    SwDBSetNumberFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Datenbankfeld
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBSetNumberField : public SwDBNameInfField
{
    long    nNumber;

public:
    SwDBSetNumberField(SwDBSetNumberFieldType*, const SwDBData& rDBData, sal_uLong nFmt = 0);

    virtual String  Expand() const;
    virtual         SwField* Copy() const;
    void            Evaluate(SwDoc*);

    inline long     GetSetNumber() const;
    inline void     SetSetNumber(long nNum);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline long SwDBSetNumberField::GetSetNumber() const
    { return nNumber; }

inline void SwDBSetNumberField::SetSetNumber(long nNum)
    { nNumber = nNum; }


#endif // SW_DBFLD_HXX
