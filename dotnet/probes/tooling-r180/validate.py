#!/usr/bin/env python3
"""Re-run round 180's three validations of the comparison tooling.

Each is a claim about an instrument rather than about the tree, so each is checked against a
case whose answer was established by a separate channel.

  1. the page-alignment screen fires on the documents known to be offset and stays silent on
     the ones known to be aligned;
  2. `compare-images.py` names colour quantisation rather than reporting it as a difference;
  3. the metric set still sees a block that merely moved.
"""
import difflib, importlib.util, pathlib, struct, sys, zlib

SKILLS = pathlib.Path("/home/user/libreoffice-core/.claude/skills")


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def png(path, width, height, shade):
    rows = b"".join(b"\x00" + bytes(shade(x, y) for x in range(width)) for y in range(height))

    def chunk(tag, body):
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body))

    head = struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", head)
                     + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b""))


def alignment(scratch):
    """Arm 1, against the five documents whose pagination was established separately."""
    module = load("pid", SKILLS / "render-comparison/scripts/pdf-image-diff.py")
    cases = [
        ("Annex-10-to-the", "/home/user/r176-words/ours", 0, "aligned; quantisation only"),
        ("f445896e", "/home/user/r176-words/ours", 0, "aligned"),
        ("02_mcar_part-2_and_IS_v2.10", "/home/user/r179-words/ours", 1, "aligned after r175"),
        ("SPA-02_mcar_part-2_and_IS_v2.9", "/home/user/r179-words/ours", 37,
         "equal page counts, content offset"),
        ("24-25_FAA_Holdover", "/home/user/r179-words/ours", 49, "one word-parity page lost"),
    ]
    listing = pathlib.Path("/home/user/libreoffice-core/dotnet/probes/wordsink-r174/words.txt")
    print("expected\tgot\tfirst\tdocument")
    for stem, root, expected, note in cases:
        document = next(line.strip() for line in open(listing) if stem in line)
        import hashlib
        key = hashlib.md5(document.encode()).hexdigest()[:16]
        ours = next(pathlib.Path(f"{root}/{key}").glob("*.pdf"), None)
        reference = next(pathlib.Path(f"/home/user/r174-words/ref/{key}").glob("*.pdf"), None)
        if ours is None or reference is None:
            print(f"{expected}\t-\t-\t{stem} (renders absent)")
            continue
        drift = module.misaligned_pages(ours, reference) or []
        print(f"{expected}\t{len(drift)}\t{drift[0] if drift else '-'}\t{stem} — {note}")


def metrics(scratch):
    """Arms 2 and 3, on synthetic pages so nothing but the one variable moves."""
    module = load("cmp", SKILLS / "render-comparison/scripts/compare-images.py")
    w = h = 200
    png(scratch / "flat-128.png", w, h, lambda x, y: 128 if y < h // 2 else 255)
    png(scratch / "flat-127.png", w, h, lambda x, y: 127 if y < h // 2 else 255)
    png(scratch / "block-left.png", w, h,
        lambda x, y: 0 if (60 <= y < 80 and 20 <= x < 40) else (128 if y < h // 2 else 255))
    png(scratch / "block-right.png", w, h,
        lambda x, y: 0 if (60 <= y < 80 and 60 <= x < 80) else (128 if y < h // 2 else 255))
    for label, (a, b) in {
        "one grey level over half the page": ("flat-128.png", "flat-127.png"),
        "a 20x20 block moved 40 px": ("block-left.png", "block-right.png"),
    }.items():
        m = module.compare(module.read_png(scratch / a), module.read_png(scratch / b))
        print(f"--- {label} ---")
        for key in ("differing_fraction", "mean_abs_error", "max_tile_error",
                    "differing_tiles", "shifted_tiles", "ink_delta"):
            print(f"   {key:<20} {m[key]}")
        print(f"   {module.diagnose(m)}")


def main():
    scratch = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/tmp/tooling-r180")
    scratch.mkdir(parents=True, exist_ok=True)
    print("== 1. the page-alignment screen ==")
    alignment(scratch)
    print("\n== 2 and 3. the metric set ==")
    metrics(scratch)


if __name__ == "__main__":
    main()
