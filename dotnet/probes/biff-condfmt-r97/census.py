#!/usr/bin/env python3
"""Does 26.2.4.2 actually paint a BIFF8 conditional format?

Round 95 concluded that reading `CONDFMT` "moves nothing", on the strength of 64 of 64
byte-identical renderings.  That measurement was taken with a reader that parsed the
`CONDFMT` record — a count and a `SqRef` range list — and nothing else.  The rules live in
the `CF` records that follow it, and the formatting lives in the styles those rules name.
A reader that never reads a rule cannot move a pixel, so the byte-identity was a property of
the instrument.

This asks the reference instead, which cannot be argued with: convert each of the four
witnesses with 26.2.4.2's own `--convert-to fods` and count the conditional formats it
resolves, the conditions inside them, and how many of the styles those conditions name carry
ink (a fill, a font colour, or bold).

Note the style-name escaping: a condition names `Excel_CondFormat_1_1_1`, and the style that
carries the formatting is declared as `Excel_5f_CondFormat_5f_1_5f_1_5f_1` with the
unescaped form in `style:display-name`.  Matching on the unescaped name alone finds nothing
and reports a clean zero.
"""
import glob, os, re, subprocess, sys, tempfile

WITNESSES = [
    "Background_Declaration_Template.xls",
    "NPA_21_21_Sentenced_Comments.xls",
    "Hazard Analysis Template.xls",
    "TICAPCapability_Final.xls",
]
SOFFICE = "/opt/libreoffice26.2/program/soffice"
CORPUS = "/home/user/sample-files"


def find(name):
    for p in glob.glob(os.path.join(CORPUS, "**", name), recursive=True):
        return p
    return None


def convert(src, outdir):
    subprocess.run(
        [SOFFICE, "--headless", f"-env:UserInstallation=file://{outdir}/prof",
         "--convert-to", "fods", "--outdir", outdir, src],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=240)
    return os.path.join(outdir, os.path.splitext(os.path.basename(src))[0] + ".fods")


def measure(fods):
    s = open(fods, encoding="utf-8", errors="replace").read()
    names = set(re.findall(r'calcext:apply-style-name="([^"]+)"', s))
    inked = 0
    for n in names:
        esc = n.replace("_", "_5f_")
        m = re.search(r'<style:style style:name="%s".*?</style:style>' % re.escape(esc), s, re.S)
        if m and ("background-color" in m.group(0) or "fo:color" in m.group(0) or "bold" in m.group(0)):
            inked += 1
    return s.count("<calcext:conditional-format "), s.count("<calcext:condition "), len(names), inked


def main():
    with tempfile.TemporaryDirectory() as tmp:
        print("document\tformats\tconditions\tstyles\tinked")
        for w in WITNESSES:
            src = find(w)
            if not src:
                print(f"{w}\tMISSING", file=sys.stderr)
                continue
            cf, cond, styles, inked = measure(convert(src, tmp))
            print(f"{w}\t{cf}\t{cond}\t{styles}\t{inked}")


if __name__ == "__main__":
    main()
