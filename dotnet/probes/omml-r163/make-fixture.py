#!/usr/bin/env python3
"""Author `dotnet/tests/corpus/features/words-formula-complexity.docx`.

A complexity ladder of OMML constructs, each one rendered BOTH inline
(`m:oMath` inside a `w:p`, beside body text) and as display maths
(`m:oMathPara`), because the two take different vertical metrics: an inline
formula is an as-character object in a text line and a display one is a
centred paragraph of its own.

A `word/settings.xml` part is included -- without one a hand-built DOCX does
not get LibreOffice's OOXML compatibility defaults and quietly answers a
different question (see the `office-corpus` skill).

Usage:  make-fixture.py [OUTPUT.docx]
"""
import sys
import zipfile
from pathlib import Path

MATH_FONT = '<w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr>'


def r(text: str) -> str:
    """One OMML run."""
    return f"<m:r>{MATH_FONT}<m:t>{text}</m:t></m:r>"


# ---------------------------------------------------------------- the ladder
# (id, human label, the m:oMath body).  Ordered by complexity.
LADDER: list[tuple[str, str, str]] = [
    ("sub", "bare subscript",
     f"<m:sSub><m:e>{r('x')}</m:e><m:sub>{r('1')}</m:sub></m:sSub>"),

    ("sup", "bare superscript",
     f"<m:sSup><m:e>{r('x')}</m:e><m:sup>{r('2')}</m:sup></m:sSup>"),

    ("subsup", "subscript and superscript together",
     f"<m:sSubSup><m:e>{r('x')}</m:e><m:sub>{r('1')}</m:sub>"
     f"<m:sup>{r('2')}</m:sup></m:sSubSup>"),

    ("presub", "pre-subscript and pre-superscript",
     f"<m:sPre><m:sub>{r('1')}</m:sub><m:sup>{r('2')}</m:sup>"
     f"<m:e>{r('X')}</m:e></m:sPre>"),

    ("frac", "fraction",
     f"<m:f><m:fPr><m:type m:val=\"bar\"/></m:fPr>"
     f"<m:num>{r('a')}</m:num><m:den>{r('b')}</m:den></m:f>"),

    ("fracsub", "fraction whose terms are subscripted",
     "<m:f><m:fPr><m:type m:val=\"bar\"/></m:fPr>"
     f"<m:num><m:sSub><m:e>{r('a')}</m:e><m:sub>{r('1')}</m:sub></m:sSub></m:num>"
     f"<m:den><m:sSub><m:e>{r('b')}</m:e><m:sub>{r('2')}</m:sub></m:sSub></m:den>"
     "</m:f>"),

    ("rad", "square root",
     "<m:rad><m:radPr><m:degHide m:val=\"1\"/></m:radPr>"
     f"<m:deg/><m:e>{r('x')}</m:e></m:rad>"),

    ("radn", "nth root over a fraction",
     "<m:rad><m:radPr><m:degHide m:val=\"0\"/></m:radPr>"
     f"<m:deg>{r('3')}</m:deg>"
     "<m:e><m:f><m:fPr><m:type m:val=\"bar\"/></m:fPr>"
     f"<m:num>{r('a')}</m:num><m:den>{r('b')}</m:den></m:f></m:e></m:rad>"),

    ("narysum", "n-ary sum with lower and upper limits",
     "<m:nary><m:naryPr><m:chr m:val=\"∑\"/><m:limLoc m:val=\"undOvr\"/>"
     "<m:subHide m:val=\"0\"/><m:supHide m:val=\"0\"/></m:naryPr>"
     f"<m:sub>{r('i=1')}</m:sub><m:sup>{r('n')}</m:sup>"
     f"<m:e><m:sSub><m:e>{r('x')}</m:e><m:sub>{r('i')}</m:sub></m:sSub></m:e></m:nary>"),

    ("naryint", "n-ary integral with limits beside the sign",
     "<m:nary><m:naryPr><m:chr m:val=\"∫\"/><m:limLoc m:val=\"subSup\"/>"
     "<m:subHide m:val=\"0\"/><m:supHide m:val=\"0\"/></m:naryPr>"
     f"<m:sub>{r('0')}</m:sub><m:sup>{r('∞')}</m:sup>"
     f"<m:e>{r('f(t)dt')}</m:e></m:nary>"),

    ("delim", "nested delimiters",
     "<m:d><m:dPr><m:begChr m:val=\"(\"/><m:endChr m:val=\")\"/></m:dPr><m:e>"
     f"{r('a+')}"
     "<m:d><m:dPr><m:begChr m:val=\"[\"/><m:endChr m:val=\"]\"/></m:dPr>"
     "<m:e><m:f><m:fPr><m:type m:val=\"bar\"/></m:fPr>"
     f"<m:num>{r('b')}</m:num><m:den>{r('c')}</m:den></m:f></m:e></m:d>"
     "</m:e></m:d>"),

    ("acc", "accent (overbar)",
     "<m:acc><m:accPr><m:chr m:val=\"̅\"/></m:accPr>"
     f"<m:e>{r('x')}</m:e></m:acc>"),

    ("func", "function name applied to an argument",
     f"<m:func><m:fName>{r('sin')}</m:fName>"
     f"<m:e><m:d><m:dPr><m:begChr m:val=\"(\"/><m:endChr m:val=\")\"/></m:dPr>"
     f"<m:e>{r('2θ')}</m:e></m:d></m:e></m:func>"),

    ("matrix", "2x2 matrix inside delimiters",
     "<m:d><m:dPr><m:begChr m:val=\"(\"/><m:endChr m:val=\")\"/></m:dPr><m:e>"
     "<m:m><m:mPr><m:mcs><m:mc><m:mcPr><m:count m:val=\"2\"/>"
     "<m:mcJc m:val=\"center\"/></m:mcPr></m:mc></m:mcs></m:mPr>"
     f"<m:mr><m:e>{r('a')}</m:e><m:e>{r('b')}</m:e></m:mr>"
     f"<m:mr><m:e>{r('c')}</m:e><m:e>{r('d')}</m:e></m:mr>"
     "</m:m></m:e></m:d>"),

    ("mixed", "fraction, radical, subscript and n-ary in one formula",
     "<m:f><m:fPr><m:type m:val=\"bar\"/></m:fPr>"
     "<m:num><m:rad><m:radPr><m:degHide m:val=\"1\"/></m:radPr><m:deg/>"
     f"<m:e><m:sSub><m:e>{r('W')}</m:e><m:sub>{r('MTOM')}</m:sub></m:sSub></m:e>"
     "</m:rad></m:num>"
     f"<m:den><m:sSub><m:e>{r('ρ')}</m:e><m:sub>{r('0')}</m:sub></m:sSub>"
     f"<m:sSub><m:e>{r('S')}</m:e><m:sub>{r('w')}</m:sub></m:sSub></m:den></m:f>"),
]


def paragraph(runs: str, style: str | None = None) -> str:
    props = f'<w:pPr><w:pStyle w:val="{style}"/></w:pPr>' if style else ""
    return f"<w:p>{props}{runs}</w:p>"


def wrun(text: str) -> str:
    return f'<w:r><w:t xml:space="preserve">{text}</w:t></w:r>'


def body() -> str:
    parts = [paragraph(wrun("OMML complexity ladder. "
                            "Each construct appears inline, then as display maths."))]
    for ident, label, math in LADDER:
        # Inline: the formula sits inside a line of ordinary body text, so the
        # question is where its baseline lands relative to the surrounding run.
        parts.append(paragraph(
            wrun(f"{ident} inline: before ")
            + f"<m:oMath>{math}</m:oMath>"
            + wrun(f" after ({label}).")))
        # Display: its own centred paragraph, which takes different metrics.
        # `m:oMathPara` must wrap an `m:oMath`; 26.2.4.2 silently draws nothing
        # for a bare construct under it, which is how a fixture bug hides.
        parts.append(paragraph(
            f"<m:oMathPara><m:oMath>{math}</m:oMath></m:oMathPara>"))

    # One paragraph of running text with several constructs inline in it, so
    # inline-vs-display placement and line breaking are exercised together.
    inline_mix = (
        wrun("Running text with ")
        + "<m:oMath>"
        + f"<m:sSub><m:e>{r('c')}</m:e><m:sub>{r('L')}</m:sub></m:sSub>"
        + "</m:oMath>"
        + wrun(" then ")
        + ("<m:oMath><m:f><m:fPr><m:type m:val=\"bar\"/></m:fPr>"
           f"<m:num>{r('1')}</m:num><m:den>{r('2')}</m:den></m:f></m:oMath>")
        + wrun(" then ")
        + ("<m:oMath><m:rad><m:radPr><m:degHide m:val=\"1\"/></m:radPr><m:deg/>"
           f"<m:e>{r('2')}</m:e></m:rad></m:oMath>")
        + wrun(" and the sentence carries on to the end of the measure so that a "
               "line break has to be taken somewhere after the formulas."))
    parts.append(paragraph(inline_mix))
    return "".join(parts)


DOCUMENT = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<w:document '
    'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
    'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
    'xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math">'
    "<w:body>{body}"
    '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
    '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134" '
    'w:header="709" w:footer="709" w:gutter="0"/></w:sectPr>'
    "</w:body></w:document>"
)

CONTENT_TYPES = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
    '<Default Extension="xml" ContentType="application/xml"/>'
    '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
    '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>'
    '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
    "</Types>"
)

RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" '
    'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" '
    'Target="word/document.xml"/></Relationships>'
)

DOC_RELS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
    '<Relationship Id="rId1" '
    'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" '
    'Target="settings.xml"/>'
    '<Relationship Id="rId2" '
    'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" '
    'Target="styles.xml"/></Relationships>'
)

# Present, and deliberately not empty: `w:compat` is what carries LibreOffice's
# OOXML compatibility defaults, and a fixture without this part is answering a
# different question from the one it was written for.
SETTINGS = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
    '<w:compat><w:compatSetting w:name="compatibilityMode" '
    'w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>'
    "</w:settings>"
)

STYLES = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    '<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
    "<w:docDefaults><w:rPrDefault><w:rPr>"
    '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>'
    "</w:rPr></w:rPrDefault></w:docDefaults>"
    '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
    '<w:name w:val="Normal"/></w:style>'
    "</w:styles>"
)


def main() -> None:
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(
        "dotnet/tests/corpus/features/words-formula-complexity.docx")
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/document.xml", DOCUMENT.format(body=body()))
        z.writestr("word/settings.xml", SETTINGS)
        z.writestr("word/styles.xml", STYLES)
    print(f"{out}  {out.stat().st_size} bytes  {len(LADDER)} constructs")


if __name__ == "__main__":
    main()
