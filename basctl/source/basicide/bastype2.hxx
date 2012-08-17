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
#ifndef _BASTYPE2_HXX
#define _BASTYPE2_HXX

#include "doceventnotifier.hxx"

#include <memory>
#include "tools/solar.h"

#include <svtools/svtreebx.hxx>
#include <svl/lstner.hxx>
#include <basic/sbstar.hxx>
#include <sbxitem.hxx>
#include "basobj.hxx"

enum BasicEntryType
{
    OBJ_TYPE_UNKNOWN,
    OBJ_TYPE_DOCUMENT,
    OBJ_TYPE_LIBRARY,
    OBJ_TYPE_MODULE,
    OBJ_TYPE_DIALOG,
    OBJ_TYPE_METHOD,
    OBJ_TYPE_DOCUMENT_OBJECTS,
    OBJ_TYPE_USERFORMS,
    OBJ_TYPE_NORMAL_MODULES,
    OBJ_TYPE_CLASS_MODULES
};

#define BROWSEMODE_MODULES      0x01
#define BROWSEMODE_SUBS         0x02
#define BROWSEMODE_DIALOGS      0x04

class SbModule;
class SvLBoxEntry;
class SbxVariable;


class BasicEntry
{
private:
    BasicEntryType  m_eType;

public:
                    BasicEntry( BasicEntryType eType )  { m_eType = eType; }
                    BasicEntry( const BasicEntry& r )   { m_eType = r.m_eType; }
    virtual         ~BasicEntry();

    BasicEntryType  GetType() const                     { return m_eType; }
};

class BasicDocumentEntry : public BasicEntry
{
private:
    ScriptDocument      m_aDocument;
    LibraryLocation     m_eLocation;

public:
                    BasicDocumentEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, BasicEntryType eType = OBJ_TYPE_DOCUMENT );
    virtual         ~BasicDocumentEntry();

    const ScriptDocument&
                    GetDocument() const { return m_aDocument; }
    LibraryLocation GetLocation() const { return m_eLocation; }
};

class BasicLibEntry : public BasicDocumentEntry
{
private:
    ::rtl::OUString m_aLibName;

public:
    BasicLibEntry( const ScriptDocument& rDocument, LibraryLocation eLocation, const ::rtl::OUString& rLibName, BasicEntryType eType = OBJ_TYPE_LIBRARY );
    virtual         ~BasicLibEntry();

    const ::rtl::OUString&   GetLibName() const { return m_aLibName; }
};

class BasicEntryDescriptor
{
    ScriptDocument          m_aDocument;
    LibraryLocation         m_eLocation;
    ::rtl::OUString         m_aLibName;
    ::rtl::OUString         m_aLibSubName;  // for vba entry:  Document Objects, Class Modules, Forms and Normal Modules
    ::rtl::OUString         m_aName;
    ::rtl::OUString         m_aMethodName;
    BasicEntryType          m_eType;

public:
                            BasicEntryDescriptor();
    BasicEntryDescriptor( const ScriptDocument& rDocument, LibraryLocation eLocation, const ::rtl::OUString& rLibName, const ::rtl::OUString& rLibSubName, const ::rtl::OUString& rName, BasicEntryType eType );
    BasicEntryDescriptor( const ScriptDocument& rDocument, LibraryLocation eLocation, const ::rtl::OUString& rLibName, const ::rtl::OUString& rLibSubName, const ::rtl::OUString& rName, const ::rtl::OUString& rMethodName, BasicEntryType eType );
    virtual                 ~BasicEntryDescriptor();

                            BasicEntryDescriptor( const BasicEntryDescriptor& rDesc );
    BasicEntryDescriptor&   operator=( const BasicEntryDescriptor& rDesc );
    bool                    operator==( const BasicEntryDescriptor& rDesc ) const;

    const ScriptDocument&
                            GetDocument() const { return m_aDocument; }
    void                    SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    LibraryLocation         GetLocation() const { return m_eLocation; }
    void                    SetLocation( LibraryLocation eLocation ) { m_eLocation = eLocation; }

    const ::rtl::OUString&  GetLibName() const { return m_aLibName; }
    void                    SetLibName( const ::rtl::OUString& aLibName ) { m_aLibName = aLibName; }

    const ::rtl::OUString&  GetLibSubName() const { return m_aLibSubName; }
    void                    SetLibSubName( const ::rtl::OUString& aLibSubName ) { m_aLibSubName = aLibSubName; }

    const ::rtl::OUString&  GetName() const { return m_aName; }
    void                    SetName( const ::rtl::OUString& aName ) { m_aName = aName; }

    const ::rtl::OUString&           GetMethodName() const { return m_aMethodName; }
    void                    SetMethodName( const ::rtl::OUString& aMethodName ) { m_aMethodName = aMethodName; }

    BasicEntryType          GetType() const { return m_eType; }
    void                    SetType( BasicEntryType eType ) { m_eType = eType; }
};


/************************************************************
    Classification of types and pointers in the BasicEntries:

    OBJ_TYPE_DOCUMENT        BasicDocumentEntry
    OBJ_TYPE_LIBRARY         BasicEntry
    OBJ_TYPE_MODULE          BasicEntry
    OBJ_TYPE_DIALOG          BasicEntry
    OBJ_TYPE_METHOD          BasicEntry

**************************************************************/

class BasicTreeListBox  :public SvTreeListBox
                        ,public ::basctl::DocumentEventListener
{
private:
    sal_uInt16                          nMode;
    ::basctl::DocumentEventNotifier m_aNotifier;

    void            SetEntryBitmaps( SvLBoxEntry * pEntry, const Image& rImage );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

protected:
    virtual void            RequestingChildren( SvLBoxEntry* pParent );
    virtual void            ExpandedHdl();
    virtual SvLBoxEntry*    CloneEntry( SvLBoxEntry* pSource );
    virtual long            ExpandingHdl();

    void                    ImpCreateLibEntries( SvLBoxEntry* pShellRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                    ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName );
    void                    ImpCreateLibSubEntriesInVBAMode( SvLBoxEntry* pLibRootEntry, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName );
    void                    ImpCreateLibSubSubEntriesInVBAMode( SvLBoxEntry* pLibSubRootEntry, const ScriptDocument& rDocument, const ::rtl::OUString& rLibName );
    SvLBoxEntry*            ImpFindEntry( SvLBoxEntry* pParent, const ::rtl::OUString& rText );

    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument );
    virtual void onDocumentOpened( const ScriptDocument& _rDocument );
    virtual void onDocumentSave( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument );
    virtual void onDocumentClosed( const ScriptDocument& _rDocument );
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument );
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument );

public:
                    BasicTreeListBox( Window* pParent, const ResId& rRes );
                    ~BasicTreeListBox();

    void            ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void            ScanAllEntries();
    void            UpdateEntries();

    bool            IsEntryProtected( SvLBoxEntry* pEntry );

    void            SetMode( sal_uInt16 nM ) { nMode = nM; }
    sal_uInt16          GetMode() const { return nMode; }

    SbModule*       FindModule( SvLBoxEntry* pEntry );
    SbxVariable*    FindVariable( SvLBoxEntry* pEntry );
    SvLBoxEntry*    FindRootEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    SvLBoxEntry*    FindEntry( SvLBoxEntry* pParent, const ::rtl::OUString& rText, BasicEntryType eType );

    BasicEntryDescriptor    GetEntryDescriptor( SvLBoxEntry* pEntry );

    BasicIDEType    ConvertType( BasicEntryType eType );
    bool            IsValidEntry( SvLBoxEntry* pEntry );

    SvLBoxEntry*    AddEntry( const ::rtl::OUString& rText, const Image& rImage,
                              SvLBoxEntry* pParent, bool bChildrenOnDemand,
                              std::auto_ptr< BasicEntry > aUserData );
    void            RemoveEntry (SvLBoxEntry*);
    void            RemoveEntry (ScriptDocument const&);

    ::rtl::OUString GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation ) const;
    void            GetRootEntryBitmaps( const ScriptDocument& rDocument, Image& rImage );

    void            SetCurrentEntry( BasicEntryDescriptor& rDesc );

private:
    LibraryType     GetLibraryType() const;
};

#endif  // _BASTYPE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
