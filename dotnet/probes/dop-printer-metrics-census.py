#!/usr/bin/env python3
"""Which corpus documents ask to be laid out against a printer.

Word's "use printer metrics to lay out document" is `WW8Dop::fUsePrinterMetrics` in a
`.doc` and `<w:usePrinterMetrics/>` in a `.docx`, and LibreOffice honours it literally:
`sw/source/filter/ww8/ww8par.cxx`:2008 makes it `!USE_VIRTUAL_DEVICE`, and
`DocumentDeviceManager::getReferenceDevice` then formats against an `SfxPrinter` at
300 dpi instead of the 8640 dpi virtual device. Both the line heights and the advance
widths change — see `printer-metric-advance.py`.

**This counts over both halves of the track and says so**, which is the point. An earlier
version of this census globbed `words/*/*`, which is the *directories*, so it read no
OOXML at all and reported "no DOCX states it" with complete confidence. There is one, and
it is a document that passes its batch.

Two things it is careful about, both of which have silently produced a wrong answer here:

  * **A `.doc` keeps the compatibility dword twice**, at Dop+0x54 and again at Dop+0x1fc,
    and `WW8Dop::WW8Dop` overwrites the first with the second whenever `lcbDop >= 516`
    ("500 -> 508, Appear to be repeated here in 2000+"). Reading only 0x54 is wrong in
    principle; on this corpus the two copies agree on all 66 files, so it is a latent
    defect rather than a miscount. The census reports both and flags any disagreement.
  * **A file's extension is a hint.** Everything is sniffed: OLE2 by signature, OOXML by
    the local-file-header magic.

    dop-printer-metrics-census.py [corpus-root]
"""

import glob
import os
import re
import struct
import sys
import zipfile

import olefile

COMPAT_OFFSETS = (0x54, 0x1FC)
PRINTER_METRICS_BIT = 0x80000000


def ole_flags(path):
    """(printer-metrics, per-offset values) for an OLE2 Word document, else None."""
    if not olefile.isOleFile(path):
        return None
    ole = olefile.OleFileIO(path)
    try:
        if not ole.exists("WordDocument"):
            return None
        fib = ole.openstream("WordDocument").read(0x400)
        if len(fib) < 0x19A:
            return None
        table = "1Table" if struct.unpack_from("<H", fib, 0x0A)[0] & 0x0200 else "0Table"
        if not ole.exists(table):
            return None
        fc_dop, lcb_dop = struct.unpack_from("<ii", fib, 0x192)
        data = ole.openstream(table).read()
        seen = {}
        for off in COMPAT_OFFSETS:
            if lcb_dop < off + 4 or len(data) < fc_dop + off + 4:
                continue
            seen[off] = struct.unpack_from("<I", data, fc_dop + off)[0]
        if not seen:
            return None
        # The late copy is what LibreOffice ends up with when it is present.
        effective = seen[max(seen)]
        return bool(effective & PRINTER_METRICS_BIT), seen, lcb_dop
    finally:
        ole.close()


def ooxml_flag(path):
    """True/False for a zip Word document that does or does not state the element."""
    try:
        with open(path, "rb") as handle:
            if handle.read(2) != b"PK":
                return None
        archive = zipfile.ZipFile(path)
    except Exception:
        return None
    try:
        parts = [n for n in archive.namelist() if n.endswith("settings.xml")]
        if not parts:
            return None
        for name in parts:
            try:
                text = archive.read(name).decode("utf8", "replace")
            except Exception:
                continue
            found = re.search(r"<w:usePrinterMetrics[^/>]*/?>", text)
            if found and 'w:val="0"' not in found.group(0) and 'w:val="false"' not in found.group(0):
                return True
        return False
    finally:
        archive.close()


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "/workspace/sample-files/words"
    files = [p for p in sorted(glob.glob(os.path.join(root, "**", "*"), recursive=True))
             if os.path.isfile(p)]

    ole_total = ooxml_total = other = 0
    hits, disagree = [], []
    for path in files:
        binary = ole_flags(path)
        if binary is not None:
            ole_total += 1
            on, seen, lcb = binary
            if len({v & PRINTER_METRICS_BIT for v in seen.values()}) > 1:
                disagree.append((path, seen, lcb))
            if on:
                hits.append(("doc", path))
            continue
        zipped = ooxml_flag(path)
        if zipped is not None:
            ooxml_total += 1
            if zipped:
                hits.append(("docx", path))
            continue
        other += 1

    print(f"read {ole_total} OLE2 and {ooxml_total} OOXML word documents "
          f"({other} files were neither)")
    print(f"asking for printer metrics: {len(hits)}")
    for kind, path in hits:
        print(f"  {kind:4s} {os.path.relpath(path, root)}")
    if disagree:
        print("\nthe two copies of the compatibility dword DISAGREE on:")
        for path, seen, lcb in disagree:
            print(f"  {os.path.relpath(path, root)} lcbDop={lcb} "
                  + " ".join(f"0x{o:03x}={v:08x}" for o, v in sorted(seen.items())))
    else:
        print("\nthe two copies of the compatibility dword agree on every .doc read")
    return 0


if __name__ == "__main__":
    sys.exit(main())
