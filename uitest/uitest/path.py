# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
from urllib.parse import urljoin
from urllib.request import pathname2url

def get_src_dir_fallback():
    current_dir = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(current_dir, "../../"))

def path2url(path):
        return urljoin('file:', pathname2url(os.path.normpath(path)))

def get_workdir_url():
    workdir_path = os.environ.get('WORKDIR', os.path.join(get_src_dir_fallback(), 'workdir'))
    return path2url(workdir_path)

def get_srcdir_url():
    srcdir_path = os.environ.get('SRCDIR', get_src_dir_fallback())
    return path2url(srcdir_path)

def get_instdir_url():
    instdir_path = os.environ.get('INSTDIR', os.path.join(get_src_dir_fallback(), 'instdir'))
    return path2url(instdir_path)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
