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

#include <sal/config.h>

#include <memory>

#include "doceventnotifier.hxx"

#include <vcl/weld.hxx>
#include <basctl/sbxitem.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>


class SbModule;
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
        ScriptDocument aDocument,
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
        OUString aLibName
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
        ScriptDocument aDocument,
        LibraryLocation eLocation,
        OUString aLibName,
        OUString aLibSubName,
        OUString aName,
        EntryType eType
    );
    EntryDescriptor (
        ScriptDocument aDocument,
        LibraryLocation eLocation,
        OUString aLibName,
        OUString aLibSubName,
        OUString aName,
        OUString aMethodName,
        EntryType eType
    );

    ScriptDocument const&   GetDocument()   const { return m_aDocument; }
    LibraryLocation         GetLocation()   const { return m_eLocation; }

    const OUString&         GetLibName()    const { return m_aLibName; }
    const OUString&         GetLibSubName() const { return m_aLibSubName; }
    const OUString&         GetName()       const { return m_aName; }
    const OUString&         GetMethodName() const { return m_aMethodName; }
    void                    SetMethodName( const OUString& aMethodName ) { m_aMethodName = aMethodName; }

    EntryType               GetType() const { return m_eType; }
    void                    SetType( EntryType eType ) { m_eType = eType; }

    friend bool operator==(const basctl::EntryDescriptor& aDesc1, const basctl::EntryDescriptor& aDesc2) = default;
};


/*
    Classification of types and pointers in the Entries:

    OBJ_TYPE_DOCUMENT        DocumentEntry
    OBJ_TYPE_LIBRARY         Entry
    OBJ_TYPE_MODULE          Entry
    OBJ_TYPE_DIALOG          Entry
    OBJ_TYPE_METHOD          Entry

*/

class SbTreeListBox : public DocumentEventListener
{
private:
    std::unique_ptr<weld::TreeView> m_xControl;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    weld::Window* m_pTopLevel;
    bool m_bFreezeOnFirstAddRemove;
    BrowseMode nMode;
    DocumentEventNotifier m_aNotifier;
    void            SetEntryBitmaps(const weld::TreeIter& rIter, const OUString& rImage);

protected:
    DECL_LINK(RequestingChildrenHdl, const weld::TreeIter&, bool);
    DECL_LINK(OpenCurrentHdl, weld::TreeView&, bool);
    // Creates popup menu to select between alphabetic or unsorted view in Macro List Tree
    DECL_LINK(ContextMenuHdl, const CommandEvent&, bool);
    void                    ImpCreateLibEntries(const weld::TreeIter& rShellRootEntry, const ScriptDocument& rDocument, LibraryLocation eLocation);
    void                    ImpCreateLibSubEntries(const weld::TreeIter& rLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName);
    void                    ImpCreateLibSubEntriesInVBAMode(const weld::TreeIter& rLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName );
    void                    ImpCreateLibSubSubEntriesInVBAMode(const weld::TreeIter& rLibRootEntry, const ScriptDocument& rDocument, const OUString& rLibName);
    bool                    ImpFindEntry(weld::TreeIter& rIter, std::u16string_view rText);

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
    SbTreeListBox(std::unique_ptr<weld::TreeView> xControl, weld::Window* pTopLevel);
    virtual ~SbTreeListBox() override;

    void            ScanEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void            ScanAllEntries();
    void            UpdateEntries();
    /**
     * Removes then reloads all items in tree, restoring expansion state of entries
     *
     * @see SbTreeListBox::UpdateEntries()
     * @see SbTreeListBox::FindRootEntry(const ScriptDocument&, LibraryLocation, weld::TreeIter&)
     * @param None
     * @return None
     */
    void            ReloadAllEntries();

    bool            IsEntryProtected(const weld::TreeIter* pEntry);

    void            SetMode( BrowseMode nM ) { nMode = nM; }
    BrowseMode      GetMode() const { return nMode; }

    SbModule*       FindModule(const weld::TreeIter* pEntry);
    SbxVariable*    FindVariable(const weld::TreeIter* pEntry);
    bool            FindRootEntry(const ScriptDocument& rDocument, LibraryLocation eLocation, weld::TreeIter& rIter);
    bool            FindEntry(std::u16string_view rText, EntryType eType, weld::TreeIter& rIter);
    EntryDescriptor GetEntryDescriptor(const weld::TreeIter* pEntry);

    static SbxItemType ConvertType (EntryType eType);
    bool            IsValidEntry(const weld::TreeIter& rEntry);
    void AddEntry(const OUString& rText, const OUString& rImage,
                  const weld::TreeIter* pParent, bool bChildrenOnDemand,
                  std::unique_ptr<Entry>&& rUserData,
                  weld::TreeIter* pRet = nullptr);

    void connect_changed(const Link<weld::TreeView&, void>& rLink)
    {
        m_xControl->connect_selection_changed(rLink);
    }
    std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pIter = nullptr) const { return m_xControl->make_iterator(pIter); }
    void copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const { m_xControl->copy_iterator(rSource, rDest); }
    bool get_selected(weld::TreeIter* pIter) const { return m_xControl->get_selected(pIter); }
    void select(const weld::TreeIter& rIter) { m_xControl->select(rIter); }
    void unselect(const weld::TreeIter& rIter) { m_xControl->unselect(rIter); }
    void remove(const weld::TreeIter& rIter) { m_xControl->remove(rIter); }
    bool get_cursor(weld::TreeIter* pIter) const { return m_xControl->get_cursor(pIter); }
    void set_cursor(const weld::TreeIter& rIter) { m_xControl->set_cursor(rIter); }
    OUString get_text(const weld::TreeIter& rIter) const { return m_xControl->get_text(rIter); }
    void set_text(const weld::TreeIter& rIter, const OUString& rText) { m_xControl->set_text(rIter, rText); }
    OUString get_id(const weld::TreeIter& rIter) const { return m_xControl->get_id(rIter); }
    bool get_iter_first(weld::TreeIter& rIter) const { return m_xControl->get_iter_first(rIter); }
    bool iter_next_sibling(weld::TreeIter& rIter) const { return m_xControl->iter_next_sibling(rIter); }
    bool iter_children(weld::TreeIter& rIter) const { return m_xControl->iter_children(rIter); }
    bool iter_parent(weld::TreeIter& rIter) const { return m_xControl->iter_parent(rIter); }
    int get_iter_depth(const weld::TreeIter& rIter) const { return m_xControl->get_iter_depth(rIter); }
    bool get_row_expanded(const weld::TreeIter& rIter) const { return m_xControl->get_row_expanded(rIter); }
    void expand_row(const weld::TreeIter& rIter) { m_xControl->expand_row(rIter); }
    void set_size_request(int nWidth, int nHeight) { m_xControl->set_size_request(nWidth, nHeight); }
    float get_approximate_digit_width() const { return m_xControl->get_approximate_digit_width(); }
    int get_height_rows(int nRows) const { return m_xControl->get_height_rows(nRows); }
    int get_iter_index_in_parent(const weld::TreeIter& rIter) const { return m_xControl->get_iter_index_in_parent(rIter); }
    void connect_editing(const Link<const weld::TreeIter&, bool>& rStartLink,
                         const Link<const std::pair<const weld::TreeIter&, OUString>&, bool>& rEndLink)
    {
        m_xControl->connect_editing(rStartLink, rEndLink);
    }

    void make_sorted() { m_xControl->make_sorted(); };

    void            RemoveEntry(const weld::TreeIter& rIter);
    void            RemoveEntry(const ScriptDocument&);

    OUString        GetRootEntryName(const ScriptDocument& rDocument, LibraryLocation eLocation) const;
    static OUString GetRootEntryBitmaps(const ScriptDocument& rDocument);

    void            SetCurrentEntry (EntryDescriptor const &);

    weld::TreeView& get_widget() { return *m_xControl; }

private:
    LibraryType     GetLibraryType() const;
};

} // namespace basctl


namespace std {
template<> class hash<basctl::EntryDescriptor>
{
    public:
    std::size_t operator()(const basctl::EntryDescriptor& rDesc) const noexcept
    {
        // Serialize EntryDescriptor as OUString
        OUString sDescSerial =
            OUString::number(rDesc.GetDocument().hashCode())    + "|" +
            OUString::number(rDesc.GetLocation())               + "|" +
            rDesc.GetLibName()                                  + "|" +
            rDesc.GetLibSubName()                               + "|" +
            rDesc.GetName()                                     + "|" +
            rDesc.GetMethodName()                               + "|" +
            OUString::number(rDesc.GetType());

        return sDescSerial.hashCode();
    }
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
