#!/usr/bin/env python3
"""Tie each Escher OPT table to the TextHeaderAtom kind of the text it carries.

Round 51 censused the OPT tables alone and found 36 of 51 .ppt documents holding at least
one wrapNone-and-not-fitShapeToText text shape -- and said explicitly that 36 was an UPPER
BOUND, because PptSlideLayout.Autofits additionally requires the text kind to be
Body/HalfBody/QuarterBody.  This resolves that.

It also records the discriminator svdfppt.cxx actually uses, which the r51 census did not
model at all:

    line  846: bDeleteSource = aTextObj.GetOEPlaceHolderAtom().has_value();
               ... if so, pRet = nullptr   (the imported shape is thrown away)
    line 1041: if destination != TextInShape and (no placeholder atom or id == NONE)
               ... eTextKind = SdrObjKind::Rectangle
    line 1052: if (pRet is SdrObjCustomShape && eTextKind == Rectangle)
                    bAutoGrowWidth = !bWordWrap        <-- the wrap term
               else
                    bAutoGrowWidth = false             <-- wrap is inert
    line 1097: if (bAutoFit && !bAutoGrowHeight && !bAutoGrowWidth) -> AUTOFIT

So the wrap term can only fire on a shape that carries NO OEPlaceholderAtom.  On a real
placeholder LibreOffice shrinks the text whatever the wrap says, and we do not.

Escher and PPT records share one 8-byte header: <verInstance:u16, type:u16, length:u32>,
and a low nibble of 0xF means "container", so one walker reads both.
"""
import collections, glob, os, struct, sys
import olefile

OPT            = 0xF00B
TERTIARY_OPT   = 0xF121
SP_CONTAINER   = 0xF004
SPGR_CONTAINER = 0xF003
SP             = 0xF00A
CLIENT_TEXTBOX = 0xF00D
CLIENT_DATA    = 0xF011

TEXT_HEADER_ATOM      = 3999
OUTLINE_TEXT_REF_ATOM = 3998
PLACEHOLDER_ATOM      = 3011
SLIDE                 = 1006
MAIN_MASTER           = 1016
NOTES                 = 1008

WRAP_TEXT          = 133
FIT_TEXT_TO_SHAPE  = 191
WRAP_NONE          = 2
FIT_SHAPE_TO_TEXT  = 2

BODY_KINDS = {1, 7, 8}      # Body, HalfBody, QuarterBody -- PptTextKind / TSS_Type


class Rec:
    __slots__ = ("ver", "inst", "type", "start", "end", "children", "data")

    def __init__(self, ver, inst, rtype, start, end, data):
        self.ver, self.inst, self.type = ver, inst, rtype
        self.start, self.end, self.data = start, end, data
        self.children = []

    def content(self):
        return self.data[self.start:self.end]

    def find(self, rtype):
        return [c for c in self.children if c.type == rtype]

    def descend(self, rtype, out=None):
        out = [] if out is None else out
        for c in self.children:
            if c.type == rtype:
                out.append(c)
            c.descend(rtype, out)
        return out


def parse(data, start, end, depth=0):
    """Records between [start, end), recursively."""
    out = []
    pos = start
    while pos + 8 <= end:
        ver_inst, rtype, size = struct.unpack_from("<HHI", data, pos)
        body = pos + 8
        stop = body + size
        if stop > end:
            break
        rec = Rec(ver_inst & 0x0F, ver_inst >> 4, rtype, body, stop, data)
        if rec.ver == 0x0F and depth < 24:
            rec.children = parse(data, body, stop, depth + 1)
        out.append(rec)
        pos = stop
    return out


def props(rec):
    """The msofbtOPT property table as {pid: value}."""
    body, out, off = rec.content(), {}, 0
    for _ in range(rec.inst):
        if off + 6 > len(body):
            break
        pid, value = struct.unpack_from("<HI", body, off)
        off += 6
        out[pid & 0x3FFF] = value
    return out


def shape_kind(sp, page_headers):
    """The TextHeaderAtom instance of the text this shape carries, or None."""
    for box in sp.find(CLIENT_TEXTBOX):
        for r in box.children or parse(sp.data, box.start, box.end):
            if r.type == TEXT_HEADER_ATOM:
                # The kind is the atom's 4-byte CONTENT, not its recInstance.
                c = sp.data[r.start:r.end]
                return struct.unpack_from("<I", c, 0)[0] if len(c) >= 4 else 4
            if r.type == OUTLINE_TEXT_REF_ATOM:
                content = sp.data[r.start:r.end]
                if len(content) >= 4:
                    ref = struct.unpack_from("<I", content, 0)[0]
                    if ref < len(page_headers):
                        return page_headers[ref]
                return None
    return None


def has_placeholder(sp):
    for cd in sp.find(CLIENT_DATA):
        for r in (cd.children or parse(sp.data, cd.start, cd.end)):
            if r.type == PLACEHOLDER_ATOM:
                return True
    return False


def page_text_kinds(page):
    """The page's own text list, in order -- what OutlineTextRefAtom indexes into."""
    kinds = []
    for r in page.children:
        if r.type == TEXT_HEADER_ATOM:
            c = page.data[r.start:r.end]
            kinds.append(struct.unpack_from("<I", c, 0)[0] if len(c) >= 4 else 4)
    return kinds


def census(path):
    ole = olefile.OleFileIO(path)
    if not ole.exists("PowerPoint Document"):
        ole.close()
        return None
    data = ole.openstream("PowerPoint Document").read()
    ole.close()

    top = parse(data, 0, len(data))
    counters = collections.Counter()
    detail = []

    pages = []
    def collect(recs):
        for r in recs:
            if r.type in (SLIDE, MAIN_MASTER, NOTES):
                pages.append(r)
            collect(r.children)
    collect(top)

    for page in pages:
        headers = page_text_kinds(page)
        for sp in page.descend(SP_CONTAINER):
            opts = {}
            for o in sp.find(OPT) + sp.find(TERTIARY_OPT):
                opts.update(props(o))
            kind = shape_kind(sp, headers)
            if kind is None:
                continue
            counters["textshapes"] += 1
            wrap_none = opts.get(WRAP_TEXT, 0) == WRAP_NONE
            fit_shape = (opts.get(FIT_TEXT_TO_SHAPE, 0) & FIT_SHAPE_TO_TEXT) != 0
            ph = has_placeholder(sp)
            if kind not in BODY_KINDS:
                counters["nonbody"] += 1
                continue
            counters["body"] += 1
            if fit_shape:
                counters["body_fitshape"] += 1
                continue
            if not wrap_none:
                counters["body_wraps"] += 1
                continue
            # wrapNone, not fitShapeToText, body kind: we suppress autofit here.
            counters["body_wrapnone"] += 1
            if ph:
                counters["body_wrapnone_placeholder"] += 1   # <-- the defect
            else:
                counters["body_wrapnone_plain"] += 1
            detail.append((kind, ph))
    return counters, detail


rows = []
for path in sorted(glob.glob("/c/sandbox/workdir/sample-files/slides/*/ppt/*.ppt")):
    try:
        got = census(path)
    except Exception as exc:
        print("SKIP", os.path.basename(path), exc, file=sys.stderr)
        continue
    if got is None:
        continue
    counters, _ = got
    rows.append((os.path.basename(path), counters))

hdr = ("document", "textsh", "body", "fitsh", "wraps", "wrapNone", "  ->ph", "->plain")
print(f"{hdr[0]:58} {hdr[1]:>6} {hdr[2]:>5} {hdr[3]:>5} {hdr[4]:>5} {hdr[5]:>8} {hdr[6]:>6} {hdr[7]:>7}")
print("-" * 104)
for name, c in sorted(rows, key=lambda r: -r[1]["body_wrapnone_placeholder"]):
    print(f"{name[:57]:58} {c['textshapes']:6d} {c['body']:5d} {c['body_fitshape']:5d} "
          f"{c['body_wraps']:5d} {c['body_wrapnone']:8d} {c['body_wrapnone_placeholder']:6d} "
          f"{c['body_wrapnone_plain']:7d}")

tot = collections.Counter()
for _, c in rows:
    tot.update(c)
print()
print(f"documents                                              : {len(rows)}")
print(f"text-bearing shapes                                    : {tot['textshapes']}")
print(f"  of Body/HalfBody/QuarterBody kind                    : {tot['body']}")
print(f"    fFitShapeToText (autofit suppressed, both agree)    : {tot['body_fitshape']}")
print(f"    wrapping        (autofit applied, both agree)       : {tot['body_wraps']}")
print(f"    wrapNone        (WE suppress autofit)               : {tot['body_wrapnone']}")
print(f"      with an OEPlaceholderAtom -> LO STILL AUTOFITS    : {tot['body_wrapnone_placeholder']}")
print(f"      without one               -> LO agrees with us    : {tot['body_wrapnone_plain']}")
print()
print(f"documents with >=1 body wrapNone shape                  : "
      f"{sum(1 for _, c in rows if c['body_wrapnone'])}")
print(f"documents with >=1 body wrapNone PLACEHOLDER shape      : "
      f"{sum(1 for _, c in rows if c['body_wrapnone_placeholder'])}")
