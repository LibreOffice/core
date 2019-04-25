/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#pragma once

#include <set>

#include "plugin.hxx"

namespace loplugin
{

class SalLogAreas
    : public FilteringPlugin< SalLogAreas >
    {
    public:
        explicit SalLogAreas( const InstantiationData& data );
        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* function );
        bool VisitCallExpr( const CallExpr* call );
    private:
        void checkArea( StringRef area, SourceLocation location );
        void checkAreaSyntax(StringRef area, SourceLocation location);
        void readLogAreas();
        const FunctionDecl* inFunction;
        SourceLocation lastSalDetailLogStreamMacro;
        std::set< std::string > logAreas;
#if 0
        std::string firstSeenLogArea;
        SourceLocation firstSeenLocation;
#endif
    };

} // namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
