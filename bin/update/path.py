# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import errno

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

class UpdaterPath(object):

    def __init__(self, workdir):
        self._workdir = workdir

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
        mkdir_p(self.get_update_dir())
        mkdir_p(self.get_current_build_dir())
        mkdir_p(self.get_mar_dir())
        mkdir_p(self.get_previous_build_dir())
        mkdir_p(self.get_language_dir())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
