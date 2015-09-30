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
#ifndef INCLUDED_TOOLS_PSTM_HXX
#define INCLUDED_TOOLS_PSTM_HXX

#include <tools/toolsdllapi.h>
#include <tools/unqidx.hxx>
#include <tools/ref.hxx>
#include <tools/rtti.hxx>
#include <tools/stream.hxx>
#include <map>
#include <unordered_map>

#define ERRCODE_IO_NOFACTORY ERRCODE_IO_WRONGFORMAT

class SvPersistBase;

typedef void * (*SvCreateInstancePersist)( SvPersistBase ** );

#define SV_CLASS_REGISTER( Class )                          \
    Register( Class::StaticClassId(), Class::CreateInstance )

class TOOLS_DLLPUBLIC SvClassManager
{
    typedef std::unordered_map<sal_Int32, SvCreateInstancePersist> Map;
    Map aAssocTable;

public:
    void Register( sal_Int32 nClassId, SvCreateInstancePersist pFunc );
    SvCreateInstancePersist Get( sal_Int32 nClassId );
};

class TOOLS_DLLPUBLIC SvRttiBase : public SvRefBase
{
public:
            TYPEINFO();
};

#define SV_DECL_PERSIST1( Class, Super1, CLASS_ID )                 \
    TYPEINFO_OVERRIDE();                                            \
    static  sal_Int32  StaticClassId() { return CLASS_ID; }         \
    static  void *  CreateInstance( SvPersistBase ** ppBase );      \
    friend SvPersistStream& operator >> ( SvPersistStream & rStm,   \
                                          Class *& rpObj);          \
    virtual sal_Int32  GetClassId() const SAL_OVERRIDE;              \
    virtual void    Load( SvPersistStream & ) SAL_OVERRIDE;          \
    virtual void    Save( SvPersistStream & ) SAL_OVERRIDE;

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
                        rpObj = dynamic_cast< Class*>( pObj );      \
                        return rStm;                                \
                    }

#define SV_IMPL_PERSIST1( Class, Super1 )                           \
    TYPEINIT1( Class, Super1 )                                      \
    PRV_SV_IMPL_PERSIST( Class )

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

typedef std::map<SvPersistBase*, sal_uIntPtr> PersistBaseMap;

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
    SvClassManager &    rClassMgr;
    SvStream *          pStm;
    PersistBaseMap      aPTable; // reversed pointer and key
    UniqueIndex<SvPersistBase>
                        aPUIdx;
    sal_uIntPtr         nStartIdx;
    const SvPersistStream * pRefStm;

    virtual sal_uIntPtr GetData( void* pData, sal_uIntPtr nSize ) SAL_OVERRIDE;
    virtual sal_uIntPtr PutData( const void* pData, sal_uIntPtr nSize ) SAL_OVERRIDE;
    virtual sal_uInt64  SeekPos(sal_uInt64 nPos) SAL_OVERRIDE;
    virtual void        FlushData() SAL_OVERRIDE;

protected:
    void                WriteObj( sal_uInt8 nHdr, SvPersistBase * pObj );
    sal_uInt32          ReadObj( SvPersistBase * & rpObj, bool bRegister );

public:
    virtual void        ResetError() SAL_OVERRIDE;

                        SvPersistStream( SvClassManager &, SvStream * pStream,
                                         sal_uInt32 nStartIdx = 1 );
                        virtual ~SvPersistStream();

    void                SetStream( SvStream * pStream );
    SvStream *          GetStream() const { return pStm; }

    SvPersistBase *     GetObject( sal_uIntPtr nIdx ) const;
    sal_uIntPtr         GetIndex( SvPersistBase * ) const;

    static void         WriteCompressed( SvStream & rStm, sal_uInt32 nVal );
    static sal_uInt32   ReadCompressed( SvStream & rStm );

    sal_uInt32          WriteDummyLen();
    void                WriteLen( sal_uInt32 nLenPos );
    sal_uInt32          ReadLen( sal_uInt32 * pTestPos );

    SvPersistStream&    WritePointer( SvPersistBase * pObj );
    SvPersistStream&    ReadPointer( SvPersistBase * & rpObj );
    TOOLS_DLLPUBLIC friend SvPersistStream& WriteSvPersistBase(SvPersistStream &, SvPersistBase *);
    TOOLS_DLLPUBLIC friend SvPersistStream& operator >> (SvPersistStream &, SvPersistBase * &);

    // Objects maintain their IDs while storing and loading to/from stream
    friend SvStream& operator >> ( SvStream &, SvPersistStream & );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
