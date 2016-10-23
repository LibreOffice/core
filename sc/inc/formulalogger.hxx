/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_FORMULALOGGER_HXX
#define INCLUDED_SC_INC_FORMULALOGGER_HXX

#include <osl/file.hxx>
#include <memory>
#include <vector>

class ScFormulaCell;
class ScDocument;

namespace sc {

/**
 * Outputs formula calculation log outputs to specified file.
 */
class FormulaLogger
{
    std::unique_ptr<osl::File> mpLogFile;
    std::vector<OUString> maMessages;

    sal_Int32 mnNestLevel = 0;

    void writeAscii( const char* s );
    void writeAscii( const char* s, size_t n );
    void write( const OUString& ou );
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
            const ScDocument& rDoc, const ScFormulaCell& rCell );

    public:
        GroupScope( GroupScope&& r );
        ~GroupScope();

        /**
         * Add an arbitrary message to dump to the log.
         */
        void addMessage( const OUString& rMsg );

        /**
         * Call this when the group calculation has finished successfullly.
         */
        void setCalcComplete();
    };

    FormulaLogger( const FormulaLogger& ) = delete;
    FormulaLogger& operator= ( const FormulaLogger& ) = delete;

    FormulaLogger();
    ~FormulaLogger();

    GroupScope enterGroup( const ScDocument& rDoc, const ScFormulaCell& rCell );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
