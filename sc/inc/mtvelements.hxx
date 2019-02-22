/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_MTVELEMENTS_HXX
#define INCLUDED_SC_INC_MTVELEMENTS_HXX

#include "formulacell.hxx"
#include <svl/broadcast.hxx>
#include <svl/sharedstring.hxx>
#include <editeng/editobj.hxx>
#include "calcmacros.hxx"
#include "postit.hxx"
#include "celltextattr.hxx"
#include <osl/mutex.hxx>

#if DEBUG_COLUMN_STORAGE
#ifdef NDEBUG
#undef NDEBUG
#endif
#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#endif

#include <mdds/multi_type_vector_macro.hpp>
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>
#include <mdds/multi_type_vector_custom_func3.hpp>

#include <unordered_map>
#include <memory>

class ScDocument;
class ScColumn;
struct ScRefCellValue;

namespace sc {

/// Custom element type IDs for multi_type_vector.

const mdds::mtv::element_t element_type_broadcaster = mdds::mtv::element_type_user_start;
const mdds::mtv::element_t element_type_celltextattr = mdds::mtv::element_type_user_start + 1;

const mdds::mtv::element_t element_type_string = mdds::mtv::element_type_user_start + 2;
const mdds::mtv::element_t element_type_edittext = mdds::mtv::element_type_user_start + 3;
const mdds::mtv::element_t element_type_formula = mdds::mtv::element_type_user_start + 4;

const mdds::mtv::element_t element_type_cellnote = mdds::mtv::element_type_user_start + 5;

/// Mapped standard element types (for convenience).
const mdds::mtv::element_t element_type_numeric = mdds::mtv::element_type_numeric;
const mdds::mtv::element_t element_type_empty = mdds::mtv::element_type_empty;
const mdds::mtv::element_t element_type_uint16 = mdds::mtv::element_type_ushort;

/// Custom element blocks.

typedef mdds::mtv::noncopyable_managed_element_block<element_type_cellnote, ScPostIt> cellnote_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_broadcaster, SvtBroadcaster> broadcaster_block;
typedef mdds::mtv::default_element_block<element_type_celltextattr, CellTextAttr> celltextattr_block;
typedef mdds::mtv::default_element_block<element_type_string, svl::SharedString> string_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_edittext, EditTextObject> edittext_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_formula, ScFormulaCell> formula_block;

/// Mapped standard element blocks (for convenience).
typedef mdds::mtv::numeric_element_block numeric_block;
typedef mdds::mtv::ushort_element_block uint16_block;

/// This needs to be in the same namespace as CellTextAttr.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(CellTextAttr, element_type_celltextattr, CellTextAttr(), celltextattr_block)
}

/// These need to be in global namespace just like their respective types are.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(ScPostIt, sc::element_type_cellnote, nullptr, sc::cellnote_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(SvtBroadcaster, sc::element_type_broadcaster, nullptr, sc::broadcaster_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(ScFormulaCell, sc::element_type_formula, nullptr, sc::formula_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(EditTextObject, sc::element_type_edittext, nullptr, sc::edittext_block)

namespace svl {

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(SharedString, sc::element_type_string, SharedString(), sc::string_block)

}

namespace sc {

class CellStoreEvent
{
    ScColumn* mpCol;
public:
    CellStoreEvent();
    CellStoreEvent(ScColumn* pCol);

    void element_block_acquired(const mdds::mtv::base_element_block* block);
    void element_block_released(const mdds::mtv::base_element_block* block);
};

/// Cell note container
typedef mdds::mtv::custom_block_func1<sc::cellnote_block> CNoteFunc;
typedef mdds::multi_type_vector<CNoteFunc> CellNoteStoreType;

/// Broadcaster storage container
typedef mdds::mtv::custom_block_func1<sc::broadcaster_block> BCBlkFunc;
typedef mdds::multi_type_vector<BCBlkFunc> BroadcasterStoreType;

/// Cell text attribute container.
typedef mdds::mtv::custom_block_func1<sc::celltextattr_block> CTAttrFunc;
typedef mdds::multi_type_vector<CTAttrFunc> CellTextAttrStoreType;

/// Cell container
typedef mdds::mtv::custom_block_func3<sc::string_block, sc::edittext_block, sc::formula_block> CellFunc;
typedef mdds::multi_type_vector<CellFunc, CellStoreEvent> CellStoreType;

/**
 * Store position data for column array storage.
 */
struct ColumnBlockPosition
{
    CellNoteStoreType::iterator miCellNotePos;
    BroadcasterStoreType::iterator miBroadcasterPos;
    CellTextAttrStoreType::iterator miCellTextAttrPos;
    CellStoreType::iterator miCellPos;

    ColumnBlockPosition(): miCellPos() {}
};

struct ColumnBlockConstPosition
{
    CellNoteStoreType::const_iterator miCellNotePos;
    CellTextAttrStoreType::const_iterator miCellTextAttrPos;
    CellStoreType::const_iterator miCellPos;

    ColumnBlockConstPosition(): miCellPos() {}
};

class ColumnBlockPositionSet
{
    typedef std::unordered_map<SCCOL, ColumnBlockPosition> ColumnsType;
    typedef std::unordered_map<SCTAB, ColumnsType> TablesType;

    ScDocument& mrDoc;
    TablesType maTables;
    osl::Mutex maMtxTables;

public:
    ColumnBlockPositionSet(ScDocument& rDoc);

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);

    void clear();
};

/**
 * Set of column block positions only for one table.
 */
class TableColumnBlockPositionSet
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    TableColumnBlockPositionSet( ScDocument& rDoc, SCTAB nTab );
    TableColumnBlockPositionSet( TableColumnBlockPositionSet&& rOther );
    ~TableColumnBlockPositionSet();

    ColumnBlockPosition* getBlockPosition( SCCOL nCol );
};

ScRefCellValue toRefCell( const sc::CellStoreType::const_iterator& itPos, size_t nOffset );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
