#!/usr/bin/env bash

set -e

echo start downloading dependencies at `date -u`

# convert FOO := BAR$(MICRO) to FOO=BAR$MICRO
source <(sed -e's# := #=#g' download.lst | sed -e 's#[)(]##g')

cd $SRC

#cache build dependencies
curl --no-progress-meter -S \
    -C - -O https://dev-www.libreoffice.org/src/$FONT_LIBERATION_TARBALL \
    -C - -O https://dev-www.libreoffice.org/extern/$OPENSYMBOL_TTF \
    -C - -O https://dev-www.libreoffice.org/src/$RASQAL_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$CLUCENE_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$HYPHEN_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$RAPTOR_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$MYTHES_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$REDLAND_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$BOOST_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$BOX2D_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$DTOA_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$EXPAT_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$CUCKOO_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBJPEG_TURBO_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LCMS2_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBEXTTEXTCAT_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$CPPUNIT_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$CAIRO_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$CURL_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$XMLSEC_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBLANGTAG_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ABW_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$CDR_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBCMIS_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$EBOOK_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ETONYEK_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$FREEHAND_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$MSPUB_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$MWAW_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ODFGEN_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ORCUS_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$PAGEMAKER_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBPNG_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$REVENGE_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$STAROFFICE_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$VISIO_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$WPD_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$WPG_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$WPS_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ZMF_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$PIXMAN_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ZLIB_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$MDDS_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$OPENSSL_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LANGTAGREG_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$GRAPHITE_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$HARFBUZZ_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$GLM_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ICU_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$ICU_DATA_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBXML_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBXSLT_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$HUNSPELL_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$FREETYPE_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$FONTCONFIG_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$EPOXY_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$EPUBGEN_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$LIBNUMBERTEXT_TARBALL \
    -C - -O https://dev-www.libreoffice.org/src/$QXP_TARBALL
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
