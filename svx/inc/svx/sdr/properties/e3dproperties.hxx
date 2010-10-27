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

#ifndef _SDR_PROPERTIES_E3DPROPERTIES_HXX
#define _SDR_PROPERTIES_E3DPROPERTIES_HXX

#include <svx/sdr/properties/attributeproperties.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class SVX_DLLPUBLIC E3dProperties : public AttributeProperties
        {
        protected:
            // create a new itemset
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& rPool);

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet);

        public:
            // basic constructor
            E3dProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            E3dProperties(const E3dProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~E3dProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_PROPERTIES_E3DPROPERTIES_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
