#!/usr/bin/env python3
"""Find the corpus pages where an embedded metafile puts the word gate out of reach.

    metafile-pages.py <corpus-root> <outdir> [workers]

LibreOffice sometimes *rasterises* an embedded EMF or WMF instead of playing it, so its PDF
holds a picture where ours holds real, searchable glyph runs. The rendered pages look alike;
the extracted word counts do not, and ours is the better output. Chasing those numbers means
making the output worse, so they need flagging rather than fixing.

The scan is in two stages, and the second is the one that matters.

  1. Which documents embed a metafile at all. Cheap and exact: package entries ending .emf or
     .wmf, plus the EMF header signature and the WMF placeable magic inside embedded OLE .bin
     parts and inside whole binary documents.

  2. Which *pages* actually show the effect. A document embedding a metafile is not evidence
     of anything by itself -- most play correctly. The signature is a page where the reference
     carries a raster image and we extract materially more words than the reference does.

Stage 2 needs both renderings, so it is slow. Stage 1 alone is `--documents-only`.

Output is `metafile-pages.tsv`, one row per flagged page, plus a summary on stdout.
"""

import os
import re
import subprocess
import sys
import zipfile
import zlib
from concurrent.futures import ThreadPoolExecutor

EMF_SIGNATURE = b" EMF"          # at offset 40 of an EMF header record
WMF_PLACEABLE = b"\xd7\xcd\xc6\x9a"

# A binary DOC/PPT/XLS keeps its pictures in Escher blip records, and the metafile ones are
# *deflated* -- so a raw signature search finds nothing at all in a .ppt that plainly contains
# an EMF. Measured: three documents already established as the rasterisation ceiling scanned as
# carrying no metafile whatsoever until this was added.
#
# msofbtBlipEMF/WMF/PICT are record types 0xF01A/0xF019/0xF01B; each holds a header followed by
# a zlib stream. Rather than parse Escher, inflate every plausible zlib stream in the file and
# look inside -- decisive, and blind to how the record around it is laid out.
ZLIB_HEADERS = re.compile(rb"\x78[\x01\x5e\x9c\xda]")
MAX_ZLIB_PROBES = 4000

# A page is flagged when the reference draws a raster there and we extract at least this many
# more words than it does. Two-thirds of a line of prose: below that the difference is
# hyphenation and soft breaks rather than a rasterised metafile.
MIN_EXTRA_WORDS = 8
MIN_EXTRA_RATIO = 0.25

EXTENSIONS = (
    ".doc", ".docx", ".rtf", ".odt", ".ott",
    ".xls", ".xlsx", ".ods", ".csv",
    ".ppt", ".pptx", ".odp", ".otp",
)


def deflated_metafiles(blob):
    """(emf, wmf) found inside deflated streams — how a binary document stores them."""
    emf = wmf = 0
    seen = 0
    for match in ZLIB_HEADERS.finditer(blob):
        seen += 1
        if seen > MAX_ZLIB_PROBES:
            break
        try:
            # decompressobj rather than zlib.decompress: the stream runs to the end of its
            # Escher record and the bytes after it are not ours to explain.
            inflated = zlib.decompressobj().decompress(blob[match.start():], 1 << 22)
        except Exception:
            continue
        if len(inflated) < 64:
            continue
        emf += inflated.count(EMF_SIGNATURE)
        wmf += inflated.count(WMF_PLACEABLE)
    return emf, wmf


def metafiles_in(path):
    """(emf, wmf) counts for one document, however it stores them."""
    emf = wmf = 0
    try:
        if zipfile.is_zipfile(path):
            package = zipfile.ZipFile(path)
            for entry in package.namelist():
                lowered = entry.lower()
                if lowered.endswith(".emf"):
                    emf += 1
                elif lowered.endswith(".wmf"):
                    wmf += 1
                elif lowered.endswith(".bin"):
                    # An embedded OLE object carries its presentation as a metafile inside
                    # the .bin, where an extension scan cannot see it.
                    try:
                        blob = package.read(entry)
                        emf += blob.count(EMF_SIGNATURE)
                        wmf += blob.count(WMF_PLACEABLE)
                    except Exception:
                        pass
        else:
            with open(path, "rb") as handle:
                blob = handle.read()
            emf = blob.count(EMF_SIGNATURE)
            wmf = blob.count(WMF_PLACEABLE)
            deflated_emf, deflated_wmf = deflated_metafiles(blob)
            emf += deflated_emf
            wmf += deflated_wmf
    except Exception:
        return (0, 0)
    return (emf, wmf)


def corpus_documents(root):
    for track in ("words", "slides", "sheets"):
        base = os.path.join(root, track)
        for directory, _, names in os.walk(base):
            for name in sorted(names):
                if name.lower().endswith(EXTENSIONS):
                    yield os.path.join(directory, name)


def run(command, timeout=240):
    try:
        done = subprocess.run(
            command, capture_output=True, timeout=timeout, check=False)
        return done.stdout.decode("utf8", "replace")
    except Exception:
        return ""


def rasters_by_page(pdf):
    """{page: {(width, height), ...}} for every raster the PDF draws."""
    listing = run(["pdfimages", "-list", pdf])
    pages = {}
    for line in listing.splitlines()[2:]:
        fields = line.split()
        # page num type width height colour comp bpc enc ...
        if len(fields) > 4 and fields[0].isdigit() and fields[3].isdigit():
            pages.setdefault(int(fields[0]), set()).add((int(fields[3]), int(fields[4])))
    return pages


def unmatched_rasters(ours, reference):
    """{page: rasters the reference draws there and we do not}.

    The flag's first condition used to be "the reference draws a raster here", and that is
    met just as well by a logo *both* sides draw. Measured: four pages of one document were
    flagged on a 162x109 JPEG of the EU flag in its footer, drawn identically by both
    renderers, while the real surplus was a header block on 13 of its 18 pages. The
    signature misfires on any document that puts a small picture in its page furniture and
    has a furniture defect elsewhere.

    Matching on dimensions rather than on content is deliberate: a rasterised metafile and a
    logo differ in size by orders of magnitude, and decoding every image to compare pixels
    would cost more than the whole scan.
    """
    mine = rasters_by_page(ours)
    theirs = rasters_by_page(reference)
    out = {}
    for page, rasters in theirs.items():
        only_theirs = rasters - mine.get(page, set())
        if only_theirs:
            out[page] = only_theirs
    return out


def words_per_page(pdf, count):
    words = {}
    for page in range(1, count + 1):
        text = run(["pdftotext", "-f", str(page), "-l", str(page), pdf, "-"])
        words[page] = len(text.split())
    return words


def page_count(pdf):
    for line in run(["pdfinfo", pdf]).splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return -1


def render(document, outdir, worker):
    """Render one document both ways. Returns (ours, reference) paths or (None, None)."""
    stem, extension = os.path.splitext(os.path.basename(document))
    identity = f"{stem}__{extension.lstrip('.').lower()}"
    ours = os.path.join(outdir, "ours", f"{identity}.pdf")
    reference = os.path.join(outdir, "ref", f"{identity}.pdf")
    if os.path.exists(ours) and os.path.exists(reference):
        return ours, reference                     # resumable: a killed run costs nothing

    repo = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
    cli = os.path.join(
        repo, "dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli")
    scratch = os.path.join(outdir, f"t{worker}")
    profile = os.path.join(outdir, f"prof{worker}")
    os.makedirs(scratch, exist_ok=True)
    os.makedirs(profile, exist_ok=True)

    run([cli, "render", document, "--format", "pdf", "--outdir", scratch])
    produced = os.path.join(scratch, f"{stem}.pdf")
    if os.path.exists(produced):
        os.replace(produced, ours)

    # Each worker needs its own profile: two headless instances sharing one block on the
    # lock and the loser converts nothing, silently, with exit status 0.
    run(["soffice", f"-env:UserInstallation=file://{profile}",
         "--headless", "--convert-to", "pdf", "--outdir", scratch, document])
    if os.path.exists(produced):
        os.replace(produced, reference)

    return (ours if os.path.exists(ours) else None,
            reference if os.path.exists(reference) else None)


def main():
    if len(sys.argv) < 3:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    root, outdir = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    documents_only = "--documents-only" in sys.argv
    workers = int(sys.argv[3]) if len(sys.argv) > 3 and sys.argv[3].isdigit() else 2

    # Every document, not only the metafile carriers. Filtering the page scan by "embeds a
    # metafile" was the first design and it was wrong: a page whose ceiling comes from some
    # other embedded object is then invisible to the whole tool. Measured -- one .ppt already
    # established as this class holds no metafile at all, and scanning only carriers hid it.
    # The metafile count is an *attribution* carried alongside each row, not a gate.
    carrying = []
    with_metafiles = 0
    for document in corpus_documents(root):
        emf, wmf = metafiles_in(document)
        if emf or wmf:
            with_metafiles += 1
        if documents_only and not (emf or wmf):
            continue
        carrying.append((os.path.relpath(document, root), document, emf, wmf))

    print(f"{with_metafiles} documents embed a metafile", file=sys.stderr)
    if documents_only:
        for relative, _, emf, wmf in carrying:
            print(f"{relative}\t{emf}\t{wmf}")
        return 0
    print(f"comparing pages across all {len(carrying)} documents", file=sys.stderr)

    for sub in ("ours", "ref"):
        os.makedirs(os.path.join(outdir, sub), exist_ok=True)

    rows = []

    def examine(job):
        index, (relative, absolute, emf, wmf) = job
        ours, reference = render(absolute, outdir, index % workers)
        if not ours or not reference:
            return [(relative, 0, emf, wmf, "-", "-", "render-failed")]
        mine, theirs = page_count(ours), page_count(reference)
        if mine != theirs or mine < 1:
            # Pagination has to agree before a per-page comparison means anything.
            return [(relative, 0, emf, wmf, "-", "-", "pagination-differs")]
        rastered = unmatched_rasters(ours, reference)
        if not rastered:
            return []
        my_words = words_per_page(ours, mine)
        their_words = words_per_page(reference, theirs)
        found = []
        for page in sorted(rastered):
            extra = my_words.get(page, 0) - their_words.get(page, 0)
            floor = max(MIN_EXTRA_WORDS, their_words.get(page, 0) * MIN_EXTRA_RATIO)
            if extra >= floor:
                found.append((relative, page, emf, wmf,
                              my_words[page], their_words[page], "ceiling"))
        return found

    with ThreadPoolExecutor(max_workers=workers) as pool:
        for found in pool.map(examine, enumerate(carrying)):
            rows.extend(found)

    destination = os.path.join(outdir, "metafile-pages.tsv")
    with open(destination, "w") as handle:
        handle.write("document\tpage\temf\twmf\tour_words\tref_words\tverdict\n")
        for row in rows:
            handle.write("\t".join(str(field) for field in row) + "\n")

    flagged = [r for r in rows if r[6] == "ceiling"]
    documents = {r[0] for r in flagged}
    print(f"{len(flagged)} pages flagged across {len(documents)} documents", file=sys.stderr)
    print(destination)
    return 0


if __name__ == "__main__":
    sys.exit(main())
