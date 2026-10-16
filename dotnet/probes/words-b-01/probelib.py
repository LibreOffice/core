"""Minimal authored-DOCX builder and a soffice/pdftotext read-back, for the words-b probes.

Every render gets its own -env:UserInstallation profile: two headless soffice sharing
~/.config/libreoffice block on the profile lock and one of them silently converts nothing.
SOURCE_DATE_EPOCH and TZ are pinned on every render.
"""
from __future__ import annotations

import os
import re
import subprocess
import zipfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
<Relationship Id="rIdN" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
<Relationship Id="rIdT" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
</Relationships>"""


def settings(compat: int = 15) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings {NS}><w:compat><w:compatSetting w:name="compatibilityMode"
 w:uri="http://schemas.microsoft.com/office/word" w:val="{compat}"/></w:compat></w:settings>"""


def styles(family: str = "Liberation Serif", points: float = 12.0) -> str:
    half = int(round(points * 2))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="{half}"/>
</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>
<w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="{half}"/></w:rPr>
</w:style></w:styles>"""


def numbering(points: float, family: str = "Liberation Serif", text: str = "%1.",
              hanging: int = 720) -> str:
    """One single-level decimal list whose level states its own face and size."""
    half = int(round(points * 2))
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:numbering {NS}>
<w:abstractNum w:abstractNumId="0"><w:multiLevelType w:val="singleLevel"/>
<w:lvl w:ilvl="0"><w:start w:val="1"/><w:numFmt w:val="decimal"/>
<w:lvlText w:val="{text}"/><w:lvlJc w:val="left"/>
<w:pPr><w:ind w:left="{hanging}" w:hanging="{hanging}"/></w:pPr>
<w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="{half}"/></w:rPr>
</w:lvl></w:abstractNum>
<w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
</w:numbering>"""


SECT = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"'
        ' w:header="0" w:footer="0"/></w:sectPr>')


def document(body: str) -> str:
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:document {NS}><w:body>{body}{SECT}</w:body></w:document>')


def build(path: Path, body: str, *, level_points: float = 12.0,
          level_family: str = "Liberation Serif", level_text: str = "%1.",
          hanging: int = 720, family: str = "Liberation Serif", points: float = 12.0,
          compat: int = 15) -> None:
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", ROOT_RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/settings.xml", settings(compat))
        z.writestr("word/styles.xml", styles(family, points))
        z.writestr("word/numbering.xml",
                   numbering(level_points, level_family, level_text, hanging))
        z.writestr("word/document.xml", document(body))


CLI = Path(os.environ.get(
    "XRAY_OFFICE_CLI",
    "/c/sandbox/workdir/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/"
    "net10.0/linux-x64/XRay.Office.Cli"))


def render_ours(sources: list[Path], outdir: Path, workers: int = 12) -> list[Path]:
    """Renders each source with our own CLI."""
    outdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

    def one(src: Path) -> Path:
        subprocess.run([str(CLI), "render", "--outdir", str(outdir), str(src)],
                       capture_output=True, timeout=600, env=env)
        return outdir / (src.stem + ".pdf")

    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(one, sources))


def render(sources: list[Path], outdir: Path, workers: int = 12) -> list[Path]:
    """Converts each source to PDF, one soffice profile per job."""
    outdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

    def one(job: tuple[int, Path]) -> Path:
        index, src = job
        profile = outdir / f"prof{index}"
        subprocess.run(
            ["soffice", "--headless", "--norestore",
             f"-env:UserInstallation=file://{profile}",
             "--convert-to", "pdf", "--outdir", str(outdir), str(src)],
            capture_output=True, timeout=600, env=env)
        return outdir / (src.stem + ".pdf")

    with ThreadPoolExecutor(max_workers=workers) as pool:
        return list(pool.map(one, list(enumerate(sources))))


WORD = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">'
                  r'([^<]*)</word>')


def words(pdf: Path, page: int = 1) -> list[tuple[str, float, float, float]]:
    """(text, yMin, yMax, xMin) for every word on a page, in poppler's top-down coordinates."""
    out = subprocess.run(["pdftotext", "-bbox", "-f", str(page), "-l", str(page),
                          str(pdf), "-"], capture_output=True, text=True).stdout
    found = []
    for m in WORD.finditer(out):
        found.append((m.group(5), float(m.group(2)), float(m.group(4)), float(m.group(1))))
    return found


def marks(pdf: Path, page: int = 1) -> dict[str, tuple[float, float]]:
    """Each marker word's (yMin, yMax). Markers are unique upper-case tokens."""
    got: dict[str, tuple[float, float]] = {}
    for text, ymin, ymax, _ in words(pdf, page):
        token = re.sub(r"[^A-Z]", "", text)
        if len(token) >= 3 and token not in got:
            got[token] = (ymin, ymax)
    return got


def pages(pdf: Path) -> int:
    out = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    m = re.search(r"^Pages:\s+(\d+)", out, re.M)
    return int(m.group(1)) if m else 0
