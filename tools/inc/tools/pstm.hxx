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

class SvPersistListWriteable
{
public:
    virtual ~SvPersistListWriteable() {}
    virtual size_t size() const = 0;
    virtual SvPersistBase* GetPersistBase(size_t idx) const = 0;
};
class SvPersistListReadable
{
public:
    virtual ~SvPersistListReadable() {}
    virtual void push_back(SvPersistBase* p) = 0;
};

void TOOLS_DLLPUBLIC WritePersistListObjects(const SvPersistListWriteable& rList, SvPersistStream & rStm, bool bOnlyStreamed = false );

void TOOLS_DLLPUBLIC ReadObjects( SvPersistListReadable& rLst, SvPersistStream & rStm);

// T has to be a subtype of "SvPersistBase*"
template<typename T>
class SvDeclPersistList : public SvRefMemberList<T>, public SvPersistListWriteable, public SvPersistListReadable
{
public:
   // implement the reader/writer adapter methods
    size_t size() const { return SvRefMemberList<T>::size(); }
    SvPersistBase* GetPersistBase(size_t idx) const { return SvRefMemberList<T>::operator[](idx); }
    void push_back(SvPersistBase* p) { SvRefMemberList<T>::push_back(static_cast<T>(p)); }
    void WriteObjects(SvPersistStream & rStm, bool bOnlyStreamed ) const { WritePersistListObjects(*this, rStm, bOnlyStreamed); }
};

template<typename T>
SvPersistStream& operator << (SvPersistStream &rStm, const SvDeclPersistList<T> &rLst)
{
    WritePersistListObjects( rLst, rStm );
    return rStm;
};

template<typename T>
SvPersistStream& operator >> (SvPersistStream &rStm, SvDeclPersistList<T> &rLst)
{
    ReadObjects( rLst, rStm );
    return rStm;
};

typedef UniqueIndex<SvPersistBase> SvPersistUIdx;

typedef std::map<SvPersistBase*, sal_uIntPtr> PersistBaseMap;

//=========================================================================
class SvStream;

/** Persistent Stream

    This class provides accessor to storing and loading runtime objects.
    All dependent objects have to be stored as well.
    In order to load objects automatically, every object class must
    provide a Factory method to read an object from stream.
    The list of all classes is stored in a <SvClassManager> object
    and is sent to SvPersistStream upon initialization.
    By using the Method SvPersistStream::WriteCompressed and
    SvPersistStream::ReadCompressed, compressed sal_uInt32 values may be
    written to / read from the Stream.
    Several helper methods exists for writing and reading
    object lengths to the stream: SvPersistStream::WriteDummyLen,
    SvPersistStream::WriteLen and SvPersistStream::ReadLen.

    [Example]

    One example is described in the constructor.
    Assume a ring-like dependency, where A referenes B,
    B itself references C, and C references to both D and A.

    The order of the objects upon saving and loading does not matter,
    as long objects are loaded in the same order they were stored.

    Saving:         Loading:
    A,B,C,D         A,B,C,D     correct
    B,A,C,D         B,A,C,D     correct
    C,A,B,D         A,B,C,D     wrong
    A,B,C,D         A,B,C       wrong

    @note The file formats DBG_UTIL and !DBG_UTIL differ, but we can read from
          both versions.
*/
class TOOLS_DLLPUBLIC SvPersistStream : public SvStream
{
    SvClassManager &        rClassMgr;
    SvStream *              pStm;
    PersistBaseMap          aPTable; // reversed pointer and key
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

    // Objects maintain their IDs while storing and loading to/from stream
    friend SvStream& operator >> ( SvStream &, SvPersistStream & );
    friend SvStream& operator << ( SvStream &, SvPersistStream & );
};

#endif // _PSTM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
