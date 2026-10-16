#!/usr/bin/env python3
"""Vary the witness chart's inner plot height and read the axes back.

`040_Blood_pressure_tracker` states `c:layoutTarget val="inner"`, so its plot
rectangle is a fixed fraction of the chart frame and `VDiagram::adjustInnerSize`
never runs (`ChartView.cxx`:589-619 is all guarded on `!mbUseFixedInnerSize`).
That makes the axis' drawn length a quantity the file states, and sweeping it
moves `estimateMaximumAutoMainIncrementCount`'s numerator alone: the label
height, the data and everything else are held.

Each flip of the drawn step therefore sits at an integer of
`axis length / label height`, which is what solves for the denominator.
"""
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from axisticks import axes                                    # noqa: E402

SOFFICE = "/opt/libreoffice26.2/program/soffice"
SOURCE = Path("/home/user/sample-files/sheets/chartset-007/xlsx/"
              "040_Blood_pressure_tracker_872b6833.xlsx")
PART = "xl/charts/chart11.xml"
ORIGINAL = "0.52149889537189142"


def patched(out: Path, height: float) -> Path:
    """The witness with one attribute changed and every other byte kept."""
    target = out / f"h{height:.4f}.xlsx"
    with zipfile.ZipFile(SOURCE) as src, zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == PART:
                text = data.decode("utf-8")
                assert f'<c:h val="{ORIGINAL}"/>' in text
                text = text.replace(f'<c:h val="{ORIGINAL}"/>',
                                    f'<c:h val="{height:.10f}"/>')
                data = text.encode("utf-8")
            dst.writestr(item, data)
    return target


def render(doc: Path, out: Path) -> Path:
    profile = out / "profile"
    subprocess.run(
        ["timeout", "-k", "30", "240", SOFFICE,
         f"-env:UserInstallation=file://{profile}",
         "--headless", "--convert-to", "pdf", "--outdir", str(out), str(doc)],
        check=True, capture_output=True)
    return out / (doc.stem + ".pdf")


def main():
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    heights = [float(a) for a in sys.argv[2:]]
    print("h\tprimary_step\tprimary_span\tsecondary_step\tsecondary_span\tlabel_h")
    for h in heights:
        work = out / f"w{h:.4f}"
        shutil.rmtree(work, ignore_errors=True)
        work.mkdir(parents=True)
        pdf = render(patched(work, h), work)
        found = axes(pdf)
        left = [a for a in found if a["x"] < 300]
        right = [a for a in found if a["x"] >= 300]
        f = lambda xs, k: (xs[0][k] if xs else "-")
        print(f"{h:.4f}\t{f(left,'step')}\t{f(left,'length')}\t"
              f"{f(right,'step')}\t{f(right,'length')}\t{f(right,'height')}",
              flush=True)


if __name__ == "__main__":
    main()
