#ifndef DBU_EXTENSIONNOTPRESENT_HXX
#define DBU_EXTENSIONNOTPRESENT_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExtensionNotPresent.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-31 09:15:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

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

