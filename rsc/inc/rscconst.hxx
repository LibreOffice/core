/*************************************************************************
 *
 *  $RCSfile: rscconst.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:54 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscconst.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

**************************************************************************/

#ifndef _RSCCONST_HXX
#define _RSCCONST_HXX

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

/******************* R s c C o n s t *************************************/
class RscConst : public RscTop
{
protected:
    struct VarEle {
        HASHID  nId;    // Name der Konstante
        INT32   lValue; // Wert der Konstante
    };
    VarEle *    pVarArray;  // Zeiger auf das Feld mit Konstanten
    USHORT      nEntries;   // Anzahle der Eintraege im Feld
public:
                    RscConst( HASHID nId, USHORT nTypId );
                    ~RscConst();
    virtual RSCCLASS_TYPE   GetClassType() const;
    USHORT          GetEntryCount() const { return nEntries; }
                    // Die erlaubten Werte werden gesetzt
    ERRTYPE         SetConstant( HASHID nVarName, INT32 lValue );
    HASHID          GetConstant( USHORT nPos );
    BOOL            GetConstValue( HASHID nConstId, INT32 * pVal ) const;
    BOOL            GetValueConst( INT32 nValue, HASHID  * pConstId ) const;
    USHORT          GetConstPos( HASHID nConstId );
    virtual void    WriteSyntax( FILE * fOutput, RscTypCont * pTC );
    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c E n u m ***************************************/
class RscEnum : public RscConst {
    struct RscEnumInst {
        USHORT  nValue; // Position der Konstanten im Array
        BOOL    bDflt;  // Ist Default
    };
    USHORT          nSize;
    BOOL            bUSHORT;
public:
                    RscEnum( HASHID nId, USHORT nTypId, BOOL bUSHORT = TRUE );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, BOOL );
    USHORT          Size(){ return nSize; }

    virtual void    SetToDefault( const RSCINST & rInst )
                    {
                        ((RscEnumInst*)rInst.pData)->bDflt = TRUE;
                    }
    BOOL            IsDefault( const RSCINST & rInst )
                    {
                        return( ((RscEnumInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    ERRTYPE         SetConst( const RSCINST & rInst, HASHID nValueId,
                              long nValue );
    ERRTYPE         SetNumber( const RSCINST & rInst, long nValue );
    ERRTYPE         GetConst( const RSCINST & rInst, HASHID * );
    ERRTYPE         GetNumber( const RSCINST & rInst, long * nValue );
    void            WriteSrc( const RSCINST &rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
};

#endif // _RSCCONST_HXX
