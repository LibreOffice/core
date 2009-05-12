/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscrange.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _RSCRANGE_HXX
#define _RSCRANGE_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

/******************* R s c R a n g e *************************************/
class RscRange : public RscTop
{
protected:
    struct RscRangeInst {
        sal_uInt16  nValue; // nValue = Ausgangswert - nMin
        BOOL    bDflt;  // Ist Default
    };
    INT32    nMin;   // Minimum des Bereiches
    INT32    nMax;   // Maximum des Bereiches
    sal_uInt32  nSize;
public:
                    RscRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( INT32 nMinimum, INT32 nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return nSize; }
                    // Eine Zuweisung an eine Variable
    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscRangeInst*)rInst.pData)->bDflt = TRUE;
                    }
    BOOL            IsDefault( const RSCINST & rInst)
                    {
                        return( ((RscRangeInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetNumber( const RSCINST &, INT32 );
    ERRTYPE         GetNumber( const RSCINST &, INT32 * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c L o n g R a n g e ******************************/
class RscLongRange : public RscTop
{
protected:
    struct RscLongRangeInst
    {
        INT32    nValue; // nValue = Ausgangswert - nMin
        BOOL    bDflt;  // Ist Default
    };
    INT32    nMin;   // Minimum des Bereiches
    INT32    nMax;   // Maximum des Bereiches
    sal_uInt32  nSize;
public:
                    RscLongRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( INT32 nMinimum, INT32 nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return nSize; }
                    // Eine Zuweisung an eine Variable
    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscLongRangeInst*)rInst.pData)->bDflt = TRUE;
                    }
    BOOL            IsDefault( const RSCINST & rInst)
                    {
                        return( ((RscLongRangeInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetNumber( const RSCINST &, INT32 );
    ERRTYPE         GetNumber( const RSCINST &, INT32 * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

/******************* R s c L o n g E n u m R a n g e ******************/
class RscLongEnumRange : public RscLongRange
{
public:
                    RscLongEnumRange( Atom nId, sal_uInt32 nTypId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              INT32 nValue );
};

/******************* R s c I d R a n g e ***********************************/
class RscIdRange : public RscTop
{
    sal_uInt32  nSize;
protected:
    INT32    nMin;   // Minimum des Bereiches
    INT32    nMax;   // Maximum des Bereiches
public:
                    RscIdRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( INT32 nMinimum, INT32 nMaximum ){
                        nMin = nMinimum;
                        nMax = nMaximum;
                        return ERR_OK;
                    }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
    void            Destroy( const RSCINST & rInst );
    sal_uInt32          Size(){ return nSize; }
    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscId*)rInst.pData)->aExp.cUnused = TRUE;
                    }
    BOOL            IsDefault( const RSCINST & rInst)
                    {
                        //cUnused wird fuer Defaultkennung verwendet
                        return ((RscId*)rInst.pData)->aExp.cUnused
                               ? TRUE : FALSE;
                    }
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetNumber( const RSCINST &, INT32 );
    ERRTYPE         GetNumber( const RSCINST &, INT32 * );
    ERRTYPE         SetRef( const RSCINST &, const RscId & rRscId );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

/******************* R s c B o o l ***************************************/
class RscBool : public RscRange
{
public:
                    RscBool( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( INT32, INT32 ){
                        return( ERR_UNKNOWN_METHOD );
                    };
    ERRTYPE         SetBool( const RSCINST & rInst, BOOL b ){
                        return( SetNumber( rInst, (INT32)b ) );
                    };
    ERRTYPE         GetBool( const RSCINST & rInst, BOOL * pB){
                        INT32 l;
                        GetNumber( rInst, &l );
                        *pB = (0 != l);
                        return( ERR_OK );
                    };
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

class RscBreakRange : public RscRange {
    INT32    nOutRange;
public:
                    RscBreakRange( Atom nId, sal_uInt32 nTypId );
    void            SetOutRange( INT32 nNumber ){
                        nOutRange = nNumber;
                    }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
    ERRTYPE         SetNumber( const RSCINST &, INT32 );
};

#endif // _RSCRANGE_HXX
