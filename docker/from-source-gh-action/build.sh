#! /bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# -- Available env vars --
# * ENGINE_ASSETS  - URL of prebuilt engine assets tarball (skips building engine)
# * COLLABORA_ONLINE_REPO - which git repo to clone the online monorepo from
# * COLLABORA_ONLINE_BRANCH - which branch to build
# * ENGINE_BUILD_TARGET - which make target to run for the engine (when building from source)
# * ONLINE_EXTRA_BUILD_OPTIONS - extra build options for online

if [ -z "$COLLABORA_ONLINE_REPO" ]; then
  COLLABORA_ONLINE_REPO="https://gerrit.collaboraoffice.com/online"
fi;
if [ -z "$COLLABORA_ONLINE_BRANCH" ]; then
  COLLABORA_ONLINE_BRANCH="main"
fi;
echo "Building branch '$COLLABORA_ONLINE_BRANCH' from '$COLLABORA_ONLINE_REPO'"

if [ -z "$ENGINE_ASSETS" ]; then
  echo "Building engine from source"
else
  echo "Using prebuilt engine assets from $ENGINE_ASSETS"
fi;

if [ -z "$ENGINE_BUILD_TARGET" ]; then
  ENGINE_BUILD_TARGET=""
fi;
echo "Engine build target: '$ENGINE_BUILD_TARGET'"

SRCDIR=$(realpath `dirname $0`)
INSTDIR="$SRCDIR/instdir"
BUILDDIR="$SRCDIR/builddir"

mkdir -p "$BUILDDIR"
cd "$BUILDDIR"

rm -rf "$INSTDIR" || true
mkdir -p "$INSTDIR"

##### build static poco #####

if test ! -f poco/lib/libPocoFoundation.a ; then
    wget https://pocoproject.org/releases/poco-1.12.5p2/poco-1.12.5p2-all.tar.gz
    tar -xzf poco-1.12.5p2-all.tar.gz
    cd poco-1.12.5p2-all/
    ./configure --static --no-tests --no-samples --no-sharedlibs --cflags="-fPIC" --omit=Zip,Data,Data/SQLite,Data/ODBC,Data/MySQL,MongoDB,PDF,CppParser,PageCompiler,Redis,Encodings,ActiveRecord --prefix=$BUILDDIR/poco
    make -j $(nproc)
    make install
    cd ..
fi


##### cloning & updating #####

# Clone the online monorepo (engine/ contains the rendering engine)
if test ! -d online ; then
  git clone --depth=1 --branch $COLLABORA_ONLINE_BRANCH "$COLLABORA_ONLINE_REPO" online || exit 1
fi

( cd online && git fetch --all && git checkout -f $COLLABORA_ONLINE_BRANCH && git clean -f -d && git pull -r ) || exit 1

##### engine #####

if [ -z "$ENGINE_ASSETS" ]; then
  # build engine from source
  ( cd online/engine && ./autogen.sh --with-distro=CPLinux-LOKit --disable-epm --without-package-format --disable-symbols ) || exit 1
  ( cd online/engine && make $ENGINE_BUILD_TARGET ) || exit 1
else
  # drop in prebuilt engine assets
  ( cd online/engine && wget "$ENGINE_ASSETS" -O engine-assets.tar.xz && tar -xzf engine-assets.tar.xz && rm engine-assets.tar.xz ) || exit 1
fi

mkdir -p "$INSTDIR"/opt/
cp -a online/engine/instdir "$INSTDIR"/opt/collaboraoffice

##### coolwsd & cool #####

# build
( cd online && ./autogen.sh ) || exit 1
( cd online && ./configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var --enable-silent-rules --disable-tests --with-lokit-path="$BUILDDIR"/online/engine/include --with-lo-path=/opt/collaboraoffice --with-poco-includes=$BUILDDIR/poco/include --with-poco-libs=$BUILDDIR/poco/lib $ONLINE_EXTRA_BUILD_OPTIONS) || exit 1
( cd online && make -j $(nproc)) || exit 1

# copy stuff
( cd online && DESTDIR="$INSTDIR" make install ) || exit 1

# Build online branding if available
if test -d online-branding ; then
  if ! which sass &> /dev/null; then npm install -g sass; fi
  cd online-branding
  ./brand.sh $INSTDIR/opt/collaboraoffice $INSTDIR/usr/share/coolwsd/browser/dist CODE # CODE
  ./brand.sh $INSTDIR/opt/collaboraoffice $INSTDIR/usr/share/coolwsd/browser/dist NC-theme-community # Nextcloud Office
  cd ..
fi
