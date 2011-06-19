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

#ifndef _OFFAPP_CONNPOOLOPTIONS_HXX_
#define _OFFAPP_CONNPOOLOPTIONS_HXX_

#include <svl/solar.hrc>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/databaseregistrationui.hxx>

//........................................................................
namespace offapp
{
//........................................................................

    //====================================================================
    //= ConnectionPoolOptionsPage
    //====================================================================
    class DriverListControl;
    class ConnectionPoolOptionsPage : public SfxTabPage
    {
        using TabPage::ActivatePage;

    protected:
        FixedLine               m_aFrame;
        CheckBox                m_aEnablePooling;
        FixedText               m_aDriversLabel;
        DriverListControl*      m_pDriverList;
        FixedText               m_aDriverLabel;
        FixedText               m_aDriver;
        CheckBox                m_aDriverPoolingEnabled;
        FixedText               m_aTimeoutLabel;
        NumericField            m_aTimeout;

    protected:
        ConnectionPoolOptionsPage(Window* _pParent, const SfxItemSet& _rAttrSet);

    public:
        static SfxTabPage*  Create(Window* _pParent, const SfxItemSet& _rAttrSet);

        ~ConnectionPoolOptionsPage();

    protected:
        virtual long        Notify( NotifyEvent& _rNEvt );

        virtual sal_Bool        FillItemSet(SfxItemSet& _rSet);
        virtual void        Reset(const SfxItemSet& _rSet);
        virtual void        ActivatePage( const SfxItemSet& _rSet);

    protected:
        DECL_LINK( OnEnabledDisabled, const CheckBox* );
        DECL_LINK( OnDriverRowChanged, const void* );

        void implInitControls(const SfxItemSet& _rSet, sal_Bool _bFromReset);

        void commitTimeoutField();
    };

//........................................................................
}   // namespace offapp
//........................................................................

#endif // _OFFAPP_CONNPOOLOPTIONS_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
