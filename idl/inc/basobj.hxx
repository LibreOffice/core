/*************************************************************************
 *
 *  $RCSfile: basobj.hxx,v $
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

#ifndef _BASOBJ_HXX
#define _BASOBJ_HXX

#include <tools/ref.hxx>
#include <bastype.hxx>

#ifndef _PSTM_HXX //autogen
#include <tools/pstm.hxx>
#endif

class SvTokenStream;
class SvMetaObject;
class SvAttributeList;
class SvIdlDataBase;

typedef SvMetaObject * (*CreateMetaObjectType)();
#define IDL_WRITE_COMPILER  0x8000
#define IDL_WRITE_BROWSER   0x4000
#define IDL_WRITE_CALLING   0x2000
#define IDL_WRITE_MASK      0xE000

#define C_PREF  "C_"

enum WriteType
{
    WRITE_IDL, WRITE_ODL, WRITE_SLOTMAP, WRITE_C_HEADER, WRITE_C_SOURCE,
    WRITE_CXX_HEADER, WRITE_CXX_SOURCE, WRITE_DOCU
};

enum
{
    WA_METHOD = 0x1,    WA_VARIABLE = 0x2,  WA_ARGUMENT = 0x4,
    WA_STRUCT = 0x8,    WA_READONLY = 0x10
};
typedef int WriteAttribute;

/******************** Meta Factory **************************************/
#ifdef IDL_COMPILER

#define PRV_SV_DECL_META_FACTORY( Class )                               \
    static SvAttributeList * pAttribList;                               \
    static SvAttributeList & GetAttributeList();                        \
    static SvMetaObject * Create() { return new Class; }                \
    static const char *   GetClassName() { return #Class; }

#define PRV_SV_IMPL_META_FACTORY( Class )                               \
    SvAttributeList * Class::pAttribList = NULL;

#else

#define PRV_SV_DECL_META_FACTORY( Class )

#define PRV_SV_IMPL_META_FACTORY( Class )

#endif // IDL_COMPILER

#define SV_DECL_META_FACTORY( Class, CLASS_ID )                         \
    SV_DECL_PERSIST( Class, CLASS_ID )                                  \
    PRV_SV_DECL_META_FACTORY( Class )                                   \
    SvMetaObject *  MakeClone() const;                                  \
    Class *         Clone() const { return (Class *)MakeClone(); }


#define SV_DECL_META_FACTORY1( Class, Super1, CLASS_ID )                \
    SV_DECL_PERSIST1( Class, Super1, CLASS_ID )                         \
    PRV_SV_DECL_META_FACTORY( Class )                                   \
    SvMetaObject *  MakeClone() const;                                  \
    Class *         Clone() const { return (Class *)MakeClone(); }

#define SV_IMPL_META_FACTORY( Class )                                   \
    PRV_SV_IMPL_META_FACTORY( Class )                                   \
    SV_IMPL_PERSIST( Class )                                            \
    SvMetaObject *  Class::MakeClone() const { return new Class( *this ); }


#define SV_IMPL_META_FACTORY1( Class, Super1 )                          \
    PRV_SV_IMPL_META_FACTORY( Class )                                   \
    SV_IMPL_PERSIST1( Class, Super1 )                                   \
    SvMetaObject *  Class::MakeClone() const { return new Class( *this ); }


/******************** class SvMetaObject ********************************/
class SvMetaObject : public SvPersistBase
{
public:
            SV_DECL_META_FACTORY1( SvMetaObject, SvPersistBase, 14 )
            SvMetaObject();

#ifdef IDL_COMPILER
    static void         WriteTab( SvStream & rOutStm, USHORT nTab );
    static BOOL         TestAndSeekSpaceOnly( SvStream &, ULONG nBegPos );
    static void         Back2Delemitter( SvStream & );
    static void         WriteStars( SvStream & );

    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                WriteType, WriteAttribute = 0 );

    virtual void        WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
#endif
};
SV_DECL_IMPL_REF(SvMetaObject)
//SV_DECL_IMPL_PERSIST_LIST(SvMetaObject,SvMetaObject *)
SV_DECL_PERSIST_LIST(SvMetaObject,SvMetaObject *)
SV_IMPL_PERSIST_LIST(SvMetaObject,SvMetaObject *)


class SvMetaObjectMemberStack
{
    SvMetaObjectMemberList aList;
public:
            SvMetaObjectMemberStack() {;}

    void            Push( SvMetaObject * pObj )
                    { aList.Insert( pObj, LIST_APPEND ); }
    SvMetaObject *  Pop() { return aList.Remove( aList.Count() -1 ); }
    SvMetaObject *  Top() const { return aList.GetObject( aList.Count() -1 ); }
    void            Clear() { aList.Clear(); }
    ULONG           Count() const { return aList.Count(); }

    SvMetaObject *  Get( TypeId nType )
                    {
                        SvMetaObject * pObj = aList.Last();
                        while( pObj )
                        {
                            if( pObj->IsA( nType ) )
                                return pObj;
                            pObj = aList.Prev();
                        }
                        return NULL;
                    }
};

/******************** class SvMetaName **********************************/
class SvMetaName : public SvMetaObject
{
    SvString      aName;
    SvHelpContext aHelpContext;
    SvHelpText    aHelpText;
    SvString      aConfigName;
    SvString      aDescription;

protected:
#ifdef IDL_COMPILER
    virtual BOOL ReadNameSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
            void DoReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm,
                                     char c = '\0' );
    virtual void ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void WriteContextSvIdl( SvIdlDataBase & rBase,
                                    SvStream & rOutStm, USHORT nTab );
    virtual void ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm );
    virtual void WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                       SvStream & rOutStm, USHORT nTab );
    virtual void WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                        WriteType, WriteAttribute = 0);
    virtual void WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                        WriteType, WriteAttribute = 0);
#endif
public:
            SV_DECL_META_FACTORY1( SvMetaName, SvMetaObject, 15 )
            SvMetaName();

    virtual BOOL                SetName( const ByteString & rName, SvIdlDataBase * = NULL  );
    void                        SetDescription( const ByteString& rText )
                                { aDescription = rText; }
    const SvHelpContext&        GetHelpContext() const { return aHelpContext; }
    virtual const SvString &    GetName() const { return aName; }
    virtual const SvString &    GetHelpText() const { return aHelpText; }
    virtual const SvString &    GetConfigName() const{ return aConfigName; }
    virtual const SvString&     GetDescription() const{ return aDescription; }

#ifdef IDL_COMPILER
    virtual BOOL        Test( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );
    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                       WriteType, WriteAttribute = 0);
    void                WriteDescription( SvIdlDataBase& rBase, SvStream& rOutStm );
#endif
};
SV_DECL_IMPL_REF(SvMetaName)
SV_DECL_IMPL_PERSIST_LIST(SvMetaName,SvMetaName *)


/******************** class SvMetaReference *****************************/
SV_DECL_REF(SvMetaReference)
class SvMetaReference : public SvMetaName
{
protected:
    SvMetaReferenceRef  aRef;
public:
            SV_DECL_META_FACTORY1( SvMetaReference, SvMetaName, 17 )
            SvMetaReference();

    const SvString &    GetName() const
                        {
                            return ( !aRef.Is()
                                    || SvMetaName::GetName().Len() )
                                ? SvMetaName::GetName()
                                : aRef->GetName();
                        }

    const SvString &    GetHelpText() const
                        {
                            return ( !aRef.Is()
                                    || SvMetaName::GetHelpText().Len() )
                                ? SvMetaName::GetHelpText()
                                : aRef->GetHelpText();
                        }

    const SvString &    GetConfigName() const
                        {
                            return ( !aRef.Is()
                                    || SvMetaName::GetConfigName().Len() )
                                ? SvMetaName::GetConfigName()
                                : aRef->GetConfigName();
                        }

    const SvString &    GetDescription() const
                        {
                            return ( !aRef.Is()
                                    || SvMetaName::GetDescription().Len() )
                                ? SvMetaName::GetDescription()
                                : aRef->GetDescription();
                        }
    SvMetaReference *   GetRef() const { return aRef; }
    void                SetRef( SvMetaReference * pRef  )
                        { aRef = pRef; }
};
SV_IMPL_REF(SvMetaReference)
SV_DECL_IMPL_PERSIST_LIST(SvMetaReference,SvMetaReference *)


/******************** class SvMetaExtern *********************************/
class SvMetaModule;
class SvMetaExtern : public SvMetaReference
{
    SvMetaModule *          pModule;    // in welchem Modul enthalten

    SvUUId                  aUUId;
    SvVersion               aVersion;
    BOOL                    bReadUUId;
    BOOL                    bReadVersion;
public:
                        SV_DECL_META_FACTORY1( SvMetaExtern, SvMetaName, 16 )
                        SvMetaExtern();

    SvMetaModule *      GetModule() const;

    const SvGlobalName &GetUUId() const;
    const SvVersion &   GetVersion() const { return aVersion; }
#ifdef IDL_COMPILER
    void                SetModule( SvIdlDataBase & rBase );
    virtual BOOL        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab );

    virtual void        Write( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                  WriteType, WriteAttribute = 0);
protected:
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    virtual void        WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                              SvStream & rOutStm, USHORT nTab );
    virtual void        WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm, USHORT nTab,
                                          WriteType, WriteAttribute = 0);
#endif
};
SV_DECL_IMPL_REF(SvMetaExtern)
SV_DECL_IMPL_PERSIST_LIST(SvMetaExtern,SvMetaExtern *)


#endif // _BASOBJ_HXX


