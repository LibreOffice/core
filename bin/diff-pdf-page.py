#!/usr/bin/env python3
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Diffs two PDF pages: a reference and our own output. Red output means some to fix: the amount of
# red is meant to reduce as fixing progresses. Sample usage:
#
# bin/diff-pdf-page.py reference.pdf test.pdf diff.png

import argparse
import tempfile
import subprocess

def run(argv):
    print(" ".join(argv))
    subprocess.run(argv, check=True)

def main():
    parser = argparse.ArgumentParser(description="Diffs two PDF pages, first is painted red instead of black, the second is painted on top of the first.")
    parser.add_argument("-d", "--density", default="384")
    parser.add_argument("-p", "--page", default="0")
    parser.add_argument("a_pdf")
    parser.add_argument("b_pdf")
    parser.add_argument("diff_png")
    args = parser.parse_args()

    a_png = tempfile.NamedTemporaryFile(suffix=".png")
    a_pdf = args.a_pdf + "[" + args.page + "]"
    run(["convert", "-density", args.density, a_pdf, "-colorspace", "RGB", "-fuzz", "95%", "-fill", "red", "-opaque", "black", a_png.name])
    b_png = tempfile.NamedTemporaryFile(suffix=".png")
    b_pdf = args.b_pdf + "[" + args.page + "]"
    run(["convert", "-density", args.density, b_pdf, "-colorspace", "RGB", b_png.name])
    composite_png = tempfile.NamedTemporaryFile(suffix=".png")
    run(["convert", "-composite", a_png.name, b_png.name, composite_png.name])
    run(["convert", composite_png.name, "-background", "white", "-flatten", args.diff_png])

if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
