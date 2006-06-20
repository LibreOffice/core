/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscclass.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:44:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
        Atom            nVarName;   // Variablenname
        RSCVAR          nVarType;   // Variablentyp
        sal_uInt32      nMask;      // Maskierungsbit
        sal_uInt32      nOffset;    // Beginn der Instanzdaten
        RscTop *        pClass;     // Klasse
        CLASS_DATA      pDefault;   // Zeiger auf DefaultDaten
        Atom            nDataBaseName;//Name fuer Fremddatenbereich
    };
    sal_uInt32              nSuperSize; // Groesse der Instanzdaten der SuperKl.
    sal_uInt32              nSize;      // Groesse der Instanzdaten dieser Klasse
                                    // mit Superklassen
    sal_uInt32              nEntries;   // Eintraege in pVarTypeList
    VARTYPE_STRUCT *    pVarTypeList;   // Variablenliste
    RSCINST             GetInstData( CLASS_DATA pData, sal_uInt32 nEle,
                                     BOOL bGetCopy = FALSE );
    CLASS_DATA          GetDfltData( sal_uInt32 nEle );
    BOOL                IsDflt( CLASS_DATA pData, sal_uInt32 nEle );
    BOOL                IsValueDflt( CLASS_DATA pData, sal_uInt32 nEle );
    void                SetVarDflt( CLASS_DATA pData, sal_uInt32 nEle,
                                    BOOL bSet );
    INT32               GetCorrectValues( const RSCINST & rInst, sal_uInt32 nVarPos,
                                        sal_uInt32 nTupelIdx, RscTypCont * pTC );
public:
                    RscClass( Atom nId, sal_uInt32 nTypId, RscTop * pSuperCl );
                    ~RscClass();

    virtual RSCCLASS_TYPE   GetClassType() const;

    void            Pre_dtor();
    ERRTYPE         SetVariable( Atom nVarName, RscTop * pClass,
                                 RSCINST * pDflt,
                                 RSCVAR nVarType, sal_uInt32 nMask,
                                 Atom nDataBaseName );
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc );
    RSCINST         GetVariable( const RSCINST & rInst, Atom nVarName,
                                 const RSCINST & rInitInst,
                                 BOOL nInitDflt = FALSE,
                                 RscTop * pCreateClass = NULL );
    RSCINST         GetCopyVar( const RSCINST & rInst, Atom nVarName );

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return( nSize ); };

    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    void            SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    void            SetDefault( const RSCINST & rData, Atom nVarId );
    using RscTop::GetDefault;
    RSCINST         GetDefault( Atom nVarId );

    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    void            Destroy( const RSCINST & rInst );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteInstRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    void            WriteSyntax( FILE * fOutput, RscTypCont * pTC );

    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                const char * );
    void            WriteRcCtor( FILE * fOutput, RscTypCont * pTC );
};

class RscSysDepend : public RscClass
{
public:
                    RscSysDepend( Atom nId, sal_uInt32 nTypId, RscTop * pSuper );
    ERRTYPE         WriteSysDependRc( const RSCINST &, RscWriteRc & aMem,
                                    RscTypCont * pTC, sal_uInt32, BOOL bExtra,
                                    BOOL bFirst = FALSE );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
};

class RscFirstSysDepend : public RscSysDepend
{
public:
                    RscFirstSysDepend( Atom nId, sal_uInt32 nTypId,
                                        RscTop * pSuper );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
};

class RscTupel : public RscClass
{
public:
    RscTupel( Atom nId, sal_uInt32 nTypId, RscTop * pSuper );
    RSCINST         GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                                const RSCINST & rInitInst );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
};

#endif //_RSCCLASS_HXX
