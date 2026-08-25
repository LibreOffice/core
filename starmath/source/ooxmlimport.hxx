/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

#include <memory>

class SmParser5;
namespace oox::formulaimport { class XmlStream; }
/**
 Class implementing reading of formulas from OOXML. The toplevel element is expected
 to be oMath (handle oMathPara outside of this code).
 */
class SmOoxmlImport
{
public:
    explicit SmOoxmlImport( oox::formulaimport::XmlStream& stream );
    ~SmOoxmlImport();
    OUString ConvertToStarMath();
private:
    /// The parser that tells whether a run's text stands as an expression of its own.
    SmParser5& getParser();
    OUString handleStream();
    OUString handleAcc();
    OUString handleBar();
    OUString handleBox();
    OUString handleBorderBox();
    OUString handleD();
    OUString handleEqArr();
    OUString handleF();
    OUString handleFunc();
    enum LimLowUpp_t { LimLow, LimUpp };
    OUString handleLimLowUpp( LimLowUpp_t limlowupp );
    OUString handleGroupChr();
    OUString handleM();
    OUString handleNary();
    OUString handleR();
    OUString handleRad();
    OUString handleSpre();
    OUString handleSsub();
    OUString handleSsubsup();
    OUString handleSsup();
    OUString readCtrlPrColorCommand();
    OUString readOMathArg( int stoptoken );
    OUString readOMathArgInElement( int token );
    static OUString handleSetString(const OUString& setOUstring);

    oox::formulaimport::XmlStream& m_rStream;
    // The color command that applies to the part of the formula being read now, or
    // an empty string for the default color.
    OUString m_sColorCommandInEffect;
    // Built on first use, because constructing one creates a character classification
    // service, and most formulas never need it.
    std::unique_ptr<SmParser5> m_pParser;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
