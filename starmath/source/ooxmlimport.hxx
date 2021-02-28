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

namespace oox::formulaimport { class XmlStream; }
/**
 Class implementing reading of formulas from OOXML. The toplevel element is expected
 to be oMath (handle oMathPara outside of this code).
 */
class SmOoxmlImport
{
public:
    explicit SmOoxmlImport( oox::formulaimport::XmlStream& stream );
    OUString ConvertToStarMath();
private:
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
    OUString readOMathArg( int stoptoken );
    OUString readOMathArgInElement( int token );

    oox::formulaimport::XmlStream& m_rStream;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
