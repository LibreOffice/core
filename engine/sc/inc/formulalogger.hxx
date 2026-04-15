/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

#include <memory>
#include <string_view>
#include <vector>

#include <config_options_calc.h>

class ScFormulaCell;
class ScDocument;
class ScAddress;
struct ScFormulaCellGroup;

namespace formula {

class FormulaToken;
struct VectorRefArray;

}

namespace osl { class File; }

namespace sc {

#if ENABLE_FORMULA_LOGGER

/**
 * Outputs formula calculation log outputs to specified file.
 */
class FormulaLogger
{
    std::unique_ptr<osl::File> mpLogFile;

    sal_Int32 mnNestLevel = 0;
    const ScFormulaCellGroup* mpLastGroup = nullptr;

    void writeAscii( const char* s );
    void writeAscii( const char* s, size_t n );
    void write( std::u16string_view ou );
    void write( sal_Int32 n );

    void sync();

    void writeNestLevel();

public:

    static FormulaLogger& get();

    /**
     * This class is only moveable.
     */
    class GroupScope
    {
        friend class FormulaLogger;

        struct Impl;
        std::unique_ptr<Impl> mpImpl;

    public:
        GroupScope() = delete;
        GroupScope( const GroupScope& ) = delete;
        GroupScope& operator= ( const GroupScope& ) = delete;

    private:
        GroupScope(
            FormulaLogger& rLogger, const OUString& rPrefix,
            const ScDocument& rDoc, const ScFormulaCell& rCell,
            bool bOutputEnabled );

    public:
        GroupScope(GroupScope&& r) noexcept;
        ~GroupScope();

        /**
         * Add an arbitrary message to dump to the log.
         */
        void addMessage( const OUString& rMsg );

        /**
         * Add to the log a vector reference information for a single
         * reference.
         */
        void addRefMessage(
            const ScAddress& rCellPos, const ScAddress& rRefPos, size_t nLen,
            const formula::VectorRefArray& rArray );

        /**
         * Add to the log a vector reference information for a range
         * reference.
         */
        void addRefMessage(
            const ScAddress& rCellPos, const ScAddress& rRefPos, size_t nLen,
            const std::vector<formula::VectorRefArray>& rArrays );

        /**
         * Add to the log a single cell reference information.
         */
        void addRefMessage(
            const ScAddress& rCellPos, const ScAddress& rRefPos,
            const formula::FormulaToken& rToken );

        void addGroupSizeThresholdMessage( const ScFormulaCell& rCell );

        /**
         * Call this when the group calculation has finished successfully.
         */
        void setCalcComplete();
    };

    FormulaLogger( const FormulaLogger& ) = delete;
    FormulaLogger& operator= ( const FormulaLogger& ) = delete;

    FormulaLogger();
    ~FormulaLogger();

    GroupScope enterGroup( const ScDocument& rDoc, const ScFormulaCell& rCell );
};

#else

/**
 * Dummy class with all empty inline methods.
 */
class FormulaLogger
{
public:

    static FormulaLogger get()
    {
        return FormulaLogger();
    }

    class GroupScope
    {
    public:
        void addMessage( [[maybe_unused]] const OUString& /*rMsg*/ ) { (void) this; /* loplugin:staticmethods */ }

        void addRefMessage(
            const ScAddress& /*rCellPos*/, const ScAddress& /*rRefPos*/, size_t /*nLen*/,
            const formula::VectorRefArray& /*rArray*/ )
        {
            (void) this; /* loplugin:staticmethods */
        }

        void addRefMessage(
            const ScAddress& /*rCellPos*/, const ScAddress& /*rRefPos*/, size_t /*nLen*/,
            const std::vector<formula::VectorRefArray>& /*rArrays*/ )
        {
            (void) this; /* loplugin:staticmethods */
        }

        void addRefMessage(
            const ScAddress& /*rCellPos*/, const ScAddress& /*rRefPos*/,
            const formula::FormulaToken& /*rToken*/ )
        {
            (void) this; /* loplugin:staticmethods */
        }

        void addGroupSizeThresholdMessage( const ScFormulaCell& /*rCell*/ )
        {
            (void) this; /* loplugin:staticmethods */
        }

        void setCalcComplete() { (void) this; /* loplugin:staticmethods */ }
    };

    GroupScope enterGroup( const ScDocument& /*rDoc*/, const ScFormulaCell& /*rCell*/ )
    {
        (void) this; /* loplugin:staticmethods */
        return GroupScope();
    }
};

#endif // ENABLE_FORMULA_LOGGER

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
