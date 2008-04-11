/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: eventdlg.hxx,v $
 * $Revision: 1.7 $
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

#include <sfx2/evntconf.hxx>
#include <sfx2/macropg.hxx>

class Menu;
class SfxMacroInfoItem;
class SvxMacroItem;

class SfxEventConfigPage : public _SfxMacroTabPage
{
    RadioButton     aStarOfficeRB;
    RadioButton     aDocumentRB;

    SvxMacroItem*   pAppItem;
    SvxMacroItem*   pDocItem;
    BOOL            bAppConfig;

    DECL_LINK(      SelectHdl_Impl, RadioButton* );

public:
                    SfxEventConfigPage( Window *pParent, const SfxItemSet& rSet );
                    ~SfxEventConfigPage();

    void            Init();
    void            SelectMacro( const SfxMacroInfoItem* );
    void            SelectMacro( const SvxMacro* );
    void            Apply();

    virtual BOOL    FillItemSet( SfxItemSet& );
    virtual void    Reset( const SfxItemSet& );
};


#endif

