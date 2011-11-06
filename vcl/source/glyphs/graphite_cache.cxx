/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifdef WNT
#include <tools/svwin.h>
#include <svsys.h>
#endif

#include <tools/debug.hxx>
#include <sallayout.hxx>

#include <preextstl.h>
#include <graphite/GrClient.h>
#include <graphite/Segment.h>
#include <postextstl.h>

#include <rtl/ustring.hxx>
#include <graphite_layout.hxx>
#include <graphite_cache.hxx>

#include "graphite_textsrc.hxx"

GrSegRecord::GrSegRecord(rtl::OUString * rope, TextSourceAdaptor * textSrc, gr::Segment * seg, bool bIsRtl)
    : m_rope(rope), m_text(textSrc), m_seg(seg), m_nextKey(NULL),
    m_fontScale(0.0f), mbIsRtl(bIsRtl), m_lockCount(0)
{
    m_pStr = textSrc->getLayoutArgs().mpStr + seg->startCharacter();
    m_startChar = seg->startCharacter();
}

GrSegRecord::~GrSegRecord()
{
    clear();
}

void GrSegRecord::reuse(rtl::OUString * rope, TextSourceAdaptor * textSrc, gr::Segment * seg, bool bIsRtl)
{
    clear();
    mnWidth = 0;
    m_rope = rope;
    m_text = textSrc;
    m_seg = seg;
    m_nextKey = NULL;
    m_pStr = textSrc->getLayoutArgs().mpStr + seg->startCharacter();
    m_startChar = seg->startCharacter();
    mbIsRtl = bIsRtl;
}

void GrSegRecord::clearVectors()
{
    mvGlyphs.clear();
    mvCharDxs.clear();
    mvChar2BaseGlyph.clear();
    mvGlyph2Char.clear();
}

void GrSegRecord::clear()
{
#ifdef GR_DEBUG_TEXT
    if (m_lockCount != 0)
      OutputDebugString("GrSegRecord locked!");
#endif
    clearVectors();
    delete m_rope;
    delete m_seg;
    delete m_text;
    m_rope = NULL;
    m_seg = NULL;
    m_text = NULL;
    m_fontScale = 0.0f;
    m_lockCount = 0;
}

GrSegRecord * GraphiteSegmentCache::cacheSegment(TextSourceAdaptor * adapter, gr::Segment * seg, bool bIsRtl)
{
    GrSegRecord * record = NULL;
    // We keep a record of the oldest key and the last key added
    // when the next key is added, the record for the prevKey's m_nextKey field
    // is updated to the newest key so that m_oldestKey can be updated to the
    // next oldest key when the record for m_oldestKey is deleted
    if (m_segMap.size() > m_nSegCacheSize)
    {
      GraphiteSegMap::iterator oldestPair = m_segMap.find(reinterpret_cast<long>(m_oldestKey));
      // oldest record may no longer exist if a buffer was changed
      if (oldestPair != m_segMap.end())
      {
        record = oldestPair->second;
        m_segMap.erase(reinterpret_cast<long>(m_oldestKey));
        GrRMEntry range = m_ropeMap.equal_range((*(record->m_rope)).hashCode());
        while (range.first != range.second)
        {
          if (range.first->second == record)
          {
            m_ropeMap.erase(range.first);
            break;
          }
          ++range.first;
        }
        m_oldestKey = record->m_nextKey;
        // record will be reused, so don't delete
      }
    }


//    const int seg_char_limit = min(adapter->maLayoutArgs().mnLength,
//      adapter->maLayoutArgs().mnEndCharPos
//      + GraphiteLayout::EXTRA_CONTEXT_LENGTH);
//    if (seg->stopCharacter() - seg->startCharacter() <= 0)
//      OutputDebugString("Invalid seg indices\n");
    rtl::OUString * pRope = new rtl::OUString(adapter->getLayoutArgs().mpStr + seg->startCharacter(),
       seg->stopCharacter() - seg->startCharacter());
    if (!pRope) return NULL;
    bool reuse = false;
    if (record)
      record->reuse(pRope, adapter, seg, bIsRtl);
    else
      record = new GrSegRecord(pRope, adapter, seg, bIsRtl);
    if (!record)
    {
      delete pRope;
      return NULL;
    }
    GraphiteSegMap::iterator iMap =
      m_segMap.find(reinterpret_cast<long>(record->m_pStr));
    if (iMap != m_segMap.end())
    {
      // the buffer has changed, so the old cached Segment is useless
      reuse = true;
      GrSegRecord * found = iMap->second;
      // Note: we reuse the old next key to avoid breaking our history
      // chain. This means it will be prematurely deleted, but this is
      // unlikely to happen very often.
      record->m_nextKey = found->m_nextKey;
      // overwrite the old record
      m_segMap[reinterpret_cast<long>(record->m_pStr)] = record;
      // erase the old rope key and save the new one
      GrRMEntry range = m_ropeMap.equal_range((*(found->m_rope)).hashCode());
      while (range.first != range.second)
      {
        if (range.first->second == found)
        {
          m_ropeMap.erase(range.first);
          break;
        }
        ++range.first;
      }
      GraphiteRopeMap::value_type mapEntry(record->m_rope->hashCode(), record);
      m_ropeMap.insert(mapEntry);
      // remove the old record
      delete found;
      record->m_lockCount++;
      return record;
    }
    m_segMap[reinterpret_cast<long>(record->m_pStr)] = record;
    GraphiteRopeMap::value_type mapEntry((*(record->m_rope)).hashCode(), record);
    m_ropeMap.insert(mapEntry);

    if (m_oldestKey == NULL)
    {
      m_oldestKey = record->m_pStr;
      m_prevKey = record->m_pStr;
    }
    else if (reuse == false)
    {
      DBG_ASSERT(m_segMap.count(reinterpret_cast<long>(m_prevKey)),
        "Previous key got lost somehow!");
      m_segMap.find(reinterpret_cast<long>(m_prevKey))
        ->second->m_nextKey = record->m_pStr;
      m_prevKey = record->m_pStr;
    }
    record->m_lockCount++;
    return record;
}
