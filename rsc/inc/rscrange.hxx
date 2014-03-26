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
#ifndef _RSCRANGE_HXX
#define _RSCRANGE_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

class RscRange : public RscTop
{
protected:
    struct RscRangeInst
    {
        sal_uInt16  nValue; // nValue = Ausgangswert - nMin
        bool        bDflt;  // Ist Default
    };
    sal_Int32       nMin;   // Minimum des Bereiches
    sal_Int32       nMax;   // Maximum des Bereiches
    sal_uInt32      nSize;
public:
                    RscRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool ) SAL_OVERRIDE;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32      Size() SAL_OVERRIDE { return nSize; }
                    // Eine Zuweisung an eine Variable
    virtual void    SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE
                        {
                            ((RscRangeInst*)rInst.pData)->bDflt = true;
                        }
    bool            IsDefault( const RSCINST & rInst) SAL_OVERRIDE
                        {
                            return( ((RscRangeInst*)rInst.pData)->bDflt );
                        };
                    // Als Default setzen
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) SAL_OVERRIDE;
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

class RscLongRange : public RscTop
{
protected:
    struct RscLongRangeInst
    {
        sal_Int32    nValue; // nValue = Ausgangswert - nMin
        bool    bDflt;  // Ist Default
    };
    sal_Int32    nMin;   // Minimum des Bereiches
    sal_Int32    nMax;   // Maximum des Bereiches
    sal_uInt32  nSize;
public:
                    RscLongRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool ) SAL_OVERRIDE;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32      Size() SAL_OVERRIDE { return nSize; }
                    // Eine Zuweisung an eine Variable
    virtual void    SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE
                        {
                            ((RscLongRangeInst*)rInst.pData)->bDflt = true;
                        }
    bool            IsDefault( const RSCINST & rInst) SAL_OVERRIDE
                        {
                            return( ((RscLongRangeInst*)rInst.pData)->bDflt );
                        };
                    // Als Default setzen
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) SAL_OVERRIDE;
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;

};

class RscLongEnumRange : public RscLongRange
{
public:
                    RscLongEnumRange( Atom nId, sal_uInt32 nTypId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) SAL_OVERRIDE;
};

class RscIdRange : public RscTop
{
    sal_uInt32  nSize;
protected:
    sal_Int32    nMin;   // Minimum des Bereiches
    sal_Int32    nMax;   // Maximum des Bereiches
public:
                    RscIdRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum )
                        {
                            nMin = nMinimum;
                            nMax = nMaximum;
                            return ERR_OK;
                        }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool ) SAL_OVERRIDE;
    void            Destroy( const RSCINST & rInst ) SAL_OVERRIDE;
    sal_uInt32          Size() SAL_OVERRIDE { return nSize; }
    virtual void    SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE
                        {
                            ((RscId*)rInst.pData)->aExp.cUnused = true;
                        }
    bool            IsDefault( const RSCINST & rInst) SAL_OVERRIDE
                        {
                            //cUnused wird fuer Defaultkennung verwendet
                            return ((RscId*)rInst.pData)->aExp.cUnused;
                        }
                    // Als Default setzen
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) SAL_OVERRIDE;
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * ) SAL_OVERRIDE;
    ERRTYPE         SetRef( const RSCINST &, const RscId & rRscId ) SAL_OVERRIDE;
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
    bool            IsConsistent( const RSCINST & rInst ) SAL_OVERRIDE;
};

class RscBool : public RscRange
{
public:
                    RscBool( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
                    // Der zulaessige Bereich wird gesetzt
    ERRTYPE         SetRange( sal_Int32, sal_Int32 ){ return ERR_UNKNOWN_METHOD; }
    ERRTYPE         SetBool( const RSCINST & rInst, bool b ) SAL_OVERRIDE
                        {
                            return SetNumber( rInst, (sal_Int32)b );
                        };
    ERRTYPE         GetBool( const RSCINST & rInst, bool * pB) SAL_OVERRIDE
                        {
                            sal_Int32 l;
                            GetNumber( rInst, &l );
                            *pB = (0 != l);
                            return ERR_OK;
                        };
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
};

class RscBreakRange : public RscRange
{
    sal_Int32        nOutRange;
public:
                    RscBreakRange( Atom nId, sal_uInt32 nTypId );
    void            SetOutRange( sal_Int32 nNumber ) { nOutRange = nNumber; }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool ) SAL_OVERRIDE;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) SAL_OVERRIDE;
};

#endif // _RSCRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
