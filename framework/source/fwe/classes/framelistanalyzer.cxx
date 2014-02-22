/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "framework/framelistanalyzer.hxx"

#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <targets.h>
#include <properties.h>
#include <services.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

namespace framework{

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

/**
 */

FrameListAnalyzer::~FrameListAnalyzer()
{
}

/** returns an analyzed list of all currently opened (top!) frames inside the desktop tree.

    We try to get a snapshot of all opened frames, which are part of the desktop frame container.
    Of course we can't access frames, which stands outside of this tree.
    But it's necessary to collect top frames here only. Otherwhise we interpret closing of last
    frame wrong. Further we analyze this list and split into different parts.
    E.g. for "CloseDoc" we must know, which frames of the given list referr to the same model.
    These frames must be closed then. But all other frames must be untouched.
    In case the request was "CloseWin" these splitted lists can be used too, to decide if the last window
    or document was closed. Then we have to initialize the backing window ...
    Last but not least we must know something about our special help frame. It must be handled
    separately. And last but not least - the backing component frame must be detected too.
*/

void FrameListAnalyzer::impl_analyze()
{
    
    m_bReferenceIsHidden  = sal_False;
    m_bReferenceIsHelp    = sal_False;
    m_bReferenceIsBacking = sal_False;
    m_xHelp               = css::uno::Reference< css::frame::XFrame >();
    m_xBackingComponent   = css::uno::Reference< css::frame::XFrame >();

    
    css::uno::Reference< css::container::XIndexAccess > xFrameContainer(m_xSupplier->getFrames(), css::uno::UNO_QUERY);

    
    
    sal_Int32 nVisibleStep = 0;
    sal_Int32 nHiddenStep  = 0;
    sal_Int32 nModelStep   = 0;
    sal_Int32 nCount       = xFrameContainer->getCount();

    m_lOtherVisibleFrames.realloc(nCount);
    m_lOtherHiddenFrames.realloc(nCount);
    m_lModelFrames.realloc(nCount);

    
    
    
    
    css::uno::Reference< css::frame::XModel > xReferenceModel;
    if ((m_eDetectMode & E_MODEL) == E_MODEL )
    {
        css::uno::Reference< css::frame::XController > xReferenceController;
        if (m_xReferenceFrame.is())
            xReferenceController = m_xReferenceFrame->getController();
        if (xReferenceController.is())
            xReferenceModel = xReferenceController->getModel();
    }

    
    
    css::uno::Reference< css::beans::XPropertySet > xSet(m_xReferenceFrame, css::uno::UNO_QUERY);
    if (
        ((m_eDetectMode & E_HIDDEN) == E_HIDDEN) &&
        (xSet.is()                             )
       )
    {
        xSet->getPropertyValue(FRAME_PROPNAME_ISHIDDEN) >>= m_bReferenceIsHidden;
    }

    
    
    if (((m_eDetectMode & E_BACKINGCOMPONENT) == E_BACKINGCOMPONENT) && m_xReferenceFrame.is() )
    {
        try
        {
            css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            css::uno::Reference< css::frame::XModuleManager2 > xModuleMgr = css::frame::ModuleManager::create(xContext);
            OUString sModule = xModuleMgr->identify(m_xReferenceFrame);
            m_bReferenceIsBacking = sModule.equals("com.sun.star.frame.StartModule");
        }
        catch(const css::frame::UnknownModuleException&)
        {
        }
        catch(const css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    
    
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
        
        for (sal_Int32 i=0; i<nCount; ++i)
        {
            
            
            
            css::uno::Reference< css::frame::XFrame > xFrame;
            if (
                !(xFrameContainer->getByIndex(i) >>= xFrame) ||
                !(xFrame.is()                              ) ||
                 (xFrame==m_xReferenceFrame                )
               )
                continue;

            if (
                ((m_eDetectMode & E_ZOMBIE) == E_ZOMBIE) &&
                (
                 (!xFrame->getContainerWindow().is()) ||
                 (!xFrame->getComponentWindow().is())
                )
               )
            {
                SAL_INFO("fwk", "FrameListAnalyzer::impl_analyze(): ZOMBIE!");
            }

            
            
            
            if (
                ((m_eDetectMode & E_HELP) == E_HELP      ) &&
                (xFrame->getName()==SPECIALTARGET_HELPTASK)
               )
            {
                m_xHelp = xFrame;
                continue;
            }

            
            
            
            
            
            if ((m_eDetectMode & E_BACKINGCOMPONENT) == E_BACKINGCOMPONENT)
            {
                try
                {
                    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                    css::uno::Reference< css::frame::XModuleManager2 > xModuleMgr = css::frame::ModuleManager::create(xContext);
                    OUString sModule = xModuleMgr->identify(xFrame);
                    if (sModule.equals("com.sun.star.frame.StartModule"))
                    {
                        m_xBackingComponent = xFrame;
                        continue;
                    }
                }
                catch (const css::uno::Exception&)
                {
                }
            }

            
            
            
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
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        
        
        
    }

    
    
    
    m_lOtherVisibleFrames.realloc(nVisibleStep);
    m_lOtherHiddenFrames.realloc(nHiddenStep);
    m_lModelFrames.realloc(nModelStep);
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
