# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import argparse
import os
import sys
from os.path import abspath, basename, isdir, join, splitext

import uno
from com.sun.star.beans import PropertyValue
from com.sun.star.connection import NoConnectException

PROG = "$OFFICE_PROGRAM_PATH/python {}".format(basename(sys.argv[0]))
SOFFICE_CONNECTION_URI = "uno:socket,host=localhost,port=2083;urp;StarOffice.ComponentContext"


def connect_soffice():
    """Connect to remote running LibreOffice"""
    local_context = uno.getComponentContext()
    resolver = local_context.ServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", local_context
    )
    try:
        remote_context = resolver.resolve(SOFFICE_CONNECTION_URI)
    except NoConnectException:
        raise Exception("Cannot establish a connection to LibreOffice.")

    return remote_context.ServiceManager.createInstanceWithContext(
        "com.sun.star.frame.Desktop", remote_context
    )


def convert(src_file, dest_file, to_type):
    src_url = "file://{}".format(src_file).replace("\\", "/")
    dest_url = "file://{}".format(dest_file).replace("\\", "/")

    soffice = connect_soffice()
    doc = soffice.loadComponentFromURL(
        src_url, "_blank", 0, (PropertyValue(Name="Hidden", Value=True),)
    )

    opts = (
        PropertyValue(Name="Overwrite", Value=True),
        PropertyValue(Name="FilterName", Value=to_type),
    )
    try:
        doc.storeAsURL(dest_url, opts)
    finally:
        doc.dispose()


def is_dir(value):
    if not isdir(value):
        raise argparse.ArgumentTypeError("{} is not a directory.".format(value))
    return value


def main():
    parser = argparse.ArgumentParser(description="Document Converter", prog=PROG)
    parser.add_argument("from_dir",
                        type=is_dir,
                        help="Convert documents searched from this directory recursively")
    parser.add_argument("to_type", help="Type to convert to, example: MS Word 97.")
    parser.add_argument("extension",
                        help="Extension of the converted document, examples: doc, docx")
    parser.add_argument("output_dir",
                        type=is_dir,
                        help="Converted document is stored into this directory")

    args = parser.parse_args()

    for dir_path, dir_names, file_names in os.walk(args.from_dir):
        for name in file_names:
            src_file = join(abspath(dir_path), name)
            dest_file = "{}.{}".format(join(args.output_dir, splitext(name)[0]), args.extension)
            convert(src_file, dest_file, args.to_type)
            print("Converted", src_file, "to", dest_file)


if __name__ == "__main__":
    main()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
