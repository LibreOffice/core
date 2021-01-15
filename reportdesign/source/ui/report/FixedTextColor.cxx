/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <FixedTextColor.hxx>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <RptObject.hxx>
#include <RptModel.hxx>
#include <RptPage.hxx>
#include <ReportSection.hxx>
#include <ReportController.hxx>
#include <strings.hxx>

#include <tools/color.hxx>

// DBG_UNHANDLED_EXCEPTION
#include <tools/diagnose_ex.h>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

namespace rptui
{
    using namespace ::com::sun::star;


    FixedTextColor::FixedTextColor(const OReportController& _aController)
            :m_rReportController(_aController)
    {
    }


    FixedTextColor::~FixedTextColor()
    {
    }


    void FixedTextColor::notifyPropertyChange( const beans::PropertyChangeEvent& _rEvent )
    {
        uno::Reference< report::XFixedText > xFixedText( _rEvent.Source, uno::UNO_QUERY );
        if ( ! xFixedText.is() )
        {
            return;
        }

        try
        {
            uno::Reference< lang::XComponent > xComponent( xFixedText, uno::UNO_QUERY_THROW );
            handle(xComponent);
        }
        catch (uno::Exception const&)
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }


    void FixedTextColor::setPropertyTextColor(const uno::Reference< awt::XVclWindowPeer >& _xVclWindowPeer, Color _nTextColor)
    {
        _xVclWindowPeer->setProperty(PROPERTY_TEXTCOLOR, uno::makeAny(_nTextColor));
    }


    void FixedTextColor::notifyElementInserted( const uno::Reference< uno::XInterface >& _rxElement )
    {
        handle(_rxElement);
    }


    void FixedTextColor::handle( const uno::Reference< uno::XInterface >& _rxElement )
    {
        uno::Reference< report::XFixedText > xFixedText( _rxElement, uno::UNO_QUERY );
        if ( ! xFixedText.is() )
        {
            return;
        }

        try
        {
            bool bIsDark = false;
            const Color nBackColor( ColorTransparency, xFixedText->getControlBackground() );
            if (nBackColor == COL_TRANSPARENT)
            {
                uno::Reference <report::XSection> xSection(xFixedText->getParent(), uno::UNO_QUERY_THROW);

                bool bSectionBackColorIsTransparent = xSection->getBackTransparent();
                if (bSectionBackColorIsTransparent)
                {
                    // Label Transparent, Section Transparent set LabelTextColor
                    const StyleSettings& aStyleSettings = Application::GetSettings().GetStyleSettings();
                    Color aWindowColor  = aStyleSettings.GetWindowColor();
                    bIsDark = aWindowColor.IsDark();
                }
                else
                {
                    css::util::Color aColor2 = xSection->getBackColor();
                    Color aBackColor(ColorTransparency, aColor2);
                    bIsDark = aBackColor.IsDark();
                }
            }
            else
            {
                Color aLabelBackColor(nBackColor);
                bIsDark = aLabelBackColor.IsDark();
            }

            uno::Reference<awt::XVclWindowPeer> xVclWindowPeer = getVclWindowPeer(xFixedText);
            if (bIsDark)
            {
                const StyleSettings& aStyleSettings = Application::GetSettings().GetStyleSettings();
                Color aLabelTextColor  = aStyleSettings.GetLabelTextColor();
                setPropertyTextColor(xVclWindowPeer, aLabelTextColor);
            }
            else
            {
                util::Color aLabelColor = xFixedText->getCharColor();
                setPropertyTextColor(xVclWindowPeer, ::Color(ColorTransparency, aLabelColor));
            }

        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }


    // XPropertyChangeListener
    uno::Reference<awt::XControl> FixedTextColor::getXControl(const uno::Reference< report::XFixedText >& _xFixedText)
    {

        uno::Reference<awt::XControl> xControl;
        OReportController *pController = const_cast<OReportController *>(&m_rReportController);

        std::shared_ptr<OReportModel> pModel = pController->getSdrModel();

        uno::Reference<report::XSection> xSection(_xFixedText->getSection());
        if ( xSection.is() )
        {
            OReportPage *pPage = pModel->getPage(xSection);
            const size_t nIndex = pPage->getIndexOf(_xFixedText.get());
            if (nIndex < pPage->GetObjCount() )
            {
                SdrObject *pObject = pPage->GetObj(nIndex);
                OUnoObject* pUnoObj = dynamic_cast<OUnoObject*>(pObject);
                if ( pUnoObj ) // this doesn't need to be done for shapes
                {
                    OSectionWindow* pSectionWindow = pController->getSectionWindow(xSection);
                    if (pSectionWindow != nullptr)
                    {
                        OReportSection& aOutputDevice = pSectionWindow->getReportSection(); // OutputDevice
                        OSectionView& aSdrView = aOutputDevice.getSectionView(); // SdrView
                        xControl = pUnoObj->GetUnoControl(aSdrView, aOutputDevice);
                    }
                }
            }
        }
        return xControl;
    }


    uno::Reference<awt::XVclWindowPeer> FixedTextColor::getVclWindowPeer(const uno::Reference< report::XFixedText >& _xComponent)
    {
        uno::Reference<awt::XVclWindowPeer> xVclWindowPeer;
        uno::Reference<awt::XControl> xControl = getXControl(_xComponent);

        xVclWindowPeer.set( xControl->getPeer(), uno::UNO_QUERY);

        return xVclWindowPeer;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
