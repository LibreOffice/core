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
#ifndef _BASMGR_HXX
#define _BASMGR_HXX

#include <tools/string.hxx>
#include <svl/brdcst.hxx>
#include <basic/sbstar.hxx>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XStarBasicAccess.hpp>
#include "basicdllapi.h"
#include <vector>

// Basic XML Import/Export
BASIC_DLLPUBLIC com::sun::star::uno::Reference< com::sun::star::script::XStarBasicAccess >
    getStarBasicAccess( BasicManager* pMgr );

class SotStorage;

#define BASERR_REASON_OPENSTORAGE       0x0001
#define BASERR_REASON_OPENLIBSTORAGE    0x0002
#define BASERR_REASON_OPENMGRSTREAM     0x0004
#define BASERR_REASON_OPENLIBSTREAM     0x0008
#define BASERR_REASON_LIBNOTFOUND       0x0010
#define BASERR_REASON_STORAGENOTFOUND   0x0020
#define BASERR_REASON_BASICLOADERROR    0x0040
#define BASERR_REASON_NOSTORAGENAME     0x0080

#define BASERR_REASON_STDLIB            0x0100

class BASIC_DLLPUBLIC BasicError
{
private:
    sal_uIntPtr nErrorId;
    sal_uInt16  nReason;
    String  aErrStr;

public:
            BasicError( const BasicError& rErr );
            BasicError( sal_uIntPtr nId, sal_uInt16 nR, const String& rErrStr );

    sal_uIntPtr     GetErrorId() const                  { return nErrorId; }
    sal_uInt16  GetReason() const                   { return nReason; }
    String  GetErrorStr()                       { return aErrStr; }

    void    SetErrorId( sal_uIntPtr n )             { nErrorId = n; }
    void    SetReason( sal_uInt16 n )               { nReason = n; }
    void    SetErrorStr( const String& rStr)    { aErrStr = rStr; }
};

class BasicLibs;
class ErrorManager;
class BasicLibInfo;

namespace basic { class BasicManagerCleaner; }

// Library password handling for 5.0 documents
class BASIC_DLLPUBLIC OldBasicPassword
{
public:
    virtual void setLibraryPassword( const String& rLibraryName, const String& rPassword ) = 0;
    virtual String getLibraryPassword( const String& rLibraryName ) = 0;
    virtual void clearLibraryPassword( const String& rLibraryName ) = 0;
    virtual sal_Bool hasLibraryPassword( const String& rLibraryName ) = 0;

protected:
    ~OldBasicPassword() {}
};

struct LibraryContainerInfo
{
    ::com::sun::star::uno::Reference< com::sun::star::script::XPersistentLibraryContainer > mxScriptCont;
    ::com::sun::star::uno::Reference< com::sun::star::script::XPersistentLibraryContainer > mxDialogCont;
    OldBasicPassword* mpOldBasicPassword;

    LibraryContainerInfo()
        :mpOldBasicPassword( NULL )
    {
    }

    LibraryContainerInfo
    (
        com::sun::star::uno::Reference< com::sun::star::script::XPersistentLibraryContainer > xScriptCont,
        com::sun::star::uno::Reference< com::sun::star::script::XPersistentLibraryContainer > xDialogCont,
        OldBasicPassword* pOldBasicPassword
    )
        : mxScriptCont( xScriptCont )
        , mxDialogCont( xDialogCont )
        , mpOldBasicPassword( pOldBasicPassword )
    {}
};

struct BasicManagerImpl;


#define LIB_NOTFOUND    0xFFFF

class BASIC_DLLPUBLIC BasicManager : public SfxBroadcaster
{
    friend class LibraryContainer_Impl;
    friend class StarBasicAccess_Impl;
    friend class BasMgrContainerListenerImpl;
    friend class ::basic::BasicManagerCleaner;

private:
    BasicLibs*          pLibs;
    std::vector<BasicError> aErrors;

    String              aName;
    String              maStorageName;
    bool                mbDocMgr;

    BasicManagerImpl*   mpImpl;

    BASIC_DLLPRIVATE void Init();

protected:
    sal_Bool            ImpLoadLibrary( BasicLibInfo* pLibInfo ) const;
    sal_Bool            ImpLoadLibrary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, sal_Bool bInfosOnly = sal_False );
    void            ImpCreateStdLib( StarBASIC* pParentFromStdLib );
    void            ImpMgrNotLoaded(  const String& rStorageName  );
    BasicLibInfo*   CreateLibInfo();
    void            LoadBasicManager( SotStorage& rStorage, const String& rBaseURL, sal_Bool bLoadBasics = sal_True );
    void            LoadOldBasicManager( SotStorage& rStorage );
    sal_Bool            ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const;
    sal_Bool            ImplEncryptStream( SvStream& rStream ) const;
    BasicLibInfo*   FindLibInfo( StarBASIC* pBasic ) const;
    void            CheckModules( StarBASIC* pBasic, sal_Bool bReference ) const;
    ~BasicManager();

public:
                    TYPEINFO();
                    BasicManager( SotStorage& rStorage, const String& rBaseURL, StarBASIC* pParentFromStdLib = NULL, String* pLibPath = NULL, bool bDocMgr = false );
                    BasicManager( StarBASIC* pStdLib, String* pLibPath = NULL, bool bDocMgr = false );

    /** deletes the given BasicManager instance

        This method is necessary since normally, BasicManager instances are owned by the BasicManagerRepository,
        and expected to be deleted by the repository only. However, there exists quite some legacy code,
        which needs to explicitly delete a BasicManager itself. This code must not use the (protected)
        destructor, but LegacyDeleteBasicManager.
    */
    static void     LegacyDeleteBasicManager( BasicManager*& _rpManager );

    void            SetStorageName( const String& rName )   { maStorageName = rName; }
    String          GetStorageName() const                  { return maStorageName; }
    void            SetName( const String& rName )          { aName = rName; }
    String          GetName() const                         { return aName; }


    sal_uInt16          GetLibCount() const;
    StarBASIC*      GetLib( sal_uInt16 nLib ) const;
    StarBASIC*      GetLib( const String& rName ) const;
    sal_uInt16          GetLibId( const String& rName ) const;

    String          GetLibName( sal_uInt16 nLib );

    /** announces the library containers which belong to this BasicManager

        The method will automatically add two global constants, BasicLibraries and DialogLibraries,
        to the BasicManager.
    */
    void            SetLibraryContainerInfo( const LibraryContainerInfo& rInfo );

    const ::com::sun::star::uno::Reference< com::sun::star::script::XPersistentLibraryContainer >&
                    GetDialogLibraryContainer()  const;
    const ::com::sun::star::uno::Reference< com::sun::star::script::XPersistentLibraryContainer >&
                    GetScriptLibraryContainer()  const;

    sal_Bool            LoadLib( sal_uInt16 nLib );
    sal_Bool            RemoveLib( sal_uInt16 nLib, sal_Bool bDelBasicFromStorage );

    // Modify-Flag will be reset only during save.
    sal_Bool            IsBasicModified() const;

    std::vector<BasicError>& GetErrors();

    /** sets a global constant in the basic library, referring to some UNO object, to a new value.

        If a constant with this name already existed before, its value is changed, and the old constant is
        returned. If it does not yet exist, it is newly created, and inserted into the basic library.
    */
    ::com::sun::star::uno::Any
                    SetGlobalUNOConstant( const sal_Char* _pAsciiName, const ::com::sun::star::uno::Any& _rValue );

    /** retrieves a global constant in the basic library, referring to some UNO object, returns true if a value is found ( value is in aOut ) false otherwise. */
                    bool GetGlobalUNOConstant( const sal_Char* _pAsciiName, ::com::sun::star::uno::Any& aOut );
    /** determines whether there are password-protected modules whose size exceedes the
        legacy module size
        @param _out_rModuleNames
            takes the names of modules whose size exceeds the legacy limit
    */
    bool            LegacyPsswdBinaryLimitExceeded( ::com::sun::star::uno::Sequence< rtl::OUString >& _out_rModuleNames );
    bool HasExeCode( const String& );
    /// determines whether the Basic Manager has a given macro, given by fully qualified name
    bool            HasMacro( String const& i_fullyQualifiedName ) const;
    /// executes a given macro
    ErrCode         ExecuteMacro( String const& i_fullyQualifiedName, SbxArray* i_arguments, SbxValue* i_retValue );
    /// executes a given macro
    ErrCode         ExecuteMacro( String const& i_fullyQualifiedName, String const& i_commaSeparatedArgs, SbxValue* i_retValue );

private:
    BASIC_DLLPRIVATE sal_Bool IsReference( sal_uInt16 nLib );

    BASIC_DLLPRIVATE sal_Bool SetLibName( sal_uInt16 nLib, const String& rName );

    BASIC_DLLPRIVATE StarBASIC* GetStdLib() const;
    BASIC_DLLPRIVATE StarBASIC* AddLib( SotStorage& rStorage, const String& rLibName, sal_Bool bReference );
    BASIC_DLLPRIVATE sal_Bool RemoveLib( sal_uInt16 nLib );
    BASIC_DLLPRIVATE sal_Bool HasLib( const String& rName ) const;

    BASIC_DLLPRIVATE StarBASIC* CreateLibForLibContainer( const String& rLibName,
                        const com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >&
                            xScriptCont );
    // For XML import/export:
    BASIC_DLLPRIVATE StarBASIC* CreateLib( const String& rLibName );
    BASIC_DLLPRIVATE StarBASIC* CreateLib( const String& rLibName, const String& Password,
                               const String& LinkTargetURL );
};

#endif  //_BASMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
