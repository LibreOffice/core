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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERVIEW_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERVIEW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <vcl/window.hxx>
#include <tools/resid.hxx>
#include <com/sun/star/awt/Size.hpp>


namespace pcr
{


    class OPropertyEditor;

    class OPropertyBrowserView : public vcl::Window
    {
        VclPtr<OPropertyEditor>     m_pPropBox;
        sal_uInt16                  m_nActivePage;
        Link<LinkParamNone*,void>   m_aPageActivationHandler;

    protected:
        virtual void Resize() SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;
        virtual bool Notify( NotifyEvent& _rNEvt ) SAL_OVERRIDE;

    public:
        OPropertyBrowserView( vcl::Window* pParent, WinBits nBits = 0);

        virtual ~OPropertyBrowserView();
        virtual void dispose() SAL_OVERRIDE;

        OPropertyEditor&    getPropertyBox() { return *m_pPropBox; }

        // page handling
        sal_uInt16  getActivaPage() const { return m_nActivePage; }
        void        activatePage(sal_uInt16 _nPage);

        void    setPageActivationHandler(const Link<LinkParamNone*,void>& _rHdl) { m_aPageActivationHandler = _rHdl; }

        ::com::sun::star::awt::Size getMinimumSize();

    protected:
        DECL_LINK_TYPED(OnPageActivation, LinkParamNone*, void);
    };



} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
