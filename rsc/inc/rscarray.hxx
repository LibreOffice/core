/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscarray.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _RSCARRAY_HXX
#define _RSCARRAY_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctop.hxx>

class RscEnum;

/******************* R s c A r r a y ************************************/
class RscInstNode : public IdNode
{
    sal_uInt32  nTypeId;
protected:
    using NameNode::Search;
public:
    RSCINST aInst;
    RscInstNode( sal_uInt32 nId );
    ~RscInstNode();
    virtual sal_uInt32  GetId() const;
    RscInstNode *   Left() const { return (RscInstNode *)pLeft  ; };
    RscInstNode *   Right() const{ return (RscInstNode *)pRight ; };
    RscInstNode *   Search( sal_uInt32 nId ) const
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
    sal_uInt32          nSize;      // Groesse der Instanzdaten dieser Klasse
                                // mit Superklassen
    sal_uInt32          nOffInstData;// Offset auf eigen Instanzdaten
    void            WriteSrcArray( const RSCINST & rInst, FILE * fOutput,
                                  RscTypCont * pTC, sal_uInt32 nTab, const char * );
public:
                    RscArray( Atom nId, sal_uInt32 nTypId,
                                RscTop * pSuper, RscEnum * pTypeClass );
                    ~RscArray();
    virtual RSCCLASS_TYPE   GetClassType() const;

    void            SetTypeClass( RscEnum * pClass )
                    {
                        pTypeClass = pClass;
                    }
    virtual RscTop *    GetTypeClass() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    void            Destroy( const RSCINST & rInst );
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, INT32 lValue,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, Atom nId,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return( nSize ); };

    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    virtual void    SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    virtual void    WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

class RscClassArray : public RscArray
{
public:
                    RscClassArray( Atom nId, sal_uInt32 nTypId,
                                RscTop * pSuper, RscEnum * pTypeClass );
                    ~RscClassArray();
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                    sal_uInt32 nDeep, BOOL bExtra );
};


class RscLangArray : public RscArray
{
public:
                    RscLangArray( Atom nId, sal_uInt32 nTypId,
                                RscTop * pSuper, RscEnum * pTypeClass );
    virtual RSCCLASS_TYPE   GetClassType() const;
};

#endif //_RSCARRAY
