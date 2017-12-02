#!/bin/bash -e

if [ -z "${OUT}" ] || [ -z "${SRC}" ] || [ -z "${WORK}" ]; then
    echo "OUT, SRC or WORK not set - script expects to be called inside oss-fuzz build env"
    exit 1
fi

#shuffle CXXFLAGS -stdlib=libc++ arg into CXX as well because we use
#the CXX as the linker and need to pass -stdlib=libc++ to build
export CXX="$CXX -stdlib=libc++ -fsanitize-blacklist=$SRC/libreoffice/bin/sanitize-blacklist.txt"
#similarly force the -fsanitize etc args in as well as pthread to get
#things to link successfully during the build
export LDFLAGS="$CFLAGS -lpthread"

cd $WORK
$SRC/libreoffice/autogen.sh --with-distro=LibreOfficeOssFuzz --with-external-tar=$SRC

#build-time rsc tool leaks a titch
export ASAN_OPTIONS="detect_leaks=0"

make fuzzers

#some minimal fonts required
cp $SRC/libreoffice/extras/source/truetype/symbol/opens___.ttf instdir/share/fonts/truetype/Liberation* $OUT
#minimal runtime requirements
rm -rf $OUT/services $OUT/types $OUT/*rdb
mkdir $OUT/services
pushd instdir/program
cp -r *fuzzer *rc types.rdb types $OUT
head -c -14 services.rdb  > $OUT/services.rdb
tail -c +85 ./services/services.rdb >> $OUT/services.rdb
popd

#starting corpuses
cp $SRC/*_seed_corpus.zip $OUT
#fuzzing dictionaries
cp $SRC/gif.dict $OUT/giffuzzer.dict
cp $SRC/jpeg.dict $OUT/jpgfuzzer.dict
cp $SRC/png.dict $OUT/pngfuzzer.dict
cp $SRC/tiff.dict $OUT/tiffuzzer.dict
cp $SRC/xml.dict $OUT/fodtfuzzer.dict
cp $SRC/xml.dict $OUT/fodsfuzzer.dict
cp $SRC/xml.dict $OUT/fodpfuzzer.dict
cp $SRC/xml.dict $OUT/fodgfuzzer.dict
cp $SRC/xml.dict $OUT/mmlfuzzer.dict
