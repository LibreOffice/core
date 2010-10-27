/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBU_EXTENSIONNOTPRESENT_HXX
#define DBU_EXTENSIONNOTPRESENT_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>

namespace dbaui
{

/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class OExtensionNotPresentDialog :  public ModalDialog
{
    FixedImage                              m_aFI_WARNING;
    FixedText                               m_aFT_TEXT;

    PushButton                              m_aPB_DOWNLOAD;
    CancelButton                            m_aPB_CANCEL;

    ::com::sun::star::lang::Locale          m_nLocale;
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xMultiServiceFactory;

    /** returns the format string.
    */
    // ::rtl::OUString getFormatString(::sal_Int16 _nNumberFormatIndex);

    DECL_LINK(Download_Click,PushButton*);

    // not CopyCTOR, no self assignment
    OExtensionNotPresentDialog(const OExtensionNotPresentDialog&);
    void operator =(const OExtensionNotPresentDialog&);

    // get some values out of the configuration
    rtl::OUString getFromConfigurationExtension(rtl::OUString const& _sPropertyName) const;
    rtl::OUString getFromConfigurationExtension(rtl::OString const& _sPropertyName) const; // syntactic sugar

    ::com::sun::star::uno::Reference< ::com::sun::star::system::XSystemShellExecute > getShellExecuter() const;

public:
    OExtensionNotPresentDialog( Window* pParent, com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > );
    virtual ~OExtensionNotPresentDialog();
    virtual short   Execute();

    inline String getText() const { return m_aFT_TEXT.GetText(); }
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // DBU_EXTENSIONNOTPRESENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
