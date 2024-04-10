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
#pragma once

#include <utility>
#include <comphelper/errcode.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <basic/sbstar.hxx>
#include <basic/basicdllapi.h>
#include <memory>
#include <string_view>
#include <vector>

namespace basic { class SfxScriptLibraryContainer; }
namespace com::sun::star::script { class XLibraryContainer; }
namespace com::sun::star::script { class XPersistentLibraryContainer; }
namespace com::sun::star::script { class XStarBasicAccess; }

class BasicManager;

// Basic XML Import/Export
BASIC_DLLPUBLIC css::uno::Reference< css::script::XStarBasicAccess >
    getStarBasicAccess( BasicManager* pMgr );

class SotStorage;

class BasicError
{
private:
    ErrCodeMsg nErrorId;

public:
            BasicError( const BasicError& rErr );
            BasicError( ErrCodeMsg nId );

    ErrCodeMsg const & GetErrorId() const { return nErrorId; }
};

class BasicLibInfo;

namespace basic { class ImplRepository; }

struct LibraryContainerInfo
{
    css::uno::Reference< css::script::XPersistentLibraryContainer > mxScriptCont;
    css::uno::Reference< css::script::XPersistentLibraryContainer > mxDialogCont;
    basic::SfxScriptLibraryContainer* mpOldBasicPassword;

    LibraryContainerInfo()
        :mpOldBasicPassword( nullptr )
    {
    }

    LibraryContainerInfo
    (
        css::uno::Reference< css::script::XPersistentLibraryContainer > xScriptCont,
        css::uno::Reference< css::script::XPersistentLibraryContainer > xDialogCont,
        basic::SfxScriptLibraryContainer* pOldBasicPassword
    )
        : mxScriptCont(std::move( xScriptCont ))
        , mxDialogCont(std::move( xDialogCont ))
        , mpOldBasicPassword( pOldBasicPassword )
    {}
};

#define LIB_NOTFOUND    0xFFFF

class BASIC_DLLPUBLIC BasicManager final : public SfxBroadcaster
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

    LibraryContainerInfo    maContainerInfo;
    std::vector<std::unique_ptr<BasicLibInfo>> maLibs;
    OUString         aBasicLibPath;

    bool            ImpLoadLibrary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage );
    void            ImpCreateStdLib( StarBASIC* pParentFromStdLib );
    void            ImpMgrNotLoaded(  const OUString& rStorageName  );
    BasicLibInfo*   CreateLibInfo();
    void            LoadBasicManager( SotStorage& rStorage, std::u16string_view rBaseURL );
    void            LoadOldBasicManager( SotStorage& rStorage );
    bool            ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const;
    static bool     ImplEncryptStream( SvStream& rStream );
    BasicLibInfo*   FindLibInfo( StarBASIC const * pBasic );
    static void     CheckModules( StarBASIC* pBasic, bool bReference );

public:
                    BasicManager( SotStorage& rStorage, std::u16string_view rBaseURL, StarBASIC* pParentFromStdLib = nullptr, OUString const * pLibPath = nullptr, bool bDocMgr = false );
                    BasicManager( StarBASIC* pStdLib, OUString const * pLibPath = nullptr, bool bDocMgr = false );

    virtual ~BasicManager() override;

    void            SetStorageName( const OUString& rName )   { maStorageName = rName; }
    const OUString& GetStorageName() const                  { return maStorageName; }
    void            SetName( const OUString& rName )          { aName = rName; }
    const OUString& GetName() const                         { return aName; }


    sal_uInt16 GetLibCount() const;
    StarBASIC* GetLib( sal_uInt16 nLib ) const;
    StarBASIC* GetLib( std::u16string_view rName ) const;
    sal_uInt16      GetLibId( std::u16string_view rName ) const;

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
        returned in pOldValue. If it does not yet exist, it is newly created, and inserted into the basic library.
    */
    void            SetGlobalUNOConstant( const OUString& rName, const css::uno::Any& _rValue, css::uno::Any* pOldValue = nullptr );

    /** retrieves a global constant in the basic library, referring to some UNO object, returns true if a value is found ( value is in aOut ) false otherwise. */
    bool            GetGlobalUNOConstant( const OUString& rName, css::uno::Any& aOut );
    /** determines whether there are password-protected modules whose size exceeds the
        B_IMG_VERSION_12 module size
        @param _out_rModuleNames
            takes the names of modules whose size exceeds the B_IMG_VERSION_12 limit
    */
    bool            ImgVersion12PsswdBinaryLimitExceeded( std::vector< OUString >& _out_rModuleNames );
    bool HasExeCode( std::u16string_view );
    /// determines whether the Basic Manager has a given macro, given by fully qualified name
    bool            HasMacro( OUString const& i_fullyQualifiedName ) const;
    /// executes a given macro
    ErrCode         ExecuteMacro( OUString const& i_fullyQualifiedName, SbxArray* i_arguments, SbxValue* i_retValue );
    /// executes a given macro
    ErrCode         ExecuteMacro( OUString const& i_fullyQualifiedName, std::u16string_view i_commaSeparatedArgs, SbxValue* i_retValue );

private:
    BASIC_DLLPRIVATE bool IsReference( sal_uInt16 nLib );

    BASIC_DLLPRIVATE StarBASIC* GetStdLib() const;
    BASIC_DLLPRIVATE StarBASIC* AddLib( SotStorage& rStorage, const OUString& rLibName, bool bReference );
    BASIC_DLLPRIVATE void RemoveLib( sal_uInt16 nLib );
    BASIC_DLLPRIVATE bool HasLib( std::u16string_view rName ) const;

    BASIC_DLLPRIVATE StarBASIC* CreateLibForLibContainer( const OUString& rLibName,
                        const css::uno::Reference< css::script::XLibraryContainer >& xScriptCont );
    // For XML import/export:
    BASIC_DLLPRIVATE StarBASIC* CreateLib( const OUString& rLibName );
    BASIC_DLLPRIVATE StarBASIC* CreateLib( const OUString& rLibName, const OUString& Password,
                                           const OUString& LinkTargetURL );
    BasicManager& operator=(BasicManager const &) = delete; //MSVC2015 workaround
    BasicManager( BasicManager const&) = delete; //MSVC2015 workaround
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
