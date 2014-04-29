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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "framework/framelistanalyzer.hxx"

//_______________________________________________
//  my own includes
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <targets.h>
#include <properties.h>
#include <services.h>

//_______________________________________________
//  interface includes
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

//_______________________________________________
//  includes of other projects
#include <unotools/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

//_______________________________________________
//  namespace

namespace framework{

//_______________________________________________
//  non exported const

//_______________________________________________
//  non exported definitions

//_______________________________________________
//  declarations

//_______________________________________________

/**
 */

FrameListAnalyzer::FrameListAnalyzer( const css::uno::Reference< css::frame::XFramesSupplier >& xSupplier       ,
                                      const css::uno::Reference< css::frame::XFrame >&          xReferenceFrame ,
                                            sal_uInt32                                          eDetectMode     )
    : m_xSupplier      (xSupplier      )
    , m_xReferenceFrame(xReferenceFrame)
    , m_eDetectMode    (eDetectMode    )
{
    impl_analyze();
}

//_______________________________________________

/**
 */

FrameListAnalyzer::~FrameListAnalyzer()
{
}

//_______________________________________________

/** returns an analyzed list of all currently opened (top!) frames inside the desktop tree.

    We try to get a snapshot of all opened frames, which are part of the desktop frame container.
    Of course we can't access frames, which stands outside of this tree.
    But it's necessary to collect top frames here only. Otherwise we interpret closing of last
    frame wrong. Further we analyze this list and split into different parts.
    E.g. for "CloseDoc" we must know, which frames of the given list referr to the same model.
    These frames must be closed then. But all other frames must be untouched.
    In case the request was "CloseWin" these splitted lists can be used too, to decide if the last window
    or document was closed. Then we have to initialize the backing window ...
    Last but not least we must know something about our special help frame. It must be handled
    seperatly. And last but not least - the backing component frame must be detected too.
*/

void FrameListAnalyzer::impl_analyze()
{
    // reset all members to get a consistent state
    m_bReferenceIsHidden  = sal_False;
    m_bReferenceIsHelp    = sal_False;
    m_bReferenceIsBacking = sal_False;
    m_xHelp               = css::uno::Reference< css::frame::XFrame >();
    m_xBackingComponent   = css::uno::Reference< css::frame::XFrame >();

    // try to get the task container by using the given supplier
    css::uno::Reference< css::container::XIndexAccess > xFrameContainer(m_xSupplier->getFrames(), css::uno::UNO_QUERY);

    // All return list get an initial size to include all possible frames.
    // They will be packed at the end of this method ... using the actual step positions then.
    sal_Int32 nVisibleStep = 0;
    sal_Int32 nHiddenStep  = 0;
    sal_Int32 nModelStep   = 0;
    sal_Int32 nCount       = xFrameContainer->getCount();

    m_lOtherVisibleFrames.realloc(nCount);
    m_lOtherHiddenFrames.realloc(nCount);
    m_lModelFrames.realloc(nCount);

    // ask for the model of the given reference frame.
    // It must be compared with the model of every frame of the container
    // to sort it into the list of frames with the same model.
    // Suppress this step, if right detect mode isn't set.
    css::uno::Reference< css::frame::XModel > xReferenceModel;
    if ((m_eDetectMode & E_MODEL) == E_MODEL )
    {
        css::uno::Reference< css::frame::XController > xReferenceController;
        if (m_xReferenceFrame.is())
            xReferenceController = m_xReferenceFrame->getController();
        if (xReferenceController.is())
            xReferenceModel = xReferenceController->getModel();
    }

    // check, if the reference frame is in hidden mode.
    // But look, if this analyze step is really needed.
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xReferenceFrame, css::uno::UNO_QUERY);
    if (
        ((m_eDetectMode & E_HIDDEN) == E_HIDDEN) &&
        (xSet.is()                             )
       )
    {
        xSet->getPropertyValue(FRAME_PROPNAME_ISHIDDEN) >>= m_bReferenceIsHidden;
    }

    // check, if the reference frame includes the backing component.
    // But look, if this analyze step is really needed.
    if (((m_eDetectMode & E_BACKINGCOMPONENT) == E_BACKINGCOMPONENT) && m_xReferenceFrame.is() )
    {
        try
        {
            css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::utl::getProcessServiceFactory();
            css::uno::Reference< css::frame::XModuleManager > xModuleMgr(xSMGR->createInstance(SERVICENAME_MODULEMANAGER), css::uno::UNO_QUERY_THROW);
            ::rtl::OUString sModule = xModuleMgr->identify(m_xReferenceFrame);
            m_bReferenceIsBacking = (sModule.equals(SERVICENAME_STARTMODULE));
        }
        catch(const css::frame::UnknownModuleException&)
        {
        }
        catch(const css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // check, if the reference frame includes the help module.
    // But look, if this analyze step is really needed.
    if (
        ((m_eDetectMode & E_HELP)     == E_HELP                ) &&
        (m_xReferenceFrame.is()                                ) &&
        (m_xReferenceFrame->getName() == SPECIALTARGET_HELPTASK)
       )
    {
        m_bReferenceIsHelp = sal_True;
    }

    try
    {
        // Step over all frames of the desktop frame container and analyze it.
        for (sal_Int32 i=0; i<nCount; ++i)
        {
            // Ignore invalid items ... and of course the reference frame.
            // It will be a member of the given frame list too - but it was already
            // analyzed before!
            css::uno::Reference< css::frame::XFrame > xFrame;
            if (
                !(xFrameContainer->getByIndex(i) >>= xFrame) ||
                !(xFrame.is()                              ) ||
                 (xFrame==m_xReferenceFrame                )
               )
                continue;

            #ifdef ENABLE_WARNINGS
            if (
                ((m_eDetectMode & E_ZOMBIE) == E_ZOMBIE) &&
                (
                 (!xFrame->getContainerWindow().is()) ||
                 (!xFrame->getComponentWindow().is())
                )
               )
            {
                LOG_WARNING("FrameListAnalyzer::impl_analyze()", "ZOMBIE!")
            }
            #endif

            // -------------------------------------------------
            // a) Is it the special help task?
            //    Return it separated from any return list.
            if (
                ((m_eDetectMode & E_HELP) == E_HELP      ) &&
                (xFrame->getName()==SPECIALTARGET_HELPTASK)
               )
            {
                m_xHelp = xFrame;
                continue;
            }

            // -------------------------------------------------
            // b) Or is includes this task the special backing component?
            //    Return it separated from any return list.
            //    But check if the reference task itself is the backing frame.
            //    Our user mst know it to decide right.
            if ((m_eDetectMode & E_BACKINGCOMPONENT) == E_BACKINGCOMPONENT)
            {
                try
                {
                    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::utl::getProcessServiceFactory();
                    css::uno::Reference< css::frame::XModuleManager > xModuleMgr(xSMGR->createInstance(SERVICENAME_MODULEMANAGER), css::uno::UNO_QUERY);
                    ::rtl::OUString sModule = xModuleMgr->identify(xFrame);
                    if (sModule.equals(SERVICENAME_STARTMODULE))
                    {
                        m_xBackingComponent = xFrame;
                        continue;
                    }
                }
                catch(const css::uno::Exception&)
                    {}
            }

            // -------------------------------------------------
            // c) Or is it the a task, which uses the specified model?
            //    Add it to the list of "model frames".
            if ((m_eDetectMode & E_MODEL) == E_MODEL)
            {
                css::uno::Reference< css::frame::XController > xController = xFrame->getController();
                css::uno::Reference< css::frame::XModel >      xModel      ;
                if (xController.is())
                    xModel = xController->getModel();
                if (xModel==xReferenceModel)
                {
                    m_lModelFrames[nModelStep] = xFrame;
                    ++nModelStep;
                    continue;
                }
            }

            // -------------------------------------------------
            // d) Or is it the a task, which use another or no model at all?
            //    Add it to the list of "other frames". But look for it's
            //    visible state ... if it's allowed to do so.
            // -------------------------------------------------
            sal_Bool bHidden = sal_False;
            if ((m_eDetectMode & E_HIDDEN) == E_HIDDEN )
            {
                xSet = css::uno::Reference< css::beans::XPropertySet >(xFrame, css::uno::UNO_QUERY);
                if (xSet.is())
                {
                    xSet->getPropertyValue(FRAME_PROPNAME_ISHIDDEN) >>= bHidden;
                }
            }

            if (bHidden)
            {
                m_lOtherHiddenFrames[nHiddenStep] = xFrame;
                ++nHiddenStep;
            }
            else
            {
                m_lOtherVisibleFrames[nVisibleStep] = xFrame;
                ++nVisibleStep;
            }
        }
    }
    catch(css::lang::IndexOutOfBoundsException)
    {
        // stop copying if index seams to be wrong.
        // This interface can't really guarantee its count for multithreaded
        // environments. So it can occur!
    }

    // Pack both lists by using the actual step positions.
    // All empty or ignorable items should exist at the end of these lists
    // behind the position pointers. So they will be removed by a reallocation.
    m_lOtherVisibleFrames.realloc(nVisibleStep);
    m_lOtherHiddenFrames.realloc(nHiddenStep);
    m_lModelFrames.realloc(nModelStep);
}

} //  namespace framework
