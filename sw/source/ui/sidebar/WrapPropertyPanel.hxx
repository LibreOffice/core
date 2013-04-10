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

#ifndef SW_SIDEBAR_WRAP_PROPERTY_PANEL_HXX
#define SW_SIDEBAR_WRAP_PROPERTY_PANEL_HXX

#include <vcl/button.hxx>
#include <vcl/image.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#include <boost/scoped_ptr.hpp>


namespace sw { namespace sidebar {

    class WrapPropertyPanel
        : public Control
        , public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
    {
    public:
        static WrapPropertyPanel* Create(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
            SfxBindings* pBindings );

        // interface of ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState );

    private:
        WrapPropertyPanel(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* pBindings );

        virtual ~WrapPropertyPanel();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;
        SfxBindings* mpBindings;

        ::boost::scoped_ptr<ImageRadioButton> mpRBNoWrap;
        ::boost::scoped_ptr<ImageRadioButton> mpRBWrapLeft;
        ::boost::scoped_ptr<ImageRadioButton> mpRBWrapRight;
        ::boost::scoped_ptr<ImageRadioButton> mpRBWrapParallel;
        ::boost::scoped_ptr<ImageRadioButton> mpRBWrapThrough;
        ::boost::scoped_ptr<ImageRadioButton> mpRBIdealWrap;

        //Image resource.
        ImageList aWrapIL;
        ImageList aWrapILH;

        //Controler Items==================================
        ::sfx2::sidebar::ControllerItem maSwNoWrapControl;
        ::sfx2::sidebar::ControllerItem maSwWrapLeftControl;
        ::sfx2::sidebar::ControllerItem maSwWrapRightControl;
        ::sfx2::sidebar::ControllerItem maSwWrapParallelControl;
        ::sfx2::sidebar::ControllerItem maSwWrapThroughControl;
        ::sfx2::sidebar::ControllerItem maSwWrapIdealControl;

        void Initialize();

        DECL_LINK(WrapTypeHdl, void*);
    };

} } // end of namespace ::sw::sidebar

#endif
