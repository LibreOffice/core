# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import subprocess
from sys import platform


def convert_to_unix(path):
    if platform == "cygwin":
        return subprocess.check_output(["cygpath", "-u", path]).decode("utf-8", "strict").rstrip()
    else:
        return path


def convert_to_native(path):
    if platform == "cygwin":
        return subprocess.check_output(["cygpath", "-m", path]).decode("utf-8", "strict").rstrip()
    else:
        return path


class UpdaterPath(object):

    def __init__(self, workdir):
        self._workdir = convert_to_unix(workdir)

    def get_workdir(self):
        return self._workdir

    def get_update_dir(self):
        return os.path.join(self._workdir, "update-info")

    def get_current_build_dir(self):
        return os.path.join(self._workdir, "mar", "current-build")

    def get_mar_dir(self):
        return os.path.join(self._workdir, "mar")

    def get_previous_build_dir(self):
        return os.path.join(self._workdir, "mar", "previous-build")

    def get_language_dir(self):
        return os.path.join(self.get_mar_dir(), "language")

    def ensure_dir_exist(self):
        os.makedirs(self.get_update_dir(), exist_ok=True)
        os.makedirs(self.get_current_build_dir(), exist_ok=True)
        os.makedirs(self.get_mar_dir(), exist_ok=True)
        os.makedirs(self.get_previous_build_dir(), exist_ok=True)
        os.makedirs(self.get_language_dir(), exist_ok=True)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
