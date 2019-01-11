# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import getopt
import os
import unittest
import calc_tests
import importlib
import importlib.machinery

import uitest.config

from uitest.framework import UITestCase

from libreoffice.connection import OfficeConnection

test_name_limit_found = False

def parseArgs(argv):
    (optlist,args) = getopt.getopt(argv[1:], "hdr",
            ["help", "debug", "soffice=", "userdir=", "dir=", "file=", "gdb"])
    return (dict(optlist), args)

def usage():
    message = """usage: {program} [option]... [task_file]..."
 -h | --help:      print usage information
 {connection_params}
 the 'task_file' parameters should be
  full absolute pathnames, not URLs."""
    print(message.format(program = os.path.basename(sys.argv[0]), \
        connection_params = OfficeConnection.getHelpText()))


def find_test_files(dir_path):
    valid_files = []
    for f in os.listdir(dir_path):
        file_path = os.path.join(dir_path, f)

        # don't go through the sub-directories
        if not os.path.isfile(file_path):
            continue

        # fail on any non .py files
        if not os.path.splitext(file_path)[1] == ".py":
            raise Exception("file with an extension which is not .py: " + file_path)

        # ignore the __init__.py file
        # it is obviously not a test file
        if f is "__init__.py":
            continue

        valid_files.append(file_path)

    return valid_files

def get_classes_of_module(module):
    md = module.__dict__
    return [ md[c] for c in md if (
            isinstance(md[c], type) and md[c].__module__ == module.__name__ ) ]

def get_test_case_classes_of_module(module):
    classes = get_classes_of_module(module)
    return [ c for c in classes if issubclass(c, UITestCase) ]

def add_tests_for_file(test_file, test_suite):
    test_name_limit = os.environ.get('UITEST_TEST_NAME', '')
    test_loader = unittest.TestLoader()
    module_name = os.path.splitext(os.path.split(test_file)[1])[0]

    loader = importlib.machinery.SourceFileLoader(module_name, test_file)
    mod = loader.load_module()
    classes = get_test_case_classes_of_module(mod)
    global test_name_limit_found
    for c in classes:
        test_names = test_loader.getTestCaseNames(c)
        for test_name in test_names:
            full_name = ".".join([module_name, c.__name__, test_name])
            if len(test_name_limit) > 0:
                if not test_name_limit.startswith(full_name):
                    continue
                test_name_limit_found = True

            obj = c(test_name, opts)
            test_suite.addTest(obj)

def get_test_suite_for_dir(opts):
    test_suite = unittest.TestSuite()

    valid_test_files = find_test_files(opts['--dir'])
    for test_file in valid_test_files:
        add_tests_for_file(test_file, test_suite)
    return test_suite


if __name__ == '__main__':
    (opts,args) = parseArgs(sys.argv)
    if "-h" in opts or "--help" in opts:
        usage()
        sys.exit()
    elif not "--soffice" in opts:
        usage()
        sys.exit(1)
    elif "--dir" in opts:
        test_suite = get_test_suite_for_dir(opts)
        test_name_limit = os.environ.get('UITEST_TEST_NAME', '')
        print(test_name_limit_found)
        if len(test_name_limit) > 0 and not test_name_limit_found:
            print("UITEST_TEST_NAME '%s' does not match any test" % test_name_limit)
            sys.exit(1)
    elif "--file" in opts:
        test_suite = unittest.TestSuite()
        add_tests_for_file(opts['--file'], test_suite)
    else:
        usage()
        sys.exit()

    if "-d" in opts or "--debug" in opts:
        uitest.config.use_sleep = True

    result = unittest.TextTestRunner(stream=sys.stdout, verbosity=2).run(test_suite)
    print("Tests run: %d" % result.testsRun)
    print("Tests failed: %d" % len(result.failures))
    print("Tests errors: %d" % len(result.errors))
    print("Tests skipped: %d" % len(result.skipped))
    if not result.wasSuccessful():
        sys.exit(1)
    sys.exit(0)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
