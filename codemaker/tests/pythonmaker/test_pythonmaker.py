#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import os
import subprocess
import shutil
import filecmp
import glob
import sys

from typing import Optional, List, Set


class TestPythonMaker(unittest.TestCase):
    """Test pythonmaker tool by comparing generated stubs with expected files"""

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
        self.pythonmaker: Optional[str] = None

        sdk_patterns: List[str] = [
            os.path.join(self.instdir, "sdk", "bin"),
            os.path.join(self.instdir, "LibreOffice*_SDK", "bin"),
        ]

        exe_suffix: str = ".exe" if os.name == "nt" else ""

        for pattern in sdk_patterns:
            for sdk_dir in glob.glob(pattern):
                unoidl_path = os.path.join(sdk_dir, "unoidl-write" + exe_suffix)
                pythonmaker_path = os.path.join(sdk_dir, "pythonmaker" + exe_suffix)

                if os.path.exists(unoidl_path) and os.path.exists(pythonmaker_path):
                    self.unoidl_write = unoidl_path
                    self.pythonmaker = pythonmaker_path
                    break

            if self.unoidl_write and self.pythonmaker:
                break

        # Program directory
        self.program_dir: Optional[str]

        if sys.platform == "darwin":
            self.program_dir = None
            app_patterns: List[str] = [
                os.path.join(
                    self.instdir,
                    "LibreOfficeDev.app",
                    "Contents",
                    "Resources",
                ),
                os.path.join(
                    self.instdir,
                    "LibreOffice.app",
                    "Contents",
                    "Resources",
                ),
            ]
            for pattern in app_patterns:
                if os.path.exists(pattern):
                    self.program_dir = pattern
                    break
        else:
            self.program_dir = os.path.join(self.instdir, "program")

        # Test paths
        self.golden_dir: str = os.path.join(
            self.srcdir,
            "codemaker",
            "tests",
            "pythonmaker",
            "expected_pyi_stubs",
        )

        self.idl_file: str = os.path.join(
            self.srcdir,
            "codemaker",
            "tests",
            "pythonmaker",
            "idl",
            "pythontypes.idl",
        )

        self.test_workdir: str = os.path.join(self.workdir, "pythonmaker_test")
        os.makedirs(self.test_workdir, exist_ok=True)

        self.types_rdb: str = os.path.join(
            self.workdir, "UnoApiTarget", "udkapi.rdb"
        )

        self.temp_rdb: str = os.path.join(self.test_workdir, "temptest.rdb")
        self.output_dir: str = os.path.join(self.test_workdir, "generated_stubs")

    def tearDown(self) -> None:
        if os.path.exists(self.test_workdir):
            shutil.rmtree(self.test_workdir, ignore_errors=True)

    def test_pythonmaker_golden_comparison(self) -> None:
        self.assertIsNotNone(self.unoidl_write)
        self.assertIsNotNone(self.pythonmaker)
        self.assertIsNotNone(self.program_dir)

        assert self.unoidl_write is not None
        assert self.pythonmaker is not None

        self.assertTrue(os.path.exists(self.unoidl_write))
        self.assertTrue(os.path.exists(self.pythonmaker))
        self.assertTrue(os.path.exists(self.types_rdb))

        self._convert_idl_to_rdb()
        self._generate_python_stubs()
        self._compare_with_expected_files()

    def _convert_idl_to_rdb(self) -> None:
        assert self.unoidl_write is not None

        cmd: List[str] = [
            self.unoidl_write,
            self.types_rdb,
            self.idl_file,
            self.temp_rdb,
        ]

        try:
            subprocess.run(
                cmd,
                cwd=self.test_workdir,
                capture_output=True,
                text=True,
                check=True,
            )
            self.assertTrue(os.path.exists(self.temp_rdb))
        except subprocess.CalledProcessError as e:
            self.fail(
                "Failed to convert IDL to RDB:\n"
                + (e.stderr or "")
                + "\nCommand: "
                + " ".join(cmd)
            )

    def _generate_python_stubs(self) -> None:
        assert self.pythonmaker is not None

        os.makedirs(self.output_dir, exist_ok=True)

        cmd: List[str] = [
            self.pythonmaker,
            "-O",
            self.output_dir,
            self.temp_rdb,
            "-X",
            self.types_rdb,
        ]

        try:
            subprocess.run(
                cmd,
                cwd=self.test_workdir,
                capture_output=True,
                text=True,
                check=True,
            )

            generated_files: List[str] = []
            for _, _, files in os.walk(self.output_dir):
                for name in files:
                    if not name.lower().endswith(".tmp"):
                        generated_files.append(name)

            self.assertGreater(len(generated_files), 0)

        except subprocess.CalledProcessError as e:
            self.fail(
                "Failed to generate Python stubs:\n"
                + (e.stderr or "")
                + "\nCommand: "
                + " ".join(cmd)
            )

    def _compare_with_expected_files(self) -> None:
        if not os.path.exists(self.golden_dir):
            self.fail("Expected directory does not exist: " + self.golden_dir)

        expected_files: List[str] = []
        for root, _, files in os.walk(self.golden_dir):
            for name in files:
                expected_files.append(
                    os.path.relpath(os.path.join(root, name), self.golden_dir)
                )

        generated_files: List[str] = []
        for root, _, files in os.walk(self.output_dir):
            for name in files:
                if not name.lower().endswith(".tmp"):
                    generated_files.append(
                        os.path.relpath(os.path.join(root, name), self.output_dir)
                    )

        expected_set: Set[str] = set(expected_files)
        generated_set: Set[str] = set(generated_files)

        missing = expected_set - generated_set
        extra = generated_set - expected_set

        if missing:
            self.fail("Missing generated files: " + str(missing))
        if extra:
            self.fail("Unexpected generated files: " + str(extra))

        differences: List[str] = []
        for rel_path in expected_files:
            if not filecmp.cmp(
                os.path.join(self.golden_dir, rel_path),
                os.path.join(self.output_dir, rel_path),
                shallow=False,
            ):
                differences.append(rel_path)

        if differences:
            self.fail("File content differences found in: " + str(differences))


if __name__ == "__main__":
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab: