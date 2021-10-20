#!/usr/bin/env bash

set -e

echo start downloading dependencies at `date -u`

cd $SRC

#cache build dependencies
curl --no-progress-meter -S \
    -C - -O https://dev-www.libreoffice.org/src/liberation-fonts-ttf-2.1.4.tar.gz \
    -C - -O https://dev-www.libreoffice.org/extern/f543e6e2d7275557a839a164941c0a86e5f2c3f2a0042bfc434c88c6dde9e140-opens___.ttf \
    -C - -O https://dev-www.libreoffice.org/src/1f5def51ca0026cd192958ef07228b52-rasqal-0.9.33.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/5ade6ae2a99bc1e9e57031ca88d36dad-hyphen-2.8.8.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/a39f6c07ddb20d7dd2ff1f95fa21e2cd-raptor2-2.0.15.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/a8c2c5b8f09e7ede322d5c602ff6a4b6-mythes-1.2.4.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/e5be03eda13ef68aabab6e42aa67715e-redland-1.0.17.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/boost_1_76_0.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/box2d-2.3.1.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/dtoa-20180411.tgz \
    -C - -O https://dev-www.libreoffice.org/src/expat-2.4.1.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/libcuckoo-93217f8d391718380c508a722ab9acd5e9081233.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/libjpeg-turbo-1.5.3.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/lcms2-2.12.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/libexttextcat-3.4.5.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/cppunit-1.15.1.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/cairo-1.17.4.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/curl-7.79.1.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/xmlsec1-1.2.32.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/liblangtag-0.6.2.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/libabw-0.1.3.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libcdr-0.1.7.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libcmis-0.5.2.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libe-book-0.1.3.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libetonyek-0.1.10.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libfreehand-0.1.2.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libmspub-0.1.4.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libmwaw-0.3.20.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libodfgen-0.1.8.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/liborcus-0.16.1.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/libpagemaker-0.0.4.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libpng-1.6.37.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/librevenge-0.0.4.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/libstaroffice-0.0.7.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libvisio-0.1.7.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libwpd-0.10.3.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libwpg-0.3.3.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libwps-0.4.12.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libzmf-0.0.2.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/pixman-0.40.0.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/zlib-1.2.11.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/mdds-1.7.0.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/openssl-1.1.1l.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/language-subtag-registry-2021-08-06.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/graphite2-minimal-1.3.14.tgz \
    -C - -O https://dev-www.libreoffice.org/src/harfbuzz-2.8.2.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/glm-0.9.9.7.zip \
    -C - -O https://dev-www.libreoffice.org/src/icu4c-69_1-src.tgz \
    -C - -O https://dev-www.libreoffice.org/src/icu4c-69_1-data.zip \
    -C - -O https://dev-www.libreoffice.org/src/libxml2-2.9.12.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/libxslt-1.1.34.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/hunspell-1.7.0.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/freetype-2.9.1.tar.bz2 \
    -C - -O https://dev-www.libreoffice.org/src/fontconfig-2.13.91.tar.gz \
    -C - -O https://dev-www.libreoffice.org/src/libepoxy-1.5.9.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libepubgen-0.1.1.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libnumbertext-1.0.7.tar.xz \
    -C - -O https://dev-www.libreoffice.org/src/libqxp-0.0.2.tar.xz
#fuzzing dictionaries
curl --no-progress-meter -S \
    -C - -O https://raw.githubusercontent.com/rc0r/afl-fuzz/master/dictionaries/gif.dict \
    -C - -O https://raw.githubusercontent.com/rc0r/afl-fuzz/master/dictionaries/jpeg.dict \
    -C - -O https://raw.githubusercontent.com/rc0r/afl-fuzz/master/dictionaries/png.dict \
    -C - -O https://raw.githubusercontent.com/rc0r/afl-fuzz/master/dictionaries/tiff.dict \
    -C - -O https://raw.githubusercontent.com/rc0r/afl-fuzz/master/dictionaries/xml.dict \
    -C - -O https://raw.githubusercontent.com/rc0r/afl-fuzz/master/dictionaries/html_tags.dict
#fuzzing corpuses
curl --no-progress-meter -S -C - -O https://lcamtuf.coredump.cx/afl/demo/afl_testcases.tgz
mkdir -p afl-testcases && cd afl-testcases/ && tar xf $SRC/afl_testcases.tgz && cd .. && \
    zip -q $SRC/jpgfuzzer_seed_corpus.zip afl-testcases/jpeg*/full/images/* && \
    zip -q $SRC/giffuzzer_seed_corpus.zip afl-testcases/gif*/full/images/* && \
    zip -q $SRC/bmpfuzzer_seed_corpus.zip afl-testcases/bmp*/full/images/* && \
    zip -q $SRC/pngfuzzer_seed_corpus.zip afl-testcases/png*/full/images/*
# using github's svn view to use svn export as a hack to just export part of the git repo
svn export --force -q https://github.com/khaledhosny/ots/trunk/tests/fonts $SRC/sample-sft-fonts/ots
svn export --force -q https://github.com/unicode-org/text-rendering-tests/trunk/fonts/ $SRC/sample-sft-fonts/unicode-org
svn export --force -q https://github.com/harfbuzz/harfbuzz/trunk/test/shape/data/in-house/fonts $SRC/sample-sft-fonts/harfbuzz
mkdir -p $SRC/sample-sft-fonts/adobe
curl --no-progress-meter -S \
    -C - -o $SRC/sample-sft-fonts/adobe/AdobeVFPrototype.otf https://github.com/adobe-fonts/adobe-variable-font-prototype/releases/download/1.001/AdobeVFPrototype.otf
zip -qr $SRC/sftfuzzer_seed_corpus.zip $SRC/sample-sft-fonts
curl --no-progress-meter -S \
    -C - -O https://dev-www.libreoffice.org/corpus/wmffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/xbmfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/xpmfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/svmfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/pcdfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/dxffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/metfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/ppmfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/psdfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/epsfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/pctfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/pcxfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/rasfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/tgafuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/tiffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/hwpfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/602fuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/lwpfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/pptfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/rtffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/olefuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/cgmfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/ww2fuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/ww6fuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/ww8fuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/qpwfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/slkfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/fodtfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/fodsfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/fodgfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/fodpfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/xlsfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/scrtffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/wksfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/dbffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/diffuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/docxfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/xlsxfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/pptxfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/mmlfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/mtpfuzzer_seed_corpus.zip \
    -C - -O https://dev-www.libreoffice.org/corpus/htmlfuzzer_seed_corpus.zip

echo end downloading dependencies at `date -u`
