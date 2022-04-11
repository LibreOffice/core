#!/usr/bin/gawk -f
# -*- tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Create dependency information from the output of cl.exe's showInclude.  It
# needs additional information - the output name to which to write, objfile
# that depends on the includes, and the original file name.
# For best results all arguments should be absolute paths.
#
# It also consolidates the file names to a canonical form, and filters out
# duplicates.
#
# based on filter-showInclude.pl by Jan Holesovsky <kendy@suse.cz>

BEGIN {
    if (!depfile || !objectfile || !sourcefile) {
        print "usage: filter-showIncludes.awk -vdepfile=depfile.d " \
          "-vobjectfile=objfile.o -vsourcefile=source.cxx" > "/dev/stderr"
        exit 1
    }
    tempfile = depfile ".tmp"
    print objectfile " : \\\n " sourcefile " \\" > tempfile

    showincludes_prefix = ENVIRON["SHOWINCLUDES_PREFIX"];
    if (!showincludes_prefix) {
        showincludes_prefix = "Note: including file:"
    }

    # to match especially drive letters in allowlist case insensitive
    IGNORECASE = 1
    allowlist = \
        "^(" ENVIRON["SRCDIR"] "|" ENVIRON["BUILDDIR"] ")"
    firstline = 1
}

{
    sub(/\r$/, "")
    sub(/^ */, "")
    if (index($0, showincludes_prefix) == 1) {
        $0 = substr($0, length(showincludes_prefix) + 1)
        sub(/^ */, "")

        # The output from MSVC may contain a carriage return character at the
        # end of filenames, in which case the translation unit will depend on a
        # non-existing header, resulting in constant rebuild of all files,
        # prevent that.
        sub(//, "")

        gsub(/\\/, "/")
        gsub(/ /, "\\ ")
        if ($0 ~ allowlist) { # filter out system headers
            if (!($0 in incfiles)) {
                incfiles[$0]
                print " " $0 " \\" > tempfile
            }
        }
    } else {
        # because MSVC stupidly prints errors on stdout, it's
        # necessary to forward everything that isn't matched by the pattern
        # so users get to see them.
        if (firstline) { # ignore the line that just prints name of sourcefile
            firstline = 0
        } else {
            print $0 > "/dev/stderr"
        }
    }
}

END {
    if (!tempfile) {
        exit 1
    }
    print "" > tempfile

    # fdo#40099 if header.h does not exist, it will simply be considered out of
    # date and any targets that use it as a prerequisite will be updated,
    # which avoid misery when the header is deliberately deleted and removed
    # as an include
    # see http://www.makelinux.net/make3/make3-CHP-8-SECT-3
    for (file in incfiles) {
        print file " :\n" > tempfile
    }

    close(tempfile)
    movecmd = "mv " tempfile " " depfile
    ret = system(movecmd)
    if (ret) {
        print "ERROR: " movecmd " FAILED with status " ret > "/dev/stderr"
        exit ret
    }
}

# vim: set noet sw=4 ts=4:
