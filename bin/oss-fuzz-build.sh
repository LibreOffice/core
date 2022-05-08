#!/usr/bin/env bash

set -e

if [ -z "${OUT}" ] || [ -z "${SRC}" ] || [ -z "${WORK}" ]; then
    echo "OUT, SRC or WORK not set - script expects to be called inside oss-fuzz build env"
    exit 1
fi

print_stamp()
{
    local do_df="$1"; shift
    echo "[[" `date -u` "]]" "build -" "$@"
    if [ "$do_df" != "0" ]; then
        df -h $OUT $WORK
    fi
}

print_stamp 1 start
echo git: `git -C $SRC/libreoffice log -1 --pretty=reference`

#shuffle CXXFLAGS -stdlib=libc++ arg into CXX as well because we use
#the CXX as the linker and need to pass -stdlib=libc++ to build
export CXX="$CXX -stdlib=libc++ -fsanitize-blacklist=$SRC/libreoffice/bin/sanitize-excludelist.txt"
export CXX_FOR_BUILD="$CXX"
export CC="$CC -fsanitize-blacklist=$SRC/libreoffice/bin/sanitize-excludelist.txt"
export CC_FOR_BUILD="$CC"
#similarly force the -fsanitize etc args in as well as pthread to get
#things to link successfully during the build
export LDFLAGS="$CFLAGS -Wl,--compress-debug-sections,zlib -lpthread"
#build-time concat-deps tool leaks a titch
export ASAN_OPTIONS="detect_leaks=0"

cd $WORK
if [ -f Makefile ]; then
    print_stamp 0 clean
    make clean
fi

print_stamp 0 autogen.sh
$SRC/libreoffice/autogen.sh --with-distro=LibreOfficeOssFuzz --with-external-tar=$SRC/external-tar

print_stamp 1 make
make

print_stamp 1 prepare '$OUT'

pushd instdir/program
head -c -14 services.rdb  > templateservices.rdb
tail -c +85 ./services/services.rdb >> templateservices.rdb
for a in *fuzzer; do
    #some minimal fonts required
    mv $a $OUT
    mkdir -p $OUT/$a.fonts
    tar -x -C $OUT/$a.fonts --strip-components=1 --wildcards --no-anchored '*.ttf' -f $SRC/external-tar/liberation-fonts-ttf*
    cp $SRC/external-tar/*opens___.ttf $OUT/$a.fonts
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

#starting corpuses
for zip_file in $SRC/*_seed_corpus.zip; do
    cp $zip_file $OUT
done

#fuzzing dictionaries
cp $SRC/*.dict $OUT

#options files
cp $SRC/libreoffice/vcl/workben/*.options $OUT

print_stamp 1 end
