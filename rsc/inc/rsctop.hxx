/*************************************************************************
 *
 *  $RCSfile: rsctop.hxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rsctop.hxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

**************************************************************************/

#ifndef _RSCTOP_HXX
#define _RSCTOP_HXX

#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCCLOBJ_HXX
#include <rscclobj.hxx>
#endif

/****************** T Y P E S ********************************************/
typedef short RSCVAR;
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
    USHORT          nTypId;
    RscTop *        pRefClass;

protected:
                    RscTop( HASHID nId, USHORT nTypIdent,
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
            USHORT  GetTypId() const
                    { return nTypId; };
                    // Gibt die Oberklasse zurueck
            BOOL    InHierarchy( RscTop * pClass );
            BOOL    IsCodeWriteable() const
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

    virtual HASHID  GetConstant( USHORT );

    virtual RscTop *    GetTypeClass() const;

                    // Gibt die Groesse der Klasse in Bytes
    virtual USHORT  Size();

                    // Gibt die Referenz zurueck
    virtual ERRTYPE GetRef( const RSCINST & rInst, RscId * );

                    // Gibt die Referenz zurueck
    virtual ERRTYPE SetRef( const RSCINST & rInst, const RscId & rRefId );

                    // Variable anlegen
    virtual ERRTYPE SetVariable( HASHID nVarName, RscTop * pClass,
                                 RSCINST * pDflt = NULL,
                                 RSCVAR nVarType = 0, USHORT nMask = 0,
                                 HASHID nDataBaseName = HASH_NONAME );

                    // Zaehlt alle Variablen auf
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc );

                    // Liefert Instanz der Variablen zurueck
                    // pData, pClass im return koennen NULL sein
    virtual RSCINST GetVariable( const RSCINST & rInst, HASHID nVarName,
                                 const RSCINST & rInitInst,
                                 BOOL nInitDflt = FALSE,
                                 RscTop * pCreateClass = NULL );
    virtual RSCINST GetCopyVar( const RSCINST & rInst, HASHID nVarName );

    virtual RSCINST GetTupelVar( const RSCINST & rInst, USHORT nPos,
                                 const RSCINST & rInitInst );

                    // Liefert Instanz aus einem Feld zurueck
                    // pGetInst im return kann NULL sein
    virtual ERRTYPE GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop *pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst );

                    // Liefert Instanz aus einem Feld zurueck
                    // pGetInst im return kann NULL sein
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, long lValue,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

                    // Liefert Instanz aus einem Feld zurueck
                    // pGetInst im return kann NULL sein
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, HASHID nId,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

    virtual RSCINST SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass );

                    // Liefert Instanz an der Position zurueck
    virtual RSCINST GetPosEle( const RSCINST & rInst, USHORT nPos );

                    // verschiebt eine Instanz
    virtual ERRTYPE MovePosEle( const RSCINST & rInst, USHORT nDestPos,
                                USHORT nSourcePos );

                    // aendert RscId an Position
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, USHORT nPos,
                                 const RscId & rRscId);

                    // Liefert Information ueber Instanz
                    // an der Position zurueck
    virtual SUBINFO_STRUCT GetInfoEle( const RSCINST & rInst, USHORT nPos );

                    // Anzahl der Eintraege
    virtual USHORT GetCount( const RSCINST & rInst );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetNumber( const RSCINST & rInst, long lValue );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetBool( const RSCINST & rInst, BOOL bValue );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetConst( const RSCINST & rInst, HASHID nValueId,
                              long nValue );

                    // Eine Zuweisung an eine Variable
    virtual ERRTYPE SetNotConst( const RSCINST & rInst, HASHID nId );

    virtual ERRTYPE SetString( const RSCINST & rInst, char * pStr );

    virtual ERRTYPE GetNumber( const RSCINST & rInst, long * pN );

    virtual ERRTYPE GetBool( const RSCINST & rInst, BOOL * pB );

    virtual ERRTYPE GetConst( const RSCINST & rInst, HASHID * pH );

    virtual ERRTYPE GetString( const RSCINST & rInst, char ** ppStr );

    virtual RSCINST Create( RSCINST * pInst,
                            const RSCINST & rDefInst, BOOL bOwnClass = FALSE );

                    // Instanz zerstoeren
    virtual void    Destroy( const RSCINST & rInst );

                    // prueft auf konsistenz
    virtual BOOL    IsConsistent( const RSCINST & rInst,
                                  RscInconsList * pList = NULL );

                    // Alles auf Default setzen
    virtual void    SetToDefault( const RSCINST & rInst );

                    // Ist Eingabe = Default
    virtual BOOL    IsDefault( const RSCINST & rInst );

                    // Gleiche Werte auf Default setzen
    virtual BOOL    IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

                    // Instanz auf Default setzen
    virtual void    SetDefault( const RSCINST & rInst, HASHID nVarId );

                    // Default zu einer Variablen holen
    virtual RSCINST GetDefault( HASHID nVarId );

    virtual void    Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId );

    virtual void    DeletePos( const RSCINST & rInst, USHORT nPos );

                    // Schreibt den Kopf und das Ende einer Resource
                    // Script Datei
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, USHORT nTab,
                                    const RscId & aId, const char * );
    virtual void    WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab,const char * );
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                    USHORT nDeep, BOOL bExtra );
    virtual ERRTYPE WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT nDeep, BOOL bExtra );

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
