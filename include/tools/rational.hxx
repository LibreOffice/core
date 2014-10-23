/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_TOOLS_RATIONAL_HXX
#define INCLUDED_TOOLS_RATIONAL_HXX

#include <boost/rational.hpp>
#include <tools/toolsdllapi.h>

class SvStream;

TOOLS_DLLPUBLIC boost::rational<long> rational_FromDouble(double dVal);
TOOLS_DLLPUBLIC void rational_ReduceInaccurate(boost::rational<long>& rRational, unsigned nSignificantBits);
TOOLS_DLLPUBLIC SvStream& ReadFraction(SvStream& rIStream, boost::rational<long>& rRational);
TOOLS_DLLPUBLIC SvStream& WriteFraction(SvStream& rOStream, const boost::rational<long>& rRational);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
