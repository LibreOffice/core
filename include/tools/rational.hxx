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

class SvStream;

boost::rational<long> rational_FromDouble(double dVal);
void rational_ReduceInaccurate(boost::rational<long>& rRational, unsigned nSignificantBits);
SvStream& ReadFraction(SvStream& rIStream, boost::rational<long>& rRational);
SvStream& WriteFraction(SvStream& rOStream, const boost::rational<long>& rRational);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
