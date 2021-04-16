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

class Graphic;
class Size;
class SvStream;
class SwOLENode;
class SwFrameFormat;

namespace SwReqIfReader
{
/**
 * Extracts an OLE2 container binary from an RTF fragment.
 *
 * @param bOwnFormat if the extracted data has an ODF class ID or not.
 */
bool ExtractOleFromRtf(SvStream& rRtf, SvStream& rOle, bool& bOwnFormat);

/// Wraps an OLE2 container binary in an RTF fragment.
bool WrapOleInRtf(SvStream& rOle2, SvStream& rRtf, SwOLENode& rOLENode,
                  const SwFrameFormat& rFormat);

/**
 * Wraps an image in an RTF fragment.
 *
 * @param rLogicSize the size used in the document model (not pixel size)
 */
bool WrapGraphicInRtf(const Graphic& rGraphic, const Size& rLogicSize, SvStream& rRtf);
}

#endif // INCLUDED_SW_SOURCE_FILTER_HTML_HTMLREQIFREADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
