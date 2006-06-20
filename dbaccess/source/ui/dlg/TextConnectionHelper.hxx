/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextConnectionHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:04:34 $
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

#ifndef DBAUI_TEXTCONNECTIONHELPER_HXX
#define DBAUI_TEXTCONNECTIONHELPER_HXX

#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _DBAUI_CURLEDIT_HXX_
#include "curledit.hxx"
#endif
#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif



//.........................................................................
namespace dbaui

{
//.........................................................................


    //========================================================================
    //= OTextConnectionPage
    //========================================================================
    class OTextConnectionHelper : public Control
    {
    OTextConnectionHelper();
    public:
        OTextConnectionHelper( Window* pParent, sal_Bool _bWizardMode = sal_False);
        Link        m_aModifiedHandler;     /// to be called if something on the page has been modified
          CheckBox  m_aHeader;
        FixedLine   m_aLineFormat;
        FixedText   m_aFTExtensionHeader;
        RadioButton m_aRBAccessTextFiles;
        RadioButton m_aRBAccessCSVFiles;
        RadioButton m_aRBAccessOtherFiles;
        Edit        m_aETOwnExtension;
        FixedText   m_aFTExtensionExample;

        FixedText   m_aFieldSeparatorLabel;
        ComboBox    m_aFieldSeparator;
        FixedText   m_aTextSeparatorLabel;
        ComboBox    m_aTextSeparator;
        FixedText   m_aDecimalSeparatorLabel;
        ComboBox    m_aDecimalSeparator;
        FixedText   m_aThousandsSeparatorLabel;
        ComboBox    m_aThousandsSeparator;
        String      m_aFieldSeparatorList;
        String      m_aTextSeparatorList;
        String      m_aTextNone;
        sal_Bool    m_bWizardMode;
        String      m_aOldExtension;
        Link        m_aGetExtensionHandler; /// to be called if a new type is selected


        virtual ~OTextConnectionHelper();
    protected:
        void callModifiedHdl() const { if (m_aModifiedHandler.IsSet()) m_aModifiedHandler.Call((void*)this); }
        Link getControlModifiedLink() { return LINK(this, OTextConnectionHelper, OnControlModified); }
        DECL_LINK(OnSetExtensionHdl,RadioButton*);
        DECL_LINK(OnControlModified,Control*);
        DECL_LINK(OnEditModified,Edit*);

    private:
        String      GetSeparator( const ComboBox& rBox, const String& rList );
        void        SetSeparator( ComboBox& rBox, const String& rList, const String& rVal );
        void        SetExtension(const String& _rVal);


    public:
        void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue, sal_Bool _bValid);
        void        fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        void        fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        void        SetClickHandler(const Link& _rHandler) { m_aGetExtensionHandler = _rHandler; }
        String      GetExtension();
        sal_Bool FillItemSet( SfxItemSet& rSet, const sal_Bool bChangedSomething );
        sal_Bool checkItems();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_DBWIZ2_HXX

