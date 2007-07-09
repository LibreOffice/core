#ifndef RPTUI_PAGENUMBER_HXX
#define RPTUI_PAGENUMBER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageNumber.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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


namespace rptui
{
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class OPageNumberDialog :   public ModalDialog
{
    FixedLine                               m_aFormat;
    RadioButton                             m_aPageN;
    RadioButton                             m_aPageNofM;

    FixedLine                               m_aPosition;
    RadioButton                             m_aTopPage;
    RadioButton                             m_aBottomPage;
    FixedLine                               m_aMisc;
    FixedText                               m_aAlignment;
    ListBox                                 m_aAlignmentLst;

    CheckBox                                m_aShowNumberOnFirstPage;
    FixedLine                               m_aFl1;
    OKButton                                m_aPB_OK;
    CancelButton                            m_aPB_CANCEL;
    HelpButton                              m_aPB_Help;


    ::rptui::OReportController*             m_pController;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>
                                            m_xHoldAlive;

    OPageNumberDialog(const OPageNumberDialog&);
    void operator =(const OPageNumberDialog&);
public:
    OPageNumberDialog( Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xHoldAlive
                        ,::rptui::OReportController* _pController);
    virtual ~OPageNumberDialog();
    virtual short   Execute();
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_PAGENUMBER_HXX
