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

#include "precompiled_sfx2.hxx"

#include "sfx2/sidebar/ControllerFactory.hxx"
#include "sfx2/sidebar/CommandInfoProvider.hxx"
#include "sfx2/sidebar/Tools.hxx"

#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <framework/sfxhelperfunctions.hxx>
#include <svtools/generictoolboxcontroller.hxx>
#include <comphelper/processfactory.hxx>


using namespace css;
using namespace cssu;
using ::rtl::OUString;


namespace sfx2 { namespace sidebar {

Reference<frame::XToolbarController> ControllerFactory::CreateToolBoxController(
    ToolBox* pToolBox,
    const sal_uInt16 nItemId,
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    // Create a controller for the new item.
    Reference<frame::XToolbarController> xController(
        static_cast<XWeak*>(::framework::CreateToolBoxController(
                rxFrame,
                pToolBox,
                nItemId,
                rsCommandName)),
            UNO_QUERY);
    if ( ! xController.is())
        xController.set(
            static_cast<XWeak*>(new svt::GenericToolboxController(
                    ::comphelper::getProcessServiceFactory(),
                    rxFrame,
                    pToolBox,
                    nItemId,
                    rsCommandName)),
            UNO_QUERY);

    // Initialize the controller with eg a service factory.
    Reference<lang::XInitialization> xInitialization (xController, UNO_QUERY);
    if (xInitialization.is())
    {
        beans::PropertyValue aPropValue;
        std::vector<Any> aPropertyVector;

        aPropValue.Name = A2S("Frame");
        aPropValue.Value <<= rxFrame;
        aPropertyVector.push_back(makeAny(aPropValue));

        aPropValue.Name = A2S("ServiceManager");
        aPropValue.Value <<= ::comphelper::getProcessServiceFactory();
        aPropertyVector.push_back(makeAny(aPropValue));

        aPropValue.Name = A2S("CommandURL");
        aPropValue.Value <<= rsCommandName;
        aPropertyVector.push_back(makeAny(aPropValue));

        Sequence<Any> aArgs (comphelper::containerToSequence(aPropertyVector));
        xInitialization->initialize(aArgs);
    }

    Reference<util::XUpdatable> xUpdatable (xController, UNO_QUERY);
    if (xUpdatable.is())
        xUpdatable->update();

    // Add label.
    if (xController.is())
    {
        const OUString sLabel (sfx2::sidebar::CommandInfoProvider::Instance().GetLabelForCommand(
                rsCommandName,
                rxFrame));
        pToolBox->SetQuickHelpText(nItemId, sLabel);
        pToolBox->EnableItem(nItemId);
    }

    return xController;
}


} } // end of namespace sfx2::sidebar
