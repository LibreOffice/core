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
#ifndef INCLUDED_BASIC_BASMGR_HXX
#define INCLUDED_BASIC_BASMGR_HXX

#include <svl/SfxBroadcaster.hxx>
#include <basic/sbstar.hxx>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XStarBasicAccess.hpp>
#include <basic/basicdllapi.h>
#include <vector>

// Basic XML Import/Export
BASIC_DLLPUBLIC css::uno::Reference< css::script::XStarBasicAccess >
    getStarBasicAccess( BasicManager* pMgr );

class SotStorage;

enum class BasicErrorReason
{
    OPENSTORAGE      = 0x0001,
    OPENLIBSTORAGE   = 0x0002,
    OPENMGRSTREAM    = 0x0004,
    OPENLIBSTREAM    = 0x0008,
    LIBNOTFOUND      = 0x0010,
    STORAGENOTFOUND  = 0x0020,
    BASICLOADERROR   = 0x0040,
    NOSTORAGENAME    = 0x0080,
    STDLIB           = 0x0100
};

class BASIC_DLLPUBLIC BasicError
{
private:
    sal_uInt64 nErrorId;
    BasicErrorReason  nReason;
    OUString  aErrStr;

public:
            BasicError( const BasicError& rErr );
            BasicError( sal_uInt64 nId, BasicErrorReason nR, const OUString& rErrStr );

    sal_uInt64 GetErrorId() const                  { return nErrorId; }
};

class ErrorManager;
class BasicLibInfo;

namespace basic { class ImplRepository; }

// Library password handling for 5.0 documents
class BASIC_DLLPUBLIC OldBasicPassword
{
public:
    virtual void     setLibraryPassword( const OUString& rLibraryName, const OUString& rPassword ) = 0;

protected:
    ~OldBasicPassword() {}
};

struct LibraryContainerInfo
{
    css::uno::Reference< css::script::XPersistentLibraryContainer > mxScriptCont;
    css::uno::Reference< css::script::XPersistentLibraryContainer > mxDialogCont;
    OldBasicPassword* mpOldBasicPassword;

    LibraryContainerInfo()
        :mpOldBasicPassword( nullptr )
    {
    }

    LibraryContainerInfo
    (
        css::uno::Reference< css::script::XPersistentLibraryContainer > xScriptCont,
        css::uno::Reference< css::script::XPersistentLibraryContainer > xDialogCont,
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
    friend class ::basic::ImplRepository;

private:
    std::vector<BasicError> aErrors;

    OUString            aName;
    OUString            maStorageName;
    bool                mbDocMgr;

    BasicManagerImpl*   mpImpl;

    BASIC_DLLPRIVATE void Init();

protected:
    bool            ImpLoadLibrary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, bool bInfosOnly = false );
    void            ImpCreateStdLib( StarBASIC* pParentFromStdLib );
    void            ImpMgrNotLoaded(  const OUString& rStorageName  );
    BasicLibInfo*   CreateLibInfo();
    void            LoadBasicManager( SotStorage& rStorage, const OUString& rBaseURL, bool bLoadBasics = true );
    void            LoadOldBasicManager( SotStorage& rStorage );
    bool            ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const;
    static bool     ImplEncryptStream( SvStream& rStream );
    BasicLibInfo*   FindLibInfo( StarBASIC* pBasic );
    static void     CheckModules( StarBASIC* pBasic, bool bReference );
    virtual ~BasicManager();

public:
                    TYPEINFO_OVERRIDE();
                    BasicManager( SotStorage& rStorage, const OUString& rBaseURL, StarBASIC* pParentFromStdLib = nullptr, OUString* pLibPath = nullptr, bool bDocMgr = false );
                    BasicManager( StarBASIC* pStdLib, OUString* pLibPath = nullptr, bool bDocMgr = false );

    /** deletes the given BasicManager instance

        This method is necessary since normally, BasicManager instances are owned by the BasicManagerRepository,
        and expected to be deleted by the repository only. However, there exists quite some legacy code,
        which needs to explicitly delete a BasicManager itself. This code must not use the (protected)
        destructor, but LegacyDeleteBasicManager.
    */
    static void     LegacyDeleteBasicManager( BasicManager*& _rpManager );

    void            SetStorageName( const OUString& rName )   { maStorageName = rName; }
    OUString        GetStorageName() const                  { return maStorageName; }
    void            SetName( const OUString& rName )          { aName = rName; }
    OUString        GetName() const                         { return aName; }


    sal_uInt16      GetLibCount() const;
    StarBASIC*      GetLib( sal_uInt16 nLib ) const;
    StarBASIC*      GetLib( const OUString& rName ) const;
    sal_uInt16      GetLibId( const OUString& rName ) const;

    OUString        GetLibName( sal_uInt16 nLib );

    /** announces the library containers which belong to this BasicManager

        The method will automatically add two global constants, BasicLibraries and DialogLibraries,
        to the BasicManager.
    */
    void            SetLibraryContainerInfo( const LibraryContainerInfo& rInfo );

    const css::uno::Reference< css::script::XPersistentLibraryContainer >&
                    GetDialogLibraryContainer()  const;
    const css::uno::Reference< css::script::XPersistentLibraryContainer >&
                    GetScriptLibraryContainer()  const;

    bool            LoadLib( sal_uInt16 nLib );
    bool            RemoveLib( sal_uInt16 nLib, bool bDelBasicFromStorage );

    // Modify-Flag will be reset only during save.
    bool            IsBasicModified() const;

    std::vector<BasicError>& GetErrors() { return aErrors;}

    /** sets a global constant in the basic library, referring to some UNO object, to a new value.

        If a constant with this name already existed before, its value is changed, and the old constant is
        returned. If it does not yet exist, it is newly created, and inserted into the basic library.
    */
    css::uno::Any
                    SetGlobalUNOConstant( const sal_Char* _pAsciiName, const css::uno::Any& _rValue );

    /** retrieves a global constant in the basic library, referring to some UNO object, returns true if a value is found ( value is in aOut ) false otherwise. */
                    bool GetGlobalUNOConstant( const sal_Char* _pAsciiName, css::uno::Any& aOut );
    /** determines whether there are password-protected modules whose size exceedes the
        legacy module size
        @param _out_rModuleNames
            takes the names of modules whose size exceeds the legacy limit
    */
    bool            LegacyPsswdBinaryLimitExceeded( css::uno::Sequence< OUString >& _out_rModuleNames );
    bool HasExeCode( const OUString& );
    /// determines whether the Basic Manager has a given macro, given by fully qualified name
    bool            HasMacro( OUString const& i_fullyQualifiedName ) const;
    /// executes a given macro
    ErrCode         ExecuteMacro( OUString const& i_fullyQualifiedName, SbxArray* i_arguments, SbxValue* i_retValue );
    /// executes a given macro
    ErrCode         ExecuteMacro( OUString const& i_fullyQualifiedName, OUString const& i_commaSeparatedArgs, SbxValue* i_retValue );

private:
    BASIC_DLLPRIVATE bool IsReference( sal_uInt16 nLib );

    BASIC_DLLPRIVATE StarBASIC* GetStdLib() const;
    BASIC_DLLPRIVATE StarBASIC* AddLib( SotStorage& rStorage, const OUString& rLibName, bool bReference );
    BASIC_DLLPRIVATE bool RemoveLib( sal_uInt16 nLib );
    BASIC_DLLPRIVATE bool HasLib( const OUString& rName ) const;

    BASIC_DLLPRIVATE StarBASIC* CreateLibForLibContainer( const OUString& rLibName,
                        const css::uno::Reference< css::script::XLibraryContainer >& xScriptCont );
    // For XML import/export:
    BASIC_DLLPRIVATE StarBASIC* CreateLib( const OUString& rLibName );
    BASIC_DLLPRIVATE StarBASIC* CreateLib( const OUString& rLibName, const OUString& Password,
                                           const OUString& LinkTargetURL );
};

#endif // INCLUDED_BASIC_BASMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
