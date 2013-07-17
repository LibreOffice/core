/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_SCOPETOOLS_HXX
#define SC_SCOPETOOLS_HXX

class ScDocument;

namespace sc {

/**
 * Temporarily switch on/off auto calculation mode.
 */
class AutoCalcSwitch
{
    ScDocument& mrDoc;
    bool mbOldValue;
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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
