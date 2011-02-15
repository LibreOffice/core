/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _PSTM_HXX
#define _PSTM_HXX

#include <boost/unordered_map.hpp>
#include "tools/toolsdllapi.h"

#include <tools/table.hxx>

#include <tools/unqidx.hxx>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <tools/stream.hxx>

#define ERRCODE_IO_NOFACTORY ERRCODE_IO_WRONGFORMAT

/*************************************************************************
*************************************************************************/
class SvPersistBase;
typedef void * (*SvCreateInstancePersist)( SvPersistBase ** );
#define SV_CLASS_REGISTER( Class )                          \
    Register( Class::StaticClassId(), Class::CreateInstance )

class TOOLS_DLLPUBLIC SvClassManager
{
    typedef boost::unordered_map< USHORT, SvCreateInstancePersist > Map;
    Map aAssocTable;
public:
    void        Register( USHORT nClassId, SvCreateInstancePersist pFunc );
    SvCreateInstancePersist Get( USHORT nClassId );
};

/************************** S v R t t i B a s e **************************/

class TOOLS_DLLPUBLIC SvRttiBase : public SvRefBase
{
public:
            TYPEINFO();
};
SV_DECL_IMPL_REF(SvRttiBase)

/*************************************************************************/
#define SV_DECL_PERSIST( Class, CLASS_ID )                          \
    TYPEINFO();                                                     \
    static  USHORT  StaticClassId() { return CLASS_ID; }            \
    static  void *  CreateInstance( SvPersistBase ** ppBase );      \
    friend SvPersistStream& operator >> ( SvPersistStream & rStm,   \
                                          Class *& rpObj);          \
    virtual USHORT  GetClassId() const;                             \
    virtual void    Load( SvPersistStream & );                      \
    virtual void    Save( SvPersistStream & );

#define SV_DECL_PERSIST1( Class, Super1, CLASS_ID )                 \
    SV_DECL_PERSIST( Class, CLASS_ID )

#define PRV_SV_IMPL_PERSIST( Class )                                \
    void *          Class::CreateInstance( SvPersistBase ** ppBase )\
                    {                                               \
                        Class * p = new Class();                    \
                        *ppBase = p;                                \
                        return p;                                   \
                    }                                               \
    USHORT          Class::GetClassId() const                       \
                    { return StaticClassId(); }                     \
    SvPersistStream& operator >> (SvPersistStream & rStm, Class *& rpObj)\
                    {                                               \
                        SvPersistBase * pObj;                       \
                        rStm >> pObj;                               \
                        rpObj = PTR_CAST( Class, pObj );            \
                        return rStm;                                \
                    }

#define SV_IMPL_PERSIST( Class )                                    \
    TYPEINIT0( Class )                                              \
    PRV_SV_IMPL_PERSIST( Class )

#define SV_IMPL_PERSIST1( Class, Super1 )                           \
    TYPEINIT1( Class, Super1 )                                      \
    PRV_SV_IMPL_PERSIST( Class )

/*************************************************************************/
class SvPersistStream;
class SvPersistBase : public SvRttiBase
{
public:
    virtual USHORT  GetClassId() const = 0;
    virtual void    Load( SvPersistStream & ) = 0;
    virtual void    Save( SvPersistStream & ) = 0;
    TOOLS_DLLPUBLIC friend SvPersistStream& operator >> ( SvPersistStream & rStm,
                                          SvPersistBase *& rpObj );
};
SV_DECL_IMPL_REF(SvPersistBase)

/*************************************************************************/
// Damit die Liste, anders benannt wird
typedef SvPersistBase SuperSvPersistBase;
SV_DECL_REF_LIST_VISIBILITY(SuperSvPersistBase,SuperSvPersistBase*,TOOLS_DLLPUBLIC)
SV_IMPL_REF_LIST(SuperSvPersistBase,SuperSvPersistBase*)

class TOOLS_DLLPUBLIC SvPersistBaseMemberList : public SuperSvPersistBaseMemberList
{
public:
    SvPersistBaseMemberList();
    SvPersistBaseMemberList(USHORT nInitSz, USHORT nResize );

    void   WriteObjects( SvPersistStream &, BOOL bOnlyStreamedObj = FALSE ) const;
    TOOLS_DLLPUBLIC friend SvPersistStream& operator << (SvPersistStream &, const SvPersistBaseMemberList &);
    TOOLS_DLLPUBLIC friend SvPersistStream& operator >> (SvPersistStream &, SvPersistBaseMemberList &);
};

/*************************************************************************/
#define SV_DECL_PERSIST_LIST(ClassName,EntryName)\
class ClassName##MemberList : public SvPersistBaseMemberList\
{\
public:\
    PRV_SV_DECL_MEMBER_LIST(ClassName,EntryName)\
};

#define SV_IMPL_PERSIST_LIST(ClassName,EntryName)\
    PRV_SV_IMPL_MEMBER_LIST(ClassName,EntryName,SvPersistBaseMemberList)

#define SV_DECL_IMPL_PERSIST_LIST(ClassName,EntryName)\
SV_DECL_PERSIST_LIST(ClassName,EntryName)\
SV_IMPL_PERSIST_LIST(ClassName,EntryName)

DECLARE_UNIQUEINDEX( SvPersistUIdx,SvPersistBase *)

//=========================================================================
class SvStream;
class TOOLS_DLLPUBLIC SvPersistStream : public SvStream
/*  [Beschreibung]

    Mit dieser Klasse k"onnen Laufzeit Objektstrukturen gespeichert
    und geladen werden. Es m"ussen immer alle beteiligten Objekte
    gespeichert oder geladen werden. Um die Objekte automatisch
    laden zu k"onnen, wird eine Factory f"ur jede Klasse von Objekten,
    die im Stream vorkommen k"onnen, ben"otigt. Die Liste aller Klassen
    wird in einem <SvClassManager> Objekt gespeichert und dem
     SvPersistStream "ubergeben, wenn er erzeugt wird.
    Weiterhin wird die M"oglichkeit geboten UINT32 Werte komprimiert
    zu schreiben und zu lesen (<SvPersistStream::WriteCompressed>,
    <SvPersistStream::ReadCompressed>).
    Es gibt auch die drei Hilfsmethoden <SvPersistStream::WriteDummyLen>,
    <SvPersistStream::WriteLen> und <SvPersistStream::ReadLen> um eine
    L"ange vor das Objekt zu schreiben.

    [Beispiel]

    Ein konkretes Beispiel ist im Konstruktor beschrieben.
    Objekt A verweist auf B, dieses auf C und das wieder auf A.
    C verweist auf D.

    Die Reihenfolge beim Speichern und Laden ist egal, sie muss nur
    gleich sein:
    Speichern:      Laden:
    A,B,C,D         A,B,C,D     richtig
    B,A,C,D         B,A,C,D     richtig
    C,A,B,D         A,B,C,D     falsch
    A,B,C,D         A,B,C       falsch

    [Anmerkung]

    Das Dateiformat zwischen DBG_UTIL und !DBG_UTIL ist unterschiedlich,
    kann aber von beiden Versionen gelesen werden.
*/
{
    SvClassManager &        rClassMgr;
    SvStream *              pStm;
    Table                   aPTable; // Pointer und Key gedreht
    SvPersistUIdx           aPUIdx;
    ULONG                   nStartIdx;
    const SvPersistStream * pRefStm;
    UINT32                  nFlags;

    virtual ULONG       GetData( void* pData, ULONG nSize );
    virtual ULONG       PutData( const void* pData, ULONG nSize );
    virtual ULONG       SeekPos( ULONG nPos );
    virtual void        FlushData();
protected:
    ULONG               GetCurMaxIndex( const SvPersistUIdx & ) const;
    ULONG               GetCurMaxIndex() const
                        { return GetCurMaxIndex( aPUIdx ); }

    void                WriteObj( BYTE nHdr, SvPersistBase * pObj );
    UINT32              ReadObj( SvPersistBase * & rpObj,
                                BOOL bRegister );
public:
    BOOL                IsStreamed( SvPersistBase * pObj ) const
                        { return 0 != GetIndex( pObj ); }
    virtual void        ResetError();

                        SvPersistStream( SvClassManager &, SvStream * pStream,
                                         UINT32 nStartIdx = 1 );
                        SvPersistStream( SvClassManager &, SvStream * pStream,
                                         const SvPersistStream & rPersStm );
                        ~SvPersistStream();

    void                SetStream( SvStream * pStream );
    SvStream *          GetStream() const { return pStm; }
    virtual USHORT      IsA() const;

    SvPersistBase *     GetObject( ULONG nIdx ) const;
    ULONG               GetIndex( SvPersistBase * ) const;

    void                SetContextFlags( UINT32 n ) { nFlags = n; }
    UINT32              GetContextFlags() const { return nFlags; }

    static void         WriteCompressed( SvStream & rStm, UINT32 nVal );
    static UINT32       ReadCompressed( SvStream & rStm );

    UINT32              WriteDummyLen();
    void                WriteLen( UINT32 nLenPos );
    UINT32              ReadLen( UINT32 * pTestPos );

    SvPersistStream&    WritePointer( SvPersistBase * pObj );
    SvPersistStream&    ReadPointer( SvPersistBase * & rpObj );
    TOOLS_DLLPUBLIC friend SvPersistStream& operator << (SvPersistStream &, SvPersistBase *);
    TOOLS_DLLPUBLIC friend SvPersistStream& operator >> (SvPersistStream &, SvPersistBase * &);

                        // Objekte halten ihre Id's w"ahrend geladen und
                        // gespeichert werden.
    friend SvStream& operator >> ( SvStream &, SvPersistStream & );
    friend SvStream& operator << ( SvStream &, SvPersistStream & );
    ULONG               InsertObj( SvPersistBase * );
    ULONG               RemoveObj( SvPersistBase * );
};

#endif // _PSTM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
