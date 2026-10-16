#!/usr/bin/env python3
"""Each fixture's four lines, their left edge and their pitch, from both renderers."""
import os, pathlib, subprocess, sys

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SOFFICE = "/opt/libreoffice26.2/program/soffice"
WORDS = {"Alpha", "Bravo", "Charlie", "Delta", "1.", "2.", "3.", "4."}


def lines(pdf):
    import fitz
    out = []
    with fitz.open(pdf) as doc:
        for page in doc:
            for block in page.get_text("dict")["blocks"]:
                for line in block.get("lines", []):
                    text = "".join(s["text"] for s in line["spans"]).strip()
                    if text and any(w in text for w in WORDS):
                        out.append((text, round(line["bbox"][0] - 72, 2),
                                    round(line["bbox"][1] - 144, 2)))
    return out


def main():
    fixtures, work = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    for docx in sorted(fixtures.glob("*.docx")):
        print(f"=== {docx.stem} ===")
        for label in ("reference", "ours"):
            outdir = work / docx.stem / label
            outdir.mkdir(parents=True, exist_ok=True)
            pdf = next(outdir.glob("*.pdf"), None)
            if pdf is None:
                if label == "reference":
                    subprocess.run([SOFFICE, f"-env:UserInstallation=file://{outdir / 'p'}",
                                    "--headless", "--norestore", "--convert-to", "pdf",
                                    "--outdir", str(outdir), str(docx)],
                                   check=True, capture_output=True, timeout=300)
                else:
                    subprocess.run([CLI, "render", str(docx), "--outdir", str(outdir)], check=True,
                                   capture_output=True, timeout=300,
                                   env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
                pdf = next(outdir.glob("*.pdf"), None)
            got = lines(pdf) if pdf else []
            pitch = [round(b[2] - a[2], 2) for a, b in zip(got, got[1:])]
            print(f"  {label:<10} " + " | ".join(f"{t} @{x},{y}" for t, x, y in got))
            print(f"  {'':<10} pitch {pitch}")


if __name__ == "__main__":
    main()
