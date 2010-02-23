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

#ifndef DBAUI_TEXTCONNECTIONHELPER_HXX
#define DBAUI_TEXTCONNECTIONHELPER_HXX

#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef CHARSETLISTBOX_HXX
#include "charsetlistbox.hxx"
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
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif


//.........................................................................
namespace dbaui

{
//.........................................................................

    #define TC_EXTENSION    ((short)0x01)   // a section specifying the extension of the files to connect to
    #define TC_SEPARATORS   ((short)0x02)   // a section specifying the various separators
    #define TC_HEADER       ((short)0x04)   // a section containing the "Text contains header" check box only
    #define TC_CHARSET      ((short)0x08)   // not yet implemented

    //========================================================================
    //= OTextConnectionPage
    //========================================================================
    class OTextConnectionHelper : public Control
    {
        OTextConnectionHelper();

        Link        m_aModifiedHandler;     /// to be called if something on the page has been modified

    public:
        OTextConnectionHelper( Window* pParent, const short _nAvailableSections );
        virtual ~OTextConnectionHelper();

    private:
        FixedText   m_aFTExtensionHeader;
        RadioButton m_aRBAccessTextFiles;
        RadioButton m_aRBAccessCSVFiles;
        RadioButton m_aRBAccessOtherFiles;
        Edit        m_aETOwnExtension;
        FixedText   m_aFTExtensionExample;
        FixedLine   m_aLineFormat;
        FixedText   m_aFieldSeparatorLabel;
        ComboBox    m_aFieldSeparator;
        FixedText   m_aTextSeparatorLabel;
        ComboBox    m_aTextSeparator;
        FixedText   m_aDecimalSeparatorLabel;
        ComboBox    m_aDecimalSeparator;
        FixedText   m_aThousandsSeparatorLabel;
        ComboBox    m_aThousandsSeparator;
          CheckBox  m_aRowHeader;
        FixedLine   m_aCharSetHeader;
        FixedText   m_aCharSetLabel;
        CharSetListBox  m_aCharSet;
        String      m_aFieldSeparatorList;
        String      m_aTextSeparatorList;
        String      m_aTextNone;
        String      m_aOldExtension;
        Link        m_aGetExtensionHandler; /// to be called if a new type is selected

        short       m_nAvailableSections;

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
        void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bValid);
        void        fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        void        fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        void        SetClickHandler(const Link& _rHandler) { m_aGetExtensionHandler = _rHandler; }
        String      GetExtension();
        sal_Bool FillItemSet( SfxItemSet& rSet, const sal_Bool bChangedSomething );
        sal_Bool prepareLeave();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_DBWIZ2_HXX

