# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import getopt
import os
import importlib

from connection import PersistentConnection, OfficeConnection

def load_test(name):
    module_name, obj_name = name.rsplit(".", 1)
    module = importlib.import_module(module_name)
    obj = getattr(module, obj_name)
    return obj

def generic_test(opts, test_name):
    connection = PersistentConnection(opts)
    connection.setUp()
    xContext = connection.getContext()
    func = load_test(test_name)
    func(xContext)
    connection.tearDown()

def parseArgs(argv):
    (optlist,args) = getopt.getopt(argv[1:], "hr",
            ["help", "soffice=", "userdir=", "calc-demo", "file="])
    return (dict(optlist), args)

def usage():
    message = """usage: {program} [option]... [task_file]..."
 -h | --help:      print usage information
 {connection_params}
 the 'task_file' parameters should be
  full absolute pathnames, not URLs."""
    print(message.format(program = os.path.basename(sys.argv[0]), \
        connection_params = OfficeConnection.getHelpText()))

if __name__ == "__main__":
    (opts,args) = parseArgs(sys.argv)
    if "-h" in opts or "--help" in opts:
        usage()
        sys.exit()
    elif not "--soffice" in opts:
        usage()
        sys.exit(1)
    elif "--file" in opts:
        file_name = opts["--file"]
        with open(file_name) as f:
            lines = f.readlines()
            for line in lines:
                line = line.strip()
                generic_test(opts, line)

    elif "--calc-demo" in opts:
        generic_test(opts, "calc_tests.about_test.test_about_dlg")
        generic_test(opts, "calc_tests.create_range_name.create_range_name")
    else:
        usage()
        sys.exit(1)

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
