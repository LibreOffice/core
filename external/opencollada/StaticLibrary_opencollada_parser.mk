# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,opencollada_parser))

$(eval $(call gb_StaticLibrary_use_unpacked,opencollada_parser,opencollada))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,opencollada_parser))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,opencollada_parser,cpp))

$(eval $(call gb_StaticLibrary_use_externals,opencollada_parser,\
	libxml2 \
))

# Avoid warnings
$(eval $(call gb_StaticLibrary_add_cxxflags,opencollada_parser,-w))
$(eval $(call gb_StaticLibrary_add_cflags,opencollada_parser,-w))

# Use libxml2 for xml parsing (other option is expat)
$(eval $(call gb_StaticLibrary_add_defs,opencollada_parser,\
	-DGENERATEDSAXPARSER_XMLPARSER_LIBXML \
	-DGENERATEDSAXPARSER_VALIDATION \
	-DPCRE_STATIC \
))

# OpenCollada parser part
$(eval $(call gb_StaticLibrary_set_include,opencollada_parser,\
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/COLLADABaseUtils/include \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/COLLADABaseUtils/include/Math \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/COLLADAFramework/include \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/COLLADASaxFrameworkLoader/include \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/COLLADASaxFrameworkLoader/include/generated14 \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/COLLADASaxFrameworkLoader/include/generated15 \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/MathMLSolver/include \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/MathMLSolver/include/AST \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/UTF/include \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/GeneratedSaxParser/include \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,opencollada_parser,\
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUHashFunctions \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUIDList \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUNativeString \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUPcreCompiledPattern \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUPrecompiledHeaders \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUStringUtils \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUURI \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/COLLADABUUtils \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/Math/COLLADABUMathMatrix3 \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/Math/COLLADABUMathMatrix4 \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/Math/COLLADABUMathQuaternion \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/Math/COLLADABUMathUtils \
	UnpackedTarball/opencollada/COLLADABaseUtils/src/Math/COLLADABUMathVector3 \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWAxisInfo \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWCamera \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWColor \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWConstants \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWEffect \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWEffectCommon \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWFileInfo \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWFloatOrDoubleArray \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWFormula \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWFormulas \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWGeometry \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWImage \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWInstanceKinematicsScene \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWKinematicsController \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWKinematicsModel \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWKinematicsScene \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWLight \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWLoaderUtils \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWMaterial \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWMatrix \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWMesh \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWMeshPrimitive \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWMorphController \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWNode \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWPrecompiledHeaders \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWRenderState \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWRoot \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWRotate \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWSampler \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWScale \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWSkinController \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWSkinControllerData \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWSpline \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWTexture \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWTransformation \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWTranslate \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWUniqueId \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWValidate \
	UnpackedTarball/opencollada/COLLADAFramework/src/COLLADAFWVisualScene \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLAssetLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLCOLLADACsymbol \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLDocumentProcessor \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLExtraDataElementHandler \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLExtraDataLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLFileLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLFilePartLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLFormulasLinker \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLFormulasLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLGeometryLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLGeometryMaterialIdInfo \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLIError \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLIErrorHandler \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLIExtraDataCallbackHandler \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLIFilePartLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLInputUnshared \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLInstanceArticulatedSystemLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLInstanceKinematicsModelLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLIParserImpl14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLIParserImpl15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLJointsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLKinematicsIntermediateData \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLKinematicsSceneCreator \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryAnimationsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryArticulatedSystemsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryCamerasLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryControllersLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryEffectsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryFormulasLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryImagesLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryJointsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryKinematicsModelsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryKinematicsScenesLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryLightsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryMaterialsLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLibraryNodesLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLMeshLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLMeshPrimitiveInputList \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLNodeLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLPostProcessor \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLPrecompiledHeaders \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLRootParser14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLRootParser15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSaxFWLError \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSaxParserError \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSaxParserErrorHandler \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSceneLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSidAddress \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSidTreeNode \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSourceArrayLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLSplineLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLTransformationLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLTypes \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLVersionParser \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/COLLADASaxFWLVisualSceneLoader \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLAssetLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14Private \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14PrivateEnums \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14PrivateFindElementHash \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14PrivateFunctionMap \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14PrivateFunctionMapFactory \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14PrivateNameMap \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLColladaParserAutoGen14PrivateValidation \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLGeometryLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryAnimationsLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryCamerasLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryControllersLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryEffectsLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryImagesLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryLightsLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryMaterialsLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLLibraryNodesLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLMeshLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLNodeLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLSceneLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLSourceArrayLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLSplineLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated14/COLLADASaxFWLVisualSceneLoader14 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLAssetLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15Private \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15PrivateEnums \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15PrivateFindElementHash \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15PrivateFunctionMap \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15PrivateFunctionMapFactory \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15PrivateNameMap \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLColladaParserAutoGen15PrivateValidation \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLFormulasLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLGeometryLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryAnimationsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryArticulatedSystemsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryCamerasLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryControllersLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryEffectsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryFormulasLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryImagesLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryJointsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryKinematicsModelsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryKinematicsScenesLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryLightsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryMaterialsLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLLibraryNodesLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLMeshLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLNodeLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLSceneLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLSourceArrayLoader15 \
	UnpackedTarball/opencollada/COLLADASaxFrameworkLoader/src/generated15/COLLADASaxFWLVisualSceneLoader15 \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserCoutErrorHandler \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserLibxmlSaxParser \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserNamespaceStack \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserParser \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserParserError \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserParserTemplate \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserParserTemplateBase \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserRawUnknownElementHandler \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserSaxParser \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserStackMemoryManager \
	UnpackedTarball/opencollada/GeneratedSaxParser/src/GeneratedSaxParserUtils \
))

# PCRE external library compiled via OpenCollada
$(eval $(call gb_StaticLibrary_set_include,opencollada_parser,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/pcre/include \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,opencollada_parser,\
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_chartables \
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_compile \
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_exec \
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_globals \
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_newline \
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_tables \
	UnpackedTarball/opencollada/Externals/pcre/src/pcre_try_flipped \
))

# UTF external library compiled via OpenCollada
$(eval $(call gb_StaticLibrary_set_include,opencollada_parser,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/UTF/include \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,opencollada_parser,\
	UnpackedTarball/opencollada/Externals/UTF/src/ConvertUTF \
))

# MathMLSolver external library compiled via OpenCollada
$(eval $(call gb_StaticLibrary_set_include,opencollada_parser,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/MathMLSolver/include \
	-I$(call gb_UnpackedTarball_get_dir,opencollada)/Externals/MathMLSolver/include/AST \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,opencollada_parser,\
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTArithmeticExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTBinaryComparisionExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTConstantExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTFragmentExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTFunctionExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTLogicExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTStringVisitor \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTUnaryArithmeticExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/AST/MathMLASTVariableExpression \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLEvaluatorVisitor \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLSerializationUtil \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLSerializationVisitor \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLSolverFunctionExtensions \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLSolverPrecompiled \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLString \
	UnpackedTarball/opencollada/Externals/MathMLSolver/src/MathMLSymbolTable \
))

# vim: set noet sw=4 ts=4:
