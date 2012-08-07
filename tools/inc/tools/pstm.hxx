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
#ifndef _PSTM_HXX
#define _PSTM_HXX

#include <boost/unordered_map.hpp>
#include "tools/toolsdllapi.h"

#include <tools/unqidx.hxx>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <tools/stream.hxx>
#include <map>

#define ERRCODE_IO_NOFACTORY ERRCODE_IO_WRONGFORMAT

/*************************************************************************
*************************************************************************/
class SvPersistBase;
typedef void * (*SvCreateInstancePersist)( SvPersistBase ** );
#define SV_CLASS_REGISTER( Class )                          \
    Register( Class::StaticClassId(), Class::CreateInstance )

class TOOLS_DLLPUBLIC SvClassManager
{
    typedef boost::unordered_map<sal_Int32, SvCreateInstancePersist> Map;
    Map aAssocTable;
public:
    void Register( sal_Int32 nClassId, SvCreateInstancePersist pFunc );
    SvCreateInstancePersist Get( sal_Int32 nClassId );
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
    static  sal_Int32  StaticClassId() { return CLASS_ID; }            \
    static  void *  CreateInstance( SvPersistBase ** ppBase );      \
    friend SvPersistStream& operator >> ( SvPersistStream & rStm,   \
                                          Class *& rpObj);          \
    virtual sal_Int32  GetClassId() const;                             \
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
    sal_Int32          Class::GetClassId() const                       \
                    { return StaticClassId(); }                     \
    SvPersistStream& operator >> (SvPersistStream & rStm, Class *& rpObj)\
                    {                                               \
                        SvPersistBase * pObj;                       \
                        rStm >> pObj;                               \
                        rpObj = PTR_CAST( Class, pObj );            \
                        return rStm;                                \
                    }

#define SV_IMPL_PERSIST1( Class, Super1 )                           \
    TYPEINIT1( Class, Super1 )                                      \
    PRV_SV_IMPL_PERSIST( Class )

/*************************************************************************/
class SvPersistStream;
class SvPersistBase : public SvRttiBase
{
public:
    virtual sal_Int32  GetClassId() const = 0;
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

    void   WriteObjects( SvPersistStream &, sal_Bool bOnlyStreamedObj = sal_False ) const;
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

typedef UniqueIndex<SvPersistBase> SvPersistUIdx;

typedef std::map<SvPersistBase*, sal_uIntPtr> PersistBaseMap;

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
    Weiterhin wird die M"oglichkeit geboten sal_uInt32 Werte komprimiert
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
    PersistBaseMap          aPTable; // Pointer und Key gedreht
    SvPersistUIdx           aPUIdx;
    sal_uIntPtr                   nStartIdx;
    const SvPersistStream * pRefStm;
    sal_uInt32                  nFlags;

    virtual sal_uIntPtr       GetData( void* pData, sal_uIntPtr nSize );
    virtual sal_uIntPtr       PutData( const void* pData, sal_uIntPtr nSize );
    virtual sal_uIntPtr       SeekPos( sal_uIntPtr nPos );
    virtual void        FlushData();
protected:
    void                WriteObj( sal_uInt8 nHdr, SvPersistBase * pObj );
    sal_uInt32              ReadObj( SvPersistBase * & rpObj,
                                sal_Bool bRegister );
public:
    sal_Bool                IsStreamed( SvPersistBase * pObj ) const
                        { return 0 != GetIndex( pObj ); }
    virtual void        ResetError();

                        SvPersistStream( SvClassManager &, SvStream * pStream,
                                         sal_uInt32 nStartIdx = 1 );
                        ~SvPersistStream();

    void                SetStream( SvStream * pStream );
    SvStream *          GetStream() const { return pStm; }
    virtual sal_uInt16      IsA() const;

    SvPersistBase *     GetObject( sal_uIntPtr nIdx ) const;
    sal_uIntPtr               GetIndex( SvPersistBase * ) const;

    void                SetContextFlags( sal_uInt32 n ) { nFlags = n; }
    sal_uInt32              GetContextFlags() const { return nFlags; }

    static void         WriteCompressed( SvStream & rStm, sal_uInt32 nVal );
    static sal_uInt32       ReadCompressed( SvStream & rStm );

    sal_uInt32              WriteDummyLen();
    void                WriteLen( sal_uInt32 nLenPos );
    sal_uInt32              ReadLen( sal_uInt32 * pTestPos );

    SvPersistStream&    WritePointer( SvPersistBase * pObj );
    SvPersistStream&    ReadPointer( SvPersistBase * & rpObj );
    TOOLS_DLLPUBLIC friend SvPersistStream& operator << (SvPersistStream &, SvPersistBase *);
    TOOLS_DLLPUBLIC friend SvPersistStream& operator >> (SvPersistStream &, SvPersistBase * &);

                        // Objekte halten ihre Id's w"ahrend geladen und
                        // gespeichert werden.
    friend SvStream& operator >> ( SvStream &, SvPersistStream & );
    friend SvStream& operator << ( SvStream &, SvPersistStream & );
};

#endif // _PSTM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
