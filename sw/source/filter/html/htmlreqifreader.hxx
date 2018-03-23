/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_HTMLREQIFREADER_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_HTMLREQIFREADER_HXX

class SvStream;

namespace SwReqIfReader
{
/// Extracts an OLE2 container binary from an RTF fragment.
bool ExtractOleFromRtf(SvStream& rRtf, SvStream& rOle);
}

#endif // INCLUDED_SW_SOURCE_FILTER_HTML_HTMLREQIFREADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
