#!/usr/bin/env python3
"""What the wrap moves, with no renderer of ours in the loop.

`spanshift.py` compares this tree against the reference and so mixes the wrap with every other
defect a document has.  This compares the reference against *itself* -- the document as authored
beside the same document with every `w:tblpPr` removed -- so the displacement it reports is the
positioned table's wrap and nothing else.  A document that reads zero here is one where the wrap
cannot be what we are getting wrong, however far apart the two renderers are on it.
"""
import pathlib, re, sys

sys.path.insert(0, str(pathlib.Path(__file__).parent))
from spanshift import spans, pair


def main():
    root = pathlib.Path(sys.argv[2])
    print("paired\tofFloat\tmean|dx|\tmean|dy|\tmax|dy|\tover10pt\tdocument")
    for line in open(sys.argv[1]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        work = root / re.sub(r"\W+", "-", path.stem)[:60]
        a = next((work / "float").glob("*.pdf"), None)
        b = next((work / "flow").glob("*.pdf"), None)
        if a is None or b is None:
            print(f"-\t-\t-\t-\t-\t-\t{path.name} (missing leg)")
            continue
        float_spans, flow_spans = spans(a), spans(b)
        pairs = pair(float_spans, flow_spans)
        if not pairs:
            print(f"0\t{len(float_spans)}\t-\t-\t-\t-\t{path.name}")
            continue
        dx = sum(abs(p) for p, _ in pairs) / len(pairs)
        dy = sum(abs(q) for _, q in pairs) / len(pairs)
        mx = max(abs(q) for _, q in pairs)
        big = sum(1 for _, q in pairs if abs(q) > 10)
        print(f"{len(pairs)}\t{len(float_spans)}\t{dx:.2f}\t{dy:.2f}\t{mx:.2f}\t{big}\t{path.name}",
              flush=True)


if __name__ == "__main__":
    main()
