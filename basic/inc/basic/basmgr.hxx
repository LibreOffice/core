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
//
#ifndef _BASMGR_HXX
#define _BASMGR_HXX

#include <tools/string.hxx>
#include <svl/brdcst.hxx>
#include <basic/sbstar.hxx>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/XStarBasicAccess.hpp>


// Basic XML Import/Export
com::sun::star::uno::Reference< com::sun::star::script::XStarBasicAccess >
    getStarBasicAccess( BasicManager* pMgr );



class SotStorage;

#define BASERR_ID_STDLIBOPEN            ERRCODE_BASMGR_STDLIBOPEN
#define BASERR_ID_STDLIBSAVE            ERRCODE_BASMGR_STDLIBSAVE
#define BASERR_ID_LIBLOAD               ERRCODE_BASMGR_LIBLOAD
#define BASERR_ID_LIBCREATE             ERRCODE_BASMGR_LIBCREATE
#define BASERR_ID_LIBSAVE               ERRCODE_BASMGR_LIBSAVE
#define BASERR_ID_LIBDEL                ERRCODE_BASMGR_LIBDEL
#define BASERR_ID_MGROPEN               ERRCODE_BASMGR_MGROPEN
#define BASERR_ID_MGRSAVE               ERRCODE_BASMGR_MGRSAVE
#define BASERR_ID_REMOVELIB             ERRCODE_BASMGR_REMOVELIB
#define BASERR_ID_UNLOADLIB             ERRCODE_BASMGR_UNLOADLIB

#define BASERR_REASON_OPENSTORAGE       0x0001
#define BASERR_REASON_OPENLIBSTORAGE    0x0002
#define BASERR_REASON_OPENMGRSTREAM     0x0004
#define BASERR_REASON_OPENLIBSTREAM     0x0008
#define BASERR_REASON_LIBNOTFOUND       0x0010
#define BASERR_REASON_STORAGENOTFOUND   0x0020
#define BASERR_REASON_BASICLOADERROR    0x0040
#define BASERR_REASON_NOSTORAGENAME     0x0080

#define BASERR_REASON_STDLIB            0x0100

class BasicError
{
private:
    sal_uIntPtr nErrorId;
    sal_uInt16  nReason;
    String  aErrStr;

public:
            BasicError();
            BasicError( const BasicError& rErr );
            BasicError( sal_uIntPtr nId, sal_uInt16 nR, const String& rErrStr );

    sal_uIntPtr     GetErrorId() const                  { return nErrorId; }
    sal_uInt16  GetReason() const                   { return nReason; }
    String  GetErrorStr()                       { return aErrStr; }

    void    SetErrorId( sal_uIntPtr n )             { nErrorId = n; }
    void    SetReason( sal_uInt16 n )               { nReason = n; }
    void    SetErrorStr( const String& rStr)    { aErrStr = rStr; }
};


//

class BasicLibs;
class ErrorManager;
class BasicLibInfo;
class BasicErrorManager;
namespace basic { class BasicManagerCleaner; }

// Library password handling for 5.0 documents
class OldBasicPassword
{
public:
    virtual void setLibraryPassword( const String& rLibraryName, const String& rPassword ) = 0;
    virtual String getLibraryPassword( const String& rLibraryName ) = 0;
    virtual void clearLibraryPassword( const String& rLibraryName ) = 0;
    virtual sal_Bool hasLibraryPassword( const String& rLibraryName ) = 0;
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

class BasicManager : public SfxBroadcaster
{
    friend class LibraryContainer_Impl;
    friend class StarBasicAccess_Impl;
    friend class BasMgrContainerListenerImpl;
    friend class ::basic::BasicManagerCleaner;

private:
    BasicLibs*          pLibs;
    BasicErrorManager*  pErrorMgr;

    String              aName;
    String              maStorageName;
    sal_Bool                bBasMgrModified;
    sal_Bool                mbDocMgr;

    BasicManagerImpl*   mpImpl;

    void                Init();

protected:
    sal_Bool            ImpLoadLibary( BasicLibInfo* pLibInfo ) const;
    sal_Bool            ImpLoadLibary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, sal_Bool bInfosOnly = sal_False ) const;
    void            ImpCreateStdLib( StarBASIC* pParentFromStdLib );
    void            ImpMgrNotLoaded(  const String& rStorageName  );
    BasicLibInfo*   CreateLibInfo();
    void            LoadBasicManager( SotStorage& rStorage, const String& rBaseURL, sal_Bool bLoadBasics = sal_True );
    void            LoadOldBasicManager( SotStorage& rStorage );
    sal_Bool            ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const;
    sal_Bool            ImplEncryptStream( SvStream& rStream ) const;
    BasicLibInfo*   FindLibInfo( StarBASIC* pBasic ) const;
    void            CheckModules( StarBASIC* pBasic, sal_Bool bReference ) const;
    void            SetFlagToAllLibs( short nFlag, sal_Bool bSet ) const;
                    BasicManager(); // Nur zum anpassen von Pfaden bei 'Speichern unter'.
                    ~BasicManager();

public:
                    TYPEINFO();
                    BasicManager( SotStorage& rStorage, const String& rBaseURL, StarBASIC* pParentFromStdLib = NULL, String* pLibPath = NULL, sal_Bool bDocMgr = sal_False );
                    BasicManager( StarBASIC* pStdLib, String* pLibPath = NULL, sal_Bool bDocMgr = sal_False );

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

    // Modify-Flag wird nur beim Speichern zurueckgesetzt.
    sal_Bool            IsModified() const;
    sal_Bool            IsBasicModified() const;

    sal_Bool            HasErrors();
    void            ClearErrors();
    BasicError*     GetFirstError();
    BasicError*     GetNextError();

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

    /// determines whether the Basic Manager has a given macro, given by fully qualified name
    bool            HasMacro( String const& i_fullyQualifiedName ) const;
    /// executes a given macro
    ErrCode         ExecuteMacro( String const& i_fullyQualifiedName, SbxArray* i_arguments, SbxValue* i_retValue );
    /// executes a given macro
    ErrCode         ExecuteMacro( String const& i_fullyQualifiedName, String const& i_commaSeparatedArgs, SbxValue* i_retValue );

private:
    sal_Bool            IsReference( sal_uInt16 nLib );

    sal_Bool            SetLibName( sal_uInt16 nLib, const String& rName );

    StarBASIC*      GetStdLib() const;
    StarBASIC*      AddLib( SotStorage& rStorage, const String& rLibName, sal_Bool bReference );
    sal_Bool            RemoveLib( sal_uInt16 nLib );
    sal_Bool            HasLib( const String& rName ) const;

    StarBASIC*      CreateLibForLibContainer( const String& rLibName,
                        const com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >&
                            xScriptCont );
    // For XML import/export:
    StarBASIC*      CreateLib( const String& rLibName );
    StarBASIC*      CreateLib( const String& rLibName, const String& Password,
                               const String& LinkTargetURL );
};

void SetAppBasicManager( BasicManager* pBasMgr );

#endif  //_BASMGR_HXX
