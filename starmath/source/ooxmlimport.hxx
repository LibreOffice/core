/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SM_OOXMLIMPORT_HXX
#define SM_OOXMLIMPORT_HXX

#include <oox/mathml/importutils.hxx>

#include "node.hxx"

/**
 Class implementing reading of formulas from OOXML. The toplevel element is expected
 to be oMath (handle oMathPara outside of this code).
 */
class SmOoxmlImport
{
public:
    SmOoxmlImport( oox::formulaimport::XmlStream& stream );
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
    oox::formulaimport::XmlStream& stream;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
