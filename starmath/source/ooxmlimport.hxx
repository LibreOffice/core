/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
    rtl::OUString ConvertToStarMath();
private:
    rtl::OUString handleStream();
    rtl::OUString handleAcc();
    rtl::OUString handleBar();
    rtl::OUString handleBox();
    rtl::OUString handleBorderBox();
    rtl::OUString handleD();
    rtl::OUString handleEqArr();
    rtl::OUString handleF();
    rtl::OUString handleFunc();
    enum LimLowUpp_t { LimLow, LimUpp };
    rtl::OUString handleLimLowUpp( LimLowUpp_t limlowupp );
    rtl::OUString handleGroupChr();
    rtl::OUString handleM();
    rtl::OUString handleNary();
    rtl::OUString handleR();
    rtl::OUString handleRad();
    rtl::OUString handleSpre();
    rtl::OUString handleSsub();
    rtl::OUString handleSsubsup();
    rtl::OUString handleSsup();
    rtl::OUString readOMathArg();
    rtl::OUString readOMathArgInElement( int token );
    oox::formulaimport::XmlStream& stream;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
