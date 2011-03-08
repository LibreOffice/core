/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _MANIFEST_DEFINES_HXX
#define _MANIFEST_DEFINES_HXX

#include <PackageConstants.hxx>

#define MANIFEST_NSPREFIX "manifest:"
#define ELEMENT_MANIFEST "manifest:manifest"
#define ATTRIBUTE_XMLNS "xmlns:manifest"
#define MANIFEST_NAMESPACE "http://openoffice.org/2001/manifest"
#define MANIFEST_OASIS_NAMESPACE "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
#define MANIFEST_DOCTYPE "<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">"
#define ATTRIBUTE_CDATA "CDATA"

#define ELEMENT_FILE_ENTRY "manifest:file-entry"
#define ATTRIBUTE_FULL_PATH "manifest:full-path"
#define ATTRIBUTE_VERSION "manifest:version"
#define ATTRIBUTE_MEDIA_TYPE "manifest:media-type"
#define ATTRIBUTE_SIZE "manifest:size"

#define ELEMENT_ENCRYPTION_DATA "manifest:encryption-data"
#define ATTRIBUTE_CHECKSUM_TYPE "manifest:checksum-type"
#define ATTRIBUTE_CHECKSUM "manifest:checksum"

#define ELEMENT_ALGORITHM "manifest:algorithm"
#define ATTRIBUTE_ALGORITHM_NAME "manifest:algorithm-name"
#define ATTRIBUTE_INITIALISATION_VECTOR "manifest:initialisation-vector"

#define ELEMENT_START_KEY_GENERATION "manifest:start-key-generation"
#define ATTRIBUTE_START_KEY_GENERATION_NAME "manifest:start-key-generation-name"
#define ALGORITHM_SHA1 "SHA1"
#define ATTRIBUTE_KEY_SIZE "manifest:key-size"
#define START_KEY_SIZE "20"

#define ELEMENT_KEY_DERIVATION "manifest:key-derivation"
#define ATTRIBUTE_KEY_DERIVATION_NAME "manifest:key-derivation-name"
#define ATTRIBUTE_SALT "manifest:salt"
#define ATTRIBUTE_ITERATION_COUNT "manifest:iteration-count"
#define CHECKSUM_TYPE "SHA1/1K"
#define DERIVED_KEY_SIZE "16"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
