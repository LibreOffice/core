/*************************************************************************
 *
 *  $RCSfile: rscclass.hxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscclass.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

**************************************************************************/

#ifndef _RSCCLASS_HXX
#define _RSCCLASS_HXX

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

/******************* R s c C l a s s *************************************/
class RscClass : public RscTop
{
protected:
    struct RscClassInst{
        ULONG   nVarDflt;
    };
    struct VARTYPE_STRUCT {
        HASHID          nVarName;   // Variablenname
        RSCVAR          nVarType;   // Variablentyp
        USHORT          nMask;      // Maskierungsbit
        USHORT          nOffset;    // Beginn der Instanzdaten
        RscTop *        pClass;     // Klasse
        CLASS_DATA      pDefault;   // Zeiger auf DefaultDaten
        HASHID          nDataBaseName;//Name fuer Fremddatenbereich
    };
    USHORT              nSuperSize; // Groesse der Instanzdaten der SuperKl.
    USHORT              nSize;      // Groesse der Instanzdaten dieser Klasse
                                    // mit Superklassen
    USHORT              nEntries;   // Eintraege in pVarTypeList
    VARTYPE_STRUCT *    pVarTypeList;   // Variablenliste
    RSCINST             GetInstData( CLASS_DATA pData, USHORT nEle,
                                     BOOL bGetCopy = FALSE );
    CLASS_DATA          GetDfltData( USHORT nEle );
    BOOL                IsDflt( CLASS_DATA pData, USHORT nEle );
    BOOL                IsValueDflt( CLASS_DATA pData, USHORT nEle );
    void                SetVarDflt( CLASS_DATA pData, USHORT nEle,
                                    BOOL bSet );
    long                GetCorrectValues( const RSCINST & rInst, USHORT nVarPos,
                                        USHORT nTupelIdx, RscTypCont * pTC );
public:
                    RscClass( HASHID nId, USHORT nTypId, RscTop * pSuperCl );
                    ~RscClass();

    virtual RSCCLASS_TYPE   GetClassType() const;

    void            Pre_dtor();
    ERRTYPE         SetVariable( HASHID nVarName, RscTop * pClass,
                                 RSCINST * pDflt,
                                 RSCVAR nVarType, USHORT nMask,
                                 HASHID nDataBaseName );
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc );
    RSCINST         GetVariable( const RSCINST & rInst, HASHID nVarName,
                                 const RSCINST & rInitInst,
                                 BOOL nInitDflt = FALSE,
                                 RscTop * pCreateClass = NULL );
    RSCINST         GetCopyVar( const RSCINST & rInst, HASHID nVarName );

                    // Gibt die Groesse der Klasse in Bytes
    USHORT          Size(){ return( nSize ); };

    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    void            SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    void            SetDefault( const RSCINST & rData, HASHID nVarId );
    RSCINST         GetDefault( HASHID nVarId );

    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    void            Destroy( const RSCINST & rInst );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteInstRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, USHORT, BOOL bExtra );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    void            WriteSyntax( FILE * fOutput, RscTypCont * pTC );

    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                const char * );
    void            WriteRcCtor( FILE * fOutput, RscTypCont * pTC );
};

class RscSysDepend : public RscClass
{
public:
                    RscSysDepend( HASHID nId, USHORT nTypId, RscTop * pSuper );
    ERRTYPE         WriteSysDependRc( const RSCINST &, RscWriteRc & aMem,
                                    RscTypCont * pTC, USHORT, BOOL bExtra,
                                    BOOL bFirst = FALSE );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
};

class RscFirstSysDepend : public RscSysDepend
{
public:
                    RscFirstSysDepend( HASHID nId, USHORT nTypId,
                                        RscTop * pSuper );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
};

class RscTupel : public RscClass
{
public:
    RscTupel( HASHID nId, USHORT nTypId, RscTop * pSuper );
    RSCINST         GetTupelVar( const RSCINST & rInst, USHORT nPos,
                                const RSCINST & rInitInst );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
};

#endif //_RSCCLASS_HXX
