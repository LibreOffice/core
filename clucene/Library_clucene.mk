# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

# FIXME, need a way to suppress -Werror

$(eval $(call gb_Library_Library,clucene))

$(eval $(call gb_Library_use_external,clucene,zlib))

$(eval $(call gb_Library_add_package_headers,clucene,clucene_inc))

$(eval $(call gb_Library_set_include,clucene,\
    -I$(WORKDIR)/clucene/src/core \
    -I$(WORKDIR)/clucene/src/shared \
    -I$(WORKDIR)/clucene/src/contribs-lib \
    -I$(WORKDIR)/clucene/inc/internal \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,clucene,\
    -Dclucene_shared_EXPORTS\
    -Dclucene_core_EXPORTS\
    -Dclucene_contribs_lib_EXPORTS\
))

$(eval $(call gb_Library_add_linked_libs,clucene,\
    $(gb_STDLIBS) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,clucene,\
    dl \
    m \
    pthread \
))
endif

$(eval $(call gb_Library_add_generated_exception_objects,clucene,\
    clucene/src/shared/CLucene/SharedHeader \
    clucene/src/shared/CLucene/config/gunichartables \
    clucene/src/shared/CLucene/config/repl_tcslwr \
    clucene/src/shared/CLucene/config/repl_tcstoll \
    clucene/src/shared/CLucene/config/repl_tcscasecmp \
    clucene/src/shared/CLucene/config/repl_tprintf \
    clucene/src/shared/CLucene/config/repl_lltot \
    clucene/src/shared/CLucene/config/repl_tcstod \
    clucene/src/shared/CLucene/config/utf8 \
    clucene/src/shared/CLucene/config/threads \
    clucene/src/shared/CLucene/debug/condition \
    clucene/src/shared/CLucene/util/StringBuffer \
    clucene/src/shared/CLucene/util/Misc \
    clucene/src/shared/CLucene/util/dirent \
	clucene/src/core/CLucene/StdHeader \
	clucene/src/core/CLucene/debug/error \
	clucene/src/core/CLucene/util/ThreadLocal \
	clucene/src/core/CLucene/util/Reader \
	clucene/src/core/CLucene/util/Equators \
	clucene/src/core/CLucene/util/FastCharStream \
	clucene/src/core/CLucene/util/MD5Digester \
	clucene/src/core/CLucene/util/StringIntern \
	clucene/src/core/CLucene/util/BitSet \
	clucene/src/core/CLucene/queryParser/FastCharStream \
	clucene/src/core/CLucene/queryParser/MultiFieldQueryParser \
	clucene/src/core/CLucene/queryParser/QueryParser \
	clucene/src/core/CLucene/queryParser/QueryParserTokenManager \
	clucene/src/core/CLucene/queryParser/QueryToken \
	clucene/src/core/CLucene/queryParser/legacy/Lexer \
	clucene/src/core/CLucene/queryParser/legacy/MultiFieldQueryParser \
	clucene/src/core/CLucene/queryParser/legacy/QueryParser \
	clucene/src/core/CLucene/queryParser/legacy/QueryParserBase \
	clucene/src/core/CLucene/queryParser/legacy/QueryToken \
	clucene/src/core/CLucene/queryParser/legacy/TokenList \
	clucene/src/core/CLucene/analysis/standard/StandardAnalyzer \
	clucene/src/core/CLucene/analysis/standard/StandardFilter \
	clucene/src/core/CLucene/analysis/standard/StandardTokenizer \
	clucene/src/core/CLucene/analysis/Analyzers \
	clucene/src/core/CLucene/analysis/AnalysisHeader \
	clucene/src/core/CLucene/store/MMapInput \
	clucene/src/core/CLucene/store/IndexInput \
	clucene/src/core/CLucene/store/Lock \
	clucene/src/core/CLucene/store/LockFactory \
	clucene/src/core/CLucene/store/IndexOutput \
	clucene/src/core/CLucene/store/Directory \
    clucene/src/core/CLucene/store/FSDirectory \
    clucene/src/core/CLucene/store/RAMDirectory \
    clucene/src/core/CLucene/document/Document \
    clucene/src/core/CLucene/document/DateField \
    clucene/src/core/CLucene/document/DateTools \
    clucene/src/core/CLucene/document/Field \
    clucene/src/core/CLucene/document/FieldSelector \
    clucene/src/core/CLucene/document/NumberTools \
    clucene/src/core/CLucene/index/IndexFileNames \
    clucene/src/core/CLucene/index/IndexFileNameFilter \
    clucene/src/core/CLucene/index/IndexDeletionPolicy \
    clucene/src/core/CLucene/index/SegmentMergeInfo \
    clucene/src/core/CLucene/index/SegmentInfos \
    clucene/src/core/CLucene/index/MergeScheduler \
    clucene/src/core/CLucene/index/SegmentTermDocs \
    clucene/src/core/CLucene/index/FieldsWriter \
    clucene/src/core/CLucene/index/TermInfosWriter \
    clucene/src/core/CLucene/index/Term \
    clucene/src/core/CLucene/index/Terms \
    clucene/src/core/CLucene/index/MergePolicy \
    clucene/src/core/CLucene/index/DocumentsWriter \
    clucene/src/core/CLucene/index/DocumentsWriterThreadState \
    clucene/src/core/CLucene/index/SegmentTermVector \
    clucene/src/core/CLucene/index/TermVectorReader \
    clucene/src/core/CLucene/index/FieldInfos \
    clucene/src/core/CLucene/index/CompoundFile \
    clucene/src/core/CLucene/index/SkipListReader \
    clucene/src/core/CLucene/index/SkipListWriter \
    clucene/src/core/CLucene/index/IndexFileDeleter \
    clucene/src/core/CLucene/index/SegmentReader \
    clucene/src/core/CLucene/index/DirectoryIndexReader \
    clucene/src/core/CLucene/index/TermVectorWriter \
    clucene/src/core/CLucene/index/IndexReader \
    clucene/src/core/CLucene/index/SegmentTermPositions \
    clucene/src/core/CLucene/index/SegmentMerger \
    clucene/src/core/CLucene/index/IndexWriter \
    clucene/src/core/CLucene/index/MultiReader \
    clucene/src/core/CLucene/index/MultiSegmentReader \
    clucene/src/core/CLucene/index/Payload \
    clucene/src/core/CLucene/index/SegmentTermEnum \
    clucene/src/core/CLucene/index/TermInfo \
    clucene/src/core/CLucene/index/IndexModifier \
    clucene/src/core/CLucene/index/SegmentMergeQueue \
    clucene/src/core/CLucene/index/FieldsReader \
    clucene/src/core/CLucene/index/TermInfosReader \
    clucene/src/core/CLucene/index/MultipleTermPositions \
    clucene/src/core/CLucene/search/Compare \
    clucene/src/core/CLucene/search/Scorer \
    clucene/src/core/CLucene/search/ScorerDocQueue \
    clucene/src/core/CLucene/search/PhraseScorer \
    clucene/src/core/CLucene/search/SloppyPhraseScorer \
    clucene/src/core/CLucene/search/DisjunctionSumScorer \
    clucene/src/core/CLucene/search/ConjunctionScorer \
    clucene/src/core/CLucene/search/PhraseQuery \
    clucene/src/core/CLucene/search/PrefixQuery \
    clucene/src/core/CLucene/search/ExactPhraseScorer \
    clucene/src/core/CLucene/search/TermScorer \
    clucene/src/core/CLucene/search/Similarity \
    clucene/src/core/CLucene/search/BooleanScorer \
    clucene/src/core/CLucene/search/BooleanScorer2 \
    clucene/src/core/CLucene/search/HitQueue \
    clucene/src/core/CLucene/search/FieldCacheImpl \
    clucene/src/core/CLucene/search/ChainedFilter \
    clucene/src/core/CLucene/search/RangeFilter \
    clucene/src/core/CLucene/search/CachingWrapperFilter \
    clucene/src/core/CLucene/search/QueryFilter \
    clucene/src/core/CLucene/search/TermQuery \
    clucene/src/core/CLucene/search/FuzzyQuery \
    clucene/src/core/CLucene/search/SearchHeader \
    clucene/src/core/CLucene/search/RangeQuery \
    clucene/src/core/CLucene/search/IndexSearcher \
    clucene/src/core/CLucene/search/Sort \
    clucene/src/core/CLucene/search/PhrasePositions \
    clucene/src/core/CLucene/search/FieldDocSortedHitQueue \
    clucene/src/core/CLucene/search/WildcardTermEnum \
    clucene/src/core/CLucene/search/MultiSearcher \
    clucene/src/core/CLucene/search/Hits \
    clucene/src/core/CLucene/search/MultiTermQuery \
    clucene/src/core/CLucene/search/FilteredTermEnum \
    clucene/src/core/CLucene/search/FieldSortedHitQueue \
    clucene/src/core/CLucene/search/WildcardQuery \
    clucene/src/core/CLucene/search/Explanation \
    clucene/src/core/CLucene/search/BooleanQuery \
    clucene/src/core/CLucene/search/FieldCache \
    clucene/src/core/CLucene/search/DateFilter \
    clucene/src/core/CLucene/search/MatchAllDocsQuery \
    clucene/src/core/CLucene/search/MultiPhraseQuery \
    clucene/src/core/CLucene/search/ConstantScoreQuery \
    clucene/src/core/CLucene/search/CachingSpanFilter \
    clucene/src/core/CLucene/search/SpanQueryFilter \
    clucene/src/core/CLucene/search/spans/NearSpansOrdered \
    clucene/src/core/CLucene/search/spans/NearSpansUnordered \
    clucene/src/core/CLucene/search/spans/SpanFirstQuery \
    clucene/src/core/CLucene/search/spans/SpanNearQuery \
    clucene/src/core/CLucene/search/spans/SpanNotQuery \
    clucene/src/core/CLucene/search/spans/SpanOrQuery \
    clucene/src/core/CLucene/search/spans/SpanScorer \
    clucene/src/core/CLucene/search/spans/SpanTermQuery \
    clucene/src/core/CLucene/search/spans/SpanWeight \
    clucene/src/core/CLucene/search/spans/TermSpans \
    clucene/src/contribs-lib/CLucene/analysis/cjk/CJKAnalyzer \
    clucene/src/contribs-lib/CLucene/analysis/LanguageBasedAnalyzer \
    clucene/src/contribs-lib/CLucene/analysis/PorterStemmer \
    clucene/src/contribs-lib/CLucene/snowball/Snowball \
))

$(eval $(call gb_Library_add_generated_cobjects,clucene,\
	clucene/src/contribs-lib/CLucene/snowball/libstemmer/libstemmer \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_danish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_dutch \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_english \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_finnish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_french \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_german \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_italian \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_norwegian \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_porter \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_portuguese \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_spanish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_swedish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_KOI8_R_russian \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_danish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_dutch \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_english \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_finnish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_french \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_german \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_italian \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_norwegian \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_porter \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_portuguese \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_russian \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_spanish \
    clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_swedish \
	clucene/src/contribs-lib/CLucene/snowball/runtime/utilities \
	clucene/src/contribs-lib/CLucene/snowball/runtime/api \
))

#FIXME, correct dependency
$(call gb_Package_get_target,clucene_inc) : $(WORKDIR)/clucene/src/shared/CLucene/clucene-config.h

$(WORKDIR)/clucene/src/shared/CLucene/clucene-config.h : $(TARFILE_LOCATION)/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
	mkdir -p $(dir $@)
	$(GNUTAR) -x -C $(WORKDIR)/clucene --strip-component=1 -f $(TARFILE_LOCATION)/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz
	#FIXME ?, our rules expect .cxx
	for i in `find $(WORKDIR)/clucene -name "*.cpp"`; do mv $$i $${i%%cpp}cxx; done
	#dirent.h is a problem, move it around
	mkdir -p $(WORKDIR)/clucene/inc/internal/CLucene/util
	mv $(WORKDIR)/clucene/src/shared/CLucene/util/dirent.h $(WORKDIR)/clucene/inc/internal/CLucene/util
	#To generate these, run cmake for each sufficiently different platform, customize and stick into configs
	cp configs/_clucene-config-generic.h $(WORKDIR)/clucene/src/shared/CLucene/_clucene-config.h
	cp configs/clucene-config-generic.h $(WORKDIR)/clucene/src/shared/CLucene/clucene-config.h

$(call gb_Library_get_clean_target,clucene) : clucene_clean

clucene_clean :
	rm -rf $(WORKDIR)/clucene
.PHONY: clucene_clean

# vim: set noet sw=4 ts=4:
