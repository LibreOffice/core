/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_reportdesign.hxx"

#include <FormattedFieldBeautifier.hxx>

#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/report/XImageControl.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/InvalidateStyle.hpp>

#include <RptObject.hxx>
#include <RptModel.hxx>
#include <RptPage.hxx>
#include <ViewsWindow.hxx>
#include <ReportSection.hxx>
#include <ReportController.hxx>
#include <uistrings.hrc>
#include <reportformula.hxx>
#include <toolkit/helper/property.hxx>

#include <svtools/extcolorcfg.hxx>
#include <unotools/confignode.hxx>

// DBG_*
#include <tools/debug.hxx>
// DBG_UNHANDLED_EXCEPTION
#include <tools/diagnose_ex.h>

namespace rptui
{
    using namespace ::com::sun::star;

    DBG_NAME(rpt_FormattedFieldBeautifier)

    //--------------------------------------------------------------------
    FormattedFieldBeautifier::FormattedFieldBeautifier(const OReportController& _aController)
        :m_rReportController(_aController)
        ,m_nTextColor(-1)
    {
        DBG_CTOR(rpt_FormattedFieldBeautifier, NULL);
    }

    //--------------------------------------------------------------------
    sal_Int32 FormattedFieldBeautifier::getTextColor()
    {
        if (m_nTextColor == -1)
        {
            svtools::ExtendedColorConfig aConfig;
            m_nTextColor = aConfig.GetColorValue(CFG_REPORTDESIGNER, DBTEXTBOXBOUNDCONTENT).getColor();
        }
        return m_nTextColor;
    }

    //--------------------------------------------------------------------
    FormattedFieldBeautifier::~FormattedFieldBeautifier()
    {
        DBG_DTOR(rpt_FormattedFieldBeautifier,NULL);
    }

    // -----------------------------------------------------------------------------
    void FormattedFieldBeautifier::setPlaceholderText( const uno::Reference< uno::XInterface >& _rxComponent )
    {
        ::rtl::OUString sDataField;

        try
        {
            uno::Reference< report::XFormattedField > xControlModel( _rxComponent, uno::UNO_QUERY );
            if ( xControlModel.is() )
            {
                sDataField = xControlModel->getDataField();

                if ( sDataField.getLength() )
                {
                    ReportFormula aFormula( sDataField );
                    bool bSet = true;
                    if ( aFormula.getType() == ReportFormula::Field )
                    {
                        const ::rtl::OUString sColumnName = aFormula.getFieldName();
                        ::rtl::OUString sLabel = m_rReportController.getColumnLabel_throw(sColumnName);
                        if ( sLabel.getLength() )
                        {
                            ::rtl::OUStringBuffer aBuffer;
                            aBuffer.appendAscii( "=" );
                            aBuffer.append( sLabel );
                            sDataField = aBuffer.makeStringAndClear();
                            bSet = false;
                        }
                    }
                    if ( bSet )
                        sDataField = aFormula.getEqualUndecoratedContent();
                }
            }

            if ( xControlModel.is() )
                setPlaceholderText( getVclWindowPeer( xControlModel.get() ), sDataField );
        }
        catch (uno::Exception)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // -----------------------------------------------------------------------------
    void FormattedFieldBeautifier::setPlaceholderText( const uno::Reference< awt::XVclWindowPeer >& _xVclWindowPeer, const ::rtl::OUString& _rText )
    {
        OSL_ENSURE( _xVclWindowPeer.is(), "FormattedFieldBeautifier::setPlaceholderText: invalid peer!" );
        if ( !_xVclWindowPeer.is() )
            throw uno::RuntimeException();

        // the actual text
        _xVclWindowPeer->setProperty(PROPERTY_TEXT, uno::makeAny(_rText));
        // the text color
        _xVclWindowPeer->setProperty(PROPERTY_TEXTCOLOR, uno::makeAny(getTextColor()));
        // font->italic
        uno::Any aFontDescriptor = _xVclWindowPeer->getProperty(PROPERTY_FONTDESCRIPTOR);
        awt::FontDescriptor aFontDescriptorStructure;
        aFontDescriptor >>= aFontDescriptorStructure;
        aFontDescriptorStructure.Slant = ::com::sun::star::awt::FontSlant_ITALIC;
        _xVclWindowPeer->setProperty(PROPERTY_FONTDESCRIPTOR, uno::makeAny(aFontDescriptorStructure));
    }

    // -----------------------------------------------------------------------------
    void FormattedFieldBeautifier::notifyPropertyChange( const beans::PropertyChangeEvent& _rEvent )
    {
        if  ( !_rEvent.PropertyName.equalsAscii( "DataField" ) )
            // not interested in
            return;

        setPlaceholderText( _rEvent.Source );
    }

    // -----------------------------------------------------------------------------
    void FormattedFieldBeautifier::handle( const uno::Reference< uno::XInterface >& _rxElement )
    {
        setPlaceholderText( _rxElement );
    }

    // -----------------------------------------------------------------------------
    void FormattedFieldBeautifier::notifyElementInserted( const uno::Reference< uno::XInterface >& _rxElement )
    {
        handle( _rxElement );
    }

    // -----------------------------------------------------------------------------
    uno::Reference<awt::XVclWindowPeer> FormattedFieldBeautifier::getVclWindowPeer(const uno::Reference< report::XReportComponent >& _xComponent) throw(uno::RuntimeException)
    {
        uno::Reference<awt::XVclWindowPeer> xVclWindowPeer;

        ::boost::shared_ptr<OReportModel> pModel = const_cast< OReportController& >( m_rReportController ).getSdrModel();

        uno::Reference<report::XSection> xSection(_xComponent->getSection());
        if ( xSection.is() )
        {
            OReportPage *pPage = pModel->getPage(xSection);
            sal_uLong nIndex = pPage->getIndexOf(_xComponent);
            if (nIndex < pPage->GetObjCount() )
            {
                SdrObject *pObject = pPage->GetObj(nIndex);
                OUnoObject* pUnoObj = dynamic_cast<OUnoObject*>(pObject);
                if ( pUnoObj ) // this doesn't need to be done for shapes
                {
                    // Rectangle aRect = pUnoObj->GetCurrentBoundRect();
                    ::boost::shared_ptr<OSectionWindow> pSectionWindow = m_rReportController.getSectionWindow(xSection);
                    if (pSectionWindow != NULL)
                    {
                        OReportSection& aOutputDevice = pSectionWindow->getReportSection(); // OutputDevice
                        OSectionView& aSdrView = aOutputDevice.getSectionView();            // SdrView
                        uno::Reference<awt::XControl> xControl = pUnoObj->GetUnoControl(aSdrView, aOutputDevice);
                        xVclWindowPeer = uno::Reference<awt::XVclWindowPeer>( xControl->getPeer(), uno::UNO_QUERY);
                    }
                }
            }
        }
        return xVclWindowPeer;
    }
}
