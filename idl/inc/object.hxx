/*************************************************************************
 *
 *  $RCSfile: object.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#ifndef _OBJECT_HXX
#define _OBJECT_HXX

#define _SVSTDARR_ULONGS
//#include <svtools/svstdarr.hxx>
#include <types.hxx>
#include <slot.hxx>

struct SvSlotElement
{
    SvMetaSlotRef   xSlot;
    ByteString          aPrefix;
            SvSlotElement( SvMetaSlot * pS, const ByteString & rPrefix )
                : xSlot( pS )
                , aPrefix( rPrefix )
             {}
};
DECLARE_LIST( SvSlotElementList, SvSlotElement* )
class SvMetaClass;
DECLARE_LIST( SvMetaClassList, SvMetaClass* )

class SvULongs : public List
{
public:
    void    Insert( ULONG& rId, ULONG nPos ) { ULONG nId(rId ); List::Insert( (void*) nId, nPos ); }
    void    Remove( ULONG& rId ){ ULONG nId(rId ); List::Remove( (void*) nId ); }
    ULONG   GetObject( ULONG nPos ){ return (ULONG) List::GetObject( nPos ); }
};

/******************** class SvClassElement *******************************/
SV_DECL_REF(SvMetaClass)
class SvClassElement : public SvPersistBase
{
    SvBOOL                      aAutomation;
    ByteString                      aPrefix;
    SvMetaClassRef              xClass;
public:
            SV_DECL_PERSIST1( SvClassElement, SvPersistBase, 1 )
            SvClassElement();

    void            SetPrefix( const ByteString & rPrefix )
                    { aPrefix = rPrefix; }
    const ByteString &  GetPrefix() const
                    { return aPrefix; }

    void            SetAutomation( BOOL rAutomation )
                    { aAutomation = rAutomation; }
    BOOL            GetAutomation() const
                    { return aAutomation; }

    void            SetClass( SvMetaClass * pClass )
                    { xClass = pClass; }
    SvMetaClass *   GetClass() const
                    { return xClass; }
};

SV_DECL_IMPL_REF(SvClassElement)
SV_DECL_IMPL_PERSIST_LIST(SvClassElement,SvClassElement *)

/******************** class SvMetaClass *********************************/
class SvMetaModule;
SV_DECL_PERSIST_LIST(SvMetaClass,SvMetaClass *)
class SvMetaClass : public SvMetaType
{
    SvMetaAttributeMemberList   aAttrList;
    SvClassElementMemberList    aClassList;
    SvMetaClassRef              aSuperClass;
    SvBOOL                      aAutomation;
    SvMetaClassRef              xAutomationInterface;

//    void                FillSbxMemberObject( SvIdlDataBase & rBase,
//                                            SbxObject *, StringList &,
//                                            BOOL bVariable );
    BOOL                TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                     SvMetaAttribute & rAttr ) const;
#ifdef IDL_COMPILER
    void                WriteSlotStubs( const ByteString & rShellName,
                                        SvSlotElementList & rSlotList,
                                        ByteStringList & rList,
                                        SvStream & rOutStm );
    USHORT              WriteSlotParamArray( SvIdlDataBase & rBase,
                                            SvSlotElementList & rSlotList,
                                            SvStream & rOutStm );
    USHORT              WriteSlots( const ByteString & rShellName, USHORT nCount,
                                    SvSlotElementList & rSlotList,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm );

    void                InsertSlots( SvSlotElementList& rList, SvULongs& rSuperList,
                                    SvMetaClassList & rClassList,
                                    const ByteString & rPrefix, SvIdlDataBase& rBase );

protected:
    virtual void    ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void    WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
    virtual void    ReadContextSvIdl( SvIdlDataBase &,
                                     SvTokenStream & rInStm );
    virtual void    WriteContextSvIdl( SvIdlDataBase & rBase,
                                     SvStream & rOutStm, USHORT nTab );
    void            WriteOdlMembers( ByteStringList & rSuperList,
                                    BOOL bVariable, BOOL bWriteTab,
                                    SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaClass, SvMetaType, 6 )
            SvMetaClass();

    BOOL                GetAutomation() const
                        { return aAutomation; }
    SvMetaClass *       GetSuperClass() const
                        { return aSuperClass; }

    void                FillClasses( SvMetaClassList & rList );
//    virtual void        FillSbxObject( SvIdlDataBase & rBase, SbxObject * );

    const SvClassElementMemberList&
                        GetClassList() const
                        { return aClassList; }

#ifdef IDL_COMPILER
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 USHORT nTab,
                                  WriteType, WriteAttribute = 0 );
    virtual void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
    virtual void        WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                                Table* pTable );
    virtual void        WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 Table * pTable );
    virtual void        WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
#endif
};
SV_IMPL_REF(SvMetaClass)
SV_IMPL_PERSIST_LIST(SvMetaClass,SvMetaClass *)


#endif // _OBJECT_HXX

