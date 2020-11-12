/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IPICT_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IPICT_HXX

#include <sal/config.h>

class GDIMetaFile;
class SvStream;

namespace pict
{
/// Function to access PictReader::ReadPict for unit testing.
void ReadPictFile(SvStream& rStreamPict, GDIMetaFile& rGDIMetaFile);
}

#endif // INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IPICT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
