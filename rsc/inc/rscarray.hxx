/*************************************************************************
 *
 *  $RCSfile: rscarray.hxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscarray.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

**************************************************************************/

#ifndef _RSCARRAY_HXX
#define _RSCARRAY_HXX

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif

class RscEnum;

/******************* R s c A r r a y ************************************/
class RscInstNode : public IdNode
{
    USHORT  nTypeId;
public:
    RSCINST aInst;
    RscInstNode( USHORT nId );
    ~RscInstNode();
    virtual USHORT  GetId() const;
    RscInstNode *   Left() const { return (RscInstNode *)pLeft  ; };
    RscInstNode *   Right() const{ return (RscInstNode *)pRight ; };
    RscInstNode *   Search( USHORT nId ) const
                    {
                        return (RscInstNode *)IdNode::Search( nId );
                    }
};

struct RscArrayInst
{
    RscInstNode *   pNode;
};

/* Der Baum wird ueber die Werte des Enums sortiert, nicht ueber
    seine HashId.
*/
class RscArray : public RscTop
{
protected:
    RscEnum *       pTypeClass; // Typ der Eintraege
    USHORT          nSize;      // Groesse der Instanzdaten dieser Klasse
                                // mit Superklassen
    USHORT          nOffInstData;// Offset auf eigen Instanzdaten
    USHORT *        pWriteTypeValue;// Was soll geschrieben werden
    USHORT *        pWriteDfltTypeValue; // fallback fuer WriteTypeValue
    void            WriteSrcArray( const RSCINST & rInst, FILE * fOutput,
                                  RscTypCont * pTC, USHORT nTab, const char * );
public:
                    RscArray( HASHID nId, USHORT nTypId,
                                RscTop * pSuper, RscEnum * pTypeClass,
                                USHORT * pTypeValue, USHORT * pFallBackType );
                    ~RscArray();
    virtual RSCCLASS_TYPE   GetClassType() const;

    void            SetTypeClass( RscEnum * pClass )
                    {
                        pTypeClass = pClass;
                    }
    virtual RscTop *    GetTypeClass() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    void            Destroy( const RSCINST & rInst );
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, long lValue,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, HASHID nId,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

                    // Gibt die Groesse der Klasse in Bytes
    USHORT          Size(){ return( nSize ); };

    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    virtual void    SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, USHORT nTab,
                                    const RscId & aId, const char * );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, USHORT, BOOL bExtra );
    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

class RscClassArray : public RscArray
{
public:
                    RscClassArray( HASHID nId, USHORT nTypId,
                                RscTop * pSuper, RscEnum * pTypeClass,
                                USHORT * pTypeValue, USHORT * pFallBackType );
                    ~RscClassArray();
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, USHORT nTab,
                                    const RscId & aId, const char * );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, USHORT nTab, const char * );
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                    USHORT nDeep, BOOL bExtra );
};


class RscLangArray : public RscArray
{
public:
                    RscLangArray( HASHID nId, USHORT nTypId,
                                RscTop * pSuper, RscEnum * pTypeClass,
                                USHORT * pTypeValue, USHORT * pFallBackType );
    virtual RSCCLASS_TYPE   GetClassType() const;
};

#endif //_RSCARRAY
