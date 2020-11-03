#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""
Script to generate LibreOffice bash_completion file for the main applications
"""

import argparse
import sys

MASTERDOCS = ["sxg", "odm", "sgl"]

BASEDOCS = ["odb"]

CALCDOCS = ["sxc", "stc", "dif", "dbf", "xls", "xlw", "xlt", "rtf", "sdc", "vor",
            "slk", "txt", "htm", "html", "wk1", "wks", "123", "xml", "ods", "ots",
            "fods", "csv", "xlsb", "xlsm", "xlsx", "xltm", "xltx"]

DRAWDOCS = ["sxd", "std", "dxf", "emf", "eps", "met", "pct", "sgf", "sgv", "sda",
            "sdd", "vor", "svm", "wmf", "bmp", "gif", "jpg", "jpeg", "jfif", "fif",
            "jpe", "pcd", "pcx", "pgm", "png", "ppm", "psd", "ras", "tga", "tif",
            "tiff", "xbm", "xpm", "odg", "otg", "fodg", "odc", "odi", "sds",
            "wpg", "svg", "vdx", "vsd", "vsdm", "vsdx", "pdf"]

IMPRESSDOCS = ["sxi", "sti", "ppt", "pps", "pot", "sxd", "sda", "sdd", "sdp",
               "vor", "cgm", "odp", "otp", "fodp", "ppsm", "ppsx", "pptm", "pptx",
               "potm", "potx"]

MATHDOCS = ["sxm", "smf", "mml", "odf"]

WEBDOCS = ["htm", "html", "stw", "txt", "vor", "oth"]

WRITERDOCS = ["doc", "dot", "rtf", "sxw", "stw", "sdw", "vor", "txt", "htm?",
              "xml", "wp", "wpd", "wps", "odt", "ott", "fodt", "docm", "docx",
              "dotm", "dotx"]

TEMPLATES = ["stw", "dot", "vor", "stc", "xlt", "sti", "pot", "std", "stw",
             "dotm", "dotx", "potm", "potx", "xltm", "xltx"]

ALLDOCS = MASTERDOCS + BASEDOCS + CALCDOCS + DRAWDOCS + IMPRESSDOCS + MATHDOCS + WEBDOCS + WRITERDOCS + TEMPLATES

EXTENSIONS = ["oxt"]


class App(object):
    def __init__(self, name, compat_name, suffix_list):
        self.name = name
        self.compat_name = compat_name
        self.suffix_list = suffix_list


class SetAppCompatName(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest, True)
        for app in APPS.values():
            app.name = app.compat_name


class SetAppName(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        APPS[self.dest].name = values


# default names of lowrappers
# use "" for name if you want to disable any wrapper
APPS = {
    'office': App("libreoffice", 'openoffice', ALLDOCS),  # libreoffice should contain all
    'office_short': App("loffice", 'ooffice', ALLDOCS),  # libreoffice should contain all
    'master': App("", '', MASTERDOCS),
    'base': App("lobase", 'oobase', BASEDOCS),
    'calc': App("localc", 'oocalc', CALCDOCS),
    'draw': App("lodraw", 'oodraw', DRAWDOCS),
    'impress': App("loimpress", 'ooimpress', IMPRESSDOCS),
    'math': App("lomath", 'oomath', MATHDOCS),
    'template': App("lofromtemplate", 'oofromtemplate', TEMPLATES),
    'unopkg': App("unopkg", 'unopkg', EXTENSIONS),  # unopkg is a standalone tool
    'web': App("loweb", 'ooweb', WEBDOCS),
    'writer': App("lowriter", 'oowriter', WRITERDOCS + MASTERDOCS)
}


def check_open(filename, mode):
    try:
        with open(filename, mode):
            pass
    except OSError as e:
        mode = 'reading' if mode == 'r' else 'writing'
        sys.exit("Error: can't open %s for %s: %s" % (filename, mode, e))


def print_app_suffixes_check(out, app):
    if not app.suffix_list:
        sys.exit('Error: No suffix defined for %s' % app.name)

    suffix_str = '|'.join(['%s|%s' % (s, s.upper()) for s in app.suffix_list])
    out.write("    %s)\t\te=\'!*.+(%s)\' ;;\n" % (app.name, suffix_str))


def print_suffixes_check(out):
    for app in APPS.values():
        if not app.name:  # skip the disabled wrapper
            continue
        print_app_suffixes_check(out, app)


def main():
    parser = argparse.ArgumentParser(description='Script to Generate bash completion for LO wrappers',
                                     epilog='The other options allows to redefine the wrapper names.\n'
                                            'The value "" can be used to disable any wrapper.',
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('input_file')
    parser.add_argument('output_file')
    parser.add_argument('--binsuffix', metavar='suffix',
                        help='defines a suffix that is added after each wrapper')
    parser.add_argument('--compat-oowrappers', metavar='', nargs=0, action=SetAppCompatName, default=False,
                        help='set wrapper names to the old default oo* wrapper names')
    for app in APPS:
        parser.add_argument('--%s' % app, metavar='wrapper_name', action=SetAppName)

    args = parser.parse_args()

    check_open(args.input_file, 'r')
    check_open(args.output_file, 'w')

    # add binsuffix
    if args.binsuffix:
        for app in APPS.values():
            if app.name:
                app.name += args.binsuffix

    if args.compat_oowrappers:
        office_shell_function = '_ooexp_'
    else:
        office_shell_function = '_loexp_'

    # the last app will be printed without the final backslash
    apps_to_print = ' \\\n'.join(['\t\t\t\t\t%s' % app.name for app in APPS.values() if app.name])

    with open(args.input_file, 'r') as in_fh, open(args.output_file, 'w') as out_fh:
        for line in in_fh:
            line = line.replace('@OFFICE_SHELL_FUNCTION@', office_shell_function)
            if '@BASH_COMPLETION_SUFFIXES_CHECKS@' in line:
                print_suffixes_check(out_fh)
            elif '@BASH_COMPLETION_OOO_APPS@' in line:
                if not apps_to_print:
                    sys.exit('Error: No LO wrapper was selected')
                out_fh.write('%s\n' % apps_to_print)
            else:
                out_fh.write(line)


if __name__ == '__main__':
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
