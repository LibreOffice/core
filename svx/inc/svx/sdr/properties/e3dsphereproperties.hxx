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

#ifndef _SDR_PROPERTIES_E3DSPHEREPROPERTIES_HXX
#define _SDR_PROPERTIES_E3DSPHEREPROPERTIES_HXX

#include <svx/sdr/properties/e3dcompoundproperties.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class E3dSphereProperties : public E3dCompoundProperties
        {
        protected:
            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich);

        public:
            // basic constructor
            explicit E3dSphereProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            E3dSphereProperties(const E3dSphereProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~E3dSphereProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_PROPERTIES_E3DSPHEREPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
