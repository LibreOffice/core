#!/usr/bin/env bash

set -e

if [ -z "${OUT}" ] || [ -z "${SRC}" ] || [ -z "${WORK}" ]; then
    echo "OUT, SRC or WORK not set - script expects to be called inside oss-fuzz build env"
    exit 1
fi

echo start at `date -u`

#shuffle CXXFLAGS -stdlib=libc++ arg into CXX as well because we use
#the CXX as the linker and need to pass -stdlib=libc++ to build
export CXX="$CXX -stdlib=libc++ -fsanitize-blacklist=$SRC/libreoffice/bin/sanitize-excludelist.txt"
export CXX_FOR_BUILD="$CXX"
export CC="$CC -fsanitize-blacklist=$SRC/libreoffice/bin/sanitize-excludelist.txt"
export CC_FOR_BUILD="$CC"
#similarly force the -fsanitize etc args in as well as pthread to get
#things to link successfully during the build
export LDFLAGS="$CFLAGS -Wl,--compress-debug-sections,zlib -lpthread"
#build-time rsc tool leaks a titch
export ASAN_OPTIONS="detect_leaks=0"

df -h $OUT $WORK

cd $WORK
$SRC/libreoffice/autogen.sh --with-distro=LibreOfficeOssFuzz --with-external-tar=$SRC

make clean

make

pushd instdir/program
head -c -14 services.rdb  > templateservices.rdb
tail -c +85 ./services/services.rdb >> templateservices.rdb
for a in *fuzzer; do
    #some minimal fonts required
    mv $a $OUT
    mkdir -p $OUT/$a.fonts
    tar -x -C $OUT/$a.fonts --strip-components=1 --wildcards --no-anchored '*.ttf' -f $SRC/liberation-fonts-ttf*
    cp $SRC/*opens___.ttf $OUT/$a.fonts
    #minimal runtime requirements
    cp templateservices.rdb $OUT/$a.services.rdb
    cp types.rdb $OUT/$a.types.rdb
    cp types/offapi.rdb $OUT/$a.moretypes.rdb
    cat > $OUT/$a.unorc << EOF
[Bootstrap]
URE_INTERNAL_LIB_DIR=\${ORIGIN}
UNO_TYPES=\${ORIGIN}/$a.types.rdb \${ORIGIN}/$a.moretypes.rdb
UNO_SERVICES=\${ORIGIN}/$a.services.rdb
EOF
done
popd

df -h $OUT $WORK

#starting corpuses
for zip_file in $SRC/*_seed_corpus.zip; do
    cp $zip_file $OUT
done
#fuzzing dictionaries
cp $SRC/*.dict $OUT
#options files
cp $SRC/libreoffice/vcl/workben/*.options $OUT

echo end at `date -u`
