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

#ifndef _PERSIST_HXX
#define _PERSIST_HXX

#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#include <bf_so3/so2ref.hxx>
#include <bf_so3/iface.hxx>

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

/*************************************************************************
*************************************************************************/
#ifdef SvStorage
#undef SvStorage
#endif
#ifdef SvStorageRef
#undef SvStorageRef
#endif
#ifdef SvStorageStream
#undef SvStorageStream
#endif
#ifdef SvStorageStreamRef
#undef SvStorageStreamRef
#endif

namespace binfilter {

#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

class  ImpPersistStorage;
class  ImpPersistFile;
class  ImpItemContainer;
class  SvFactory;
class  SvStorage;


    /*************************************************************************
*************************************************************************/

//=========================================================================
class SO3_DLLPUBLIC SvObjectContainer : virtual public SvObject
/*	[Beschreibung]

    Diese Klasse stellt die Schnittstelle zur Verf"ugung, um auf
    Unter-Objekte, Pseudo-Objekte, Bereiche ... zuzugreifen.
    Diese Objekte werden erst auf Anforderung erzeugt. Sie m"ussen
    also nicht beim "offnen des Dokumentes angelegt werden.

*/
{
friend class ImpItemContainer;
friend class SvFactory;
protected:
                    ~SvObjectContainer();
public:
                    SvObjectContainer();
                    SO2_DECL_STANDARD_CLASS_DLL(SvObjectContainer,SOAPP)

};
#ifndef SO2_DECL_SVOBJECTCONTAINER_DEFINED
#define SO2_DECL_SVOBJECTCONTAINER_DEFINED
SO2_DECL_REF(SvObjectContainer)
#endif
SO2_IMPL_REF(SvObjectContainer)

/*************************************************************************
*************************************************************************/
class SvInfoObjectMemberList;
class SvInfoObject;

#ifndef SO2_DECL_SVPERSIST_DEFINED
#define SO2_DECL_SVPERSIST_DEFINED
SO2_DECL_REF(SvPersist)
#endif

#define SVEXT_PERSIST_STREAM "\002OlePres000"

class SO3_DLLPUBLIC SvPersist : public SvObjectContainer
{
friend class SvInfoObject;
friend class ImpPersistStorage;
friend class ImpPersistFile;
friend class ImpItemContainer;
friend class SvFactory;
private:
    BOOL            bIsModified:1,
                    bIsInit:1,
                    bOpSave:1,
                    bOpSaveAs:1,
                    bSaveExtern:1,  // Kein SetClassName
                    bOpHandsOff:1,  // Fuer Debug, ob SaveCompleted korrekt ist
                    bCreateTempStor:1, // TRUE nach InitNew(0)
                    bSaveFailed:1, 	// TRUE nach InitNew(0)
                    bEnableSetModified:1,
                    bDummy0:1,
                    bDummy1:1,
                    bIsObjectShell:1;
    USHORT          nModifyCount;
    SvStorageRef    aStorage;
/*  SvAdviseSinkRef aAdvSink;      Benachrichtigung von externen Obj, fuer Modify */
    SvPersist *     pParent;    // fuer hierarchische Persist-Objekte
private:
    SvInfoObjectMemberList * pChildList;
    Time            aModifiedTime;
    String			aFileName;	// nur fuer das IPersistFile-Interface

    SO3_DLLPRIVATE SvInfoObjectMemberList *GetInfoList();
    SO3_DLLPRIVATE void            CountModified( BOOL bDirection );
    SO3_DLLPRIVATE void            dtorClear();
    SO3_DLLPRIVATE BOOL            SaveElement( SvStorage*, SvInfoObject* );
    SO3_DLLPRIVATE SvPersistRef    CreateObjectFromStorage( SvInfoObject*, const SvStorageRef& );
    SO3_DLLPRIVATE BOOL            ImplCopy( SvPersist*, const String&, BOOL );

protected:
    SO3_DLLPRIVATE void            InitMembers( SvStorage * );
                     ~SvPersist();
#ifdef DBG_UTIL
#define ASSERT_INIT()     AssertInit();
    SO3_DLLPRIVATE void            AssertInit() const;
#else
#define ASSERT_INIT()
#endif
    SO3_DLLPRIVATE void			SetSaveFailed( BOOL bFail = TRUE )
                    { bSaveFailed = bFail; }
    SO3_DLLPRIVATE BOOL    		GetSaveFailed() const { return bSaveFailed; }

    virtual void    ModifyChanged();

    virtual BOOL    InitNew( SvStorage * );         // Rekursiv
    virtual BOOL    Load( SvStorage * );            // Rekursiv
    virtual BOOL    Save();                         // Rekursiv
    virtual BOOL    SaveAs( SvStorage * pNewStg );  // Rekursiv
    virtual void    HandsOff();                     // Rekursiv
    virtual BOOL    SaveCompleted( SvStorage * );   // Rekursiv

    SO3_DLLPRIVATE BOOL            DoLoadContent( SvStorage *, BOOL bOwner );
    virtual void    LoadContent( SvStream & rStm, BOOL bOwner );
    SO3_DLLPRIVATE BOOL            DoSaveContent( SvStorage *, BOOL bOwner );
    virtual void    SaveContent( SvStream & rStm, BOOL bOwner );

    BOOL            SaveChilds();                       // Rekursiv
    BOOL            SaveAsChilds( SvStorage * pNewStg );// Rekursiv
    BOOL            SaveCompletedChilds( SvStorage * ); // Rekursiv
    SO3_DLLPRIVATE BOOL			HasStorage() const { return aStorage.Is(); }
public:
                    SvPersist();
                    SO2_DECL_STANDARD_CLASS_DLL(SvPersist,SOAPP)
    SvStorage *     GetStorage() const;
    SvPersist *     GetParent() const { return pParent; }
    void            SetupStorage( SvStorage * pStor ) const;

    virtual void    FillClass( SvGlobalName * pClassName,
                               ULONG * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               long nFileFormat = SOFFICE_FILEFORMAT_CURRENT ) const;

                    // automatische Objektverwaltung um zu speichern
    BOOL            Insert  ( SvInfoObject * pInfoObj );
    BOOL            Move    ( SvInfoObject * pInfoObj, const String & rStorName, BOOL bCopyStorage=FALSE );
    BOOL			Copy	( const String & rNewObjName, const String & rNewStorName,
                              SvInfoObject * pSrcInfoObj, SvPersist * pSrc );

                    // call DoClose( pEle ) and unload the child
                    // TRUE, element is unloaded. FALSE cannot unload,
                    // Refcount != 0 or modified.
    BOOL			Unload	( SvInfoObject * pInfoObj );
    BOOL			Unload	( SvPersist * pEle );

    void            Remove  ( const String & rEleName );
    void            Remove  ( SvPersist * pEle );
    void            Remove  ( SvInfoObject * pInfoObj );
    SvInfoObject *  Find    ( const String & rEleName ) const;
    SvInfoObject *  Find    ( const SvPersist * pEle ) const;
    const SvInfoObjectMemberList * GetObjectList() const
                    { return pChildList; }

                    // altes Interface
    BOOL    		HasObject( const String & rObjName );
    SvPersistRef 	GetObject( const String & rObjName );
    SvStorageRef    GetObjectStorage( SvInfoObject* pEle );

                    // eigentliche Funktionalitaet
    void			EnableSetModified( BOOL bEnable = TRUE );
    BOOL			IsEnableSetModified() const
                    { return bEnableSetModified; }
    virtual void    SetModified( BOOL = TRUE );
    BOOL            IsModified();                   // Rekursiv

    SvPersistRef    CopyObject( const String& rObjName, const String& rNewName, SvPersist * pSrc );

    virtual BOOL    DoInitNew( SvStorage * );
    const String &	GetFileName() const { return aFileName; }
    void			SetFileName( const String & rFileName )
                    { aFileName = rFileName; }
    virtual BOOL    DoLoad( const String & rFileName, StreamMode,
                            short nStorMode );
    virtual BOOL    DoLoad( SvStorage * );
    BOOL			DoOwnerLoad( SvStorage * );
    virtual BOOL    DoSave();
    virtual BOOL    DoSaveAs( SvStorage * pNewStg );
    virtual void    DoHandsOff();
    virtual BOOL    DoSaveCompleted( SvStorage * = NULL );
    void            CleanUp(BOOL bRecurse=FALSE); //Rekursiv

    const Time &    GetModifyTime() const { return aModifiedTime; }
    void			SetModifyTime( const Time & rTime )
                    { aModifiedTime = rTime; }
    BOOL            IsHandsOff() const { return bOpHandsOff; }

    void			SetObjectShell( BOOL bObjSh ) { bIsObjectShell = bObjSh; }
};
SO2_IMPL_REF(SvPersist)

/*************************************************************************
*************************************************************************/

SV_DECL_REF(SvInfoObject)
class SvInfoObject_Impl;
class SO3_DLLPUBLIC SvInfoObject : public SvPersistBase
{
friend class SvPersist;
private:
    SvPersistRef    aObj;
    String          aObjName;           // object and substorage name
    String          aStorName;          // optional real storage name
    SvGlobalName    aSvClassName;       // name of corresponding SV class
    SvInfoObject_Impl* pImp;
    BOOL            bDeleted;

protected:
    SO3_DLLPRIVATE 				~SvInfoObject();
    SO3_DLLPRIVATE virtual void	Assign( const SvInfoObject * pObj );
    SO3_DLLPRIVATE void            SetRealStorageName( const String & rName );
    SO3_DLLPRIVATE void            SetStorageName( const String & rName )
                    { aStorName = rName; }
public:
                    SV_DECL_PERSIST1(SvInfoObject,SvPersistBase,1)
                    SvInfoObject();
                    SvInfoObject( SvPersist *, const String & rObjName );
                    SvInfoObject( const String& rObjName,
                                     const SvGlobalName& rClassName );
    SvInfoObjectRef CreateCopy() const;

    virtual void    SetObj( SvPersist * );

    SvObject  *     GetObj() const { return &aObj; }
    SvPersist *     GetPersist() const { return aObj; }

    void            SetObjName( const String & rName )
                    { aObjName = rName; }
    String          GetObjName() const;

                    // GetStorageName() is the same as GetObjName()
    String          GetStorageName() const;

                    // If object is stored elsewhere, the RealStorageName is not empty
    String          GetRealStorageName() const;
    SvGlobalName    GetClassName() const;
    void    		SetClassName( const SvGlobalName& rName )
                    { aSvClassName = rName; }
    BOOL            IsDeleted() const
                    { return bDeleted; }
    void            SetDeleted( BOOL bDel=TRUE );
};

SV_IMPL_REF(SvInfoObject)
SV_DECL_IMPL_PERSIST_LIST(SvInfoObject,SvInfoObject*)

}

#endif // _PERSIST_HXX
