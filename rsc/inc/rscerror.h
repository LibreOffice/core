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
#ifndef _RSCERROR_H
#define _RSCERROR_H

/****************** D E F I N I T I O N S ********************************/
/******************* R e t u r n   E r r o r s         *******************/
#define ERR_OK              0xFFFFFFFF

#define ERR_ERROR           0x0100
#define ERR_UNKNOWN_METHOD  0x0101  // Return
#define ERR_OPENFILE        0x0102  // Return
#define ERR_NOCHAR          0x0103  // Return
#define ERR_NORSCINST       0x0104  // Return
#define ERR_USAGE           0x0105
#define ERR_NOINPUT         0x0106  // Return
#define ERR_UNKNOWNSW       0x0107
#define ERR_REFTODEEP       0x0108  // Return
#define ERR_FILEFORMAT      0x0109  // Return
#define ERR_FILESIZE        0x010A  // Return
#define ERR_RENAMEFILE      0x010B  // Return
#define ERR_NOIMAGE         0x010C  // Return

#define ERR_RSCRANGE        0x0200  // Return
#define ERR_RSCRANGE_OUTDEFSET  (ERR_RSCRANGE +1 )

#define ERR_RSCENUM         0x0210  // Return
#define ERR_RSCFLAG         0x0220  // Return
#define ERR_RSCCONT         0x0240  // Return
#define ERR_CONT_INVALIDPOS     (ERR_RSCCONT +1 ) // Return
#define ERR_CONT_INVALIDTYPE    (ERR_RSCCONT +2 ) // Return

#define ERR_RSCCMPED        0x0250
#define ERR_RSCINST         0x0260
#define ERR_RSCINST_NOVARNAME   (ERR_RSCINST +1 ) // Return
#define ERR_RSCINST_RESERVEDNAME (ERR_RSCINST +2 ) // Return

#define ERR_LEX             0x0270
#define ERR_YACC            0x0280
#define ERR_DOUBLEID            (ERR_YACC    +1 )
#define ERR_FALSETYPE           (ERR_YACC    +2 )
#define ERR_NOVARIABLENAME      (ERR_YACC    +3 )
#define ERR_USHORTRANGE         (ERR_YACC    +4 )
#define ERR_IDRANGE             (ERR_YACC    +5 )
#define ERR_NOCOPYOBJ           (ERR_YACC    +6 )
#define ERR_REFNOTALLOWED       (ERR_YACC    +7 ) // Return
#define ERR_DOUBLEDEFINE        (ERR_YACC    +8 )
#define ERR_COPYNOTALLOWED      (ERR_YACC    +9 )
#define ERR_IDEXPECTED          (ERR_YACC    +10)
#define ERR_ZERODIVISION        (ERR_YACC    +11)
#define ERR_PRAGMA              (ERR_YACC    +12)
#define ERR_DECLAREDEFINE       (ERR_YACC    +13)
#define ERR_NOTUPELNAME         (ERR_YACC    +14)
#define ERR_NOTYPE              (ERR_YACC    +15)

#define ERR_RSCARRAY         0x02A0  // Return
#define ERR_ARRAY_INVALIDINDEX  (ERR_RSCARRAY +1 ) // Return

#define ERR_ERROREND                0x1000

#define ERR_WARNINGSTART        0x1001
#define WRN_LOCALID             (ERR_WARNINGSTART +1 )
#define WRN_GLOBALID            (ERR_WARNINGSTART +2 )
#define WRN_SUBINMEMBER         (ERR_WARNINGSTART +3 )
#define WRN_CONT_NOID           (ERR_WARNINGSTART +4 )
#define WRN_STR_REFNOTFOUND     (ERR_WARNINGSTART +5 )
#define WRN_MGR_REFNOTFOUND     (ERR_WARNINGSTART +6 )
#define WRN_CONT_DOUBLEID       (ERR_WARNINGSTART +7 )

#define ERR_WARNINGEND      0x2000

class ERRTYPE
{
    sal_uInt32  nError;
public:
    ERRTYPE()                { nError = ERR_OK; }
    ERRTYPE( sal_uInt32 nErr )   { nError = nErr; }
    ERRTYPE( const ERRTYPE & rErr ) { nError = rErr.nError; }
    ERRTYPE& operator = ( const ERRTYPE & rError );
    operator sal_uInt32() const { return( nError ); }
    bool IsError() const     { return nError <= ERR_ERROREND; }
    bool IsOk() const        { return !IsError(); }
    bool IsWarning() const   { return nError >= ERR_WARNINGSTART && nError <= ERR_WARNINGEND;}
    void Clear(){ nError = ERR_OK; }
};

/****************** R s c E r r o r **************************************/
class RscId;
class RscTop;

enum RscVerbosity
{
    RscVerbositySilent = 0,
    RscVerbosityNormal = 1,
    RscVerbosityVerbose = 2
};

class RscError
{
    FILE *  fListing;
    RscVerbosity m_verbosity;

    void WriteError( const ERRTYPE& rError, const char * pMessage );
    void StdLstOut( const char * pStr );
    void StdLstErr( const char * pStr );
    void ErrorFormat( const ERRTYPE& rError, RscTop * pClass,
                      const RscId & aId );
public:
    virtual ~RscError() {}

    sal_uInt32  nErrors;// Anzahl der Fehler
                    RscError( RscVerbosity _verbosity )
                        {
                            fListing = NULL;
                            nErrors = 0;
                            m_verbosity = _verbosity;
                        }
    void            SetListFile( FILE * fList ){ fListing = fList; }
    FILE *          GetListFile(){ return fListing; }
    RscVerbosity    GetVerbosity() const { return m_verbosity; }
    virtual void    StdOut( const char *, const RscVerbosity _verbosityLevel = RscVerbosityNormal );
    virtual void    StdErr( const char * );
    virtual void    LstOut( const char * );
    virtual void    Error( const ERRTYPE& rError, RscTop* pClass, const RscId &aId,
                           const char * pMessage = NULL );
    // Dieser Fehler sollte nur im Compilermodus auftreten,
    // das Programm wird mit exit() verlassen
    virtual void    FatalError( const ERRTYPE& rError, const RscId &aId,
                                const char * pMessage = NULL );
};

#endif // _RSCERROR_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
