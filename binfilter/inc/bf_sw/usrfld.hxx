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
#ifndef _USRFLD_HXX
#define _USRFLD_HXX

#include "fldbas.hxx"
namespace binfilter {

class SfxPoolItem;
class SwCalc;
class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: Benutzerfelder
 --------------------------------------------------------------------*/

class SwUserFieldType : public SwValueFieldType
{
    BOOL	bValidValue : 1;
    BOOL	bDeleted : 1;
    double 	nValue;
    String  aName;
    String  aContent;
    USHORT  nType;

public:
    SwUserFieldType( SwDoc* pDocPtr, const String& );

    virtual const String&	GetName() const;
    virtual SwFieldType*	Copy() const;

    String					Expand(sal_uInt32 nFmt, USHORT nSubType, USHORT nLng);

    String					GetContent( sal_uInt32 nFmt = 0 );
           void				SetContent( const String& rStr, sal_uInt32 nFmt = 0 );

    inline BOOL 			IsValid() const;
    inline void 			ChgValid( BOOL bNew );


           double 			GetValue(SwCalc& rCalc);	// Member nValue neu berrechnen
    inline double			GetValue() const;
    inline void				SetValue(const double nVal);

    inline USHORT			GetType() const;
    inline void				SetType(USHORT);

    BOOL					IsDeleted() const 		{ return bDeleted; }
    void					SetDeleted( BOOL b )	{ bDeleted = b; }

    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

inline BOOL SwUserFieldType::IsValid() const
    { return bValidValue; }

inline void SwUserFieldType::ChgValid( BOOL bNew )
    { bValidValue = bNew; }

inline double SwUserFieldType::GetValue() const
    { return nValue; }

inline void SwUserFieldType::SetValue(const double nVal)
    { nValue = nVal; }

inline USHORT SwUserFieldType::GetType() const
    { return nType; }

inline void SwUserFieldType::SetType(USHORT nSub)
{
    nType = nSub;
    EnableFormat(!(nSub & GSE_STRING));
}

/*--------------------------------------------------------------------
    Beschreibung: Benutzerfelder
 --------------------------------------------------------------------*/

class SwUserField : public SwValueField
{
    USHORT  nSubType;

public:
    SwUserField(SwUserFieldType*, USHORT nSub = 0, sal_uInt32 nFmt = 0);

    virtual USHORT	 		GetSubType() const;
    virtual void	 		SetSubType(USHORT nSub);

    virtual double          GetValue() const;
    virtual void            SetValue( const double& rVal );

    virtual String	 		Expand() const;
    virtual SwField* 		Copy() const;
    virtual	String	 		GetCntnt(BOOL bName = FALSE) const;

    // Name kann nicht geaendert werden
    virtual const String&   GetPar1() const;

    // Inhalt
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);
    virtual	BOOL       	QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual	BOOL		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );
};

} //namespace binfilter
#endif // _USRFLD_HXX
