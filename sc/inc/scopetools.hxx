/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"

class ScDocument;
class ScColumn;
namespace vcl
{
class Window;
}

namespace sc
{
/**
 * Temporarily switch on/off auto calculation mode.
 */
class SC_DLLPUBLIC AutoCalcSwitch
{
    ScDocument& mrDoc;
    bool mbOldValue;

    AutoCalcSwitch(AutoCalcSwitch const&) = delete;
    AutoCalcSwitch(AutoCalcSwitch&&) = delete;
    AutoCalcSwitch& operator=(AutoCalcSwitch const&) = delete;
    AutoCalcSwitch& operator=(AutoCalcSwitch&&) = delete;

public:
    AutoCalcSwitch(ScDocument& rDoc, bool bAutoCalc);
    ~AutoCalcSwitch();
};

class ExpandRefsSwitch
{
    ScDocument& mrDoc;
    bool mbOldValue;

public:
    ExpandRefsSwitch(ScDocument& rDoc, bool bExpandRefs);
    ~ExpandRefsSwitch();
};

class SC_DLLPUBLIC UndoSwitch
{
    ScDocument& mrDoc;
    bool mbOldValue;

public:
    UndoSwitch(ScDocument& rDoc, bool bUndo);
    ~UndoSwitch();
};

class IdleSwitch
{
    ScDocument& mrDoc;
    bool mbOldValue;

public:
    IdleSwitch(ScDocument& rDoc, bool bEnableIdle);
    ~IdleSwitch();
};

/// Wrapper for ScDocument::DelayFormulaGrouping()
class DelayFormulaGroupingSwitch
{
    ScDocument& mrDoc;
    bool const mbOldValue;

public:
    DelayFormulaGroupingSwitch(ScDocument& rDoc, bool delay);
    ~DelayFormulaGroupingSwitch() COVERITY_NOEXCEPT_FALSE;
    void reset();
};

/// Wrapper for ScDocument::EnableDelayStartListeningFormulaCells()
class DelayStartListeningFormulaCells
{
    ScColumn& mColumn;
    bool const mbOldValue;

public:
    DelayStartListeningFormulaCells(ScColumn& column, bool delay);
    DelayStartListeningFormulaCells(ScColumn& column);
    DelayStartListeningFormulaCells(const DelayStartListeningFormulaCells&) = delete;
    ~DelayStartListeningFormulaCells();
    void set();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
