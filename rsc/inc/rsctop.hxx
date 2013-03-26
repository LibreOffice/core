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
#ifndef _RSCTOP_HXX
#define _RSCTOP_HXX

#include <rscerror.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rscclobj.hxx>

typedef sal_uInt32 RSCVAR;
#define VAR_POINTER     0x0001
#define VAR_HIDDEN      0x0002
#define VAR_NODATAINST  0x0004
#define VAR_NORC        0x0008
#define VAR_SVDYNAMIC   0x0010
#define VAR_NOENUM      0x0020
#define VAR_EXTENDABLE  0x0040  /* Auch die Ableitung einer Klasse kann angegeben werden */

class RscTop : public RefNode
{
    RscTop *        pSuperClass;
    RSCINST         aDfltInst;
    sal_uInt32          nTypId;
    RscTop *        pRefClass;

protected:
                    RscTop( Atom nId, sal_uInt32 nTypIdent,
                            RscTop * pSuperCl = NULL );

public:
    rtl::OString    aCallPar1;      // Klassenaufruf ohne Typen bis ResId
    rtl::OString    aCallPar2;      // Klassenaufruf ohne Typen ab ResId
    rtl::OString    aCallParType;   // Klassenaufruf mit Typen

            void    SetSuperClass( RscTop * pClass )
                    {
                        pSuperClass = pClass;
                    }
            RscTop* GetSuperClass() const
                    { return pSuperClass; }
                    // Gibt den Typidentifier zurueck
            sal_uInt32  GetTypId() const
                    { return nTypId; };
                    // Gibt die Oberklasse zurueck
            sal_Bool    InHierarchy( RscTop * pClass );
            sal_Bool    IsCodeWriteable() const
                    {
                        return( 0 != aCallParType.getLength() );
                    }
            void    SetCallPar( const rtl::OString& rPar1, const rtl::OString& rPar2,
                                const rtl::OString& rParType );
            void    SetRefClass( RscTop * pRef ) { pRefClass = pRef; }
            RscTop* GetRefClass() const { return pRefClass; }
    virtual RSCCLASS_TYPE GetClassType() const = 0;
            RSCINST GetDefault();

                    // Vorbereitung auf den dtor aufruf
                    // Da die Klassen gegenseitige Abhaengigkeiten
                    // aufweisen koennen, kann man im dtor nicht davon
                    // ausgehen, dass alle Klassenzeiger noch gueltig sind
    virtual void    Pre_dtor();

    virtual Atom    GetConstant( sal_uInt32 );

    virtual RscTop *    GetTypeClass() const;

                    // Gibt die Groesse der Klasse in Bytes
    virtual sal_uInt32  Size();

                    // Gibt die Referenz zurueck
    virtual ERRTYPE GetRef( const RSCINST & rInst, RscId * );

                    // Gibt die Referenz zurueck
    virtual ERRTYPE SetRef( const RSCINST & rInst, const RscId & rRefId );

                    // Variable anlegen
    virtual ERRTYPE SetVariable( Atom nVarName, RscTop * pClass,
                                 RSCINST * pDflt = NULL,
                                 RSCVAR nVarType = 0, sal_uInt32 nMask = 0,
                                 Atom nDataBaseName = InvalidAtom );

                    // Zaehlt alle Variablen auf
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc );

                    // Liefert Instanz der Variablen zurueck
                    // pData, pClass im return koennen NULL sein
    virtual RSCINST GetVariable( const RSCINST & rInst, Atom nVarName,
                                 const RSCINST & rInitInst,
                                 sal_Bool nInitDflt = sal_False,
                                 RscTop * pCreateClass = NULL );
    virtual RSCINST GetCopyVar( const RSCINST & rInst, Atom nVarName );

    virtual RSCINST GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RSCINST & rInitInst );

                    // Liefert Instanz aus einem Feld zurueck
                    // pGetInst im return kann NULL sein
    virtual ERRTYPE GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop *pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst );

                    // Liefert Instanz aus einem Feld zurueck
                    // pGetInst im return kann NULL sein
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, sal_Int32 lValue,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

                    // Liefert Instanz aus einem Feld zurueck
                    // pGetInst im return kann NULL sein
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, Atom nId,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

    virtual RSCINST SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass );

                    // Liefert Instanz an der Position zurueck
    virtual RSCINST GetPosEle( const RSCINST & rInst, sal_uInt32 nPos );

                    // verschiebt eine Instanz
    virtual ERRTYPE MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                                sal_uInt32 nSourcePos );

                    // aendert RscId an Position
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RscId & rRscId);

                    // Liefert Information ueber Instanz
                    // an der Position zurueck
    virtual SUBINFO_STRUCT GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos );

                    // Anzahl der Eintraege
    virtual sal_uInt32 GetCount( const RSCINST & rInst );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetNumber( const RSCINST & rInst, sal_Int32 lValue );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetBool( const RSCINST & rInst, sal_Bool bValue );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetNotConst( const RSCINST & rInst, Atom nId );

    virtual ERRTYPE SetString( const RSCINST & rInst, const char * pStr );

    virtual ERRTYPE GetNumber( const RSCINST & rInst, sal_Int32 * pN );

    virtual ERRTYPE GetBool( const RSCINST & rInst, sal_Bool * pB );

    virtual ERRTYPE GetConst( const RSCINST & rInst, Atom * pH );

    virtual ERRTYPE GetString( const RSCINST & rInst, char ** ppStr );

    virtual RSCINST Create( RSCINST * pInst,
                            const RSCINST & rDefInst, sal_Bool bOwnClass = sal_False );

                    // Instanz zerstoeren
    virtual void    Destroy( const RSCINST & rInst );

                    // prueft auf konsistenz
    virtual sal_Bool    IsConsistent( const RSCINST & rInst );

                    // Alles auf Default setzen
    virtual void    SetToDefault( const RSCINST & rInst );

                    // Ist Eingabe = Default
    virtual sal_Bool    IsDefault( const RSCINST & rInst );

                    // Gleiche Werte auf Default setzen
    virtual sal_Bool    IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

                    // Instanz auf Default setzen
    virtual void    SetDefault( const RSCINST & rInst, Atom nVarId );

                    // Default zu einer Variablen holen
    virtual RSCINST GetDefault( Atom nVarId );

    virtual void    Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId );

    virtual void    DeletePos( const RSCINST & rInst, sal_uInt32 nPos );

                    // Schreibt den Kopf und das Ende einer Resource
                    // Script Datei
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * );
    virtual void    WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab,const char * );
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                    sal_uInt32 nDeep, sal_Bool bExtra );
    virtual ERRTYPE WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32 nDeep, sal_Bool bExtra );
};

#endif //_RSCTOP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
