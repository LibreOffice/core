#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# this awk script mangles makedepend output for a single object file
# usage:
# awk -f .../processdeps.awk \
#     -v OUTDIR=outdir \
#     -v SRCDIR=srcdir \
#     -v WORKDIR=workdir \
#     -v REPODIR=repodir \
#     -v OBJECTFILE=objectfile
# called like this the script will read from stdin
# and write to stdout. It will:
#  - replace the objectfile with the one given on the commandline
#  - normalize paths to mixed paths (replacing all \ with /)
#  - replace the string given as WORKDIR with $(WORKDIR)/
#  - replace the string given as OUTDIR with $(OUTDIR)/
#  - replace the string given as SRCDIR with $(SRCDIR)/
#  - replace the string given as REPODIR with $(REPODIR)/
#  - translates absolute mixed windows paths to cygwin paths by
#    substituting a path starting with X:... to /cygdrive/X/...

function mangle_path(path) {
    gsub("\\\\", "/", path);
    if( path ~ /^[a-zA-Z]:/ )
        path = tolower(substr(path,0,1)) substr(path,2);
    gsub(WORKDIR, "$(WORKDIR)/", path);
    gsub(OUTDIR, "$(OUTDIR)/", path);
    gsub(SRCDIR, "$(SRCDIR)/", path);
    gsub(REPODIR, "$(REPODIR)/", path);
    if( path ~ /^[a-zA-Z]:/ )
        path = "/cygdrive/" tolower(substr(path,0,1)) substr(path,3);
    return path;
}

BEGIN {
   WORKDIR = tolower(substr(WORKDIR,0,1)) substr(WORKDIR,2); 
   OUTDIR = tolower(substr(OUTDIR,0,1)) substr(OUTDIR,2); 
   SRCDIR = tolower(substr(SRCDIR,0,1)) substr(SRCDIR,2); 
   REPODIR = tolower(substr(REPODIR,0,1)) substr(REPODIR,2); 
#   print "# WORKDIR=" WORKDIR;
#   print "# OUTDIR=" OUTDIR;
#   print "# SRCDIR=" SRCDIR;
#   print "# REPODIR=" REPODIR;
   print mangle_path(OBJECTFILE) ": \\";
}

/^[^#]/ {
    print "\t" mangle_path($2) " \\";
}

END {
    print "\n";
}
