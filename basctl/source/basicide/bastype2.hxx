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
#ifndef BASCTL_BASTYPE2_HXX
#define BASCTL_BASTYPE2_HXX

#include "doceventnotifier.hxx"

#include <o3tl/heap_ptr.hxx>

#include <svtools/treelistbox.hxx>
#include <svl/lstner.hxx>
#include <basic/sbstar.hxx>
#include <sbxitem.hxx>
#include "basobj.hxx"

class SbModule;
class SvTreeListEntry;
class SbxVariable;

namespace basctl
{

enum EntryType
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

enum
{
    BROWSEMODE_MODULES  = 0x01,
    BROWSEMODE_SUBS     = 0x02,
    BROWSEMODE_DIALOGS  = 0x04,
};

class Entry
{
private:
    EntryType m_eType;

public:
    Entry (EntryType eType) : m_eType(eType) { }
    virtual ~Entry ();

    EntryType GetType () const { return m_eType; }
};

class DocumentEntry : public Entry
{
private:
    ScriptDocument      m_aDocument;
    LibraryLocation     m_eLocation;

public:
    DocumentEntry (
        ScriptDocument const& rDocument,
        LibraryLocation eLocation,
        EntryType eType = OBJ_TYPE_DOCUMENT
    );
    virtual ~DocumentEntry ();

    ScriptDocument const& GetDocument() const { return m_aDocument; }
    LibraryLocation GetLocation() const { return m_eLocation; }
};

class LibEntry : public DocumentEntry
{
private:
    OUString m_aLibName;

public:
    LibEntry (
        ScriptDocument const& rDocument,
        LibraryLocation eLocation,
        OUString       const& rLibName,
        EntryType eType = OBJ_TYPE_LIBRARY
    );
    virtual ~LibEntry ();

    OUString const& GetLibName () const { return m_aLibName; }
};

class EntryDescriptor
{
    ScriptDocument   m_aDocument;
    LibraryLocation  m_eLocation;
    OUString         m_aLibName;
    OUString         m_aLibSubName;  // for vba entry:  Document Objects, Class Modules, Forms and Normal Modules
    OUString         m_aName;
    OUString         m_aMethodName;
    EntryType        m_eType;

public:
    EntryDescriptor ();
    EntryDescriptor (
        ScriptDocument const& rDocument,
        LibraryLocation eLocation,
        OUString      const& rLibName,
        OUString      const& rLibSubName,
        OUString      const& rName,
        EntryType eType
    );
    EntryDescriptor (
        ScriptDocument const& rDocument,
        LibraryLocation eLocation,
        OUString      const& rLibName,
        OUString      const& rLibSubName,
        OUString      const& rName,
        OUString      const& rMethodName,
        EntryType eType
    );
    virtual ~EntryDescriptor ();

    bool operator == (EntryDescriptor const& rDesc) const;

    ScriptDocument const&   GetDocument() const { return m_aDocument; }
    void                    SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    LibraryLocation         GetLocation() const { return m_eLocation; }
    void                    SetLocation( LibraryLocation eLocation ) { m_eLocation = eLocation; }

    const OUString&         GetLibName() const { return m_aLibName; }
    void                    SetLibName( const OUString& aLibName ) { m_aLibName = aLibName; }

    const OUString&         GetLibSubName() const { return m_aLibSubName; }
    void                    SetLibSubName( const OUString& aLibSubName ) { m_aLibSubName = aLibSubName; }

    const OUString&         GetName() const { return m_aName; }
    void                    SetName( const OUString& aName ) { m_aName = aName; }

    const OUString&         GetMethodName() const { return m_aMethodName; }
    void                    SetMethodName( const OUString& aMethodName ) { m_aMethodName = aMethodName; }

    EntryType               GetType() const { return m_eType; }
    void                    SetType( EntryType eType ) { m_eType = eType; }
};


/*
    Classification of types and pointers in the Entries:

    OBJ_TYPE_DOCUMENT        DocumentEntry
    OBJ_TYPE_LIBRARY         Entry
    OBJ_TYPE_MODULE          Entry
    OBJ_TYPE_DIALOG          Entry
    OBJ_TYPE_METHOD          Entry

*/

class TreeListBox : public SvTreeListBox, public DocumentEventListener
{
private:
    sal_uInt16 nMode;
    DocumentEventNotifier m_aNotifier;
    void            Init();
    void            SetEntryBitmaps( SvTreeListEntry * pEntry, const Image& rImage );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

protected:
    virtual void            RequestingChildren( SvTreeListEntry* pParent );
    virtual void            ExpandedHdl();
    virtual SvTreeListEntry*    CloneEntry( SvTreeListEntry* pSource );
    virtual bool            ExpandingHdl();

    void                    ImpCreateLibEntries( SvTreeListEntry* pShellRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                    ImpCreateLibSubEntries( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    void                    ImpCreateLibSubEntriesInVBAMode( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    void                    ImpCreateLibSubSubEntriesInVBAMode( SvTreeListEntry* pLibSubRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    SvTreeListEntry*            ImpFindEntry( SvTreeListEntry* pParent, const OUString& rText );

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
    TreeListBox(Window* pParent, const ResId& rRes);
    TreeListBox(Window* pParent, WinBits nStyle);
    ~TreeListBox();

    void            ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void            ScanAllEntries();
    void            UpdateEntries();

    bool            IsEntryProtected( SvTreeListEntry* pEntry );

    void            SetMode( sal_uInt16 nM ) { nMode = nM; }
    sal_uInt16          GetMode() const { return nMode; }

    SbModule*       FindModule( SvTreeListEntry* pEntry );
    SbxVariable*    FindVariable( SvTreeListEntry* pEntry );
    SvTreeListEntry*    FindRootEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    SvTreeListEntry*    FindEntry( SvTreeListEntry* pParent, const OUString& rText, EntryType eType );

    EntryDescriptor GetEntryDescriptor( SvTreeListEntry* pEntry );

    ItemType        ConvertType (EntryType eType);
    bool            IsValidEntry( SvTreeListEntry* pEntry );

    SvTreeListEntry*    AddEntry(
        const OUString& rText, const Image& rImage,
        SvTreeListEntry* pParent, bool bChildrenOnDemand,
        o3tl::heap_ptr<Entry> * aUserData
    );
    void            RemoveEntry (SvTreeListEntry*);
    void            RemoveEntry (ScriptDocument const&);

    OUString GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation ) const;
    void            GetRootEntryBitmaps( const ScriptDocument& rDocument, Image& rImage );

    void            SetCurrentEntry (EntryDescriptor&);

private:
    LibraryType     GetLibraryType() const;
};

} // namespace basctl

#endif // BASCTL_BASTYPE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
