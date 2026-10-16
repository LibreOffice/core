#!/usr/bin/env python3
"""Each word's face, size and weight as drawn, for the character fixtures."""
import os, pathlib, subprocess, sys

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SOFFICE = "/opt/libreoffice26.2/program/soffice"


def main():
    fixtures, work = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    import fitz
    for stem in sys.argv[3:]:
        docx = fixtures / f"{stem}.docx"
        print(f"=== {stem} ===")
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
            with fitz.open(pdf) as doc:
                said = []
                for page in doc:
                    for b in page.get_text("dict")["blocks"]:
                        for l in b.get("lines", []):
                            for sp in l["spans"]:
                                t = sp["text"].strip()
                                if t in ("Alpha", "Bravo", "Charlie", "Delta"):
                                    said.append(f"{t} {sp['size']:.0f}pt {sp['font']}"
                                                f" y{sp['bbox'][1] - 144:.1f}")
                print(f"  {label:<10} " + " | ".join(said))


if __name__ == "__main__":
    main()
