/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _UNQIDX_HXX
#define _UNQIDX_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/contnr.hxx>

// ---------------
// - UniqueIndex -
// ---------------

#define UNIQUEINDEX_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class TOOLS_DLLPUBLIC UniqueIndex : private Container
{
private:
    sal_uIntPtr           nReSize;
    sal_uIntPtr           nStartIndex;
    sal_uIntPtr           nUniqIndex;
    sal_uIntPtr           nCount;

public:
                    using Container::GetCurObject;

                    UniqueIndex( sal_uIntPtr nStartIndex = 0,
                                 sal_uIntPtr nInitSize = 16,
                                 sal_uIntPtr nReSize = 16 );
                    UniqueIndex( const UniqueIndex& rIdx );

    sal_uIntPtr           Insert( sal_uIntPtr nIndex, void* p );
    sal_uIntPtr           Insert( void* p );
    void*           Remove( sal_uIntPtr nIndex );
    void*           Replace( sal_uIntPtr nIndex, void* p );
    void*           Get( sal_uIntPtr nIndex ) const;

    void            Clear();
    sal_uIntPtr           Count() const { return nCount; }

    sal_uIntPtr           GetCurIndex() const;
    sal_uIntPtr           GetIndex( const void* p ) const;
    sal_Bool            IsIndexValid( sal_uIntPtr nIndex ) const;

    void*           Seek( sal_uIntPtr nIndex );
    void*           Seek( void* p );
    void*           First();
    void*           Last();
    void*           Next();
    void*           Prev();

    sal_uIntPtr           GetStartIndex() const { return nStartIndex; }
    sal_uIntPtr           GetCurMaxIndex() const
                        { return (nStartIndex + Container::GetSize()); }

    UniqueIndex&    operator =( const UniqueIndex& rIdx );

    sal_Bool            operator ==( const UniqueIndex& rIdx ) const;
    sal_Bool            operator !=( const UniqueIndex& rIdx ) const
                        { return !(UniqueIndex::operator==( rIdx )); }
};

inline void UniqueIndex::Clear()
{
    Container::Clear();
    nCount     = 0;
    nUniqIndex = 0;
}

// -----------------------
// - DECLARE_UNIQUEINDEX -
// -----------------------

#define DECLARE_UNIQUEINDEX( ClassName, Type )                          \
class ClassName : private UniqueIndex                                   \
{                                                                       \
public:                                                                 \
                using UniqueIndex::Clear;                                       \
                using UniqueIndex::Count;                                       \
                using UniqueIndex::GetCurIndex;                             \
                using UniqueIndex::IsIndexValid;                                \
                using UniqueIndex::GetStartIndex;                               \
                using UniqueIndex::GetCurMaxIndex;                          \
                                                                        \
                ClassName( sal_uIntPtr _nStartIndex = 0,                      \
                           sal_uIntPtr _nInitSize = 16, sal_uIntPtr _nReSize = 16 ):\
                    UniqueIndex( _nStartIndex, _nInitSize, _nReSize ) {}\
                ClassName( const ClassName& rClassName ) :              \
                    UniqueIndex( rClassName ) {}                        \
                                                                        \
    sal_uIntPtr       Insert( sal_uIntPtr nIndex, Type p )                          \
                    { return UniqueIndex::Insert( nIndex, (void*)p ); } \
    sal_uIntPtr       Insert( Type p )                                        \
                    { return UniqueIndex::Insert( (void*)p ); }         \
    Type        Remove( sal_uIntPtr nIndex )                                  \
                    { return (Type)UniqueIndex::Remove( nIndex ); }     \
    Type        Replace( sal_uIntPtr nIndex, Type p )                         \
                    { return (Type)UniqueIndex::Replace( nIndex,        \
                                                         (void*)p ); }  \
    Type        Get( sal_uIntPtr nIndex ) const                               \
                    { return (Type)UniqueIndex::Get( nIndex ); }        \
                                                                        \
    Type        GetCurObject() const                                    \
                    { return (Type)UniqueIndex::GetCurObject(); }       \
    sal_uIntPtr       GetIndex( const Type p ) const                          \
                    { return UniqueIndex::GetIndex( (const void*)p ); } \
                                                                        \
    Type        Seek( sal_uIntPtr nKey )                                      \
                    { return (Type)UniqueIndex::Seek( nKey ); }         \
    Type        Seek( Type p )                                          \
                    { return (Type)UniqueIndex::Seek( (void*)p ); }     \
    Type        First()  { return (Type)UniqueIndex::First(); }         \
    Type        Last()   { return (Type)UniqueIndex::Last(); }          \
    Type        Next()   { return (Type)UniqueIndex::Next(); }          \
    Type        Prev()   { return (Type)UniqueIndex::Prev(); }          \
                                                                        \
    ClassName&  operator =( const ClassName& rClassName )               \
                    { UniqueIndex::operator =( rClassName );            \
                      return *this; }                                   \
                                                                        \
    sal_Bool        operator ==( const ClassName& rIdx ) const              \
                    { return UniqueIndex::operator ==( rIdx ); }        \
    sal_Bool        operator !=( const ClassName& rIdx ) const              \
                    { return UniqueIndex::operator !=( rIdx ); }        \
};

#endif // _UNQIDX_HXX
