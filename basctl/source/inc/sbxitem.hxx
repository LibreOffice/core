/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-15 16:03:07 $
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
#ifndef _SBXITEM_HXX
#define _SBXITEM_HXX

#include "scriptdocument.hxx"

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

const USHORT BASICIDE_TYPE_UNKNOWN  =   0;
const USHORT BASICIDE_TYPE_SHELL    =   1;
const USHORT BASICIDE_TYPE_LIBRARY  =   2;
const USHORT BASICIDE_TYPE_MODULE   =   3;
const USHORT BASICIDE_TYPE_DIALOG   =   4;
const USHORT BASICIDE_TYPE_METHOD   =   5;


class SbxItem : public SfxPoolItem
{
    ScriptDocument          m_aDocument;
    String                  m_aLibName;
    String                  m_aName;
    String                  m_aMethodName;
    USHORT                  m_nType;

public:
    TYPEINFO();
    SbxItem( USHORT nWhich, const ScriptDocument& rDocument, const String& aLibName, const String& aName, USHORT nType );
    SbxItem( USHORT nWhich, const ScriptDocument& rDocument, const String& aLibName, const String& aName, const String& aMethodName, USHORT nType );
    SbxItem( const SbxItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    const ScriptDocument&
                            GetDocument() const { return m_aDocument; }
    void                    SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    const String&           GetLibName() const { return m_aLibName; }
    void                    SetLibName( const String& aLibName ) { m_aLibName = aLibName; }

    const String&           GetName() const { return m_aName; }
    void                    SetName( const String& aName ) { m_aName = aName; }

    const String&           GetMethodName() const { return m_aMethodName; }
    void                    SetMethodName( const String& aMethodName ) { m_aMethodName = aMethodName; }

    USHORT                  GetType() const { return m_nType; }
    void                    SetType( USHORT nType ) { m_nType = nType; }
};


#endif
