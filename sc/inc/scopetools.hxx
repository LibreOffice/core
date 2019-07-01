/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_SCOPETOOLS_HXX
#define INCLUDED_SC_INC_SCOPETOOLS_HXX

#include "scdllapi.h"

class ScDocument;
namespace vcl { class Window; }

namespace sc {

/**
 * Temporarily switch on/off auto calculation mode.
 */
class SC_DLLPUBLIC AutoCalcSwitch
{
    ScDocument& mrDoc;
    bool const mbOldValue;
public:
    AutoCalcSwitch(ScDocument& rDoc, bool bAutoCalc);
    ~AutoCalcSwitch();
};

class SC_DLLPUBLIC ExpandRefsSwitch
{
    ScDocument& mrDoc;
    bool const mbOldValue;
public:
    ExpandRefsSwitch(ScDocument& rDoc, bool bExpandRefs);
    ~ExpandRefsSwitch();
};

class SC_DLLPUBLIC UndoSwitch
{
    ScDocument& mrDoc;
    bool const mbOldValue;
public:
    UndoSwitch(ScDocument& rDoc, bool bUndo);
    ~UndoSwitch();
};

class SC_DLLPUBLIC IdleSwitch
{
    ScDocument& mrDoc;
    bool const mbOldValue;
public:
    IdleSwitch(ScDocument& rDoc, bool bEnableIdle);
    ~IdleSwitch();
};

/// Wrapper for ScDocument::DelayFormulaGrouping()
class SC_DLLPUBLIC DelayFormulaGroupingSwitch
{
    ScDocument& mrDoc;
    bool const mbOldValue;
public:
    DelayFormulaGroupingSwitch(ScDocument& rDoc, bool delay);
    ~DelayFormulaGroupingSwitch();
    void reset();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
