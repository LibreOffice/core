#!/usr/bin/env python3
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Turns a Lightproof rule source into the two tables the build consumes:
#
#   <pkg>.dat  ->  lightproof_<pkg>.py       the rules
#   <pkg>.dlg  ->  lightproof_opts_<pkg>.py  the options and their defaults
#
# Usage: lpgenerate.py <rule source directory> [<output directory>]
#
#   lpgenerate.py dictionaries/en/lightproof
#
# Writes beside the package's other generated tables unless told otherwise.
# Compile one package per run: see the note in lightproof_compile.py.
#
# This replaces the upstream make.py, which also packed an .oxt extension.
# Nothing loads that extension any more; the tables go on to lpcompile.py,
# which produces the .lpr the checker reads.

import argparse
import configparser
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lightproof_compile


def read_config(path):
    parser = configparser.ConfigParser()
    parser.read(path, encoding="utf-8")
    return dict(parser.items("args"))


def parse_options(text):
    """The option groups of a .dlg, in the order the dialog lists them.

    Returns (every option id, the ids starred as on by default). A star marks
    a default; a bare dash is a spacer in the dialog's grid and not an option.
    """
    names = []
    defaults = []
    for line in text.split("\n"):
        line = line.strip()
        # The titles follow the groups, starting at the first [lang=...] line.
        if line.startswith("["):
            break
        if not line or line.startswith("#") or ":" not in line:
            continue
        for token in re.split(r"[\s,]+", line.split(":", 1)[1]):
            if not token or token == "-":
                continue
            name = token.lstrip("*")
            if name in names:
                continue
            names.append(name)
            if token.startswith("*"):
                defaults.append(name)
    return names, defaults


def parse_labels(text):
    """The option labels of a .dlg.

    A title is "id=label" and may carry a line of help after a \\n, which the
    dialog shows under the checkbox.
    """
    labels = {}
    started = False
    for line in text.split("\n"):
        line = line.strip()
        if line.startswith("["):
            started = True
            continue
        if not started or not line or line.startswith("#") or "=" not in line:
            continue
        name, _, label = line.partition("=")
        labels[name.strip()] = label.strip()
    return labels


def generate(source_dir, out_dir):
    # The package is named by its config file, so the directory holding the
    # source can be called whatever suits the tree.
    configs = [f for f in sorted(os.listdir(source_dir)) if f.endswith(".cfg")]
    if len(configs) != 1:
        sys.exit("lpgenerate: expected one .cfg in %s, found %d" % (source_dir, len(configs)))
    package = configs[0][:-len(".cfg")]
    config = read_config(os.path.join(source_dir, configs[0]))
    language = config.get("lang", package)

    with open(os.path.join(source_dir, package + ".dat"), encoding="utf-8") as rules:
        compiled = lightproof_compile.c(rules.read(), language)

    rules_path = os.path.join(out_dir, "lightproof_%s.py" % package)
    with open(rules_path, "w", encoding="utf-8") as out:
        out.write("# -*- encoding: UTF-8 -*-\n")
        # One rule to a line: the tables are committed, so they have to diff.
        out.write("dic = [\n")
        for rule in compiled["rules"]:
            out.write("%r,\n" % (rule,))
        out.write("]\n")

    dialog_path = os.path.join(source_dir, package + ".dlg")
    options_path = os.path.join(out_dir, "lightproof_opts_%s.py" % package)
    if os.path.exists(dialog_path):
        with open(dialog_path, encoding="utf-8") as dialog:
            names, defaults = parse_options(dialog.read())
        with open(options_path, "w", encoding="utf-8") as out:
            out.write("lopts = {}\nlopts_default = {}\n")
            out.write("lopts['%s'] = %s\n" % (package, names))
            out.write("lopts_default['%s'] = %s\n" % (package, defaults))

    sys.stderr.write("lpgenerate: %s: %d rules, %d options\n"
                     % (package, len(compiled["rules"]),
                        len(names) if os.path.exists(dialog_path) else 0))


def main():
    parser = argparse.ArgumentParser(description="Generate Lightproof rule tables")
    parser.add_argument("source", help="the package's rule source directory")
    parser.add_argument("output", nargs="?",
                        help="where to write the tables (default: the package's pythonpath)")
    args = parser.parse_args()

    out_dir = args.output
    if not out_dir:
        out_dir = os.path.join(os.path.dirname(os.path.normpath(args.source)), "pythonpath")
    generate(args.source, out_dir)


if __name__ == "__main__":
    main()
