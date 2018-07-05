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

#ifndef INCLUDED_BASCTL_SOURCE_INC_BASTYPE2_HXX
#define INCLUDED_BASCTL_SOURCE_INC_BASTYPE2_HXX

#include <sal/config.h>

#include <memory>

#include "doceventnotifier.hxx"

#include <svtools/treelistbox.hxx>
#include <basic/sbstar.hxx>
#include "sbxitem.hxx"
#include "basobj.hxx"
#include <o3tl/typed_flags_set.hxx>

class SbModule;
class SvTreeListEntry;
class SbxVariable;

enum class BrowseMode
{
    Modules  = 0x01,
    Subs     = 0x02,
    Dialogs  = 0x04,
    All      = Modules | Subs | Dialogs,
};
namespace o3tl {
    template<> struct typed_flags<BrowseMode> : is_typed_flags<BrowseMode, 0x7> {};
}

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

class Entry
{
private:
    EntryType m_eType;

public:
    explicit Entry(EntryType eType)
        : m_eType(eType)
    {
    }

    virtual ~Entry();

    Entry(Entry const &) = default;
    Entry(Entry &&) = default;
    Entry & operator =(Entry const &) = default;
    Entry & operator =(Entry &&) = default;

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
    virtual ~DocumentEntry () override;

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
        OUString       const& rLibName
    );
    virtual ~LibEntry () override;

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

    ScriptDocument const&   GetDocument() const { return m_aDocument; }

    LibraryLocation         GetLocation() const { return m_eLocation; }

    const OUString&         GetLibName() const { return m_aLibName; }

    const OUString&         GetLibSubName() const { return m_aLibSubName; }

    const OUString&         GetName() const { return m_aName; }

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
    BrowseMode            nMode;
    DocumentEventNotifier m_aNotifier;
    void            SetEntryBitmaps( SvTreeListEntry * pEntry, const Image& rImage );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;

protected:
    virtual void            RequestingChildren( SvTreeListEntry* pParent ) override;
    virtual void            ExpandedHdl() override;
    virtual SvTreeListEntry*    CloneEntry( SvTreeListEntry* pSource ) override;
    virtual bool            ExpandingHdl() override;
    virtual void            KeyInput( const KeyEvent& rEvt ) override;

    bool                    OpenCurrent();
    void                    ImpCreateLibEntries( SvTreeListEntry* pShellRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                    ImpCreateLibSubEntries( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    void                    ImpCreateLibSubEntriesInVBAMode( SvTreeListEntry* pLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    void                    ImpCreateLibSubSubEntriesInVBAMode( SvTreeListEntry* pLibSubRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    SvTreeListEntry*            ImpFindEntry( SvTreeListEntry* pParent, const OUString& rText );

    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentOpened( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSave( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentClosed( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument ) override;

public:
    TreeListBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~TreeListBox() override;
    virtual void    dispose() override;

    void            ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void            ScanAllEntries();
    void            UpdateEntries();

    bool            IsEntryProtected( SvTreeListEntry* pEntry );

    void            SetMode( BrowseMode nM ) { nMode = nM; }
    BrowseMode      GetMode() const { return nMode; }

    SbModule*       FindModule( SvTreeListEntry* pEntry );
    SbxVariable*    FindVariable( SvTreeListEntry* pEntry );
    SvTreeListEntry*    FindRootEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    SvTreeListEntry*    FindEntry( SvTreeListEntry* pParent, const OUString& rText, EntryType eType );

    EntryDescriptor GetEntryDescriptor( SvTreeListEntry* pEntry );

    static ItemType ConvertType (EntryType eType);
    bool            IsValidEntry( SvTreeListEntry* pEntry );

    SvTreeListEntry*    AddEntry(
        const OUString& rText, const Image& rImage,
        SvTreeListEntry* pParent, bool bChildrenOnDemand,
        std::unique_ptr<Entry> && aUserData
    );
    void            RemoveEntry (SvTreeListEntry const *);
    void            RemoveEntry (ScriptDocument const&);

    OUString        GetRootEntryName( const ScriptDocument& rDocument, LibraryLocation eLocation ) const;
    static void     GetRootEntryBitmaps( const ScriptDocument& rDocument, Image& rImage );

    void            SetCurrentEntry (EntryDescriptor const &);

private:
    LibraryType     GetLibraryType() const;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_BASTYPE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
