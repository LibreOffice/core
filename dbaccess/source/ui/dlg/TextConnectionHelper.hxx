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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_TEXTCONNECTIONHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_TEXTCONNECTIONHELPER_HXX

#include "ConnectionPageSetup.hxx"
#include "adminpages.hxx"
#include "charsetlistbox.hxx"
#include <ucbhelper/content.hxx>
#include "curledit.hxx"
#include <svtools/roadmapwizard.hxx>
#include <vcl/field.hxx>
#include <rtl/ustring.hxx>
#include <vcl/lstbox.hxx>

namespace dbaui

{

    #define TC_EXTENSION    ((short)0x01)   // a section specifying the extension of the files to connect to
    #define TC_SEPARATORS   ((short)0x02)   // a section specifying the various separators
    #define TC_HEADER       ((short)0x04)   // a section containing the "Text contains header" check box only
    #define TC_CHARSET      ((short)0x08)   // not yet implemented

    // OTextConnectionPage
    class OTextConnectionHelper : public TabPage
    {
        OTextConnectionHelper();

        Link<>      m_aModifiedHandler;     /// to be called if something on the page has been modified

    public:
        OTextConnectionHelper( vcl::Window* pParent, const short _nAvailableSections );
        virtual ~OTextConnectionHelper();
        virtual void dispose() override;

    private:
        VclPtr<FixedText>        m_pExtensionHeader;
        VclPtr<RadioButton>      m_pAccessTextFiles;
        VclPtr<RadioButton>      m_pAccessCSVFiles;
        VclPtr<RadioButton>      m_pAccessOtherFiles;
        VclPtr<Edit>             m_pOwnExtension;
        VclPtr<FixedText>        m_pExtensionExample;
        VclPtr<FixedText>        m_pFormatHeader;
        VclPtr<FixedText>        m_pFieldSeparatorLabel;
        VclPtr<ComboBox>         m_pFieldSeparator;
        VclPtr<FixedText>        m_pTextSeparatorLabel;
        VclPtr<ComboBox>         m_pTextSeparator;
        VclPtr<FixedText>        m_pDecimalSeparatorLabel;
        VclPtr<ComboBox>         m_pDecimalSeparator;
        VclPtr<FixedText>        m_pThousandsSeparatorLabel;
        VclPtr<ComboBox>         m_pThousandsSeparator;
        VclPtr<CheckBox>         m_pRowHeader;
        VclPtr<FixedText>        m_pCharSetHeader;
        VclPtr<FixedText>        m_pCharSetLabel;
        VclPtr<CharSetListBox>   m_pCharSet;
        OUString    m_aFieldSeparatorList;
        OUString    m_aTextSeparatorList;
        OUString    m_aTextNone;
        OUString    m_aOldExtension;
        Link<OTextConnectionHelper*, void> m_aGetExtensionHandler; /// to be called if a new type is selected

        short       m_nAvailableSections;

    protected:
        void callModifiedHdl() const { m_aModifiedHandler.Call(const_cast<OTextConnectionHelper *>(this)); }
        Link<> getControlModifiedLink() { return LINK(this, OTextConnectionHelper, OnControlModified); }
        DECL_LINK_TYPED(OnSetExtensionHdl,RadioButton&,void);
        DECL_LINK(OnControlModified,Control*);
        DECL_LINK(OnEditModified,Edit*);
        DECL_LINK_TYPED(CharsetSelectHdl,ListBox&,void);
        DECL_LINK_TYPED(ComboBoxSelectHdl,ComboBox&,void);

    private:
        OUString    GetSeparator( const ComboBox& rBox, const OUString& rList );
        void        SetSeparator( ComboBox& rBox, const OUString& rList, const OUString& rVal );
        void        SetExtension(const OUString& _rVal);

    public:
        void        implInitControls(const SfxItemSet& _rSet, bool _bValid);
        void        fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        void        fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        void        SetClickHandler(const Link<OTextConnectionHelper*, void>& _rHandler) { m_aGetExtensionHandler = _rHandler; }
        OUString    GetExtension();
        bool        FillItemSet( SfxItemSet& rSet, const bool bChangedSomething );
        bool        prepareLeave();
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_TEXTCONNECTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
