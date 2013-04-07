/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _RTFSTRINGBUFFER_HXX_
#define _RTFSTRINGBUFFER_HXX_

#include <rtl/strbuf.hxx>
#include <vector>

class SwGrfNode;
class SwFlyFrmFmt;
class RtfAttributeOutput;

/// Contains a buffered string or graphic during RTF export.
class RtfStringBufferValue
{
public:
    /// Constructor for a string buffering.
    RtfStringBufferValue();
    /// Constructor for graphic buffering.
    RtfStringBufferValue(const SwFlyFrmFmt* pFlyFrmFmt, const SwGrfNode* pGrfNode);
    /// This version handles graphics.
    void makeStringAndClear(RtfAttributeOutput* pAttributeOutput);
    /// This one doesn't.
    OString makeStringAndClear();
    bool isGraphic() const;

    OStringBuffer m_aBuffer;
    const SwFlyFrmFmt* m_pFlyFrmFmt;
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
    void append(const SwFlyFrmFmt* pFlyFrmFmt, const SwGrfNode* pGrfNode);
    /// Append all contained buffers and clear the argument.
    void appendAndClear(RtfStringBuffer& rBuf);
private:
    typedef std::vector<RtfStringBufferValue> Values_t;
    Values_t m_aValues;
};

#endif // _RTFSTRINGBUFFER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
