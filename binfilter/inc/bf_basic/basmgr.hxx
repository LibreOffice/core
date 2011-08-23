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

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <bf_svtools/brdcst.hxx>
#endif

#include "sbstar.hxx"

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSTARBASICACCESS_HPP_
#include <com/sun/star/script/XStarBasicAccess.hpp>
#endif


class SotStorage;

namespace binfilter {

// Basic XML Import/Export
com::sun::star::uno::Reference< com::sun::star::script::XStarBasicAccess >
    getStarBasicAccess( BasicManager* pMgr );

#define BASERR_ID_STDLIBOPEN			ERRCODE_BASMGR_STDLIBOPEN
#define BASERR_ID_STDLIBSAVE            ERRCODE_BASMGR_STDLIBSAVE
#define BASERR_ID_LIBLOAD				ERRCODE_BASMGR_LIBLOAD
#define BASERR_ID_LIBCREATE				ERRCODE_BASMGR_LIBCREATE
#define BASERR_ID_LIBSAVE               ERRCODE_BASMGR_LIBSAVE
#define BASERR_ID_LIBDEL				ERRCODE_BASMGR_LIBDEL
#define BASERR_ID_MGROPEN               ERRCODE_BASMGR_MGROPEN
#define BASERR_ID_MGRSAVE				ERRCODE_BASMGR_MGRSAVE
#define BASERR_ID_REMOVELIB				ERRCODE_BASMGR_REMOVELIB
#define BASERR_ID_UNLOADLIB				ERRCODE_BASMGR_UNLOADLIB

#define BASERR_REASON_OPENSTORAGE       0x0001
#define BASERR_REASON_OPENLIBSTORAGE    0x0002
#define BASERR_REASON_OPENMGRSTREAM     0x0004
#define BASERR_REASON_OPENLIBSTREAM     0x0008
#define BASERR_REASON_LIBNOTFOUND		0x0010
#define BASERR_REASON_STORAGENOTFOUND	0x0020
#define BASERR_REASON_BASICLOADERROR	0x0040
#define BASERR_REASON_NOSTORAGENAME		0x0080

#define BASERR_REASON_STDLIB			0x0100

class BasicError
{
private:
    ULONG	nErrorId;
    USHORT	nReason;
    String	aErrStr;

public:
            BasicError( const BasicError& rErr );
            BasicError( ULONG nId, USHORT nR, const String& rErrStr );

    ULONG 	GetErrorId() const					{ return nErrorId; }
    USHORT	GetReason() const					{ return nReason; }
    String	GetErrorStr()						{ return aErrStr; }

    void	SetErrorId( ULONG n )				{ nErrorId = n; }
    void	SetReason( USHORT n )				{ nReason = n; }
    void	SetErrorStr( const String& rStr)	{ aErrStr = rStr; }
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
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer > mxScriptCont;
    com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer > mxDialogCont;
    OldBasicPassword* mpOldBasicPassword;

    LibraryContainerInfo
    (
        com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer > xScriptCont,
        com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer > xDialogCont,
        OldBasicPassword* pOldBasicPassword
    )
        : mxScriptCont( xScriptCont )
        , mxDialogCont( xDialogCont )
        , mpOldBasicPassword( pOldBasicPassword )
    {}
};

struct BasicManagerImpl;


#define LIB_NOTFOUND	0xFFFF

class BasicManager : public SfxBroadcaster
{
    friend class LibraryContainer_Impl;
    friend class StarBasicAccess_Impl;
    friend class BasMgrContainerListenerImpl;
    friend class basic::BasicManagerCleaner;

private:
    BasicLibs*			pLibs;
    BasicErrorManager*	pErrorMgr;

    String				aName;
    String				maStorageName;
    BOOL				bBasMgrModified;

    BasicManagerImpl*	mpImpl;

    void				Init();

protected:
    BOOL			ImpStoreLibary( StarBASIC* pLib, SotStorage& rStorage ) const;
    BOOL			ImpLoadLibary( BasicLibInfo* pLibInfo ) const;
    BOOL			ImpLoadLibary( BasicLibInfo* pLibInfo, SotStorage* pCurStorage, BOOL bInfosOnly = FALSE ) const;
    void			ImpCreateStdLib( StarBASIC* pParentFromStdLib );
    void			ImpMgrNotLoaded(  const String& rStorageName  );
    BasicLibInfo*	CreateLibInfo();
    void            LoadBasicManager( SotStorage& rStorage, const String& rBaseURL, BOOL bLoadBasics = TRUE );
    void			LoadOldBasicManager( SotStorage& rStorage );
    BOOL 			ImplLoadBasic( SvStream& rStrm, StarBASICRef& rOldBasic ) const;
    BOOL			ImplEncryptStream( SvStream& rStream ) const;
    BasicLibInfo*	FindLibInfo( StarBASIC* pBasic ) const;
    void			CheckModules( StarBASIC* pBasic, BOOL bReference ) const;
    void			SetFlagToAllLibs( short nFlag, BOOL bSet ) const;
                    BasicManager();	// Nur zum anpassen von Pfaden bei 'Speichern unter'.
                    ~BasicManager();

public:
                    TYPEINFO();
                    BasicManager( SotStorage& rStorage, const String& rBaseURL, StarBASIC* pParentFromStdLib = NULL, String* pLibPath = NULL );
                    BasicManager( StarBASIC* pStdLib, String* pLibPath = NULL );

    /** deletes the given BasicManager instance

        This method is necessary since normally, BasicManager instances are owned by the BasicManagerRepository,
        and expected to be deleted by the repository only. However, there exists quite some legacy code,
        which needs to explicitly delete a BasicManager itself. This code must not use the (protected)
        destructor, but LegacyDeleteBasicManager.
    */
    static void     LegacyDeleteBasicManager( BasicManager*& _rpManager );

    void			SetStorageName( const String& rName )	{ maStorageName = rName; }
    String			GetStorageName() const 					{ return maStorageName; }
    void			SetName( const String& rName ) 			{ aName = rName; }
    String			GetName() const							{ return aName; }


    static BOOL     HasBasicWithModules( const SotStorage& rStorage, const String& rBaseURL );
    static BOOL     CopyBasicData( SotStorage* pFrom, const String& rSourceURL, const String& rBaseURL, SotStorage* pTo);

    USHORT		    GetLibCount() const;
    StarBASIC*	    GetLib( USHORT nLib ) const;
    StarBASIC*	    GetLib( const String& rName ) const;
    USHORT		    GetLibId( const String& rName ) const;

    void            Store( SotStorage& rStorage, const String& rBaseURL );
    void            Store( SotStorage& rStorage, const String& rBaseURL, BOOL bStoreLibs );

    String		    GetLibName( USHORT nLib );

    /** announces the library containers which belong to this BasicManager

        The method will automatically add two global constants, BasicLibraries and DialogLibraries,
        to the BasicManager.
    */
    void			SetLibraryContainerInfo( LibraryContainerInfo* pInfo );

    BOOL		    RemoveLib( USHORT nLib, BOOL bDelBasicFromStorage );

    BOOL		    HasErrors();
    void		    ClearErrors();

private:
    BOOL		    IsReference( USHORT nLib );

    BOOL		    SetLibName( USHORT nLib, const String& rName );

    StarBASIC*  	GetStdLib() const;
    StarBASIC*      AddLib( SotStorage& rStorage, const String& rLibName, BOOL bReference );
    BOOL		    RemoveLib( USHORT nLib );
    BOOL	    	HasLib( const String& rName ) const;

    StarBASIC*	    CreateLibForLibContainer( const String& rLibName,
                        const com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer >&
                            xScriptCont );
    // For XML import/export:
    StarBASIC*      CreateLib( const String& rLibName );
    StarBASIC*      CreateLib( const String& rLibName, const String& Password,
                               const String& LinkTargetURL );
};

void SetAppBasicManager( BasicManager* pBasMgr );

}

#endif	//_BASMGR_HXX
