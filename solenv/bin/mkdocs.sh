#!/bin/bash
#
# Doxygen Doc generation
#

# binaries that we need
which doxygen > /dev/null 2>&1 || {
    echo "You need doxygen for doc generation"
    exit 1
}
which dot > /dev/null 2>&1 || {
    echo "You need the graphviz tools to create the nice inheritance graphs"
    exit 1
}

# otherwise, aliases are not expanded below
shopt -s expand_aliases

# Title of the documentation
DOXYGEN_PROJECT_PREFIX="LibreOffice"

# suck setup
BINDIR=`dirname $0`
. $BINDIR/setup

. ./*.Set.sh

# get list of modules in build order - bah, blows RAM & disk, static list below
INPUT_PROJECTS="o3tl basegfx basebmp comphelper svl vcl canvas cppcanvas oox svtools goodies drawinglayer xmloff slideshow sfx2 editeng svx cui chart2 dbaccess sd starmath sc sw"

# output directory for generated documentation
BASE_OUTPUT="$1"
mkdir -p "$BASE_OUTPUT" || {
    echo "Cannot create $BASE_OUTPUT"
    exit 1
}

# paths for binary and configuration file
BASE_PATH=`pwd`
DOXYGEN_CFG="$2"
if test ! -f "$DOXYGEN_CFG"; then
    echo "doxygen.cfg not found"
    exit 1
fi

# strip -I. and bin -I prefix; exlude system headers
DOXYGEN_INCLUDE_PATH=`echo $SOLARINC | sed -e ' s/-I\.//'g | sed -e ' s/ -I/ /'g | sed -e ' s|/usr/[^ ]*| |g'`

# setup version string
DOXYGEN_VERSION="$GITTAG"


###################################################
#
# Generate docs
#
###################################################

# cleanup
rm -rf $BASE_OUTPUT/*

# make the stuff world-readable
umask 022

# generate docs
DOXYGEN_REF_TAGFILES=""
for PROJECT in $INPUT_PROJECTS;
do
  # avoid processing of full project subdirs, only add source and inc
  DOXYGEN_INPUT=`printf "%s" "$PROJECT/source $PROJECT/inc "`

  DOXYGEN_OUTPUT="$BASE_OUTPUT/$PROJECT"
  DOXYGEN_OUR_TAGFILE="$DOXYGEN_OUTPUT/$PROJECT.tags"
  DOXYGEN_PROJECTNAME="$DOXYGEN_PROJECT_PREFIX Module $PROJECT"

  # export variables referenced in doxygen config file
  export DOXYGEN_INPUT
  export DOXYGEN_OUTPUT
  export DOXYGEN_INCLUDE_PATH
  export DOXYGEN_VERSION
  export DOXYGEN_OUR_TAGFILE
  export DOXYGEN_REF_TAGFILES
  export DOXYGEN_PROJECTNAME

  # debug
  echo "Calling $DOXYGEN_PATH/doxygen $DOXYGEN_CFG with"
  echo "Input:      $DOXYGEN_INPUT"
  echo "Output:     $DOXYGEN_OUTPUT"
  echo "Include:    $DOXYGEN_INCLUDE_PATH"
  echo "Version:    $DOXYGEN_VERSION"
  echo "Tagfile:    $DOXYGEN_OUR_TAGFILE"
  echo "Ref-Tags:   $DOXYGEN_REF_TAGFILES"
  echo "Title:      $DOXYGEN_PROJECTNAME"

  nice -15 doxygen "$DOXYGEN_CFG"

  # setup referenced tagfiles for next round
  DOXYGEN_REF_TAGFILES="$DOXYGEN_REF_TAGFILES $DOXYGEN_OUR_TAGFILE=$BASE_URL/$PROJECT/html"
done

# generate entry page
cat - > $BASE_OUTPUT/index.html <<EOF
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<html>
    <head>
        <title>LibreOffice Source Code Documentation (fragmentary)</title>
    </head>
    <body>
        <h1>LibreOffice Source Code Documentation (fragmentary)</h1>
        <ul>
EOF

for PROJECT in $INPUT_PROJECTS;
do
  echo "<li><a href=\"$PROJECT/html/classes.html\">$PROJECT</a></li>" >> $BASE_OUTPUT/index.html
done

cat - >> $BASE_OUTPUT/index.html <<EOF
        </ul>
        <p>Last updated:
EOF
LANG= date >> $BASE_OUTPUT/index.html

cat - >> $BASE_OUTPUT/index.html <<EOF
        </p>
    </body>
</html>
EOF

## done
