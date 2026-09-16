# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# The sentence checker's option defaults are written down four times: in the
# rule package, in the .lpr the package is compiled to, in the configuration
# schema, and in the settings dialog. Only the first two are generated from
# one another; the other two are pasted in by hand, and nothing else notices
# when one of them is left behind. A stale schema is the one that bites,
# because the schema is what the checker actually reads: an option would come
# up in a state the package never asked for, with no build error and nothing
# in the log.

import glob
import importlib.util
import os
import re
import unittest
import xml.etree.ElementTree as ET


def engine_dir():
    # .../lingucomponent/qa/python/this file
    return os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))


def load_lpcompile():
    path = os.path.join(engine_dir(), "lingucomponent", "source", "spellcheck",
                        "lightproof", "tools", "lpcompile.py")
    spec = importlib.util.spec_from_file_location("lpcompile", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class OptionDefaults(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lpcompile = load_lpcompile()
        cls.packages = {}
        pattern = os.path.join(engine_dir(), "dictionaries", "*", "pythonpath",
                               "lightproof_opts_*.py")
        for path in sorted(glob.glob(pattern)):
            pkg = os.path.basename(path)[len("lightproof_opts_"):-len(".py")]
            opts = cls.lpcompile.read_module_assignments(
                path, {"lopts", "lopts_default"})
            names = opts.get("lopts", {}).get(pkg, [])
            if not names:
                continue
            cls.packages[pkg] = (
                names, set(opts.get("lopts_default", {}).get(pkg, [])))

    def test_packages_are_found(self):
        # Guards the rest: a glob that matches nothing would let every
        # comparison below pass by having nothing to compare.
        self.assertTrue(self.packages, "no rule package option tables found")

    def schema_groups(self):
        path = os.path.join(engine_dir(), "officecfg", "registry", "schema", "org",
                            "openoffice", "Office", "Linguistic.xcs")
        tree = ET.parse(path)
        oor = "{http://openoffice.org/2001/registry}"
        groups = {}
        for group in tree.iter("group"):
            if group.get(oor + "name") != "SentenceChecking":
                continue
            for package in group.findall("group"):
                flags = {}
                for prop in package.findall("prop"):
                    value = prop.find("value")
                    self.assertIsNotNone(
                        value, "%s has no default" % prop.get(oor + "name"))
                    flags[prop.get(oor + "name")] = value.text.strip() == "true"
                groups[package.get(oor + "name")] = flags
        return groups

    def test_schema_matches_the_packages(self):
        groups = self.schema_groups()
        self.assertEqual(sorted(groups), sorted(self.packages),
                         "the schema and the rule packages disagree on which "
                         "packages there are")
        for pkg, (names, defaults) in sorted(self.packages.items()):
            flags = groups[pkg]
            self.assertEqual(sorted(flags), sorted(names),
                             "%s: the schema and the package disagree on the "
                             "options" % pkg)
            self.assertEqual({name for name, on in flags.items() if on}, defaults,
                             "%s: the schema and the package disagree on which "
                             "options are on by default" % pkg)

    def dialog_block(self, text):
        # The text between the braces of the SentenceChecking object. Reading
        # only that leaves the rest of the file, which is a much larger
        # object of the same shape, out of the match.
        start = text.find("SentenceChecking: {")
        self.assertNotEqual(start, -1, "Xcu.ts has no SentenceChecking defaults")
        opened = text.index("{", start)
        depth = 0
        for at in range(opened, len(text)):
            if text[at] == "{":
                depth += 1
            elif text[at] == "}":
                depth -= 1
                if depth == 0:
                    return text[opened + 1:at]
        self.fail("Xcu.ts never closes its SentenceChecking defaults")

    def dialog_groups(self):
        path = os.path.join(engine_dir(), os.pardir, "browser", "admin", "src",
                            "integrator", "Xcu.ts")
        with open(path, encoding="utf-8") as source:
            block = self.dialog_block(source.read())
        groups = {}
        for match in re.finditer(r"(\w+): \{\n((?:\s*\w+: (?:true|false),\n)+)\s*\},",
                                 block):
            flags = dict((name, value == "true") for name, value
                         in re.findall(r"(\w+): (true|false),", match.group(2)))
            groups[match.group(1)] = flags
        return groups

    def test_dialog_matches_the_packages(self):
        groups = self.dialog_groups()
        self.assertEqual(sorted(groups), sorted(self.packages),
                         "the dialog and the rule packages disagree on which "
                         "packages there are")
        for pkg, (names, defaults) in sorted(self.packages.items()):
            flags = groups[pkg]
            self.assertEqual(sorted(flags), sorted(names),
                             "%s: the dialog and the package disagree on the "
                             "options" % pkg)
            self.assertEqual({name for name, on in flags.items() if on}, defaults,
                             "%s: the dialog and the package disagree on which "
                             "options are on by default" % pkg)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
