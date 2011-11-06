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



#ifndef _SVX_ACCESSIBLEEMPTYEDITSOURCE_HXX
#define _SVX_ACCESSIBLEEMPTYEDITSOURCE_HXX

#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>

#include <memory>
#include <editeng/unoedsrc.hxx>

class SdrObject;
class SdrView;
class Window;

namespace accessibility
{
    /** Proxy edit source for shapes without text

        Extracted from old SvxDummyEditSource
     */
    class AccessibleEmptyEditSource : public SvxEditSource, public SfxListener, public SfxBroadcaster
    {
    public:
        /** Create proxy edit source for shapes without text

            Since the views don't broadcast their dying, make sure that
            this object gets destroyed if the view becomes invalid

            The window is necessary, since our views can display on multiple windows

            Make sure you only create such an object if the shape _really_
            does not contain text.
        */
        AccessibleEmptyEditSource(  SdrObject& rObj, SdrView& rView, const Window& rViewWindow );
        ~AccessibleEmptyEditSource();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder();
        SvxViewForwarder*       GetViewForwarder();

        SvxEditSource*          Clone() const;

        // this method internally switches from empty to proxy mode,
        // creating an SvxTextEditSource for the functionality.
        SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );

        void                    UpdateData();
        SfxBroadcaster&         GetBroadcaster() const;

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    private:
        void Switch2ProxyEditSource();

        /** Pointer to edit source implementation. This is switched on
            a GetEditViewForwarder( true ) call, to actually create a
            SvxTextEditSource.

            @dyn
         */
        std::auto_ptr< SvxEditSource >  mpEditSource;

        SdrObject&                      mrObj;
        SdrView&                        mrView;
        const Window&                   mrViewWindow;

        bool                            mbEditSourceEmpty;
    };

} // namespace accessibility

#endif

