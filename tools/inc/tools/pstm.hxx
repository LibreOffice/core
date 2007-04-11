/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pstm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:15:22 $
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
#ifndef _PSTM_HXX
#define _PSTM_HXX

#include <hash_map>

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _TABLE_HXX
#include <tools/table.hxx>
#endif

#ifndef _INQIDX_HXX
#include <tools/unqidx.hxx>
#endif

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#define ERRCODE_IO_NOFACTORY ERRCODE_IO_WRONGFORMAT

/*************************************************************************
*************************************************************************/
class SvPersistBase;
typedef void * (*SvCreateInstancePersist)( SvPersistBase ** );
#define SV_CLASS_REGISTER( Class )                          \
    Register( Class::StaticClassId(), Class::CreateInstance )

class TOOLS_DLLPUBLIC SvClassManager
{
    typedef std::hash_map< USHORT, SvCreateInstancePersist > Map;
    Map aAssocTable;
public:
    void        Register( USHORT nClassId, SvCreateInstancePersist pFunc );
    SvCreateInstancePersist Get( USHORT nClassId );
};

/************************** S v R t t i B a s e **************************/
/*
#if defined (DOS) && defined (STC)
#ifdef WIN
#error Fuer Win muss der Kram virtual sein (MM/MH)
#endif
class SvRttiBase : public SvRefBase
#else
class SvRttiBase : virtual public SvRefBase
#endif
*/
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

//#if 0 // _SOLAR__PRIVATE
DECLARE_UNIQUEINDEX( SvPersistUIdx,SvPersistBase *)
//#else
//typedef UniqueIndex SvPersistUIdx;
//#endif

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
