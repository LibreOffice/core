/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:50:36 $
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
#ifndef _EVENTDLG_HXX
#define _EVENTDLG_HXX

#ifndef _SV_MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif

#include <vcl/fixed.hxx>
#include <sfx2/evntconf.hxx>
#include "macropg.hxx"

class Menu;
class SfxMacroInfoItem;
class SvxMacroItem;

class SvxEventConfigPage : public _SvxMacroTabPage
{
    FixedText                           aSaveInText;
    ListBox                             aSaveInListBox;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xEvents_app;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xEvents_doc;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable >   m_xModifiable_doc;

    BOOL            bAppConfig;

    DECL_LINK(      SelectHdl_Impl, ListBox* );

    // Forbidden and not implemented.
    SvxEventConfigPage (const SvxEventConfigPage &);
    SvxEventConfigPage & operator= (const SvxEventConfigPage &);

public:
                    SvxEventConfigPage( Window *pParent, const SfxItemSet& rSet );
                    ~SvxEventConfigPage();

    void            Init();
    void            Apply();

    virtual BOOL    FillItemSet( SfxItemSet& );
    virtual void    Reset( const SfxItemSet& );
    using _SvxMacroTabPage::Reset;
};


#endif

