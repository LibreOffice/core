# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,afdko))

$(eval $(call gb_StaticLibrary_use_unpacked,afdko,afdko))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,afdko))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,afdko,cpp))

$(eval $(call gb_StaticLibrary_set_include,afdko,\
	-I$(gb_UnpackedTarball_workdir)/afdko/c/shared/include \
	-I$(gb_UnpackedTarball_workdir)/afdko/c/shared/resource \
	-I$(gb_UnpackedTarball_workdir)/afdko/c/shared/source/tx_shared \
	-I$(gb_UnpackedTarball_workdir)/afdko/c/makeotf \
	-I$(gb_UnpackedTarball_workdir)/afdko/a4/runtime/src \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_use_externals,afdko,\
	libxml2 \
))

$(eval $(call gb_StaticLibrary_add_defs,afdko, \
	-DTC_EURO_SUPPORT=1 \
	-DTC_SUBR_SUPPORT=1 \
	-DTC_HINT_CHECK=1 \
	-DANTLR4CPP_STATIC \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,afdko,\
	UnpackedTarball/afdko/c/shared/source/absfont/absfont \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_afm \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_compare \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_desc \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_draw \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_dump \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_metrics \
	UnpackedTarball/afdko/c/shared/source/absfont/absfont_path \
	UnpackedTarball/afdko/c/shared/source/cfembed/cfembed \
	UnpackedTarball/afdko/c/shared/source/cffread/cffread \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_charset \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_dict \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_encoding \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_fdselect \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_sindex \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_subr \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_t2cstr \
	UnpackedTarball/afdko/c/shared/source/cffwrite/cffwrite_varstore \
	UnpackedTarball/afdko/c/shared/source/ctutil/ctutil \
	UnpackedTarball/afdko/c/shared/source/dynarr/dynarr \
	UnpackedTarball/afdko/c/shared/source/nameread/nameread \
	UnpackedTarball/afdko/c/shared/source/pdfwrite/pdfwrite \
	UnpackedTarball/afdko/c/shared/source/pstoken/pstoken \
	UnpackedTarball/afdko/c/shared/source/sfntread/sfntread \
	UnpackedTarball/afdko/c/shared/source/sfntwrite/sfntwrite \
	UnpackedTarball/afdko/c/shared/source/sha1/sha1 \
	UnpackedTarball/afdko/c/shared/source/support/canthappen \
	UnpackedTarball/afdko/c/shared/source/support/except \
	UnpackedTarball/afdko/c/shared/source/support/fixed \
	UnpackedTarball/afdko/c/shared/source/svgwrite/svgwrite \
	UnpackedTarball/afdko/c/shared/source/svread/svread \
	UnpackedTarball/afdko/c/shared/source/t1cstr/t1cstr \
	UnpackedTarball/afdko/c/shared/source/t1read/t1read \
	UnpackedTarball/afdko/c/shared/source/t1write/t1write \
	UnpackedTarball/afdko/c/shared/source/t2cstr/t2cstr \
	UnpackedTarball/afdko/c/shared/source/ttread/ttread \
	UnpackedTarball/afdko/c/shared/source/varread/varread \
	UnpackedTarball/afdko/c/shared/source/uforead/uforead \
	UnpackedTarball/afdko/c/shared/source/ufowrite/ufowrite \
	UnpackedTarball/afdko/c/shared/source/tx_shared/tx_shared \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,afdko,\
	UnpackedTarball/afdko/c/tx/source/tx \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,afdko,\
	UnpackedTarball/afdko/c/mergefonts/source/mergeFonts \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,afdko,\
	UnpackedTarball/afdko/c/makeotf/source/c_main \
	UnpackedTarball/afdko/c/makeotf/source/cb \
	UnpackedTarball/afdko/c/makeotf/source/cbpriv \
	UnpackedTarball/afdko/c/makeotf/source/fcdb \
	UnpackedTarball/afdko/c/makeotf/source/file \
	UnpackedTarball/afdko/c/makeotf/source/mac \
	UnpackedTarball/afdko/c/makeotf/lib/cffread/cffread \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/BASE \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/CFF_ \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/GDEF \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/GPOS \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/GSUB \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/OS_2 \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/STAT \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/VORG \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/anon \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/cmap \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/head \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/hhea \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/hmtx \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/hot \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/map \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/maxp \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/name \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/otl \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/post \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/sfnt \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/vhea \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/vmtx \
	UnpackedTarball/afdko/c/makeotf/lib/pstoken/pstoken \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/charset \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/cs \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/dict \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/encoding \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/fdselect \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/parse \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/recode \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/sindex \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/subr \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/t13 \
	UnpackedTarball/afdko/c/makeotf/lib/typecomp/tc \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,afdko,\
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/FeatCtx \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/FeatLexer \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/FeatParser \
	UnpackedTarball/afdko/c/makeotf/lib/hotconv/FeatVisitor \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,afdko,\
	UnpackedTarball/afdko/a4/runtime/src/ANTLRErrorListener \
	UnpackedTarball/afdko/a4/runtime/src/ANTLRErrorStrategy \
	UnpackedTarball/afdko/a4/runtime/src/ANTLRFileStream \
	UnpackedTarball/afdko/a4/runtime/src/ANTLRInputStream \
	UnpackedTarball/afdko/a4/runtime/src/BailErrorStrategy \
	UnpackedTarball/afdko/a4/runtime/src/BaseErrorListener \
	UnpackedTarball/afdko/a4/runtime/src/BufferedTokenStream \
	UnpackedTarball/afdko/a4/runtime/src/CharStream \
	UnpackedTarball/afdko/a4/runtime/src/CommonToken \
	UnpackedTarball/afdko/a4/runtime/src/CommonTokenFactory \
	UnpackedTarball/afdko/a4/runtime/src/CommonTokenStream \
	UnpackedTarball/afdko/a4/runtime/src/ConsoleErrorListener \
	UnpackedTarball/afdko/a4/runtime/src/DefaultErrorStrategy \
	UnpackedTarball/afdko/a4/runtime/src/DiagnosticErrorListener \
	UnpackedTarball/afdko/a4/runtime/src/Exceptions \
	UnpackedTarball/afdko/a4/runtime/src/FailedPredicateException \
	UnpackedTarball/afdko/a4/runtime/src/InputMismatchException \
	UnpackedTarball/afdko/a4/runtime/src/InterpreterRuleContext \
	UnpackedTarball/afdko/a4/runtime/src/IntStream \
	UnpackedTarball/afdko/a4/runtime/src/Lexer \
	UnpackedTarball/afdko/a4/runtime/src/LexerInterpreter \
	UnpackedTarball/afdko/a4/runtime/src/LexerNoViableAltException \
	UnpackedTarball/afdko/a4/runtime/src/ListTokenSource \
	UnpackedTarball/afdko/a4/runtime/src/NoViableAltException \
	UnpackedTarball/afdko/a4/runtime/src/Parser \
	UnpackedTarball/afdko/a4/runtime/src/ParserInterpreter \
	UnpackedTarball/afdko/a4/runtime/src/ParserRuleContext \
	UnpackedTarball/afdko/a4/runtime/src/ProxyErrorListener \
	UnpackedTarball/afdko/a4/runtime/src/RecognitionException \
	UnpackedTarball/afdko/a4/runtime/src/Recognizer \
	UnpackedTarball/afdko/a4/runtime/src/RuleContext \
	UnpackedTarball/afdko/a4/runtime/src/RuleContextWithAltNum \
	UnpackedTarball/afdko/a4/runtime/src/RuntimeMetaData \
	UnpackedTarball/afdko/a4/runtime/src/Token \
	UnpackedTarball/afdko/a4/runtime/src/TokenSource \
	UnpackedTarball/afdko/a4/runtime/src/TokenStream \
	UnpackedTarball/afdko/a4/runtime/src/TokenStreamRewriter \
	UnpackedTarball/afdko/a4/runtime/src/UnbufferedCharStream \
	UnpackedTarball/afdko/a4/runtime/src/UnbufferedTokenStream \
	UnpackedTarball/afdko/a4/runtime/src/Vocabulary \
	UnpackedTarball/afdko/a4/runtime/src/WritableToken \
	UnpackedTarball/afdko/a4/runtime/src/atn/ActionTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/AmbiguityInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/ArrayPredictionContext \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNConfig \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNConfigSet \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATN \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNDeserializationOptions \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNDeserializer \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNSimulator \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNState \
	UnpackedTarball/afdko/a4/runtime/src/atn/ATNStateType \
	UnpackedTarball/afdko/a4/runtime/src/atn/AtomTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/ContextSensitivityInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/DecisionEventInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/DecisionInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/DecisionState \
	UnpackedTarball/afdko/a4/runtime/src/atn/EpsilonTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/ErrorInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerActionExecutor \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerATNConfig \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerATNSimulator \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerChannelAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerCustomAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerIndexedCustomAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerModeAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerMoreAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerPopModeAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerPushModeAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerSkipAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LexerTypeAction \
	UnpackedTarball/afdko/a4/runtime/src/atn/LL1Analyzer \
	UnpackedTarball/afdko/a4/runtime/src/atn/LookaheadEventInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/NotSetTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/OrderedATNConfigSet \
	UnpackedTarball/afdko/a4/runtime/src/atn/ParseInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/ParserATNSimulator \
	UnpackedTarball/afdko/a4/runtime/src/atn/PrecedencePredicateTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/PredicateEvalInfo \
	UnpackedTarball/afdko/a4/runtime/src/atn/PredicateTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/PredictionContextCache \
	UnpackedTarball/afdko/a4/runtime/src/atn/PredictionContext \
	UnpackedTarball/afdko/a4/runtime/src/atn/PredictionContextMergeCache \
	UnpackedTarball/afdko/a4/runtime/src/atn/PredictionMode \
	UnpackedTarball/afdko/a4/runtime/src/atn/ProfilingATNSimulator \
	UnpackedTarball/afdko/a4/runtime/src/atn/RangeTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/RuleTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/SemanticContext \
	UnpackedTarball/afdko/a4/runtime/src/atn/SetTransition \
	UnpackedTarball/afdko/a4/runtime/src/atn/SingletonPredictionContext \
	UnpackedTarball/afdko/a4/runtime/src/atn/StarLoopbackState \
	UnpackedTarball/afdko/a4/runtime/src/atn/Transition \
	UnpackedTarball/afdko/a4/runtime/src/atn/TransitionType \
	UnpackedTarball/afdko/a4/runtime/src/atn/WildcardTransition \
	UnpackedTarball/afdko/a4/runtime/src/dfa/DFA \
	UnpackedTarball/afdko/a4/runtime/src/dfa/DFASerializer \
	UnpackedTarball/afdko/a4/runtime/src/dfa/DFAState \
	UnpackedTarball/afdko/a4/runtime/src/dfa/LexerDFASerializer \
	UnpackedTarball/afdko/a4/runtime/src/internal/Synchronization \
	UnpackedTarball/afdko/a4/runtime/src/misc/InterpreterDataReader \
	UnpackedTarball/afdko/a4/runtime/src/misc/Interval \
	UnpackedTarball/afdko/a4/runtime/src/misc/IntervalSet \
	UnpackedTarball/afdko/a4/runtime/src/misc/MurmurHash \
	UnpackedTarball/afdko/a4/runtime/src/misc/Predicate \
	UnpackedTarball/afdko/a4/runtime/src/support/Any \
	UnpackedTarball/afdko/a4/runtime/src/support/Arrays \
	UnpackedTarball/afdko/a4/runtime/src/support/CPPUtils \
	UnpackedTarball/afdko/a4/runtime/src/support/StringUtils \
	UnpackedTarball/afdko/a4/runtime/src/support/Utf8 \
	UnpackedTarball/afdko/a4/runtime/src/tree/ErrorNodeImpl \
	UnpackedTarball/afdko/a4/runtime/src/tree/IterativeParseTreeWalker \
	UnpackedTarball/afdko/a4/runtime/src/tree/ParseTree \
	UnpackedTarball/afdko/a4/runtime/src/tree/ParseTreeListener \
	UnpackedTarball/afdko/a4/runtime/src/tree/ParseTreeVisitor \
	UnpackedTarball/afdko/a4/runtime/src/tree/ParseTreeWalker \
	UnpackedTarball/afdko/a4/runtime/src/tree/TerminalNodeImpl \
	UnpackedTarball/afdko/a4/runtime/src/tree/Trees \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/Chunk \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/ParseTreeMatch \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/ParseTreePattern \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/ParseTreePatternMatcher \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/RuleTagToken \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/TagChunk \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/TextChunk \
	UnpackedTarball/afdko/a4/runtime/src/tree/pattern/TokenTagToken \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPath \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathElement \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathLexer \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathLexerErrorListener \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathRuleAnywhereElement \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathRuleElement \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathTokenAnywhereElement \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathTokenElement \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathWildcardAnywhereElement \
	UnpackedTarball/afdko/a4/runtime/src/tree/xpath/XPathWildcardElement \
))

# vim: set noet sw=4 ts=4:
