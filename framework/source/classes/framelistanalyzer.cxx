/*************************************************************************
 *
 *  $RCSfile: framelistanalyzer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:14:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "classes/framelistanalyzer.hxx"

//_______________________________________________
//  my own includes

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_TARGETS_H_
#include <targets.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

//_______________________________________________
//  interface includes

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//_______________________________________________
//  includes of other projects

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
    But it's neccessary to collect top frames here only. Otherwhise we interpret closing of last
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
    // Supress this step, if right detect mode isn't set.
    css::uno::Reference< css::frame::XModel > xReferenceModel;
    if ((m_eDetectMode & E_MODEL) == E_MODEL )
    {
        css::uno::Reference< css::frame::XController > xReferenceController = m_xReferenceFrame->getController();
        if (xReferenceController.is())
            xReferenceModel = xReferenceController->getModel();
    }

    // check, if the reference frame is in hidden mode.
    // But look, if this analyze step is realy needed.
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xReferenceFrame, css::uno::UNO_QUERY);
    if (
        ((m_eDetectMode & E_HIDDEN) == E_HIDDEN) &&
        (xSet.is()                             )
       )
    {
        css::uno::Any aValue = xSet->getPropertyValue(FRAME_PROPNAME_ISHIDDEN);
        aValue >>= m_bReferenceIsHidden;
    }

    // check, if the reference frame includes the backing component.
    // But look, if this analyze step is realy needed.
    if (
        ((m_eDetectMode & E_BACKINGCOMPONENT) == E_BACKINGCOMPONENT) &&
        (xSet.is()                                                 )
       )
    {
        css::uno::Any aValue = xSet->getPropertyValue(FRAME_PROPNAME_ISBACKINGMODE);
        aValue >>= m_bReferenceIsBacking;
    }

    // check, if the reference frame includes the help module.
    // But look, if this analyze step is realy needed.
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
            //    Return it seperated from any return list.
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
            //    Return it seperated from any return list.
            //    But check if the reference task itself is the backing frame.
            //    Our user mst know it to decide right.
            if ((m_eDetectMode & E_BACKINGCOMPONENT) == E_BACKINGCOMPONENT)
            {
                xSet = css::uno::Reference< css::beans::XPropertySet >(xFrame, css::uno::UNO_QUERY);
                sal_Bool bIsBacking;

                if (
                    (xSet.is()                                                        ) &&
                    (xSet->getPropertyValue(FRAME_PROPNAME_ISBACKINGMODE)>>=bIsBacking) &&
                    (bIsBacking                                                       )
                   )
                {
                    m_xBackingComponent = xFrame;
                    continue;
                }
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
                    css::uno::Any aValue = xSet->getPropertyValue(FRAME_PROPNAME_ISHIDDEN);
                    aValue >>= bHidden;
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
        // This interface can't realy guarantee its count for multithreaded
        // environments. So it can occure!
    }

    // Pack both lists by using the actual step positions.
    // All empty or ignorable items should exist at the end of these lists
    // behind the position pointers. So they will be removed by a reallocation.
    m_lOtherVisibleFrames.realloc(nVisibleStep);
    m_lOtherHiddenFrames.realloc(nHiddenStep);
    m_lModelFrames.realloc(nModelStep);
}

} //  namespace framework
