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
#ifndef INCLUDED_CUI_SOURCE_CUSTOMIZE_EVENTDLG_HXX
#define INCLUDED_CUI_SOURCE_CUSTOMIZE_EVENTDLG_HXX

#include <vcl/menubtn.hxx>

#include <vcl/fixed.hxx>
#include <sfx2/evntconf.hxx>
#include "macropg.hxx"

#include <com/sun/star/frame/XFrame.hpp>

class SvxEventConfigPage : public _SvxMacroTabPage
{
    ListBox*   m_pSaveInListBox;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xAppEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xDocumentEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable >         m_xDocumentModifiable;

    bool            bAppConfig;

    DECL_LINK(      SelectHdl_Impl, ListBox* );

    // Forbidden and not implemented.
    SvxEventConfigPage (const SvxEventConfigPage &);
    SvxEventConfigPage & operator= (const SvxEventConfigPage &);

public:

                    /// this is only to let callers know that there is a LateInit which *must* be called
                    struct EarlyInit { };
                    SvxEventConfigPage( vcl::Window *pParent, const SfxItemSet& rSet, EarlyInit );
                    virtual ~SvxEventConfigPage();

    void            LateInit( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );

protected:
    virtual bool    FillItemSet( SfxItemSet* ) SAL_OVERRIDE;

private:
    void    ImplInitDocument();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
