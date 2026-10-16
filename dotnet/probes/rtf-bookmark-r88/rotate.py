#!/usr/bin/env python3
r"""The writerfilter bookmark-naming rotation, simulated from the C++ it is read out of.

Three functions of the importer, in the order an RTF bookmark half goes through them:

  RTFDocumentImpl::popState, Destination::BOOKMARKSTART / BOOKMARKEND
      (sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx:2735-2764)
      a start takes id = m_aBookmarks.size() and writes m_aBookmarks[name] = id;
      an end takes id = m_aBookmarks[name], which default-constructs 0 for a name
      no start ever used.  Both go through lcl_getBookmarkProperties (:224-236),
      which sets LN_CT_Bookmark_name *before* LN_CT_MarkupRangeBookmark_id --
      "If present, this should be sent first".

  DomainMapper::lcl_attribute (dmapper/DomainMapper.cxx:340-347)
      routes the name to SetBookmarkName and the id to StartOrEndBookmark, in
      that order.

  DomainMapper_Impl::SetBookmarkName / StartOrEndBookmark
      (dmapper/DomainMapper_Impl.cxx:9426-9447, :9450-9543)
      SetBookmarkName writes the name onto the entry for m_sCurrentBkmkId -- the
      *previously opened* start -- and only falls back to m_sCurrentBkmkName when
      that id is not in the map.  OOXML states the id first, so the fallback is
      the OOXML path and the overwrite is the RTF one.

`emit` returns the bookmarks in the order the importer inserts them, each as
(name, opened_at, closed_at) over whatever positions the caller passes in.
"""


class Rotation:
    def __init__(self):
        self.ids = {}        # RTFDocumentImpl::m_aBookmarks -- name -> id
        self.open = {}       # DomainMapper_Impl::m_aBookmarkMap -- id -> [name, position]
        self.cur_id = None   # m_sCurrentBkmkId
        self.cur_name = ""   # m_sCurrentBkmkName
        self.out = []        # what StartOrEndBookmark inserted, in order

    def half(self, name, start, position):
        """One \\bkmkstart or \\bkmkend, at `position`."""
        if start:
            ident = len(self.ids)
            self.ids[name] = ident
        else:
            ident = self.ids.setdefault(name, 0)   # std::map::operator[]

        # SetBookmarkName
        if self.cur_id is not None and self.cur_id in self.open:
            self.open[self.cur_id][0] = name
        else:
            self.cur_name = name

        # StartOrEndBookmark
        if ident in self.open:
            held, opened = self.open.pop(ident)
            self.out.append((held, opened, position))
            self.cur_id = None
        else:
            self.open[ident] = [self.cur_name, position]
            self.cur_id = ident
            self.cur_name = ""

    def names(self):
        """The names as Writer ends up holding them, duplicates renamed as it renames them."""
        seen, named = {}, []
        for name, a, b in self.out:
            if name in seen:
                seen[name] += 1
                named.append((f"{name} Copy {seen[name]}", a, b))
            else:
                seen[name] = 0
                named.append((name, a, b))
        return named
