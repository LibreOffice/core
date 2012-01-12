#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

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
