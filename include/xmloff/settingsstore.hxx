/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
