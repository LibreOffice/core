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

$(eval $(call gb_Library_use_package,clucene,clucene_inc))

$(eval $(call gb_Library_use_unpacked,clucene,clucene))

$(eval $(call gb_Library_set_warnings_not_errors,clucene))

$(eval $(call gb_Library_set_include,clucene,\
	-I$(WORKDIR)/UnpackedTarball/clucene/inc/internal \
	-I$(WORKDIR)/UnpackedTarball/clucene/src/core \
	-I$(WORKDIR)/UnpackedTarball/clucene/src/contribs-lib \
	-I$(WORKDIR)/UnpackedTarball/clucene/src/shared \
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

$(eval $(call gb_Library_set_generated_cxx_suffix,clucene,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,clucene,\
    UnpackedTarball/clucene/src/shared/CLucene/SharedHeader \
    UnpackedTarball/clucene/src/shared/CLucene/config/gunichartables \
    UnpackedTarball/clucene/src/shared/CLucene/config/repl_tcslwr \
    UnpackedTarball/clucene/src/shared/CLucene/config/repl_tcstoll \
    UnpackedTarball/clucene/src/shared/CLucene/config/repl_tcscasecmp \
    UnpackedTarball/clucene/src/shared/CLucene/config/repl_tprintf \
    UnpackedTarball/clucene/src/shared/CLucene/config/repl_lltot \
    UnpackedTarball/clucene/src/shared/CLucene/config/repl_tcstod \
    UnpackedTarball/clucene/src/shared/CLucene/config/utf8 \
    UnpackedTarball/clucene/src/shared/CLucene/config/threads \
    UnpackedTarball/clucene/src/shared/CLucene/debug/condition \
    UnpackedTarball/clucene/src/shared/CLucene/util/StringBuffer \
    UnpackedTarball/clucene/src/shared/CLucene/util/Misc \
    UnpackedTarball/clucene/src/shared/CLucene/util/dirent \
	UnpackedTarball/clucene/src/core/CLucene/StdHeader \
	UnpackedTarball/clucene/src/core/CLucene/debug/error \
	UnpackedTarball/clucene/src/core/CLucene/util/ThreadLocal \
	UnpackedTarball/clucene/src/core/CLucene/util/Reader \
	UnpackedTarball/clucene/src/core/CLucene/util/Equators \
	UnpackedTarball/clucene/src/core/CLucene/util/FastCharStream \
	UnpackedTarball/clucene/src/core/CLucene/util/MD5Digester \
	UnpackedTarball/clucene/src/core/CLucene/util/StringIntern \
	UnpackedTarball/clucene/src/core/CLucene/util/BitSet \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/FastCharStream \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/MultiFieldQueryParser \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/QueryParser \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/QueryParserTokenManager \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/QueryToken \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/legacy/Lexer \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/legacy/MultiFieldQueryParser \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/legacy/QueryParser \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/legacy/QueryParserBase \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/legacy/QueryToken \
	UnpackedTarball/clucene/src/core/CLucene/queryParser/legacy/TokenList \
	UnpackedTarball/clucene/src/core/CLucene/analysis/standard/StandardAnalyzer \
	UnpackedTarball/clucene/src/core/CLucene/analysis/standard/StandardFilter \
	UnpackedTarball/clucene/src/core/CLucene/analysis/standard/StandardTokenizer \
	UnpackedTarball/clucene/src/core/CLucene/analysis/Analyzers \
	UnpackedTarball/clucene/src/core/CLucene/analysis/AnalysisHeader \
	UnpackedTarball/clucene/src/core/CLucene/store/MMapInput \
	UnpackedTarball/clucene/src/core/CLucene/store/IndexInput \
	UnpackedTarball/clucene/src/core/CLucene/store/Lock \
	UnpackedTarball/clucene/src/core/CLucene/store/LockFactory \
	UnpackedTarball/clucene/src/core/CLucene/store/IndexOutput \
	UnpackedTarball/clucene/src/core/CLucene/store/Directory \
    UnpackedTarball/clucene/src/core/CLucene/store/FSDirectory \
    UnpackedTarball/clucene/src/core/CLucene/store/RAMDirectory \
    UnpackedTarball/clucene/src/core/CLucene/document/Document \
    UnpackedTarball/clucene/src/core/CLucene/document/DateField \
    UnpackedTarball/clucene/src/core/CLucene/document/DateTools \
    UnpackedTarball/clucene/src/core/CLucene/document/Field \
    UnpackedTarball/clucene/src/core/CLucene/document/FieldSelector \
    UnpackedTarball/clucene/src/core/CLucene/document/NumberTools \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexFileNames \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexFileNameFilter \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexDeletionPolicy \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentMergeInfo \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentInfos \
    UnpackedTarball/clucene/src/core/CLucene/index/MergeScheduler \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentTermDocs \
    UnpackedTarball/clucene/src/core/CLucene/index/FieldsWriter \
    UnpackedTarball/clucene/src/core/CLucene/index/TermInfosWriter \
    UnpackedTarball/clucene/src/core/CLucene/index/Term \
    UnpackedTarball/clucene/src/core/CLucene/index/Terms \
    UnpackedTarball/clucene/src/core/CLucene/index/MergePolicy \
    UnpackedTarball/clucene/src/core/CLucene/index/DocumentsWriter \
    UnpackedTarball/clucene/src/core/CLucene/index/DocumentsWriterThreadState \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentTermVector \
    UnpackedTarball/clucene/src/core/CLucene/index/TermVectorReader \
    UnpackedTarball/clucene/src/core/CLucene/index/FieldInfos \
    UnpackedTarball/clucene/src/core/CLucene/index/CompoundFile \
    UnpackedTarball/clucene/src/core/CLucene/index/SkipListReader \
    UnpackedTarball/clucene/src/core/CLucene/index/SkipListWriter \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexFileDeleter \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentReader \
    UnpackedTarball/clucene/src/core/CLucene/index/DirectoryIndexReader \
    UnpackedTarball/clucene/src/core/CLucene/index/TermVectorWriter \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexReader \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentTermPositions \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentMerger \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexWriter \
    UnpackedTarball/clucene/src/core/CLucene/index/MultiReader \
    UnpackedTarball/clucene/src/core/CLucene/index/MultiSegmentReader \
    UnpackedTarball/clucene/src/core/CLucene/index/Payload \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentTermEnum \
    UnpackedTarball/clucene/src/core/CLucene/index/TermInfo \
    UnpackedTarball/clucene/src/core/CLucene/index/IndexModifier \
    UnpackedTarball/clucene/src/core/CLucene/index/SegmentMergeQueue \
    UnpackedTarball/clucene/src/core/CLucene/index/FieldsReader \
    UnpackedTarball/clucene/src/core/CLucene/index/TermInfosReader \
    UnpackedTarball/clucene/src/core/CLucene/index/MultipleTermPositions \
    UnpackedTarball/clucene/src/core/CLucene/search/Compare \
    UnpackedTarball/clucene/src/core/CLucene/search/Scorer \
    UnpackedTarball/clucene/src/core/CLucene/search/ScorerDocQueue \
    UnpackedTarball/clucene/src/core/CLucene/search/PhraseScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/SloppyPhraseScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/DisjunctionSumScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/ConjunctionScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/PhraseQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/PrefixQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/ExactPhraseScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/TermScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/Similarity \
    UnpackedTarball/clucene/src/core/CLucene/search/BooleanScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/BooleanScorer2 \
    UnpackedTarball/clucene/src/core/CLucene/search/HitQueue \
    UnpackedTarball/clucene/src/core/CLucene/search/FieldCacheImpl \
    UnpackedTarball/clucene/src/core/CLucene/search/ChainedFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/RangeFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/CachingWrapperFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/QueryFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/TermQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/FuzzyQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/SearchHeader \
    UnpackedTarball/clucene/src/core/CLucene/search/RangeQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/IndexSearcher \
    UnpackedTarball/clucene/src/core/CLucene/search/Sort \
    UnpackedTarball/clucene/src/core/CLucene/search/PhrasePositions \
    UnpackedTarball/clucene/src/core/CLucene/search/FieldDocSortedHitQueue \
    UnpackedTarball/clucene/src/core/CLucene/search/WildcardTermEnum \
    UnpackedTarball/clucene/src/core/CLucene/search/MultiSearcher \
    UnpackedTarball/clucene/src/core/CLucene/search/Hits \
    UnpackedTarball/clucene/src/core/CLucene/search/MultiTermQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/FilteredTermEnum \
    UnpackedTarball/clucene/src/core/CLucene/search/FieldSortedHitQueue \
    UnpackedTarball/clucene/src/core/CLucene/search/WildcardQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/Explanation \
    UnpackedTarball/clucene/src/core/CLucene/search/BooleanQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/FieldCache \
    UnpackedTarball/clucene/src/core/CLucene/search/DateFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/MatchAllDocsQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/MultiPhraseQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/ConstantScoreQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/CachingSpanFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/SpanQueryFilter \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/NearSpansOrdered \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/NearSpansUnordered \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanFirstQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanNearQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanNotQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanOrQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanScorer \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanTermQuery \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/SpanWeight \
    UnpackedTarball/clucene/src/core/CLucene/search/spans/TermSpans \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/analysis/cjk/CJKAnalyzer \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/analysis/LanguageBasedAnalyzer \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/analysis/PorterStemmer \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/Snowball \
))

$(eval $(call gb_Library_add_generated_cobjects,clucene,\
	UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/libstemmer/libstemmer \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_danish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_dutch \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_english \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_finnish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_french \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_german \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_italian \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_norwegian \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_porter \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_portuguese \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_spanish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_ISO_8859_1_swedish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_KOI8_R_russian \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_danish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_dutch \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_english \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_finnish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_french \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_german \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_italian \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_norwegian \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_porter \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_portuguese \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_russian \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_spanish \
    UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/src_c/stem_UTF_8_swedish \
	UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/runtime/utilities \
	UnpackedTarball/clucene/src/contribs-lib/CLucene/snowball/runtime/api \
))

# vim: set noet sw=4 ts=4:
