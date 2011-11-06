/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            E3dSphereProperties(SdrObject& rObj);

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

// eof

