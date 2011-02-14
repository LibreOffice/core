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
#ifndef _EVENTDLG_HXX
#define _EVENTDLG_HXX

#include <vcl/menubtn.hxx>

#include <vcl/fixed.hxx>
#include <sfx2/evntconf.hxx>
#include "macropg.hxx"

#include <com/sun/star/frame/XFrame.hpp>

class Menu;
class SfxMacroInfoItem;
class SvxMacroItem;

class SvxEventConfigPage : public _SvxMacroTabPage
{
    FixedText                           aSaveInText;
    ListBox                             aSaveInListBox;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xAppEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xDocumentEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable >         m_xDocumentModifiable;

    sal_Bool            bAppConfig;

    DECL_LINK(      SelectHdl_Impl, ListBox* );

    // Forbidden and not implemented.
    SvxEventConfigPage (const SvxEventConfigPage &);
    SvxEventConfigPage & operator= (const SvxEventConfigPage &);

public:
                    /// this is only to let callers know that there is a LateInit which *must* be called
                    struct EarlyInit { };
                    SvxEventConfigPage( Window *pParent, const SfxItemSet& rSet, EarlyInit );
                    ~SvxEventConfigPage();

    void            LateInit( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );

protected:
    virtual sal_Bool    FillItemSet( SfxItemSet& );
    virtual void    Reset( const SfxItemSet& );
    using _SvxMacroTabPage::Reset;

private:
    void    ImplInitDocument();
};


#endif

