/*************************************************************************
 *
 *  $RCSfile: rsccont.hxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rsccont.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.11  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.10  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.9  1997/08/27 18:18:04  MM
    neue Headerstruktur


      Rev 1.8   27 Aug 1997 18:18:04   MM
   neue Headerstruktur

      Rev 1.7   12 Sep 1996 12:45:38   MM
   Eigene Ressourcen definieren

      Rev 1.6   21 Nov 1995 19:49:16   TLX
   Neuer Link

      Rev 1.5   25 Aug 1993 15:33:58   mm
   Fehler und Warnings beseitigt

      Rev 1.4   04 Feb 1993 17:38:58   mm
   GetClassType Fehler verbessert und keine Inline -ctor.

      Rev 1.3   03 Feb 1993 15:03:12   mm
   GetClassType() implementiert

      Rev 1.2   05 Jan 1993 10:50:30   mm
   Neues Default verhalten

      Rev 1.1   23 Dec 1992 14:02:24   mm
   Sprachaenderung

      Rev 1.0   10 Aug 1992 07:14:38   MM
   Initial revision.

      Rev 1.13   29 Jul 1992 15:45:18   MM
   Aenderungen wg. MSC7.0

**************************************************************************/

#ifndef _RSCCONT_HXX
#define _RSCCONT_HXX

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif

/******************* R s c B a s e C o n t *******************************/
struct ENTRY_STRUCT {
    RscId   aName;
    RSCINST aInst;
    void Create(){ aName.Create(); aInst = RSCINST(); }
    void Destroy();
};
struct RscBaseContInst {
    USHORT          nEntries;
    ENTRY_STRUCT *  pEntries;
    BOOL            bDflt;
};

class RscBaseCont : public RscTop
{
protected:
    RscTop *        pTypeClass; // Typ der Eintraege
    RscTop *        pTypeClass1;// Zwei verschiedene Typen moeglich
    BOOL            bNoId;      // Keine Identifier
    USHORT          nSize;      // Groesse der Instanzdaten dieser Klasse
                                // mit Superklassen
    USHORT          nOffInstData;// Offset auf eigen Instanzdaten
    void            DestroyElements( RscBaseContInst * pClassData );
    RSCINST         SearchElePos( const RSCINST & rInst, const RscId & rEleName,
                                  RscTop * pClass, USHORT nPos );
protected:
    void            ContWriteSrc( const RSCINST & rInst, FILE * fOutput,
                                  RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         ContWriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, USHORT, BOOL bExtra );
    void            ContWriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                        const char *, BOOL nWriteSize );
public:
                    RscBaseCont( HASHID nId, USHORT nTypId,
                                 RscTop * pSuper = NULL,
                                 BOOL bNoId = TRUE );
                    ~RscBaseCont();
    virtual RSCCLASS_TYPE   GetClassType() const;
    void            SetTypeClass( RscTop * pClass, RscTop * pClass1 = NULL )
                    {
                        pTypeClass = pClass;
                        pTypeClass1 = pClass1;
                    };
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    void            Destroy( const RSCINST & rInst );
    ERRTYPE         GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop * pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst );
    RSCINST         SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass );
    USHORT          GetCount( const RSCINST & rInst );
    RSCINST         GetPosEle( const RSCINST & rInst, USHORT nPos );
    ERRTYPE         MovePosEle( const RSCINST & rInst, USHORT nDestPos,
                                USHORT nSourcePos );
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, USHORT nPos,
                                 const RscId & rRscId);
    SUBINFO_STRUCT  GetInfoEle( const RSCINST & rInst, USHORT nPos );
    ERRTYPE         SetString( const RSCINST &, char * pStr );
    ERRTYPE         SetNumber( const RSCINST &, long lValue );
    ERRTYPE         SetBool( const RSCINST & rInst, BOOL bValue );
    ERRTYPE         SetConst( const RSCINST & rInst, HASHID nValueId,
                              long nValue );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );

                    // Gibt die Groesse der Klasse in Bytes
    USHORT          Size(){ return( nSize ); };

    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    void            SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    void            Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId );
    void            DeletePos( const RSCINST & rInst, USHORT nPos );

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT , BOOL bExtra);
    ERRTYPE         WriteHxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId & rId );
    ERRTYPE         WriteCxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId &rId );
};

/******************* R s c C o n t W r i t e S r c ***********************/
class RscContWriteSrc : public RscBaseCont
{
public:
                    RscContWriteSrc( HASHID nId, USHORT nTypId,
                                     RscTop * pSuper = NULL,
                                     BOOL bNoId = TRUE );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
};

/******************* R s c C o n t ***************************************/
class RscCont : public RscContWriteSrc {
public:
                    RscCont( HASHID nId, USHORT nTypId,
                             RscTop * pSuper = NULL,
                             BOOL bNoId = TRUE );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c C o n t E x t r a D a t a *********************/
class RscContExtraData : public RscContWriteSrc {
public:
                    RscContExtraData( HASHID nId, USHORT nTypId,
                             RscTop * pSuper = NULL,
                             BOOL bNoId = TRUE );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
};

#endif //_RSCCONT_HXX
