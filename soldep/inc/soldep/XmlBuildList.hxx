/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XmlBuildList.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _XMLBUILDLIST_HXX
#define _XMLBUILDLIST_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#include <sal/types.h>

class FullByteStringListWrapper;

struct STRUCT_SV;

DECLARE_LIST ( FullByteStingListImpl, ByteString* )

class FullByteStringList : public FullByteStingListImpl
{
    friend  class FullByteStringListWrapper;
    ULONG           nRef;
    void            IncRef() { nRef ++; }
    void            DecRef() { nRef --; }
    ULONG           GetRef() { return nRef; }
public:
    FullByteStringList();
    ~FullByteStringList();
    ULONG           GetPos (ByteString& rStr);
    ULONG           GetPos( const ByteString* p ) const { return FullByteStingListImpl::GetPos(p); }
    ULONG           GetPos( const ByteString* p, ULONG nStartIndex,
                        BOOL bForward = TRUE ) const { return
                        FullByteStingListImpl::GetPos(p, nStartIndex, bForward); }
};

class FullByteStringListWrapper
{
private:
    FullByteStringList*  pStringList;

public:
    FullByteStringListWrapper (FullByteStringList* pList) {pStringList = pList;}
    ~FullByteStringListWrapper ();

    FullByteStringList* GetStringList() const {return pStringList;}

    void        Insert( ByteString* p ){pStringList->Insert(p);}
    void        Insert( ByteString* p, ULONG nIndex ){pStringList->Insert(p, nIndex);}
    void        Insert( ByteString* pNew, ByteString* pOld ){pStringList->Insert(pNew, pOld);}

    ByteString*       Remove(){return pStringList->Remove();}
    ByteString*       Remove( ULONG nIndex ){return pStringList->Remove(nIndex);}
    ByteString*       Remove( ByteString* p )
                    { return pStringList->Remove(p); }

    ByteString*       Replace( ByteString* p ){ return pStringList->Replace(p); }
    ByteString*       Replace( ByteString* p, ULONG nIndex ){ return pStringList->Replace(p, nIndex); }
    ByteString*       Replace( ByteString* pNew, ByteString* pOld )
                    { return pStringList->Replace( pNew, pOld ); }

    ULONG       Count() const { return pStringList->Count(); }
    void        Clear(){pStringList->Clear(); }

    ByteString*       GetCurObject() const { return pStringList->GetCurObject(); }
    ULONG       GetCurPos() const { return pStringList->GetCurPos(); }
    ByteString*       GetObject( ULONG nIndex ) const { return pStringList->GetObject( nIndex ); }
    ULONG       GetPos( const ByteString* p ) const { return pStringList->GetPos(p); }
    ULONG       GetPos( const ByteString* p, ULONG nStartIndex,
                        BOOL bForward = TRUE ) const { return
                        pStringList->GetPos(p, nStartIndex, bForward); }
    ULONG       GetPos (ByteString& rStr) { return pStringList->GetPos(rStr); }

    ByteString*       Seek( ULONG nIndex ) { return pStringList->Seek(nIndex); }
    ByteString*       Seek( ByteString* p ) { return pStringList->Seek(p); }

    ByteString*       First() { return pStringList->First(); };
    ByteString*       Last() { return pStringList->Last(); };
    ByteString*       Next() { return pStringList->Next(); };
    ByteString*       Prev() { return pStringList->Prev(); };

    FullByteStringListWrapper & operator= ( const FullByteStringListWrapper &);
};


class XmlBuildListException {
    private:
        const char* ExceptionMessage;
    public:
        XmlBuildListException(const char* Message)
            :ExceptionMessage(Message) {};
        const char* getMessage() {return ExceptionMessage;};
};

class XmlBuildList {
    protected:
        bool PerlInitialized;
        void initPerl(const char* ModulePath);
        // Set of arrays for each function, which uses char* arrays
        // as a return value. Each call to such function clears its array up
        // before it's being populated
        FullByteStringList* extractArray();
        // C objects for $string1 & $string2 Perl variables
        STRUCT_SV* string_obj1;
        STRUCT_SV* string_obj2;
        STRUCT_SV* string_obj3;
        char* getError();
        void checkOperationSuccess();
    public:
        // alternative to constructor
        XmlBuildList(const ByteString& rModulePath);
        ~XmlBuildList();
        void loadXMLFile(const ByteString& rBuildList);
        FullByteStringListWrapper getProducts();
        FullByteStringListWrapper getJobDirectories(const ByteString& rJobType, const ByteString& rJobPlatform);
        FullByteStringListWrapper getModuleDependencies(const ByteString& rProduct, const ByteString& rDependencyType);
        FullByteStringListWrapper getModuleDependencies(FullByteStringListWrapper& rProducts, const ByteString& rDependencyType);
        FullByteStringListWrapper getJobBuildReqs(const ByteString& rJobDir, const ByteString& rBuildReqPlatform);
        ByteString getModuleDepType(const ByteString& rDepModuleName);
        FullByteStringListWrapper getModuleDepTypes(FullByteStringListWrapper& rProducts);
        sal_Bool hasModuleDepType(FullByteStringListWrapper& rProducts, const ByteString& rDependencyType);
        FullByteStringListWrapper getModuleProducts(const ByteString& rDepModuleName);
        ByteString getModuleName();
        FullByteStringListWrapper getDirDependencies(const ByteString& rJobDir, const ByteString& rJobType, const ByteString& rJobPlatform);
        FullByteStringListWrapper getJobTypes(const ByteString& rJobDir);
        FullByteStringListWrapper getJobPlatforms(const ByteString& rJobDir);
        FullByteStringListWrapper getJobBuildReqPlatforms(const ByteString& rJobDir, const ByteString& rBuildReqName);
};

#endif
