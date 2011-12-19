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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGPAINT_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGPAINT_HXX

#include <svgio/svgiodllapi.h>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgPaint
        {
        private:
            basegfx::BColor     maColor;

            /// bitfield
            bool                mbSet : 1;
            bool                mbOn : 1;
            bool                mbCurrent : 1;

        public:
            SvgPaint(const basegfx::BColor& rColor = basegfx::BColor(0.0, 0.0, 0.0), bool bSet = false, bool bOn = false, bool bCurrent = false)
            :   maColor(rColor),
                mbSet(bSet),
                mbOn(bOn),
                mbCurrent(bCurrent)
            {
            }

            const basegfx::BColor& getBColor() const { return maColor; }
            bool isSet() const { return mbSet; }
            bool isOn() const { return mbOn; }
            bool isCurrent() const { return mbCurrent; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGPAINT_HXX

// eof
