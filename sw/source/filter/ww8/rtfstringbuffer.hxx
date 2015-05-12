/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_RTFSTRINGBUFFER_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_RTFSTRINGBUFFER_HXX

#include <rtl/strbuf.hxx>
#include <vector>

class SwGrfNode;
class SwFlyFrameFormat;
class RtfAttributeOutput;

/// Contains a buffered string or graphic during RTF export.
class RtfStringBufferValue
{
public:
    /// Constructor for a string buffering.
    RtfStringBufferValue();
    /// Constructor for graphic buffering.
    RtfStringBufferValue(const SwFlyFrameFormat* pFlyFrameFormat, const SwGrfNode* pGrfNode);
    /// This version handles graphics.
    void makeStringAndClear(RtfAttributeOutput* pAttributeOutput);
    /// This one doesn't.
    OString makeStringAndClear();
    bool isGraphic() const;

    OStringBuffer m_aBuffer;
    const SwFlyFrameFormat* m_pFlyFrameFormat;
    const SwGrfNode* m_pGrfNode;
};

/// Wrapper around OStringBuffers, so less hexdump of graphics have to be kept in memory during RTF export.
class RtfStringBuffer
{
public:
    RtfStringBuffer();
    /// Length of all the contained buffers.
    sal_Int32 getLength() const;
    /// Writes the contents of the buffer directly to the supplied stream.
    void makeStringAndClear(RtfAttributeOutput* pAttributeOutput);
    /// Returns the bufferent strings as a string (ignores graphic elements!)
    OString makeStringAndClear();
    /// Access to the last buffer.
    OStringBuffer& getLastBuffer();
    OStringBuffer* operator->();
    /// Similar to ->setLength(0), but for all buffers.
    void clear();
    /// Same as ->append(), but for graphics and without expanding contents to save memory.
    void append(const SwFlyFrameFormat* pFlyFrameFormat, const SwGrfNode* pGrfNode);
    /// Append all contained buffers and clear the argument.
    void appendAndClear(RtfStringBuffer& rBuf);
private:
    typedef std::vector<RtfStringBufferValue> Values_t;
    Values_t m_aValues;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFSTRINGBUFFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
