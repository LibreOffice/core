#!/usr/bin/env python3
"""Where the text sits inside each VML shape, once the shapes themselves are known to agree.

Pairing spans by string on these templates resolves nothing -- `SUBTASK` appears thirty times -- and
pairing them by position begs the question.  So each span is assigned to the filled rectangle that
contains it, the rectangles are paired between the two renderings by position (they agree to 0.1 pt),
and the comparison is of each span's offset *within its own box*.  That is the only quantity left
once placement is exact.
"""
import hashlib, pathlib, sys


def key(path):
    return hashlib.md5(str(path).encode()).hexdigest()[:16]


def boxes_and_spans(pdf):
    import fitz
    boxes, spans = [], []
    with fitz.open(pdf) as doc:
        for number, page in enumerate(doc):
            for drawing in page.get_drawings():
                if drawing["type"] not in ("f", "fs"):
                    continue
                r = drawing["rect"]
                if r.width > 20 and r.height > 10:
                    boxes.append((number, r.x0, r.y0, r.x1, r.y1))
            for block in page.get_text("dict")["blocks"]:
                for line in block.get("lines", []):
                    for span in line["spans"]:
                        if span["text"].strip():
                            spans.append((number, span["text"].strip(), span["bbox"][0],
                                          span["bbox"][1], round(span["size"], 2)))
    return boxes, spans


def assign(boxes, spans):
    """Each span against the smallest box that holds its corner, keyed by that box."""
    out = {}
    for page, text, x, y, size in spans:
        best = None
        for box in boxes:
            if box[0] != page or not (box[1] - 1 <= x <= box[3] + 1 and box[2] - 1 <= y <= box[4] + 1):
                continue
            area = (box[3] - box[1]) * (box[4] - box[2])
            if best is None or area < best[1]:
                best = (box, area)
        if best is None:
            out.setdefault(("loose", page), []).append((text, x, y, size))
        else:
            box = best[0]
            out.setdefault((box[0], round(box[1], 1), round(box[2], 1)), []).append(
                (text, x - box[1], y - box[2], size))
    return out


def main():
    root = pathlib.Path(sys.argv[1])
    for line in open(sys.argv[2]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        k = key(path)
        legs = {}
        for leg in ("after", "ref"):
            pdf = next((root / leg / k).glob("*.pdf"), None)
            legs[leg] = assign(*boxes_and_spans(pdf)) if pdf else {}
        shared = sorted(set(legs["after"]) & set(legs["ref"]), key=str)
        pairs, mismatched, loose = [], 0, 0
        for slot in shared:
            ours, theirs = legs["after"][slot], legs["ref"][slot]
            if slot[0] == "loose":
                loose += 1
                continue
            if [t for t, _, _, _ in ours] != [t for t, _, _, _ in theirs]:
                mismatched += 1
                continue
            for (_, ax, ay, asz), (_, bx, by, bsz) in zip(ours, theirs):
                pairs.append((ax - bx, ay - by, asz - bsz))
        print(f"=== {path.name[:56]} ===")
        print(f"  boxes: ours {len(legs['after'])} slots, reference {len(legs['ref'])}, "
              f"{len(shared)} shared, {mismatched} whose text differs")
        if pairs:
            print(f"  {len(pairs)} spans: mean |dx| {sum(abs(a) for a,_,_ in pairs)/len(pairs):.2f}"
                  f"  mean |dy| {sum(abs(b) for _,b,_ in pairs)/len(pairs):.2f}"
                  f"  max |dy| {max(abs(b) for _,b,_ in pairs):.2f}"
                  f"  mean |dsize| {sum(abs(c) for _,_,c in pairs)/len(pairs):.3f}")
        for slot in shared[:3]:
            if slot[0] == "loose":
                continue
            print(f"   box {slot}")
            for leg in ("after", "ref"):
                print(f"     {leg:<6} " + " | ".join(
                    f"{t} @{x:.1f},{y:.1f} {s}pt" for t, x, y, s in legs[leg][slot][:4]))


if __name__ == "__main__":
    main()
