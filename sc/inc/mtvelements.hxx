/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "formulacell.hxx"
#include <svl/broadcast.hxx>
#include <svl/sharedstring.hxx>
#include <editeng/editobj.hxx>
#include "calcmacros.hxx"
#include "postit.hxx"
#include "SparklineCell.hxx"
#include "celltextattr.hxx"

#if DEBUG_COLUMN_STORAGE
#ifdef NDEBUG
#undef NDEBUG
#endif
#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#endif

#include <mdds/multi_type_vector/macro.hpp>
#include <mdds/multi_type_vector/soa/main.hpp>
#include <mdds/multi_type_vector/block_funcs.hpp>

#include <unordered_map>
#include <memory>
#include <mutex>

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
const mdds::mtv::element_t element_type_sparkline = mdds::mtv::element_type_user_start + 6;

/// Mapped standard element types (for convenience).
const mdds::mtv::element_t element_type_numeric = mdds::mtv::element_type_double;
const mdds::mtv::element_t element_type_empty = mdds::mtv::element_type_empty;
const mdds::mtv::element_t element_type_uint16 = mdds::mtv::element_type_uint16;

/// Custom element blocks.

typedef mdds::mtv::noncopyable_managed_element_block<element_type_sparkline, sc::SparklineCell> sparkline_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_cellnote, ScPostIt> cellnote_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_broadcaster, SvtBroadcaster> broadcaster_block;
typedef mdds::mtv::default_element_block<element_type_celltextattr, CellTextAttr> celltextattr_block;
typedef mdds::mtv::default_element_block<element_type_string, svl::SharedString> string_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_edittext, EditTextObject> edittext_block;
typedef mdds::mtv::noncopyable_managed_element_block<element_type_formula, ScFormulaCell> formula_block;

/// Mapped standard element blocks (for convenience).
typedef mdds::mtv::double_element_block numeric_block;
typedef mdds::mtv::uint16_element_block uint16_block;

} // end sc namespace

/// CAUTION! The following defines must be in the same namespace as the respective type.
/// For example sc types like sc::CellTextAttr, ScFormulaCell in global namespace.
namespace sc {
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(CellTextAttr, element_type_celltextattr, CellTextAttr(), celltextattr_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(SparklineCell, sc::element_type_sparkline, nullptr, sc::sparkline_block)
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

    /** Stop processing events. */
    void stop();

    void swap(CellStoreEvent& other);

    const ScColumn* getColumn() const;
};

struct SparklineTraits : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<sc::sparkline_block>;
};

struct CellNoteTraits : public mdds::mtv::default_traits
{
    using event_func = CellStoreEvent;
    using block_funcs = mdds::mtv::element_block_funcs<sc::cellnote_block>;
};

struct BroadcasterTraits : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<sc::broadcaster_block>;
};

struct CellTextAttrTraits : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<sc::celltextattr_block>;
};

struct CellStoreTraits : public mdds::mtv::default_traits
{
    using event_func = CellStoreEvent;
    using block_funcs = mdds::mtv::element_block_funcs<
        numeric_block, sc::string_block, sc::edittext_block, sc::formula_block>;
};

/// Sparkline container
typedef mdds::mtv::soa::multi_type_vector<SparklineTraits> SparklineStoreType;

/// Cell note container
typedef mdds::mtv::soa::multi_type_vector<CellNoteTraits> CellNoteStoreType;

/// Broadcaster storage container
typedef mdds::mtv::soa::multi_type_vector<BroadcasterTraits> BroadcasterStoreType;

/// Cell text attribute container.
typedef mdds::mtv::soa::multi_type_vector<CellTextAttrTraits> CellTextAttrStoreType;

/// Cell container
typedef mdds::mtv::soa::multi_type_vector<CellStoreTraits> CellStoreType;

/**
 * Store position data for column array storage.
 */
struct ColumnBlockPosition
{
    CellNoteStoreType::iterator miCellNotePos;
    SparklineStoreType::iterator miSparklinePos;
    BroadcasterStoreType::iterator miBroadcasterPos;
    CellTextAttrStoreType::iterator miCellTextAttrPos;
    CellStoreType::iterator miCellPos;

    ColumnBlockPosition(): miCellPos() {}
};

struct ColumnBlockConstPosition
{
    CellNoteStoreType::const_iterator miCellNotePos;
    SparklineStoreType::const_iterator miSparklinePos;
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
    std::mutex maMtxTables;

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
    TableColumnBlockPositionSet(TableColumnBlockPositionSet&& rOther) noexcept;
    ~TableColumnBlockPositionSet();

    ColumnBlockPosition* getBlockPosition( SCCOL nCol );
    void invalidate(); // discards cached positions
};

ScRefCellValue toRefCell( const sc::CellStoreType::const_iterator& itPos, size_t nOffset );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
