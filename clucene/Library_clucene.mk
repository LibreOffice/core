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

$(eval $(call gb_Library_Library,clucene))

$(eval $(call gb_Library_use_external,clucene,zlib))

$(eval $(call gb_Library_use_package,clucene,clucene_source))

$(eval $(call gb_Library_set_warnings_not_errors,clucene))

$(eval $(call gb_Library_set_include,clucene,\
	-I$(WORKDIR)/CustomTarget/clucene/source/inc/internal \
	-I$(WORKDIR)/CustomTarget/clucene/source/src/core \
	-I$(WORKDIR)/CustomTarget/clucene/source/src/contribs-lib \
	-I$(WORKDIR)/CustomTarget/clucene/source/src/shared \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,clucene,\
    -Dclucene_shared_EXPORTS \
    -Dclucene_core_EXPORTS \
    -Dclucene_contribs_lib_EXPORTS \
    $(LFS_CFLAGS) \
))

# clucene does not depend on sal nor needs uwinapi here
$(eval $(call gb_Library_use_libraries,clucene,\
    $(filter-out uwinapi,$(gb_STDLIBS)) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_use_libraries,clucene,\
    dl \
    m \
    pthread \
))
endif

$(eval $(call gb_Library_add_generated_exception_objects,clucene,\
    CustomTarget/clucene/source/src/shared/CLucene/SharedHeader \
    CustomTarget/clucene/source/src/shared/CLucene/config/gunichartables \
    CustomTarget/clucene/source/src/shared/CLucene/config/repl_tcslwr \
    CustomTarget/clucene/source/src/shared/CLucene/config/repl_tcstoll \
    CustomTarget/clucene/source/src/shared/CLucene/config/repl_tcscasecmp \
    CustomTarget/clucene/source/src/shared/CLucene/config/repl_tprintf \
    CustomTarget/clucene/source/src/shared/CLucene/config/repl_lltot \
    CustomTarget/clucene/source/src/shared/CLucene/config/repl_tcstod \
    CustomTarget/clucene/source/src/shared/CLucene/config/utf8 \
    CustomTarget/clucene/source/src/shared/CLucene/config/threads \
    CustomTarget/clucene/source/src/shared/CLucene/debug/condition \
    CustomTarget/clucene/source/src/shared/CLucene/util/StringBuffer \
    CustomTarget/clucene/source/src/shared/CLucene/util/Misc \
    CustomTarget/clucene/source/src/shared/CLucene/util/dirent \
	CustomTarget/clucene/source/src/core/CLucene/StdHeader \
	CustomTarget/clucene/source/src/core/CLucene/debug/error \
	CustomTarget/clucene/source/src/core/CLucene/util/ThreadLocal \
	CustomTarget/clucene/source/src/core/CLucene/util/Reader \
	CustomTarget/clucene/source/src/core/CLucene/util/Equators \
	CustomTarget/clucene/source/src/core/CLucene/util/FastCharStream \
	CustomTarget/clucene/source/src/core/CLucene/util/MD5Digester \
	CustomTarget/clucene/source/src/core/CLucene/util/StringIntern \
	CustomTarget/clucene/source/src/core/CLucene/util/BitSet \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/FastCharStream \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/MultiFieldQueryParser \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/QueryParser \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/QueryParserTokenManager \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/QueryToken \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/legacy/Lexer \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/legacy/MultiFieldQueryParser \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/legacy/QueryParser \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/legacy/QueryParserBase \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/legacy/QueryToken \
	CustomTarget/clucene/source/src/core/CLucene/queryParser/legacy/TokenList \
	CustomTarget/clucene/source/src/core/CLucene/analysis/standard/StandardAnalyzer \
	CustomTarget/clucene/source/src/core/CLucene/analysis/standard/StandardFilter \
	CustomTarget/clucene/source/src/core/CLucene/analysis/standard/StandardTokenizer \
	CustomTarget/clucene/source/src/core/CLucene/analysis/Analyzers \
	CustomTarget/clucene/source/src/core/CLucene/analysis/AnalysisHeader \
	CustomTarget/clucene/source/src/core/CLucene/store/MMapInput \
	CustomTarget/clucene/source/src/core/CLucene/store/IndexInput \
	CustomTarget/clucene/source/src/core/CLucene/store/Lock \
	CustomTarget/clucene/source/src/core/CLucene/store/LockFactory \
	CustomTarget/clucene/source/src/core/CLucene/store/IndexOutput \
	CustomTarget/clucene/source/src/core/CLucene/store/Directory \
    CustomTarget/clucene/source/src/core/CLucene/store/FSDirectory \
    CustomTarget/clucene/source/src/core/CLucene/store/RAMDirectory \
    CustomTarget/clucene/source/src/core/CLucene/document/Document \
    CustomTarget/clucene/source/src/core/CLucene/document/DateField \
    CustomTarget/clucene/source/src/core/CLucene/document/DateTools \
    CustomTarget/clucene/source/src/core/CLucene/document/Field \
    CustomTarget/clucene/source/src/core/CLucene/document/FieldSelector \
    CustomTarget/clucene/source/src/core/CLucene/document/NumberTools \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexFileNames \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexFileNameFilter \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexDeletionPolicy \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentMergeInfo \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentInfos \
    CustomTarget/clucene/source/src/core/CLucene/index/MergeScheduler \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentTermDocs \
    CustomTarget/clucene/source/src/core/CLucene/index/FieldsWriter \
    CustomTarget/clucene/source/src/core/CLucene/index/TermInfosWriter \
    CustomTarget/clucene/source/src/core/CLucene/index/Term \
    CustomTarget/clucene/source/src/core/CLucene/index/Terms \
    CustomTarget/clucene/source/src/core/CLucene/index/MergePolicy \
    CustomTarget/clucene/source/src/core/CLucene/index/DocumentsWriter \
    CustomTarget/clucene/source/src/core/CLucene/index/DocumentsWriterThreadState \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentTermVector \
    CustomTarget/clucene/source/src/core/CLucene/index/TermVectorReader \
    CustomTarget/clucene/source/src/core/CLucene/index/FieldInfos \
    CustomTarget/clucene/source/src/core/CLucene/index/CompoundFile \
    CustomTarget/clucene/source/src/core/CLucene/index/SkipListReader \
    CustomTarget/clucene/source/src/core/CLucene/index/SkipListWriter \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexFileDeleter \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentReader \
    CustomTarget/clucene/source/src/core/CLucene/index/DirectoryIndexReader \
    CustomTarget/clucene/source/src/core/CLucene/index/TermVectorWriter \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexReader \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentTermPositions \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentMerger \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexWriter \
    CustomTarget/clucene/source/src/core/CLucene/index/MultiReader \
    CustomTarget/clucene/source/src/core/CLucene/index/MultiSegmentReader \
    CustomTarget/clucene/source/src/core/CLucene/index/Payload \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentTermEnum \
    CustomTarget/clucene/source/src/core/CLucene/index/TermInfo \
    CustomTarget/clucene/source/src/core/CLucene/index/IndexModifier \
    CustomTarget/clucene/source/src/core/CLucene/index/SegmentMergeQueue \
    CustomTarget/clucene/source/src/core/CLucene/index/FieldsReader \
    CustomTarget/clucene/source/src/core/CLucene/index/TermInfosReader \
    CustomTarget/clucene/source/src/core/CLucene/index/MultipleTermPositions \
    CustomTarget/clucene/source/src/core/CLucene/search/Compare \
    CustomTarget/clucene/source/src/core/CLucene/search/Scorer \
    CustomTarget/clucene/source/src/core/CLucene/search/ScorerDocQueue \
    CustomTarget/clucene/source/src/core/CLucene/search/PhraseScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/SloppyPhraseScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/DisjunctionSumScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/ConjunctionScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/PhraseQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/PrefixQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/ExactPhraseScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/TermScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/Similarity \
    CustomTarget/clucene/source/src/core/CLucene/search/BooleanScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/BooleanScorer2 \
    CustomTarget/clucene/source/src/core/CLucene/search/HitQueue \
    CustomTarget/clucene/source/src/core/CLucene/search/FieldCacheImpl \
    CustomTarget/clucene/source/src/core/CLucene/search/ChainedFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/RangeFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/CachingWrapperFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/QueryFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/TermQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/FuzzyQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/SearchHeader \
    CustomTarget/clucene/source/src/core/CLucene/search/RangeQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/IndexSearcher \
    CustomTarget/clucene/source/src/core/CLucene/search/Sort \
    CustomTarget/clucene/source/src/core/CLucene/search/PhrasePositions \
    CustomTarget/clucene/source/src/core/CLucene/search/FieldDocSortedHitQueue \
    CustomTarget/clucene/source/src/core/CLucene/search/WildcardTermEnum \
    CustomTarget/clucene/source/src/core/CLucene/search/MultiSearcher \
    CustomTarget/clucene/source/src/core/CLucene/search/Hits \
    CustomTarget/clucene/source/src/core/CLucene/search/MultiTermQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/FilteredTermEnum \
    CustomTarget/clucene/source/src/core/CLucene/search/FieldSortedHitQueue \
    CustomTarget/clucene/source/src/core/CLucene/search/WildcardQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/Explanation \
    CustomTarget/clucene/source/src/core/CLucene/search/BooleanQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/FieldCache \
    CustomTarget/clucene/source/src/core/CLucene/search/DateFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/MatchAllDocsQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/MultiPhraseQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/ConstantScoreQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/CachingSpanFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/SpanQueryFilter \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/NearSpansOrdered \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/NearSpansUnordered \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanFirstQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanNearQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanNotQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanOrQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanScorer \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanTermQuery \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/SpanWeight \
    CustomTarget/clucene/source/src/core/CLucene/search/spans/TermSpans \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/analysis/cjk/CJKAnalyzer \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/analysis/LanguageBasedAnalyzer \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/analysis/PorterStemmer \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/Snowball \
	,$(if $(filter TRUE,$(EXTERNAL_WARNINGS_NOT_ERRORS)),\
		$(filter-out $(gb_CXXFLAGS_WERROR),$(CXXFLAGS)),\
		$(CXXFLAGS)) \
	 $(gb_COMPILEROPTFLAGS) \
))

$(eval $(call gb_Library_add_generated_cobjects,clucene,\
	CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/libstemmer/libstemmer \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_danish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_dutch \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_english \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_finnish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_french \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_german \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_italian \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_norwegian \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_porter \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_portuguese \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_spanish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_swedish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_KOI8_R_russian \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_danish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_dutch \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_english \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_finnish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_french \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_german \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_italian \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_norwegian \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_porter \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_portuguese \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_russian \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_spanish \
    CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_swedish \
	CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/runtime/utilities \
	CustomTarget/clucene/source/src/contribs-lib/CLucene/snowball/runtime/api \
	,$(if $(filter TRUE,$(EXTERNAL_WARNINGS_NOT_ERRORS)),\
		$(filter-out $(gb_CFLAGS_WERROR),$(CFLAGS)),\
		$(CFLAGS)) \
	 $(gb_COMPILEROPTFLAGS) \
))

# vim: set noet sw=4 ts=4:
