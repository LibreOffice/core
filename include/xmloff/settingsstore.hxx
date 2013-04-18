/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Novell, Inc. <michael.meeks@novell.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// Simple interface to allow serialization of document settings

#ifndef _XMLOFF_SETTINGS_STORE_HXX
#define _XMLOFF_SETTINGS_STORE_HXX

#include <vector>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XStorage.hpp>

// Scans list of properties for certain URL properties that could refer
// to internal objects, and initializes from these.
class DocumentSettingsSerializer {
public:
  // Import objects and update properties (eliding URLs)
  virtual com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>
	filterStreamsFromStorage(
		const com::sun::star::uno::Reference< com::sun::star::embed::XStorage > &xStorage,
		const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps ) = 0;
  // Export objects and update properties with relative URLs into this storage
  virtual com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>
	filterStreamsToStorage(
		const com::sun::star::uno::Reference< com::sun::star::embed::XStorage > &xStorage,
		const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps ) = 0;

protected:
    ~DocumentSettingsSerializer() {}
};

#endif // _XMLOFF_SETTINGS_STORE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
