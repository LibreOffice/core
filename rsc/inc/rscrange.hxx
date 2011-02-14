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
        sal_Bool    bDflt;  // Ist Default
    };
    sal_Int32    nMin;   // Minimum des Bereiches
    sal_Int32    nMax;   // Maximum des Bereiches
    sal_uInt32  nSize;
public:
                    RscRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, sal_Bool );
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return nSize; }
                    // Eine Zuweisung an eine Variable
    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscRangeInst*)rInst.pData)->bDflt = sal_True;
                    }
    sal_Bool            IsDefault( const RSCINST & rInst)
                    {
                        return( ((RscRangeInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 );
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c L o n g R a n g e ******************************/
class RscLongRange : public RscTop
{
protected:
    struct RscLongRangeInst
    {
        sal_Int32    nValue; // nValue = Ausgangswert - nMin
        sal_Bool    bDflt;  // Ist Default
    };
    sal_Int32    nMin;   // Minimum des Bereiches
    sal_Int32    nMax;   // Maximum des Bereiches
    sal_uInt32  nSize;
public:
                    RscLongRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, sal_Bool );
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return nSize; }
                    // Eine Zuweisung an eine Variable
    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscLongRangeInst*)rInst.pData)->bDflt = sal_True;
                    }
    sal_Bool            IsDefault( const RSCINST & rInst)
                    {
                        return( ((RscLongRangeInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 );
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

/******************* R s c L o n g E n u m R a n g e ******************/
class RscLongEnumRange : public RscLongRange
{
public:
                    RscLongEnumRange( Atom nId, sal_uInt32 nTypId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue );
};

/******************* R s c I d R a n g e ***********************************/
class RscIdRange : public RscTop
{
    sal_uInt32  nSize;
protected:
    sal_Int32    nMin;   // Minimum des Bereiches
    sal_Int32    nMax;   // Maximum des Bereiches
public:
                    RscIdRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum ){
                        nMin = nMinimum;
                        nMax = nMaximum;
                        return ERR_OK;
                    }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, sal_Bool );
    void            Destroy( const RSCINST & rInst );
    sal_uInt32          Size(){ return nSize; }
    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscId*)rInst.pData)->aExp.cUnused = sal_True;
                    }
    sal_Bool            IsDefault( const RSCINST & rInst)
                    {
                        //cUnused wird fuer Defaultkennung verwendet
                        return ((RscId*)rInst.pData)->aExp.cUnused
                               ? sal_True : sal_False;
                    }
                    // Als Default setzen
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 );
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * );
    ERRTYPE         SetRef( const RSCINST &, const RscId & rRscId );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
    sal_Bool            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
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
    ERRTYPE         SetRange( sal_Int32, sal_Int32 ){
                        return( ERR_UNKNOWN_METHOD );
                    };
    ERRTYPE         SetBool( const RSCINST & rInst, sal_Bool b ){
                        return( SetNumber( rInst, (sal_Int32)b ) );
                    };
    ERRTYPE         GetBool( const RSCINST & rInst, sal_Bool * pB){
                        sal_Int32 l;
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
    sal_Int32    nOutRange;
public:
                    RscBreakRange( Atom nId, sal_uInt32 nTypId );
    void            SetOutRange( sal_Int32 nNumber ){
                        nOutRange = nNumber;
                    }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, sal_Bool );
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 );
};

#endif // _RSCRANGE_HXX
