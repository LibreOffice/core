#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Golden-file tests for the codemaker tools (cppumaker and pythonmaker).

Both tools are fed the same IDL (tests.idl).  The IDL is compiled to a
temporary .rdb with unoidl-write, each tool is run on it, and the generated
output is compared byte-for-byte with the golden files stored next to this
script:

    expected_cppumaker_stubs/     <- golden output of cppumaker
    expected_pythonmaker_stubs/   <- golden output of pythonmaker
"""
import filecmp
import difflib
import glob
import os
import shutil
import subprocess
import unittest

from typing import TYPE_CHECKING, List, Optional, Set

HERE: str = os.path.dirname(os.path.abspath(__file__))

if TYPE_CHECKING:
    # For the type checker the mixin is a TestCase (so self.fail() etc. are
    # known); at run time it stays a plain mixin, so unittest does not collect
    # the shared base class as a test of its own.
    _MixinBase = unittest.TestCase
else:
    _MixinBase = object


class CodemakerGoldenTestMixin(_MixinBase):
    """Shared logic for golden-file comparison of a codemaker tool.

    Subclasses must also derive from unittest.TestCase and set:
      TOOL_NAME    - executable name in the SDK (without suffix)
      GOLDEN_DIRNAME - directory (next to this script) with the golden files
      IS_OUTPUT_FILE - predicate deciding which generated files are compared
    """

    TOOL_NAME: str = ""
    GOLDEN_DIRNAME: str = ""

    @staticmethod
    def IS_OUTPUT_FILE(name: str) -> bool:  # pragma: no cover - overridden
        raise NotImplementedError

    def setUp(self) -> None:
        # Build environment
        self.srcdir: str = os.environ.get("SRCDIR", os.getcwd())
        self.builddir: str = os.environ.get("BUILDDIR", os.getcwd())
        self.instdir: str = os.environ.get(
            "INSTDIR", os.path.join(self.builddir, "instdir")
        )
        self.workdir: str = os.environ.get(
            "WORKDIR", os.path.join(self.builddir, "workdir")
        )

        # SDK tools
        self.unoidl_write: Optional[str] = None
        self.tool: Optional[str] = None

        sdk_patterns: List[str] = [
            os.path.join(self.instdir, "sdk", "bin"),
            os.path.join(self.instdir, "LibreOffice*_SDK", "bin"),
        ]
        exe_suffix: str = ".exe" if os.name == "nt" else ""

        for pattern in sdk_patterns:
            for sdk_dir in glob.glob(pattern):
                unoidl_path = os.path.join(sdk_dir, "unoidl-write" + exe_suffix)
                tool_path = os.path.join(sdk_dir, self.TOOL_NAME + exe_suffix)
                if os.path.exists(unoidl_path) and os.path.exists(tool_path):
                    self.unoidl_write = unoidl_path
                    self.tool = tool_path
                    break
            if self.unoidl_write and self.tool:
                break

        # Test paths
        self.golden_dir: str = os.path.join(HERE, self.GOLDEN_DIRNAME)
        self.idl_file: str = os.path.join(HERE, "tests.idl")

        self.test_workdir: str = os.path.join(
            self.workdir, self.TOOL_NAME + "_test"
        )
        os.makedirs(self.test_workdir, exist_ok=True)

        self.types_rdb: str = os.path.join(
            self.workdir, "UnoApiTarget", "udkapi.rdb"
        )
        self.temp_rdb: str = os.path.join(self.test_workdir, "temptest.rdb")
        self.output_dir: str = os.path.join(self.test_workdir, "generated_stubs")

    def tearDown(self) -> None:
        if os.path.exists(self.test_workdir):
            shutil.rmtree(self.test_workdir, ignore_errors=True)

    # test

    def test_golden_comparison(self) -> None:
        self.assertIsNotNone(
            self.unoidl_write, "unoidl-write or %s not found in SDK" % self.TOOL_NAME
        )
        self.assertIsNotNone(self.tool, "%s not found in SDK" % self.TOOL_NAME)

        assert self.unoidl_write is not None
        assert self.tool is not None

        self.assertTrue(
            os.path.exists(self.types_rdb),
            "udkapi.rdb not found at: " + self.types_rdb,
        )
        self.assertTrue(
            os.path.exists(self.idl_file),
            "IDL file not found at: " + self.idl_file,
        )

        self._convert_idl_to_rdb()
        self._generate_stubs()
        self._compare_with_expected_files()

    # helpers

    def _run(self, cmd: List[str], what: str) -> None:
        try:
            subprocess.run(
                cmd,
                cwd=self.test_workdir,
                capture_output=True,
                text=True,
                check=True,
            )
        except subprocess.CalledProcessError as e:
            self.fail(
                "Failed to %s:\n%s\nCommand: %s"
                % (what, e.stderr or "", " ".join(cmd))
            )

    def _convert_idl_to_rdb(self) -> None:
        assert self.unoidl_write is not None
        self._run(
            [self.unoidl_write, self.types_rdb, self.idl_file, self.temp_rdb],
            "convert IDL to RDB",
        )
        self.assertTrue(
            os.path.exists(self.temp_rdb),
            "RDB file was not created by unoidl-write",
        )

    def _generate_stubs(self) -> None:
        assert self.tool is not None
        os.makedirs(self.output_dir, exist_ok=True)
        self._run(
            [self.tool, "-O", self.output_dir, self.temp_rdb, "-X", self.types_rdb],
            "generate stubs with " + self.TOOL_NAME,
        )
        self.assertGreater(
            len(self._list_files(self.output_dir)),
            0,
            self.TOOL_NAME + " produced no output files",
        )

    def _list_files(self, base: str) -> List[str]:
        """Relative paths of all relevant files below base."""
        result: List[str] = []
        for root, _, files in os.walk(base):
            for name in files:
                if self.IS_OUTPUT_FILE(name):
                    result.append(os.path.relpath(os.path.join(root, name), base))
        return result

    def _compare_with_expected_files(self) -> None:
        if not os.path.exists(self.golden_dir):
            self.fail("Expected directory does not exist: " + self.golden_dir)

        expected_files: List[str] = self._list_files(self.golden_dir)
        generated_files: List[str] = self._list_files(self.output_dir)

        expected_set: Set[str] = set(expected_files)
        generated_set: Set[str] = set(generated_files)

        missing: Set[str] = expected_set - generated_set
        extra: Set[str] = generated_set - expected_set

        if missing:
            self.fail(
                "Missing generated files (%d):\n  %s"
                % (len(missing), "\n  ".join(sorted(missing)))
            )
        if extra:
            self.fail(
                "Unexpected generated files (%d):\n  %s"
                % (len(extra), "\n  ".join(sorted(extra)))
            )

        differences: List[str] = [
            rel
            for rel in sorted(expected_files)
            if not filecmp.cmp(
                os.path.join(self.golden_dir, rel),
                os.path.join(self.output_dir, rel),
                shallow=False,
            )
        ]

        if differences:
            self.fail(
                "File content differences found (%d):\n  %s\n\n%s"
                % (
                    len(differences),
                    "\n  ".join(differences),
                    self._diff_excerpt(differences[0]),
                )
            )

    def _diff_excerpt(self, rel_path: str, max_lines: int = 40) -> str:
        """Short unified diff of the first differing file, to ease debugging."""

        def read(path: str) -> List[str]:
            with open(path, encoding="utf-8", errors="replace") as f:
                return f.readlines()

        diff = list(
            difflib.unified_diff(
                read(os.path.join(self.golden_dir, rel_path)),
                read(os.path.join(self.output_dir, rel_path)),
                fromfile="expected/" + rel_path,
                tofile="generated/" + rel_path,
            )
        )
        text = "".join(diff[:max_lines])
        if len(diff) > max_lines:
            text += "... (%d more diff lines)\n" % (len(diff) - max_lines)
        return "First difference:\n" + text


class TestCppuMaker(CodemakerGoldenTestMixin, unittest.TestCase):
    """cppumaker must generate exactly the golden .hpp/.hdl headers."""

    TOOL_NAME = "cppumaker"
    GOLDEN_DIRNAME = "expected_cppumaker_stubs"

    @staticmethod
    def IS_OUTPUT_FILE(name: str) -> bool:
        return name.lower().endswith((".hpp", ".hdl"))


class TestPythonMaker(CodemakerGoldenTestMixin, unittest.TestCase):
    """pythonmaker must generate exactly the golden .pyi stubs."""

    TOOL_NAME = "pythonmaker"
    GOLDEN_DIRNAME = "expected_pythonmaker_stubs"

    @staticmethod
    def IS_OUTPUT_FILE(name: str) -> bool:
        # pythonmaker may leave temporary files behind; ignore them
        return not name.lower().endswith(".tmp")


if __name__ == "__main__":
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
