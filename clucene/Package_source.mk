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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Package_Package,clucene_source,$(WORKDIR)/CustomTarget/clucene/source))

$(eval $(call gb_Package_add_customtarget,clucene_source,clucene/source))

$(eval $(call gb_CustomTarget_add_dependencies,clucene/source,\
	clucene/configs/_clucene-config-generic.h \
	clucene/configs/clucene-config-generic.h \
))

ifeq ($(OS_FOR_BUILD),WNT)
FIXED_TARFILE_LOCATION=$(shell cygpath -u $(TARFILE_LOCATION))
else
FIXED_TARFILE_LOCATION=$(TARFILE_LOCATION)
endif

# FIXME: do not hardcode the path here
$(eval $(call gb_CustomTarget_add_outdir_dependencies,clucene/source,\
	$(FIXED_TARFILE_LOCATION)/48d647fbd8ef8889e5a7f422c1bfda94-clucene-core-2.3.3.4.tar.gz \
))

$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene.h,src/core/CLucene.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/clucene-config.h,src/shared/CLucene/clucene-config.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/SharedHeader.h,src/shared/CLucene/SharedHeader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/document/DateField.h,src/core/CLucene/document/DateField.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/document/DateTools.h,src/core/CLucene/document/DateTools.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/document/Document.h,src/core/CLucene/document/Document.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/document/Field.h,src/core/CLucene/document/Field.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/document/NumberTools.h,src/core/CLucene/document/NumberTools.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/DirectoryIndexReader.h,src/core/CLucene/index/DirectoryIndexReader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/Terms.h,src/core/CLucene/index/Terms.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/IndexDeletionPolicy.h,src/core/CLucene/index/IndexDeletionPolicy.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/MultipleTermPositions.h,src/core/CLucene/index/MultipleTermPositions.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/Term.h,src/core/CLucene/index/Term.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/IndexModifier.h,src/core/CLucene/index/IndexModifier.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/Payload.h,src/core/CLucene/index/Payload.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/MultiReader.h,src/core/CLucene/index/MultiReader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/MergeScheduler.h,src/core/CLucene/index/MergeScheduler.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/TermVector.h,src/core/CLucene/index/TermVector.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/MergePolicy.h,src/core/CLucene/index/MergePolicy.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/IndexReader.h,src/core/CLucene/index/IndexReader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/index/IndexWriter.h,src/core/CLucene/index/IndexWriter.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/BooleanClause.h,src/core/CLucene/search/BooleanClause.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/BooleanQuery.h,src/core/CLucene/search/BooleanQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/DateFilter.h,src/core/CLucene/search/DateFilter.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Explanation.h,src/core/CLucene/search/Explanation.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Hits.h,src/core/CLucene/search/Hits.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/IndexSearcher.h,src/core/CLucene/search/IndexSearcher.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/FilteredTermEnum.h,src/core/CLucene/search/FilteredTermEnum.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/FuzzyQuery.h,src/core/CLucene/search/FuzzyQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/MultiSearcher.h,src/core/CLucene/search/MultiSearcher.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/MultiTermQuery.h,src/core/CLucene/search/MultiTermQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Searchable.h,src/core/CLucene/search/Searchable.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/SearchHeader.h,src/core/CLucene/search/SearchHeader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Similarity.h,src/core/CLucene/search/Similarity.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Sort.h,src/core/CLucene/search/Sort.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanTermQuery.h,src/core/CLucene/search/spans/SpanTermQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanQuery.h,src/core/CLucene/search/spans/SpanQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanNotQuery.h,src/core/CLucene/search/spans/SpanNotQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanScorer.h,src/core/CLucene/search/spans/SpanScorer.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanFirstQuery.h,src/core/CLucene/search/spans/SpanFirstQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanNearQuery.h,src/core/CLucene/search/spans/SpanNearQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanOrQuery.h,src/core/CLucene/search/spans/SpanOrQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/Spans.h,src/core/CLucene/search/spans/Spans.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/spans/SpanWeight.h,src/core/CLucene/search/spans/SpanWeight.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Filter.h,src/core/CLucene/search/Filter.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/FieldSortedHitQueue.h,src/core/CLucene/search/FieldSortedHitQueue.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/PhraseQuery.h,src/core/CLucene/search/PhraseQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/PrefixQuery.h,src/core/CLucene/search/PrefixQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/Query.h,src/core/CLucene/search/Query.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/RangeQuery.h,src/core/CLucene/search/RangeQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/SpanQueryFilter.h,src/core/CLucene/search/SpanQueryFilter.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/TermQuery.h,src/core/CLucene/search/TermQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/MultiPhraseQuery.h,src/core/CLucene/search/MultiPhraseQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/search/WildcardQuery.h,src/core/CLucene/search/WildcardQuery.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/CLConfig.h,src/core/CLucene/CLConfig.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/VoidList.h,src/core/CLucene/util/VoidList.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/Reader.h,src/core/CLucene/util/Reader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/VoidMap.h,src/core/CLucene/util/VoidMap.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/CLStreams.h,src/core/CLucene/util/CLStreams.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/Array.h,src/core/CLucene/util/Array.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/PriorityQueue.h,src/core/CLucene/util/PriorityQueue.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/BitSet.h,src/core/CLucene/util/BitSet.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/util/Equators.h,src/core/CLucene/util/Equators.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/QueryParserConstants.h,src/core/CLucene/queryParser/QueryParserConstants.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/QueryToken.h,src/core/CLucene/queryParser/QueryToken.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/legacy/QueryToken.h,src/core/CLucene/queryParser/legacy/QueryToken.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/legacy/MultiFieldQueryParser.h,src/core/CLucene/queryParser/legacy/MultiFieldQueryParser.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/legacy/QueryParser.h,src/core/CLucene/queryParser/legacy/QueryParser.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/MultiFieldQueryParser.h,src/core/CLucene/queryParser/MultiFieldQueryParser.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/QueryParserTokenManager.h,src/core/CLucene/queryParser/QueryParserTokenManager.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/queryParser/QueryParser.h,src/core/CLucene/queryParser/QueryParser.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/LuceneThreads.h,src/shared/CLucene/LuceneThreads.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/LockFactory.h,src/core/CLucene/store/LockFactory.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/Lock.h,src/core/CLucene/store/Lock.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/Directory.h,src/core/CLucene/store/Directory.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/RAMDirectory.h,src/core/CLucene/store/RAMDirectory.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/IndexInput.h,src/core/CLucene/store/IndexInput.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/FSDirectory.h,src/core/CLucene/store/FSDirectory.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/store/IndexOutput.h,src/core/CLucene/store/IndexOutput.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/StdHeader.h,src/core/CLucene/StdHeader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/debug/error.h,src/core/CLucene/debug/error.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/debug/mem.h,src/core/CLucene/debug/mem.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/debug/lucenebase.h,src/core/CLucene/debug/lucenebase.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/CachingTokenFilter.h,src/core/CLucene/analysis/CachingTokenFilter.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/AnalysisHeader.h,src/core/CLucene/analysis/AnalysisHeader.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/Analyzers.h,src/core/CLucene/analysis/Analyzers.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/standard/StandardTokenizer.h,src/core/CLucene/analysis/standard/StandardTokenizer.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/standard/StandardFilter.h,src/core/CLucene/analysis/standard/StandardFilter.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/standard/StandardTokenizerConstants.h,src/core/CLucene/analysis/standard/StandardTokenizerConstants.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/standard/StandardAnalyzer.h,src/core/CLucene/analysis/standard/StandardAnalyzer.h))
$(eval $(call gb_Package_add_file,clucene_source,inc/CLucene/analysis/LanguageBasedAnalyzer.h,src/contribs-lib/CLucene/analysis/LanguageBasedAnalyzer.h))

# vim: set noet sw=4 ts=4:
