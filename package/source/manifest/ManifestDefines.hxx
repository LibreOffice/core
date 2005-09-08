/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ManifestDefines.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:02:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _MANIFEST_DEFINES_HXX
#define _MANIFEST_DEFINES_HXX

#define ELEMENT_MANIFEST "manifest:manifest"
#define ATTRIBUTE_XMLNS "xmlns:manifest"
#define MANIFEST_NAMESPACE "http://openoffice.org/2001/manifest"
#define MANIFEST_OASIS_NAMESPACE "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
#define MANIFEST_DOCTYPE "<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">"
#define ATTRIBUTE_CDATA "CDATA"

#define ELEMENT_FILE_ENTRY "manifest:file-entry"
#define ATTRIBUTE_FULL_PATH "manifest:full-path"
#define ATTRIBUTE_MEDIA_TYPE "manifest:media-type"
#define ATTRIBUTE_SIZE "manifest:size"

#define ELEMENT_ENCRYPTION_DATA "manifest:encryption-data"
#define ATTRIBUTE_CHECKSUM_TYPE "manifest:checksum-type"
#define ATTRIBUTE_CHECKSUM "manifest:checksum"

#define ELEMENT_ALGORITHM "manifest:algorithm"
#define ATTRIBUTE_ALGORITHM_NAME "manifest:algorithm-name"
#define ATTRIBUTE_INITIALISATION_VECTOR "manifest:initialisation-vector"

#define ELEMENT_KEY_DERIVATION "manifest:key-derivation"
#define ATTRIBUTE_KEY_DERIVATION_NAME "manifest:key-derivation-name"
#define ATTRIBUTE_SALT "manifest:salt"
#define ATTRIBUTE_ITERATION_COUNT "manifest:iteration-count"
#define CHECKSUM_TYPE "SHA1/1K"

#endif
