/*************************************************************************
 *
 *  $RCSfile: usrfld.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _USRFLD_HXX
#define _USRFLD_HXX

#include "fldbas.hxx"

class SfxPoolItem;
class SwCalc;
class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: Benutzerfelder
 --------------------------------------------------------------------*/

class SwUserFieldType : public SwValueFieldType
{
    BOOL    bValidValue : 1;
    BOOL    bDeleted : 1;
    double  nValue;
    String  aName;
    String  aContent;
    USHORT  nType;

public:
    SwUserFieldType( SwDoc* pDocPtr, const String& );

    virtual const String&   GetName() const;
    virtual SwFieldType*    Copy() const;

    String                  Expand(ULONG nFmt, USHORT nSubType, USHORT nLng);

    String                  GetContent( ULONG nFmt = 0 );
           void             SetContent( const String& rStr, ULONG nFmt = 0 );
           void             CtrlSetContent( const String& rStr );

    inline BOOL             IsValid() const;
    inline void             ChgValid( BOOL bNew );

    virtual void            Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

           double           GetValue(SwCalc& rCalc);    // Member nValue neu berrechnen
    inline double           GetValue() const;
    inline void             SetValue(const double nVal);

    inline USHORT           GetType() const;
    inline void             SetType(USHORT);

    BOOL                    IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( BOOL b )    { bDeleted = b; }

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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
    SwUserField(SwUserFieldType*, USHORT nSub = 0, ULONG nFmt = 0);

    virtual USHORT          GetSubType() const;
    virtual void            SetSubType(USHORT nSub);

    virtual double          GetValue() const;
    virtual void            SetValue( const double& rVal );

    virtual String          Expand() const;
    virtual SwField*        Copy() const;
    virtual String          GetCntnt(BOOL bName = FALSE) const;

    // Name kann nicht geaendert werden
    virtual const String&   GetPar1() const;

    // Inhalt
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

#endif // _USRFLD_HXX
