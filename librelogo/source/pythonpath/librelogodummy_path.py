# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import uno, re, sys, os, traceback
def get_path():
    return os.path.join(os.path.dirname(sys.modules[__name__].__file__), __name__ + ".py")

# vim: set noet sw=4 ts=4:
