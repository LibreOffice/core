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
#ifndef _RSCTOP_HXX
#define _RSCTOP_HXX

#include <rscerror.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rscclobj.hxx>

/****************** T Y P E S ********************************************/
typedef sal_uInt32 RSCVAR;
#define VAR_POINTER     0x0001
#define VAR_HIDDEN      0x0002
#define VAR_NODATAINST  0x0004
#define VAR_NORC        0x0008
#define VAR_SVDYNAMIC   0x0010
#define VAR_NOENUM      0x0020
#define VAR_EXTENDABLE  0x0040  /* Auch die Ableitung einer Klasse kann angegeben werden */

/****************** C L A S S E S ****************************************/
/******************* R s c C l a s s *************************************/
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
    ByteString      aCallPar1;      // Klassenaufruf ohne Typen bis ResId
    ByteString      aCallPar2;      // Klassenaufruf ohne Typen ab ResId
    ByteString      aCallParType;   // Klassenaufruf mit Typen

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
                        return( 0 != aCallParType.Len() );
                    }
            void    SetCallPar( const ByteString & rPar1, const ByteString & rPar2,
                                const ByteString & rParType );
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
    virtual sal_Bool    IsConsistent( const RSCINST & rInst,
                                  RscInconsList * pList = NULL );

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

    // Weiterleitung an Superklassen wird unterbunden
    virtual ERRTYPE WriteHxxHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, const RscId & rId );
    virtual ERRTYPE WriteHxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId &rId );
    virtual ERRTYPE WriteCxxHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, const RscId &rId );
    virtual ERRTYPE WriteCxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId &rId );

            void WriteSyntaxHeader( FILE * fOutput, RscTypCont * pTC  );
    virtual void WriteSyntax( FILE * fOutput, RscTypCont * pTC );

    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
    virtual void    WriteRcCtor( FILE * fOutput, RscTypCont * pTC );
};

#endif //_RSCTOP_HXX
