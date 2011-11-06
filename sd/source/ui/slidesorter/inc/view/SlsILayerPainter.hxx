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



#ifndef SD_SLIDESORTER_VIEW_LAYER_PAINTER_HXX
#define SD_SLIDESORTER_VIEW_LAYER_PAINTER_HXX

#include <boost/shared_ptr.hpp>
#include <sal/types.h>

class OutputDevice;
class Rectangle;

namespace sd { namespace slidesorter { namespace view {

class ILayerInvalidator
{
public:
    virtual void Invalidate (const Rectangle& rInvalidationBox) = 0;
};
typedef ::boost::shared_ptr<ILayerInvalidator> SharedILayerInvalidator;

class ILayerPainter
{
public:
    virtual void SetLayerInvalidator (
        const SharedILayerInvalidator& rpInvalidator) = 0;
    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea) = 0;
};
typedef ::boost::shared_ptr<ILayerPainter> SharedILayerPainter;


} } } // end of namespace ::sd::slidesorter::view

#endif
