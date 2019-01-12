/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Simple interface to allow serialization of document settings

#ifndef INCLUDED_XMLOFF_SETTINGSSTORE_HXX
#define INCLUDED_XMLOFF_SETTINGSSTORE_HXX

#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace embed { class XStorage; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

// Scans list of properties for certain URL properties that could refer
// to internal objects, and initializes from these.
class SAL_DLLPUBLIC_RTTI DocumentSettingsSerializer {
public:
  // Import objects and update properties (eliding URLs)
  virtual css::uno::Sequence<css::beans::PropertyValue>
    filterStreamsFromStorage(
        OUString const & referer,
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps ) = 0;
  // Export objects and update properties with relative URLs into this storage
  virtual css::uno::Sequence<css::beans::PropertyValue>
    filterStreamsToStorage(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps ) = 0;

protected:
    ~DocumentSettingsSerializer() {}
};

#endif // INCLUDED_XMLOFF_SETTINGSSTORE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
