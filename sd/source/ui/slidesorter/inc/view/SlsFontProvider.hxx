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



#ifndef SD_SLIDESORTER_VIEW_FONT_PROVIDER_HXX
#define SD_SLIDESORTER_VIEW_FONT_PROVIDER_HXX

#include "tools/SdGlobalResourceContainer.hxx"

#include <boost/shared_ptr.hpp>
#include <vcl/mapmod.hxx>

class Font;
class OutputDevice;
class VclWindowEvent;

namespace sd { namespace slidesorter { namespace view {

/** This simple singleton class provides fonts that are scaled so that they
    have a fixed height on the given device regardless of its map mode.
*/
class FontProvider
    : public SdGlobalResource
{
public:
    typedef ::boost::shared_ptr<Font> SharedFontPointer;

    /** Return the single instance of this class.  Throws a RuntimeException
        when no instance can be created.
    */
    static FontProvider& Instance (void);

    /** Return a font that is scaled according to the current map mode of
        the given device.  Repeated calls with a device, not necessarily the
        same device, with the same map mode will return the same font.  The
        first call with a different map mode will release the old font and
        create a new one that is correctly scaled.
    */
    SharedFontPointer GetFont (const OutputDevice& rDevice);

    /** Call this method to tell an object to release its currently used
        font.  The next call to GetFont() will then create a new one.
        Typically called after a DataChange event when for instance a system
        font has been modified.
    */
    void Invalidate (void);

private:
    static FontProvider* mpInstance;

    /** The font that was created by a previous call to GetFont().  It is
        replaced by another one only when GetFont() is called with a device
        with a different map mode or by a call to Invalidate().
    */
    SharedFontPointer maFont;
    /** The mape mode for which maFont was created.
    */
    MapMode maMapMode;

    FontProvider (void);
    virtual ~FontProvider (void);

    // Copy constructor is not implemented.
    FontProvider (const FontProvider&);
    // Assignment operator is not implemented.
    FontProvider& operator= (const FontProvider&);
};

} } } // end of namespace ::sd::slidesorter::view

#endif
