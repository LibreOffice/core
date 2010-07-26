/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// Description: Classes to cache Graphite Segments to try to improve
//              rendering performance.

#ifndef GraphiteSegmentCache_h
#define GraphiteSegmentCache_h

#include <tools/solar.h>
#include <rtl/ustring.h>

#define GRCACHE_REUSE_VECTORS 1

//#include <rope>
#include <hash_map>

class TextSourceAdaptor;
/**
* GrSegRecord stores a Graphite Segment and its associated text
*/
class GrSegRecord {
public:
    GrSegRecord(rtl::OUString * rope, TextSourceAdaptor * textSrc, gr::Segment * seg, bool bIsRtl);

    ~GrSegRecord();

    void reuse(rtl::OUString * rope, TextSourceAdaptor * textSrc, gr::Segment * seg, bool bIsRtl);

    void clearVectors();
    void clear();
#ifdef GRCACHE_REUSE_VECTORS
    void setGlyphVectors(long nWidth, GraphiteLayout::Glyphs & vGlyphs, std::vector<int> vCharDxs,
                         std::vector<int> & vChar2Base, std::vector<int> & vGlyph2Char, float fScale)
    {
        clearVectors();
        mnWidth = nWidth;
        m_fontScale = fScale;
        mvGlyphs.insert(mvGlyphs.begin(), vGlyphs.begin(), vGlyphs.end());
        mvCharDxs.insert(mvCharDxs.begin(),vCharDxs.begin(),vCharDxs.end());
        mvChar2BaseGlyph.insert(mvChar2BaseGlyph.begin(),vChar2Base.begin(),vChar2Base.end());
        mvGlyph2Char.insert(mvGlyph2Char.begin(),vGlyph2Char.begin(),vGlyph2Char.end());
    }
#endif
    gr::Segment * getSegment() { return m_seg; }
    TextSourceAdaptor * getTextSrc() { return m_text; }
    void unlock() { --m_lockCount; }
    bool isRtl() const { return mbIsRtl; }
#ifdef GRCACHE_REUSE_VECTORS
    const long & width() const { return mnWidth; }
    const GraphiteLayout::Glyphs & glyphs() const { return mvGlyphs; }
    const std::vector<int> & charDxs() const { return mvCharDxs; }
    const std::vector<int> & char2BaseGlyph() const { return mvChar2BaseGlyph; }
    const std::vector<int> & glyph2Char() const { return mvGlyph2Char; }
    float & fontScale() { return m_fontScale; }
#endif
private:
    rtl::OUString * m_rope;
    TextSourceAdaptor * m_text;
    gr::Segment * m_seg;
    const xub_Unicode * m_nextKey;
    const xub_Unicode*  m_pStr;
    size_t m_startChar;
    float m_fontScale;
    long mnWidth;
    GraphiteLayout::Glyphs mvGlyphs; // glyphs in display order
    std::vector<int> mvCharDxs; // right hand side x offset of each glyph
    std::vector<int> mvChar2BaseGlyph;
    std::vector<int> mvGlyph2Char;
    bool mbIsRtl;
    int m_lockCount;
    friend class GraphiteSegmentCache;
};

typedef std::hash_map<long, GrSegRecord*, std::hash<long> > GraphiteSegMap;
typedef std::hash_multimap<size_t, GrSegRecord*> GraphiteRopeMap;
typedef std::pair<GraphiteRopeMap::iterator, GraphiteRopeMap::iterator> GrRMEntry;

/**
* GraphiteSegmentCache contains the cached Segments for one particular font size
*/
class GraphiteSegmentCache
{
public:
  enum {
    // not really sure what good values are here,
    // bucket size should be >> cache size
    SEG_BUCKET_FACTOR = 4,
    SEG_DEFAULT_CACHE_SIZE = 2047
  };
  GraphiteSegmentCache(sal_uInt32 nSegCacheSize)
    : m_segMap(nSegCacheSize * SEG_BUCKET_FACTOR),
    m_nSegCacheSize(nSegCacheSize),
    m_oldestKey(NULL) {};
  ~GraphiteSegmentCache()
  {
    m_ropeMap.clear();
    GraphiteSegMap::iterator i = m_segMap.begin();
    while (i != m_segMap.end())
    {
      GrSegRecord *r = i->second;
      delete r;
      ++i;
    }
    m_segMap.clear();
  };
  GrSegRecord * getSegment(ImplLayoutArgs & layoutArgs, bool bIsRtl, int segCharLimit)
  {
    GrSegRecord * found = NULL;
    // try to find a segment starting at correct place, if not, try to find a
    //  match for the complete buffer
    GraphiteSegMap::iterator iMap =
      m_segMap.find(reinterpret_cast<long>(layoutArgs.mpStr +
                                           layoutArgs.mnMinCharPos));
    if (iMap != m_segMap.end())
    {
      found = iMap->second;
    }
    else
    {
      iMap = m_segMap.find(reinterpret_cast<long>(layoutArgs.mpStr));
      if (iMap != m_segMap.end())
      {
        found = iMap->second;
      }
    }
    if (found)
    {
      if (found->m_seg->startCharacter() <= layoutArgs.mnMinCharPos &&
          found->m_seg->stopCharacter() >= layoutArgs.mnEndCharPos)
      {
        DBG_ASSERT(found && found->m_seg, "null entry in GraphiteSegmentCache");
        // restore original start character, in case it has changed
        found->m_seg->setTextSourceOffset(found->m_startChar);
        // check that characters are the same, at least in the range of
        // interest
        // We could use substr and ==, but substr does a copy,
        // so its probably faster to do it like this
        for (int i = layoutArgs.mnMinCharPos; i < segCharLimit; i++)
        {
          //if (!found->m_rope->match(rtl::OUString(layoutArgs.mpStr[i], layoutArgs.mnLength), i - found->m_seg->startCharacter()))
          if (found->m_rope->getStr()[i-found->m_seg->startCharacter()] != layoutArgs.mpStr[i])
            return NULL;
        }
        if (found->isRtl() != bIsRtl)
        {
            return NULL;
        }
        if (found->m_seg->stopCharacter() > layoutArgs.mnEndCharPos &&
            static_cast<int>(found->char2BaseGlyph().size()) > layoutArgs.mnEndCharPos)
        {
            // check that the requested end character isn't mid cluster
            if (found->char2BaseGlyph()[layoutArgs.mnEndCharPos-layoutArgs.mnMinCharPos] == -1)
            {
                return NULL;
            }
        }
//        if (found->m_lockCount != 0)
//          OutputDebugString("Multple users of SegRecord!");
        found->m_lockCount++;
      }
      else found = NULL;
    }
    else
    {
      // the pointers aren't the same, but we might still have the same text in a segment
      // this is expecially needed when editing a large paragraph
      // each edit changes the pointers, but if we don't reuse any segments it gets very
      // slow.
      rtl::OUString * rope = new rtl::OUString(layoutArgs.mpStr + layoutArgs.mnMinCharPos,
                                         segCharLimit - layoutArgs.mnMinCharPos);
      if (!rope) return NULL;
      size_t nHash = (*(rope)).hashCode();
      GrRMEntry range = m_ropeMap.equal_range(nHash);
      while (range.first != range.second)
      {
        found = range.first->second;
        if (found->m_lockCount == 0)
        {
          if(rope->match(*(found->m_rope)))
          {
            // found, but the pointers are all wrong
            found->m_seg->setTextSourceOffset(layoutArgs.mnMinCharPos);
            // the switch is done in graphite_layout.cxx
            //found->m_text->switchLayoutArgs(layoutArgs);
            found->m_lockCount++;
            break;
          }
          else
            found = NULL;
        }
        else
          found = NULL;
        ++(range.first);
      }
      delete rope;
    }
    return found;
  };
  GrSegRecord * cacheSegment(TextSourceAdaptor * adapter, gr::Segment * seg, bool bIsRtl);
private:
  GraphiteSegMap m_segMap;
  GraphiteRopeMap m_ropeMap;
  sal_uInt32 m_nSegCacheSize;
  const xub_Unicode * m_oldestKey;
  const xub_Unicode * m_prevKey;
};

typedef std::hash_map<int, GraphiteSegmentCache *, std::hash<int> > GraphiteCacheMap;

/**
* GraphiteCacheHandler maps a particular font, style, size to a GraphiteSegmentCache
*/
class GraphiteCacheHandler
{
public:
  GraphiteCacheHandler() : m_cacheMap(255)
  {
    const char * pEnvCache = getenv( "SAL_GRAPHITE_CACHE_SIZE" );
    if (pEnvCache != NULL)
    {
        int envCacheSize = atoi(pEnvCache);
        if (envCacheSize <= 0)
            m_nSegCacheSize = GraphiteSegmentCache::SEG_DEFAULT_CACHE_SIZE;
        else
        {
            m_nSegCacheSize = envCacheSize;
        }
    }
    else
    {
        m_nSegCacheSize = GraphiteSegmentCache::SEG_DEFAULT_CACHE_SIZE;
    }
  };
  ~GraphiteCacheHandler()
  {
    GraphiteCacheMap::iterator i = m_cacheMap.begin();
    while (i != m_cacheMap.end())
    {
      GraphiteSegmentCache *r = i->second;
      delete r;
      ++i;
    }
    m_cacheMap.clear();
  };

  static GraphiteCacheHandler instance;

  GraphiteSegmentCache * getCache(sal_Int32 & fontHash)
  {
    if (m_cacheMap.count(fontHash) > 0)
    {
      return m_cacheMap.find(fontHash)->second;
    }
    GraphiteSegmentCache *pCache = new GraphiteSegmentCache(m_nSegCacheSize);
    m_cacheMap[fontHash] = pCache;
    return pCache;
  }
private:
  GraphiteCacheMap m_cacheMap;
  sal_uInt32 m_nSegCacheSize;
};

#endif

