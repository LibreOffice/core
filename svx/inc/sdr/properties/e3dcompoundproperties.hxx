/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SDR_PROPERTIES_E3DCOMPOUNDPROPERTIES_HXX
#define INCLUDED_SVX_SDR_PROPERTIES_E3DCOMPOUNDPROPERTIES_HXX

#include <sdr/properties/e3dproperties.hxx>
#include <svx/svxdllapi.h>


namespace sdr
{
    namespace properties
    {
        class E3dCompoundProperties : public E3dProperties
        {
        protected:
            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich) override;

        public:
            // basic constructor
            explicit E3dCompoundProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            E3dCompoundProperties(const E3dCompoundProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~E3dCompoundProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const override;

            // get itemset
            virtual const SfxItemSet& GetObjectItemSet() const override;

            // Get merged ItemSet. Normally, this maps directly to GetObjectItemSet(), but may
            // be overridden e.g for group objects to return a merged ItemSet of the object.
            // When using this method the returned ItemSet may contain items in the state
            // SfxItemState::DONTCARE which means there were several such items with different
            // values.
            virtual const SfxItemSet& GetMergedItemSet() const override;

            // Set merged ItemSet. Normally, this maps to SetObjectItemSet().
            virtual void SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems = false) override;
        };
    } // end of namespace properties
} // end of namespace sdr


#endif // INCLUDED_SVX_SDR_PROPERTIES_E3DCOMPOUNDPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
