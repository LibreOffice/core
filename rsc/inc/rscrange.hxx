/*************************************************************************
 *
 *  $RCSfile: rscrange.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscrange.hxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

**************************************************************************/

#ifndef _RSCRANGE_HXX
#define _RSCRANGE_HXX

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif

/******************* R s c R a n g e *************************************/
class RscRange : public RscTop
{
protected:
    struct RscRangeInst {
        USHORT  nValue; // nValue = Ausgangswert - nMin
        BOOL    bDflt;  // Ist Default
    };
    long    nMin;   // Minimum des Bereiches
    long    nMax;   // Maximum des Bereiches
    USHORT  nSize;
public:
                    RscRange( HASHID nId, USHORT nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( long nMinimum, long nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    USHORT          Size(){ return nSize; }
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
    ERRTYPE         SetNumber( const RSCINST &, long );
    ERRTYPE         GetNumber( const RSCINST &, long * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c L o n g R a n g e ******************************/
class RscLongRange : public RscTop
{
protected:
    struct RscLongRangeInst
    {
        long    nValue; // nValue = Ausgangswert - nMin
        BOOL    bDflt;  // Ist Default
    };
    long    nMin;   // Minimum des Bereiches
    long    nMax;   // Maximum des Bereiches
    USHORT  nSize;
public:
                    RscLongRange( HASHID nId, USHORT nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( long nMinimum, long nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    USHORT          Size(){ return nSize; }
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
    ERRTYPE         SetNumber( const RSCINST &, long );
    ERRTYPE         GetNumber( const RSCINST &, long * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

/******************* R s c L o n g E n u m R a n g e ******************/
class RscLongEnumRange : public RscLongRange
{
public:
                    RscLongEnumRange( HASHID nId, USHORT nTypId );

    ERRTYPE         SetConst( const RSCINST & rInst, HASHID nValueId,
                              long nValue );
};

/******************* R s c I d R a n g e ***********************************/
class RscIdRange : public RscTop
{
    USHORT  nSize;
protected:
    long    nMin;   // Minimum des Bereiches
    long    nMax;   // Maximum des Bereiches
    BOOL    bRcLong;// Binaere Resource 4 Byte statt 2
public:
                    RscIdRange( HASHID nId, USHORT nTypId, BOOL bRcL = FALSE );
    virtual RSCCLASS_TYPE   GetClassType() const;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( long nMinimum, long nMaximum ){
                        nMin = nMinimum;
                        nMax = nMaximum;
                        return ERR_OK;
                    }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
    void            Destroy( const RSCINST & rInst );
    USHORT          Size(){ return nSize; }
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
    ERRTYPE         SetNumber( const RSCINST &, long );
    ERRTYPE         GetNumber( const RSCINST &, long * );
    ERRTYPE         SetRef( const RSCINST &, const RscId & rRscId );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

/******************* R s c B o o l ***************************************/
class RscBool : public RscRange
{
public:
                    RscBool( HASHID nId, USHORT nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( long, long ){
                        return( ERR_UNKNOWN_METHOD );
                    };
    ERRTYPE         SetBool( const RSCINST & rInst, BOOL b ){
                        return( SetNumber( rInst, (long)b ) );
                    };
    ERRTYPE         GetBool( const RSCINST & rInst, BOOL * pB){
                        long l;
                        GetNumber( rInst, &l );
                        *pB = (0 != l);
                        return( ERR_OK );
                    };
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );

};

class RscBreakRange : public RscRange {
    long    nOutRange;
public:
                    RscBreakRange( HASHID nId, USHORT nTypId );
    void            SetOutRange( long nNumber ){
                        nOutRange = nNumber;
                    }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
    ERRTYPE         SetNumber( const RSCINST &, long );
};

#endif // _RSCRANGE_HXX
